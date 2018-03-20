#include "stdafx.h"
#include <sys/types.h>          /* See NOTES */
#include <string.h>
#include <Winsock2.h>
#include <winbase.h>
#include <Ws2tcpip.h>
#pragma comment(lib,"Ws2_32.lib")
#include <stdio.h>
#include "client.h"
int iSocketClient_data;
int iSocketClient_heartbeat;

typedef struct MyData
{
	SocketClient *socketClient;
	BOOL g_fResourceInUse;
}MYDATA;
char ReceiveFromServer[DataSize];

DWORD WINAPI ThreadFor_data(LPVOID pM);
DWORD WINAPI ThreadFor_heartbeat(LPVOID pM);
/*1. �ӿڳ�ʼ������*/
int SocketClient::Init(char* ip, int nPort_data, int nPort_heartbeat){
	ipServer = ip;
	data = nPort_data;
	heartbeat = nPort_heartbeat;
	g_fResourceInUse = FALSE;

	iSocketClient_data = SocketClientOpen(nPort_data);
	if (iSocketClient_data == -1){
		printf("nPort_data: SocketClientOpen error!\n");
		return -1;
	}
	iSocketClient_heartbeat = SocketClientOpen(nPort_heartbeat);
	if (iSocketClient_heartbeat == -1){
		printf("nPort_heartbeat: SocketClientOpen error!\n");
		return -1;
	}
	MYDATA mydt[1];
	mydt[0].socketClient = this;
	mydt[0].g_fResourceInUse = g_fResourceInUse;
	HANDLE handle0 = CreateThread(NULL, 0, ThreadFor_data, &mydt[0], 0, NULL);  
	HANDLE handle1 = CreateThread(NULL, 0, ThreadFor_heartbeat, &mydt[0], 0, NULL);  
	return iSocketClient_data;
}

/*1.1 ��ʼ������/�������˿�*/
int SocketClient::SocketClientOpen(int port){
	struct sockaddr_in tSocketServerAddr;
	int iRet;
	int iSocketClient;
	
    //AF_INET:IPV4(���޸�),SOCK_STREAM:��ʾTCP,0(���޸�)
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);
	iSocketClient = socket(AF_INET, SOCK_STREAM, 0);/*��þ��*/
	if (-1 == iSocketClient)
	{
		printf("socket error!\n");
		return -1;
	}
    /*tSocketServerAddr�����úͷ������ļ�����ͬ*/
	tSocketServerAddr.sin_family      = AF_INET;/*���޸�*/
	tSocketServerAddr.sin_port        = htons(port);/*���Ķ˿�,SERVER_PORT�궨��888�������Ķ˿�Ϊ888,ֻ���޸ĺ�SERVER_PORT,htonsΪ���˿ں�SERVER_PORTת��Ϊ�����ֽ��򣬶˿ں�888��д��ֻ��Ϳͻ�����ͬ����*/
 
 	//������ĵ�2������argv[1]����tSocketServerAddr.sin_addr
 	//����ڶ�������Ϊ��������IP��ַ
	tSocketServerAddr.sin_addr.S_un.S_addr = inet_addr(ipServer);
	//��������tSocketServerAddr
	iRet = connect(iSocketClient, (const struct sockaddr *)&tSocketServerAddr, sizeof(struct sockaddr));	
	//���Ӳ��ɹ����ӡ����ʧ��
	if (-1 == iRet)
	{
		printf("connect error!\n");
		return -1;
	}
	return iSocketClient;
}

/*1.2 ���߳�ר���������շ���˵�����*/
DWORD WINAPI ThreadFor_data(LPVOID pM)  
{
	MYDATA *pmd = (MYDATA *)pM;
	int iRecvLen;
	SocketClient *socketClient = pmd->socketClient;
	BOOL g_fResourceInUse = pmd->g_fResourceInUse;
	while (1){
		// �ȴ����ʹ�����Դ
		while (InterlockedExchange(reinterpret_cast<long*>(&g_fResourceInUse), TRUE) == TRUE)	//ʹ�û���ԭ����
			Sleep(0);
		iRecvLen = recv(iSocketClient_heartbeat, ReceiveFromServer, DataSize-1, 0);
		if (iRecvLen <= 0)//���ܵ������ݸ���С��0���ر�socket
		{
			closesocket(iSocketClient_data);
			closesocket(iSocketClient_heartbeat);
			return -1;
		}
		ReceiveFromServer[iRecvLen] = '\0';
		// ʹ����Դ
		InterlockedExchange(reinterpret_cast<long*>(&g_fResourceInUse), FALSE);
		//printf("1 ReceiveFromServer = %s\n", ReceiveFromServer); 
	}
	return 0;
}

/*1.3 ���߳�ר����������������*/
DWORD WINAPI ThreadFor_heartbeat(LPVOID pM)  
{
	MYDATA *pmd = (MYDATA *)pM;
	int iRet;
	SocketClient *socketClient = pmd->socketClient;
	char* HeartbeatBag = "I am alive!";
	while (1){
		iRet = socketClient->SendCodeToServer(HeartbeatBag, iSocketClient_heartbeat);
		if(iRet == -1)
			break;
		Sleep(3000);
	}
	return 0;
}

/*�����ַ���*/
int SocketClient::SendCodeToServer(char *buf, int iSocketClient){
	int iSendLen;
	iSendLen = send(iSocketClient, buf, strlen(buf), 0);
	//�����صķ��͵ĳ���Ϊ0���ر�socket
	if (iSendLen <= 0)
	{
		printf("Client close!\n");
		closesocket(iSocketClient_data);
		closesocket(iSocketClient_heartbeat);
		return -1;
	}
	return 0;
}

/*2. ���ݽ��պ���*/
char* SocketClient::GetDataFromServer(){
	while (InterlockedExchange(reinterpret_cast<long*>(&g_fResourceInUse), TRUE) == TRUE)//�鿴����ǰһ��ֵ���Լ�����Ƿ�ΪTRUE��������ֵ��FALSE����ô����Դ��û����ʹ�ã���InterlockedExchange������ֵ��ΪTRUE���˳���ѭ��
		Sleep(0);
	char receiveBuf[DataSize];
	for (int i = 0; i < DataSize; i++){
		receiveBuf[i] = ReceiveFromServer[i];
	}
	InterlockedExchange(reinterpret_cast<long*>(&g_fResourceInUse), FALSE);
	return receiveBuf;	/*�޸ģ������*/
}

/*3. ���ݷ��ͺ���*/
int SocketClient::SendDataToServer(char* pData){
	int iRet = SendCodeToServer(pData, iSocketClient_data);
	return iRet;
}

/*4. �ر����Ӻ���*/
int SocketClient::Close(){
	int iRet;
	Sleep(1000);
	char* CloseMessage = "I am close!";
	iRet = SendCodeToServer(CloseMessage, iSocketClient_data);
	if(iRet == -1){
		printf("SocketClientClose Error!\n");
	}
	Sleep(1000);
	closesocket(iSocketClient_data);
	closesocket(iSocketClient_heartbeat);
	return 0;
}

/*5. ��ȡ����������Ϣ����*/
int SocketClient::GetServerInfo(){
	printf("server's ip is %s\t nPort_data : %d\t nPort_heartbeat : %d\t\n",ipServer,data,heartbeat);
	return 0;
}






