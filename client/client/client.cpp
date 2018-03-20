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
/*1. 接口初始化函数*/
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

/*1.1 初始化数据/心跳包端口*/
int SocketClient::SocketClientOpen(int port){
	struct sockaddr_in tSocketServerAddr;
	int iRet;
	int iSocketClient;
	
    //AF_INET:IPV4(不修改),SOCK_STREAM:表示TCP,0(不修改)
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);
	iSocketClient = socket(AF_INET, SOCK_STREAM, 0);/*获得句柄*/
	if (-1 == iSocketClient)
	{
		printf("socket error!\n");
		return -1;
	}
    /*tSocketServerAddr的设置和服务器文件中相同*/
	tSocketServerAddr.sin_family      = AF_INET;/*不修改*/
	tSocketServerAddr.sin_port        = htons(port);/*检测的端口,SERVER_PORT宏定义888，即检测的端口为888,只需修改宏SERVER_PORT,htons为将端口号SERVER_PORT转化为网络字节序，端口号888任写，只需和客户端相同即可*/
 
 	//将输入的第2个参数argv[1]存入tSocketServerAddr.sin_addr
 	//输入第二个参数为服务器的IP地址
	tSocketServerAddr.sin_addr.S_un.S_addr = inet_addr(ipServer);
	//启动连接tSocketServerAddr
	iRet = connect(iSocketClient, (const struct sockaddr *)&tSocketServerAddr, sizeof(struct sockaddr));	
	//连接不成功则打印连接失败
	if (-1 == iRet)
	{
		printf("connect error!\n");
		return -1;
	}
	return iSocketClient;
}

/*1.2 此线程专门用来接收服务端的数据*/
DWORD WINAPI ThreadFor_data(LPVOID pM)  
{
	MYDATA *pmd = (MYDATA *)pM;
	int iRecvLen;
	SocketClient *socketClient = pmd->socketClient;
	BOOL g_fResourceInUse = pmd->g_fResourceInUse;
	while (1){
		// 等待访问共享资源
		while (InterlockedExchange(reinterpret_cast<long*>(&g_fResourceInUse), TRUE) == TRUE)	//使用互斥原子量
			Sleep(0);
		iRecvLen = recv(iSocketClient_heartbeat, ReceiveFromServer, DataSize-1, 0);
		if (iRecvLen <= 0)//接受到的数据个数小于0，关闭socket
		{
			closesocket(iSocketClient_data);
			closesocket(iSocketClient_heartbeat);
			return -1;
		}
		ReceiveFromServer[iRecvLen] = '\0';
		// 使用资源
		InterlockedExchange(reinterpret_cast<long*>(&g_fResourceInUse), FALSE);
		//printf("1 ReceiveFromServer = %s\n", ReceiveFromServer); 
	}
	return 0;
}

/*1.3 此线程专门用来发送心跳包*/
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

/*发送字符串*/
int SocketClient::SendCodeToServer(char *buf, int iSocketClient){
	int iSendLen;
	iSendLen = send(iSocketClient, buf, strlen(buf), 0);
	//若返回的发送的长度为0，关闭socket
	if (iSendLen <= 0)
	{
		printf("Client close!\n");
		closesocket(iSocketClient_data);
		closesocket(iSocketClient_heartbeat);
		return -1;
	}
	return 0;
}

/*2. 数据接收函数*/
char* SocketClient::GetDataFromServer(){
	while (InterlockedExchange(reinterpret_cast<long*>(&g_fResourceInUse), TRUE) == TRUE)//查看他的前一个值，以检查它是否为TRUE。如果这个值是FALSE，那么该资源并没有在使用，则InterlockedExchange函数将值改为TRUE并退出该循环
		Sleep(0);
	char receiveBuf[DataSize];
	for (int i = 0; i < DataSize; i++){
		receiveBuf[i] = ReceiveFromServer[i];
	}
	InterlockedExchange(reinterpret_cast<long*>(&g_fResourceInUse), FALSE);
	return receiveBuf;	/*修改：添加锁*/
}

/*3. 数据发送函数*/
int SocketClient::SendDataToServer(char* pData){
	int iRet = SendCodeToServer(pData, iSocketClient_data);
	return iRet;
}

/*4. 关闭连接函数*/
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

/*5. 获取服务器端信息函数*/
int SocketClient::GetServerInfo(){
	printf("server's ip is %s\t nPort_data : %d\t nPort_heartbeat : %d\t\n",ipServer,data,heartbeat);
	return 0;
}






