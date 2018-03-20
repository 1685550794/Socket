#ifndef _CLIENT_H
#define _CLIENT_H
#include <stdio.h>
#include <Winsock2.h>
#define DataSize 1000

class SocketClient{
public:
	int Init(char* ip, int nPort_data, int nPort_heartbeat);
	char* GetDataFromServer();
	int SendDataToServer(char* pData);
	int Close();
	int GetServerInfo();

	int SocketClientOpen(int port);
	char* GetCodeFromUser();
	int SendCodeToServer(char *buf, int iSocketClient);
	char* ConvertArrayToString(char buf[]);
	void SocketClientClose();
private:
	char* ipServer;
	BOOL g_fResourceInUse;	//����һ��ȫ�ֱ�����������֤������Դ�Ƿ����ڱ�ʹ����
	int data;	//�������ݶ˶˿ں�
	int heartbeat;	//���������˶˿ں�
};


#endif
