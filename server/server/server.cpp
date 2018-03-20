#include "stdafx.h"
#include <sys/types.h>          /* See NOTES */
#include <string.h>
#include <Winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib,"Ws2_32.lib")
#include <stdio.h>
#include <string>
#include "chaintable.h"
#include "server.h"
using namespace std;

typedef struct MyData
{
	int nPort;
	SocketServer *socketServer;
	char *ipClient;
}MYDATA;

int iSocketServer_data;
int iSocketServer_heartbeat;

DWORD WINAPI ThreadFor_ReceiveData(LPVOID pM);
DWORD WINAPI ThreadFor_SendData(LPVOID pM);
DWORD WINAPI ThreadFor_heartbeat(LPVOID pM);
DWORD WINAPI SocketDataPortInit(LPVOID pM);
DWORD WINAPI SocketHeartbeatPortInit(LPVOID pM);

/*1. �ӿڳ�ʼ������*/
int SocketServer::Init(int nPort_data, int nPort_heartbeat){
	MYDATA mydt[2];
	iClientNum = 0;	//��ʼ���ͻ��˵ĸ���
	data = nPort_data;
	heartbeat = nPort_heartbeat;

	mydt[0].socketServer = this;
	mydt[0].nPort = nPort_data;
	HANDLE handle0 = CreateThread(NULL, 0, SocketDataPortInit, &mydt[0], 0, NULL);	 //�������̣߳���ʼ���������ݵĶ˿�

	mydt[1].socketServer = this;
	mydt[1].nPort = nPort_heartbeat;
	HANDLE handle1 = CreateThread(NULL, 0, SocketHeartbeatPortInit, &mydt[1], 0, NULL);	 //�������̣߳���ʼ�������������Ķ˿�
	return 0;
}

/*1.1 ��ʼ����������/�������ݵĶ˿�*/
DWORD WINAPI SocketDataPortInit(LPVOID pM)  
{
	int iRet;
	MYDATA *pmd = (MYDATA *)pM;
	SocketServer *socketServer = pmd->socketServer;
	int nPort_data = pmd->nPort;
	iSocketServer_data = socketServer->SocketServerOpen(nPort_data);
	if (iSocketServer_data == -1){
		printf("nPort_data: SocketServerOpen error!\n");
		return -1;
	}
	iRet = socketServer->AcceptClientConnectionFromData(iSocketServer_data, nPort_data);
	if (iRet == -1){
		printf("nPort_data: AcceptClientConnection error!\n");
		return -1;
	}
	return 0;
}

/*1.2 ��ʼ����������/�������ݵĶ˿�*/
DWORD WINAPI SocketHeartbeatPortInit(LPVOID pM)  
{
	int iRet;
	MYDATA *pmd = (MYDATA *)pM;
	SocketServer *socketServer = pmd->socketServer;
	int nPort_heartbeat = pmd->nPort;
	iSocketServer_heartbeat = socketServer->SocketServerOpen(nPort_heartbeat);
	if (iSocketServer_heartbeat == -1){
		printf("nPort_heartbeat: SocketServerOpen error!\n");
		return -1;
	}
	iRet = socketServer->AcceptClientConnectionFromHeartbeat(iSocketServer_heartbeat, nPort_heartbeat);
	if (iRet == -1){
		printf("nPort_heartbeat: AcceptClientConnection error!\n");
		return -1;
	}
	return 0;
}

