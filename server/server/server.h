#ifndef _SERVER_H
#define _SERVER_H
#include <stdio.h>

#define DataSize 1000	/*一次接收的最大字符数*/
#define BACKLOG     100	/*可同时监听10路连接*/

class SocketServer{
public:
	int Init(int nPort_data, int nPort_heartbeat);
	char* GetDataFromClient(char* ipClient);
	int SendDataToClient(char* ipClient, char* pData);
	int CloseAll();
	int Close(char* ipClient);
	int GetClientNum();
	int GetClientInfo();

	int SocketServerInit();
	int SocketServerOpen(int port);
	int AcceptClientConnectionFromData(int iSocketServer, int port);
	int AcceptClientConnectionFromHeartbeat(int iSocketServer, int port);
	char *inet_ntoa_b(struct in_addr inetAddress);	//用于获得客户端的ip
	//void ListAllTheClient();
	void SocketServerClose();
private:
	int iClientNum;
	int data;	//保存数据端端口号
	int heartbeat;	//保存心跳端端口号
};


#endif
