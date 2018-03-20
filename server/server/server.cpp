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

/*1. 接口初始化函数*/
int SocketServer::Init(int nPort_data, int nPort_heartbeat){
	MYDATA mydt[2];
	iClientNum = 0;	//初始化客户端的个数
	data = nPort_data;
	heartbeat = nPort_heartbeat;

	mydt[0].socketServer = this;
	mydt[0].nPort = nPort_data;
	HANDLE handle0 = CreateThread(NULL, 0, SocketDataPortInit, &mydt[0], 0, NULL);	 //创建新线程：初始化监听数据的端口

	mydt[1].socketServer = this;
	mydt[1].nPort = nPort_heartbeat;
	HANDLE handle1 = CreateThread(NULL, 0, SocketHeartbeatPortInit, &mydt[1], 0, NULL);	 //创建新线程：初始化监听心跳包的端口
	return 0;
}

/*1.1 初始化监听接收/发送数据的端口*/
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

/*1.2 初始化监听接收/发送数据的端口*/
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

/*1.1.1/1.2.1 初始化socket，根据不同的端口号获得不同的socket句柄，并返回*/
int SocketServer::SocketServerOpen(int port){
	struct sockaddr_in tSocketServerAddr;
	int iSocketServer;
	int iRet;
	
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);
	iSocketServer = socket(AF_INET, SOCK_STREAM, 0);/*获得句柄，AF_INET:IPV4(不修改),SOCK_STREAM:表示TCP,0(不修改)*/
	if (-1 == iSocketServer)
	{
		printf("socket error!\n");
		return -1;
	}
	tSocketServerAddr.sin_family      = AF_INET;/*不修改*/
	tSocketServerAddr.sin_port        = htons(port);/*检测的端口,SERVER_PORT宏定义8888，即检测的端口为8888,只需修改宏SERVER_PORT，htons为将端口号SERVER_PORT转化为网络字节序，端口号8888任写，只需和客户端相同即可*/
 	tSocketServerAddr.sin_addr.s_addr = INADDR_ANY;/*INADDR_ANY为本机上的所有IP，不修改*/
	//绑定iSocketServer，绑定端口SERVER_PORT， sizeof(struct sockaddr)为tSocketServerAddr的长度
	iRet = bind(iSocketServer, (const struct sockaddr *)&tSocketServerAddr, sizeof(struct sockaddr));
	if (-1 == iRet)
	{
		printf("bind error!\n");
		return -1;
	}
    //启动监测数据,开始监听iSocketServer,BACKLOG宏定义为10表明可同时监听10路连接
	iRet = listen(iSocketServer, BACKLOG);
	if (-1 == iRet)
	{
		printf("listen error!\n");
		return -1;
	}
	return iSocketServer;
}