/*1.1.1/1.2.1 ��ʼ��socket�����ݲ�ͬ�Ķ˿ںŻ�ò�ͬ��socket�����������*/
int SocketServer::SocketServerOpen(int port){
	struct sockaddr_in tSocketServerAddr;
	int iSocketServer;
	int iRet;
	
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);
	iSocketServer = socket(AF_INET, SOCK_STREAM, 0);/*��þ����AF_INET:IPV4(���޸�),SOCK_STREAM:��ʾTCP,0(���޸�)*/
	if (-1 == iSocketServer)
	{
		printf("socket error!\n");
		return -1;
	}
	tSocketServerAddr.sin_family      = AF_INET;/*���޸�*/
	tSocketServerAddr.sin_port        = htons(port);/*���Ķ˿�,SERVER_PORT�궨��8888�������Ķ˿�Ϊ8888,ֻ���޸ĺ�SERVER_PORT��htonsΪ���˿ں�SERVER_PORTת��Ϊ�����ֽ��򣬶˿ں�8888��д��ֻ��Ϳͻ�����ͬ����*/
 	tSocketServerAddr.sin_addr.s_addr = INADDR_ANY;/*INADDR_ANYΪ�����ϵ�����IP�����޸�*/
	//��iSocketServer���󶨶˿�SERVER_PORT�� sizeof(struct sockaddr)ΪtSocketServerAddr�ĳ���
	iRet = bind(iSocketServer, (const struct sockaddr *)&tSocketServerAddr, sizeof(struct sockaddr));
	if (-1 == iRet)
	{
		printf("bind error!\n");
		return -1;
	}
    //�����������,��ʼ����iSocketServer,BACKLOG�궨��Ϊ10������ͬʱ����10·����
	iRet = listen(iSocketServer, BACKLOG);
	if (-1 == iRet)
	{
		printf("listen error!\n");
		return -1;
	}
	return iSocketServer;
}

/*1.1.2 ���ݲ�ͬ�˿ںŶ�Ӧ��socket����ȴ��ͻ������ӣ����ڲ�ͬ�Ķ˿��½���ͬ���̷ֱ߳�������ݺ�������*/
int SocketServer::AcceptClientConnectionFromData(int iSocketServer, int port){
	struct sockaddr_in tSocketClientAddr;
	int iSocketClient;
	char *ipClient;
	MYDATA mydt[BACKLOG];
	int iAddrLen;
	iAddrLen = sizeof(struct sockaddr);
	PT_Name ptFind;
	while (1) {
		//�ȴ��������ӣ����޿ͻ������ӣ��ڴ˴����ߣ��������ӣ���ͻ��˵���Ϣ��IP���˿ڱ��浽tSocketClientAddr�����ӳɹ�ʱ���ؿͻ��˾��iSocketClient
		iSocketClient = accept(iSocketServer, (struct sockaddr *)&tSocketClientAddr, (socklen_t*)&iAddrLen);			/*4. server��client����һ������*/
		ipClient = inet_ntoa_b(tSocketClientAddr.sin_addr);
		if (-1 != iSocketClient)		/*iSocketClient��Ϊ-1ʱ�������ӳɹ�*/
		{
			iClientNum++;
			add_one_name(ipClient);
			ptFind = get_name(ipClient);	//�����ݶ˵��ļ������¼��������
			if (ptFind == NULL)
			{
				printf("1 do not have this name\n");
				return -1;
			}
			ptFind->iSocketClient_data = iSocketClient;
			GetClientInfo();
			printf("Get connection from client DataPort : %s\n", ipClient);	/*�ͻ��˵���Ŀ��������ipת��Ϊascii�ַ���*/
			mydt[iClientNum].socketServer = this;
			mydt[iClientNum].ipClient = ipClient;
			HANDLE handle1 = CreateThread(NULL, 0, ThreadFor_ReceiveData, &mydt[iClientNum], 0, NULL);  
			HANDLE handle2 = CreateThread(NULL, 0, ThreadFor_SendData, &mydt[iClientNum], 0, NULL);  
		}
		else 
		{
			return -1;		
		}
	}
	return iSocketClient;	
}

/*1.2.2 ���ݲ�ͬ�˿ںŶ�Ӧ��socket����ȴ��ͻ������ӣ����ڲ�ͬ�Ķ˿��½���ͬ���̷ֱ߳�������ݺ�������*/
int SocketServer::AcceptClientConnectionFromHeartbeat(int iSocketServer, int port){
	struct sockaddr_in tSocketClientAddr;
	int iSocketClient;
	char *ipClient;
	MYDATA mydt[BACKLOG];
	int iAddrLen;
	iAddrLen = sizeof(struct sockaddr);
	PT_Name ptFind;
	while (1) {
		//�ȴ��������ӣ����޿ͻ������ӣ��ڴ˴����ߣ��������ӣ���ͻ��˵���Ϣ��IP���˿ڱ��浽tSocketClientAddr�����ӳɹ�ʱ���ؿͻ��˾��iSocketClient
		iSocketClient = accept(iSocketServer, (struct sockaddr *)&tSocketClientAddr, (socklen_t*)&iAddrLen);			/*4. server��client����һ������*/
		ipClient = inet_ntoa_b(tSocketClientAddr.sin_addr);
		ptFind = get_name(ipClient);
		while (ptFind == NULL)	//ȷ�����ݶ������ӣ���Ϊ�ͻ����������ݶ�����ʱ���������
		{
			Sleep(100);
			ptFind = get_name(ipClient);	//�������˵��ļ������¼��������
		}
		ptFind->iSocketClient_heartbeat = iSocketClient;
		if (-1 != iSocketClient)
		{
			printf("Get connection from client HeartBeatPort : %s\n", ipClient);	/*�ͻ��˵���Ŀ��������ipת��Ϊascii�ַ���*/
			mydt[iClientNum].socketServer = this;
			mydt[iClientNum].ipClient = ipClient;
			HANDLE handle = CreateThread(NULL, 0, ThreadFor_heartbeat, &mydt[iClientNum], 0, NULL);  
		}
		else 
		{
			return -1;		
		}
	}
	return 0;	
}

