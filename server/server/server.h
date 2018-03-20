#ifndef _SERVER_H
#define _SERVER_H
#include <stdio.h>

#define DataSize 1000	/*һ�ν��յ�����ַ���*/
#define BACKLOG     100	/*��ͬʱ����10·����*/

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
	char *inet_ntoa_b(struct in_addr inetAddress);	//���ڻ�ÿͻ��˵�ip
	//void ListAllTheClient();
	void SocketServerClose();
private:
	int iClientNum;
	int data;	//�������ݶ˶˿ں�
	int heartbeat;	//���������˶˿ں�
};


#endif
