#ifndef _CHAINTABLE_H
#define _CHAINTABLE_H
#include <stdio.h>

typedef struct NAME{
	char *name;
	char *receive_buf;
	char *send_buf;
	int iSocketClient_data;
	int iSocketClient_heartbeat;
	struct NAME *pre;
	struct NAME *next;
}T_Name, *PT_Name;

char* get_receive_buf(char* name);
void add_send_buf(char* name, char* buf);
PT_Name GetNameHead();

void add_one_name(char name[]);
void add_name(PT_Name ptNew);
void del_one_name(char name[]);
PT_Name get_name(char *name);
void del_name(PT_Name ptDel);
void list_all_name(int data, int heartbeat);

#endif