/*1.1.2.1 ���߳�ר�����ڽ��տͻ��˵����õ�����*/
DWORD WINAPI ThreadFor_ReceiveData(LPVOID pM)  
{
	MYDATA *pmd = (MYDATA *)pM;
	int iRecvLen;
	char ucRecvBuf[DataSize];
	SocketServer *socketServer = pmd->socketServer;
	char *ipClient = pmd->ipClient;
	PT_Name ptCur = get_name(ipClient);		//�������л�ÿͻ���
	char* CloseMessage = "I am close!";
	while (1)
	{
		// ���տͻ��˷��������ݴ浽ucRecvBuf���飬���ܵ�������󳤶�Ϊ999��flagΪ0
		iRecvLen = recv(ptCur->iSocketClient_data, ucRecvBuf, DataSize-1, 0);
		if (iRecvLen <= 0)//���ܵ������ݸ���С��0���ر�socket
		{
			return -1;
		}
		else
		{     
			//��ӡ�ӿͻ��˽��յ�����
			ucRecvBuf[iRecvLen] = '\0';
			if (strcmp(ucRecvBuf, CloseMessage) == 0){
				printf("get the close message from %s!\n", ipClient);
				closesocket(ptCur->iSocketClient_data);
				closesocket(ptCur->iSocketClient_heartbeat);
				//del_one_name(ptCur->name);
				//socketServer->GetClientInfo();
				Sleep(100);
				break;
			} else{
				//printf("%s: %s\n", ipClient, ucRecvBuf);
				ptCur->receive_buf = ucRecvBuf;  //�����յ����ݷ�������
			}
		}
	}
}

/*1.1.2.2 ���߳�ר��������ͻ��˷�������*/
DWORD WINAPI ThreadFor_SendData(LPVOID pM)  
{
	MYDATA *pmd = (MYDATA *)pM;
	SocketServer *socketServer = pmd->socketServer;
	char *ipClient = pmd->ipClient;
	PT_Name ptCur = get_name(ipClient);		//�������л�ÿͻ���
	while (1)
	{
		if (ptCur->send_buf != NULL){
			send(ptCur->iSocketClient_heartbeat, ptCur->send_buf, strlen(ptCur->send_buf), 0);
			ptCur->send_buf = NULL;
		}
	}
}