/*1.1.2 根据不同端口号对应的socket句柄等待客户端连接，对于不同的端口新建不同的线程分别接收数据和心跳包*/
int SocketServer::AcceptClientConnectionFromData(int iSocketServer, int port){
	struct sockaddr_in tSocketClientAddr;
	int iSocketClient;
	char *ipClient;
	MYDATA mydt[BACKLOG];
	int iAddrLen;
	iAddrLen = sizeof(struct sockaddr);
	PT_Name ptFind;
	while (1) {
		//等待接受连接，若无客户端连接，在此处休眠，若有连接，则客户端的信息若IP，端口保存到tSocketClientAddr，连接成功时返回客户端句柄iSocketClient
		iSocketClient = accept(iSocketServer, (struct sockaddr *)&tSocketClientAddr, (socklen_t*)&iAddrLen);			/*4. server和client建立一条连接*/
		ipClient = inet_ntoa_b(tSocketClientAddr.sin_addr);
		if (-1 != iSocketClient)		/*iSocketClient不为-1时表明连接成功*/
		{
			iClientNum++;
			add_one_name(ipClient);
			ptFind = get_name(ipClient);	//将数据端的文件句柄记录在链表中
			if (ptFind == NULL)
			{
				printf("1 do not have this name\n");
				return -1;
			}
			ptFind->iSocketClient_data = iSocketClient;
			GetClientInfo();
			printf("Get connection from client DataPort : %s\n", ipClient);	/*客户端的数目、将网络ip转化为ascii字符串*/
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

/*1.2.2 根据不同端口号对应的socket句柄等待客户端连接，对于不同的端口新建不同的线程分别接收数据和心跳包*/
int SocketServer::AcceptClientConnectionFromHeartbeat(int iSocketServer, int port){
	struct sockaddr_in tSocketClientAddr;
	int iSocketClient;
	char *ipClient;
	MYDATA mydt[BACKLOG];
	int iAddrLen;
	iAddrLen = sizeof(struct sockaddr);
	PT_Name ptFind;
	while (1) {
		//等待接受连接，若无客户端连接，在此处休眠，若有连接，则客户端的信息若IP，端口保存到tSocketClientAddr，连接成功时返回客户端句柄iSocketClient
		iSocketClient = accept(iSocketServer, (struct sockaddr *)&tSocketClientAddr, (socklen_t*)&iAddrLen);			/*4. server和client建立一条连接*/
		ipClient = inet_ntoa_b(tSocketClientAddr.sin_addr);
		ptFind = get_name(ipClient);
		while (ptFind == NULL)	//确保数据端先连接，因为客户端是在数据端连接时放入链表的
		{
			Sleep(100);
			ptFind = get_name(ipClient);	//将心跳端的文件句柄记录在链表中
		}
		ptFind->iSocketClient_heartbeat = iSocketClient;
		if (-1 != iSocketClient)
		{
			printf("Get connection from client HeartBeatPort : %s\n", ipClient);	/*客户端的数目、将网络ip转化为ascii字符串*/
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

/*1.1.2.1 此线程专门用于接收客户端的有用的数据*/
DWORD WINAPI ThreadFor_ReceiveData(LPVOID pM)  
{
	MYDATA *pmd = (MYDATA *)pM;
	int iRecvLen;
	char ucRecvBuf[DataSize];
	SocketServer *socketServer = pmd->socketServer;
	char *ipClient = pmd->ipClient;
	PT_Name ptCur = get_name(ipClient);		//从链表中获得客户端
	char* CloseMessage = "I am close!";
	while (1)
	{
		// 接收客户端发来的数据存到ucRecvBuf数组，接受的数据最大长度为999，flag为0
		iRecvLen = recv(ptCur->iSocketClient_data, ucRecvBuf, DataSize-1, 0);
		if (iRecvLen <= 0)//接受到的数据个数小于0，关闭socket
		{
			return -1;
		}
		else
		{     
			//打印从客户端接收的数据
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
				ptCur->receive_buf = ucRecvBuf;  //将接收的数据放入链表
			}
		}
	}
}

/*1.1.2.2 此线程专门用于向客户端发送数据*/
DWORD WINAPI ThreadFor_SendData(LPVOID pM)  
{
	MYDATA *pmd = (MYDATA *)pM;
	SocketServer *socketServer = pmd->socketServer;
	char *ipClient = pmd->ipClient;
	PT_Name ptCur = get_name(ipClient);		//从链表中获得客户端
	while (1)
	{
		if (ptCur->send_buf != NULL){
			send(ptCur->iSocketClient_heartbeat, ptCur->send_buf, strlen(ptCur->send_buf), 0);
			ptCur->send_buf = NULL;
		}
	}
}

/*1.2.2.1 此线程专门用于接收客户端的心跳包*/
DWORD WINAPI ThreadFor_heartbeat(LPVOID pM)  
{
	MYDATA *pmd = (MYDATA *)pM;

	int iRecvLen;
	int timeout_count = 0;
	char ucRecvBuf[DataSize];
	SocketServer *socketServer = pmd->socketServer;
	char *ipClient = pmd->ipClient;
	char* HeartbeatBag = "I am alive!";
	PT_Name ptCur = get_name(ipClient);		//从链表中获得客户端
	fd_set tRFds;

	struct timeval time;
	FD_ZERO(&tRFds);
	FD_SET(ptCur->iSocketClient_heartbeat, &tRFds);
	time.tv_sec=15;
	time.tv_usec=0;
	while (1)
	{
		int ret = select(NULL, &tRFds, NULL, NULL, &time);	//返回值大于0表明有某一个文件可读/可写/异常或超时
		if(ret > 0)
		{
			timeout_count = 0;
			// 接收客户端发来的数据存到ucRecvBuf数组，接受的数据最大长度为999，flag为0
			iRecvLen = recv(ptCur->iSocketClient_heartbeat, ucRecvBuf, DataSize-1, 0);
			if (iRecvLen <= 0) //接受到的数据个数小于0，关闭socket
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
				//打印从客户端接收的数据
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
			mod -= digit * 10;	//个位
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

/*2. 数据接收函数*/
char* SocketServer::GetDataFromClient(char* ipClient){
	char* receive_buf = get_receive_buf(ipClient);	//从链表中获得接收的数据
	return receive_buf;
}

/*3. 数据发送函数*/
int SocketServer::SendDataToClient(char* ipClient, char* pData){
	add_send_buf(ipClient, pData);		//将写入放入链表中的客户端
	return 0;
}

/*4. 关闭所有连接函数*/
int SocketServer::CloseAll(){
	PT_Name ptCur;
	ptCur = GetNameHead();	//获得链表头
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

/*5. 关闭指定连接函数*/
int SocketServer::Close(char* ipClient){
	PT_Name ptCur;
	ptCur = get_name(ipClient);	//将心跳端的文件句柄记录在链表中
	if (ptCur == NULL)
	{
		printf("do not have this name\n");
		return -1;
	}
	closesocket(ptCur->iSocketClient_data);
	closesocket(ptCur->iSocketClient_heartbeat);	//关闭心跳包端口时，心跳包线程函数ThreadFor_heartbeat的recv会检测到iRecvLen <= 0
	//del_one_name(ptCur->name);					//当检测到iRecvLen <= 0时，会自动执行del_one_name，故此处不重复执行
	Sleep(10);
	return 0;
}

/*6. 获取客户端个数函数*/
int SocketServer::GetClientNum(){
	return iClientNum;
}

/*7. 获取客户端信息函数*/
int SocketServer::GetClientInfo(){
	printf("Online Client List Below:\n");
	list_all_name(data, heartbeat);	//传入参数为端口号
	return 0;
}

/*8. 关闭服务端*/
void SocketServer::SocketServerClose(){
	closesocket(iSocketServer_data);
	closesocket(iSocketServer_heartbeat);
}
