#include <stdio.h>

#include <sys/types.h>          /* See NOTES */
#include <string.h>
#include <Winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib,"Ws2_32.lib")
#include <stdio.h>
#include <string>
#include <Ws2tcpip.h>

#include "stdafx.h"
#include "client.h"

typedef struct MyData
{
	SocketClient *socketClient;
	char * ipServer;
	BOOL g_fResourceInUse;
}MYDATA;

DWORD WINAPI ThreadFor_receive(LPVOID pM)  
{
	MYDATA *pmd = (MYDATA *)pM;
	int iRet;
	char* ReceiveFromServer = new char[DataSize];
	memset(ReceiveFromServer, 0, DataSize);

	char* ReceiveFromServer1 = new char[DataSize];
	memset(ReceiveFromServer1, 0, DataSize);
	strcpy(ReceiveFromServer1, ReceiveFromServer);

	SocketClient *socketClient = pmd->socketClient;
	char * ipServer = pmd->ipServer;
	while (1){
		ReceiveFromServer = socketClient->GetDataFromServer();
		if((ReceiveFromServer != NULL)&&(ReceiveFromServer[0] != '\0')&&(strcmp(ReceiveFromServer, ReceiveFromServer1) != 0)){
			printf("Receive from %s is %s\n",ipServer, ReceiveFromServer);
			strcpy(ReceiveFromServer1, ReceiveFromServer);
		}
		Sleep(1000);
	}
	return 0;
}

int main(int argc, char**argv){
	SocketClient socketClient;
	int iRet;
	char command[16];
	char ipServer[16];	
	char* ReceiveBuf = new char[DataSize];
	memset(ReceiveBuf, 0, DataSize);

	char* SendBuf = new char[DataSize];
	memset(SendBuf, 0, DataSize);

	printf("Please Input Server's IP: ");
	scanf_s("%s",ipServer);
	iRet = socketClient.Init(ipServer,8888,8889);
	if (iRet == -1){
		printf("socketClientInit error!\n");
		return -1;
	}

	MYDATA mydt[1];
	mydt[0].socketClient = &socketClient;
	mydt[0].ipServer = ipServer;
	HANDLE handle0 = CreateThread(NULL, 0, ThreadFor_receive, &mydt[0], 0, NULL);	//用于打印服务器发送给客户端的数据
		
	while (1){
		//fgets(command, 999, stdin);	//使用fgets会把回车保留到字符串中，故不使用
		scanf_s("%s",command);	//使用scanf不会把回车保留到字符串
#if 1
		if (strcmp(command, "receive") == 0)	//输入receive,则接收服务端的数据
		{
			ReceiveBuf = socketClient.GetDataFromServer();
			printf("Receive from %s is %s\n",ipServer, ReceiveBuf);
		}
#endif
		if (strcmp(command, "send") == 0)	//输入send,则向服务端发送数据
		{
			printf("Please Input a String\n");
			scanf_s("%s",SendBuf);
			int iRet = socketClient.SendDataToServer(SendBuf);
			if (iRet < 0)
			{
				printf("SendDataToServer error!\n");
			}
			printf("Send to %s is %s\n",ipServer, SendBuf);
		}
		if (strcmp(command, "close") == 0)	//输入close,则断开服务端
		{
			iRet = socketClient.Close();
			if (iRet == -1){
				printf("Close error!");
			}
		}
		if (strcmp(command, "info") == 0)	//输入info,则打印所有客户端的信息
		{
			iRet = socketClient.GetServerInfo();
			if (iRet == -1){
				printf("GetServerInfo error!");
			}
		}
	}
	return 0;
}