/*1.2.2.1 ���߳�ר�����ڽ��տͻ��˵�������*/
DWORD WINAPI ThreadFor_heartbeat(LPVOID pM)  
{
	MYDATA *pmd = (MYDATA *)pM;

	int iRecvLen;
	int timeout_count = 0;
	char ucRecvBuf[DataSize];
	SocketServer *socketServer = pmd->socketServer;
	char *ipClient = pmd->ipClient;
	char* HeartbeatBag = "I am alive!";
	PT_Name ptCur = get_name(ipClient);		//�������л�ÿͻ���
	fd_set tRFds;

	struct timeval time;
	FD_ZERO(&tRFds);
	FD_SET(ptCur->iSocketClient_heartbeat, &tRFds);
	time.tv_sec=15;
	time.tv_usec=0;
	while (1)
	{
		int ret = select(NULL, &tRFds, NULL, NULL, &time);	//����ֵ����0������ĳһ���ļ��ɶ�/��д/�쳣��ʱ
		if(ret > 0)
		{
			timeout_count = 0;
			// ���տͻ��˷��������ݴ浽ucRecvBuf���飬���ܵ�������󳤶�Ϊ999��flagΪ0
			iRecvLen = recv(ptCur->iSocketClient_heartbeat, ucRecvBuf, DataSize-1, 0);
			if (iRecvLen <= 0) //���ܵ������ݸ���С��0���ر�socket
			{
				closesocket(ptCur->iSocketClient_data);
				closesocket(ptCur->iSocketClient_heartbeat);
				del_one_name(ptCur->name);
				socketServer->GetClientInfo();
				Sleep(100);
				return -1;
			}
			else
			{
				//��ӡ�ӿͻ��˽��յ�����
				ucRecvBuf[iRecvLen] = '\0';
				if (strcmp(ucRecvBuf, HeartbeatBag) == 0)
				{
					printf("find the heartbeat from %s: %s\n", ipClient, ucRecvBuf);
					continue;
				}
			}
		}
		else
		{
			printf("time out!\n");
			closesocket(ptCur->iSocketClient_data);
			closesocket(ptCur->iSocketClient_heartbeat);
			del_one_name(ptCur->name);
			socketServer->GetClientInfo();
			Sleep(100);
			return -1;
		}
	}				
}

char *SocketServer::inet_ntoa_b(struct in_addr inetAddress)
{
	unsigned char * pIp = (unsigned char*)&inetAddress;
	char * pFirst;
	char * pLast;
	char temp;
	unsigned long digit, mod;
	int i;
	char *pString = new char[16];
	memset(pString, 0, 16);
	char *ipClient = pString;
	for( i = 0; i < 4; i++ )
	{
		digit = pIp[i];		
		pFirst = pString;
		do
		{
			mod = digit;
			digit /= 10;
			mod -= digit * 10;	//��λ
			*pString = (unsigned char)(mod + '0');
			pString++;
		}while ( digit !=0 );
		pLast = pString - 1;
		temp = *pFirst;
		*pFirst = *pLast;
		*pLast = temp;
		*pString = '.';
		pString++;
	}
	pString--;
	*pString = '\0';
	return ipClient;
}

/*2. ���ݽ��պ���*/
char* SocketServer::GetDataFromClient(char* ipClient){
	char* receive_buf = get_receive_buf(ipClient);	//�������л�ý��յ�����
	return receive_buf;
}

/*3. ���ݷ��ͺ���*/
int SocketServer::SendDataToClient(char* ipClient, char* pData){
	add_send_buf(ipClient, pData);		//��д����������еĿͻ���
	return 0;
}

/*4. �ر��������Ӻ���*/
int SocketServer::CloseAll(){
	PT_Name ptCur;
	ptCur = GetNameHead();	//�������ͷ
	if (!ptCur){
		printf("No Client Online!\n");
		return -1;
	}
	while (ptCur)
	{
		closesocket(ptCur->iSocketClient_data);
		closesocket(ptCur->iSocketClient_heartbeat);
		//del_one_name(ptCur->name);
		ptCur = ptCur->next;
		Sleep(100);
	}
	return 0;
}

/*5. �ر�ָ�����Ӻ���*/
int SocketServer::Close(char* ipClient){
	PT_Name ptCur;
	ptCur = get_name(ipClient);	//�������˵��ļ������¼��������
	if (ptCur == NULL)
	{
		printf("do not have this name\n");
		return -1;
	}
	closesocket(ptCur->iSocketClient_data);
	closesocket(ptCur->iSocketClient_heartbeat);	//�ر��������˿�ʱ���������̺߳���ThreadFor_heartbeat��recv���⵽iRecvLen <= 0
	//del_one_name(ptCur->name);					//����⵽iRecvLen <= 0ʱ�����Զ�ִ��del_one_name���ʴ˴����ظ�ִ��
	Sleep(10);
	return 0;
}

/*6. ��ȡ�ͻ��˸�������*/
int SocketServer::GetClientNum(){
	return iClientNum;
}

/*7. ��ȡ�ͻ�����Ϣ����*/
int SocketServer::GetClientInfo(){
	printf("Online Client List Below:\n");
	list_all_name(data, heartbeat);	//�������Ϊ�˿ں�
	return 0;
}

/*8. �رշ����*/
void SocketServer::SocketServerClose(){
	closesocket(iSocketServer_data);
	closesocket(iSocketServer_heartbeat);
}
