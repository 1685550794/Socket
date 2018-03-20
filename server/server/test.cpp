#include <stdio.h>

#include "stdafx.h"
#include <sys/types.h>          /* See NOTES */
#include <string.h>
#include <Winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib,"Ws2_32.lib")
#include <stdio.h>
#include <string>
#include <Ws2tcpip.h>
#include "stdafx.h"
#include "server.h"
#include "chaintable.h"

typedef struct MyData
{
	SocketServer *socketServer;
	char * ipClient;
}MYDATA;

DWORD WINAPI ThreadFor_receive(LPVOID pM)  
{
	MYDATA *pmd = (MYDATA *)pM;
	int iRet;
	char* ReceiveFromServer = new char[DataSize];
	memset(ReceiveFromServer, 0, DataSize);
	SocketServer *socketServer = pmd->socketServer;
	PT_Name ptCur;
	while (1){
		ptCur = GetNameHead();	//获得链表头
		while (ptCur)
		{
			ReceiveFromServer = socketServer->GetDataFromClient(ptCur->name);
			if((ReceiveFromServer != NULL)&&(ReceiveFromServer[0] != '\0')){
				printf("Receive from %s : %s\n", ptCur->name,ReceiveFromServer);
				memset(ReceiveFromServer, 0, DataSize);	//接完就清空
			}
			ptCur = ptCur->next;
		}
		Sleep(1000);
	}
	return 0;
}

int main(int argc, char**argv){
	int iRet;
	SocketServer socketServer;
	iRet = socketServer.Init(8888,8889);
	if (iRet == -1){
		printf("SocketServerInit error!\n");
		return -1;
	}
	char command[16];
	char ipClient[16];	
	char* ReceiveBuf = new char[DataSize];
	memset(ReceiveBuf, 0, DataSize);

	char* SendBuf = new char[DataSize];
	memset(SendBuf, 0, DataSize);

	MYDATA mydt[1];
	mydt[0].socketServer = &socketServer;
	//mydt[0].ipClient = ipServer;
	HANDLE handle0 = CreateThread(NULL, 0, ThreadFor_receive, &mydt[0], 0, NULL);	//用于打印客户端发送给服务器的数据

	while (1){
		//fgets(command, 999, stdin);	//使用fgets会把回车保留到字符串中，故不使用
		scanf_s("%s",command);	//使用scanf不会把回车保留到字符串
#if 1
		if (strcmp(command, "receive") == 0)	//输入receive,则获得客户端的数据
		{
			printf("Please Input Client's IP: ");
			scanf_s("%s",ipClient);
			ReceiveBuf = socketServer.GetDataFromClient(ipClient);
			printf("Receive from %s is %s\n",ipClient, ReceiveBuf);
		}
#endif
		if (strcmp(command, "send") == 0)	//输入send,则向指定的客户端发送数据
		{
			printf("Please Input Client's IP: ");
			scanf_s("%s",ipClient);
			printf("Please Input a String\n");
			scanf_s("%s",SendBuf);
			socketServer.SendDataToClient(ipClient,SendBuf);
			printf("Send to %s is %s\n",ipClient, SendBuf);
		}
		if (strcmp(command, "closeall") == 0)	//输入closeall,则断开所有的客户端
		{
			iRet = socketServer.CloseAll();
			if (iRet == -1){
				printf("CloseAll error!");
			}
		}
		if (strcmp(command, "closeone") == 0)	//输入closeone,则断开指定的客户端
		{
			printf("Please Input Client's IP: ");
			scanf_s("%s",ipClient);
			iRet = socketServer.Close(ipClient);
			if (iRet == -1){
				printf("Close error!");
			}
		}
		if (strcmp(command, "num") == 0)	//输入num,则获得客户端的个数
		{
			int num = socketServer.GetClientNum();
			printf("Client number is %d\n",num);
		}
		if (strcmp(command, "info") == 0)	//输入info,则打印所有客户端的信息
		{
			iRet = socketServer.GetClientInfo();
			if (iRet == -1){
				printf("GetClientInfo error!");
			}
		}
	}
	return 0;	
}






