#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chaintable.h"
static PT_Name g_ptNameHead;


/////////////////////////////////������������һ��////////////////////////////////////////////////

void add_one_name(char name[])
{
	PT_Name ptNew;
	char *str;
	//char name[128];
	
	//printf("enter the name:");
	//scanf("%s", name);
  
	str  = (char *)malloc(strlen(name) + 1);
	strcpy(str, name);
	//����������һ��ʱ���ȷ���ռ䣬�����ýṹ��ĳ�Ա
	ptNew = (PT_Name)malloc(sizeof(T_Name));
	ptNew->name = str;
	ptNew->receive_buf = NULL;
	ptNew->send_buf = NULL;
	  //˫������preָ����һ���ṹ����ʼ��ַ��nextָ����һ���ṹ����ʼ��ַ
	ptNew->pre  = NULL;
	ptNew->next = NULL;
  //���ýṹ��ĳ�Ա�󣬰Ѵ˽ṹ����ӵ������β
	add_name(ptNew);
}
void add_name(PT_Name ptNew)
{
	PT_Name ptCur;
	//�������ͷΪ�գ������������޽ṹ���Ա
	if (g_ptNameHead == NULL)
	{
		g_ptNameHead = ptNew;
	}
	//�����ҵ���������һ��
	else
	{
		ptCur = g_ptNameHead;
		while (ptCur->next)
		{
			ptCur = ptCur->next;
		}
		//���������һ��ʱ������������2���next��Աָ���³�Ա����ʼ��ַ
		//                  ��������³�Ա��pre��Աָ������2�����ʼ��ַ
		ptCur->next = ptNew;
		ptNew->pre  = ptCur;
	}
}

/////////////////////////////////��������ɾ��һ��////////////////////////////////////////////////
void del_one_name(char name[])
{	
	PT_Name ptFind;
	//char name[128];
	
	//printf("enter the name:");
	//scanf("%s", name);

	ptFind = get_name(name);
	//printf("get_name ptFind->name = %s\n",ptFind->name);
	if (ptFind == NULL)
	{
		printf("do not have this name\n");
		return ;
	}
	
	del_name(ptFind);
	
}
//���Ҫɾ���������Ա
PT_Name get_name(char *name)
{
	PT_Name ptCur;
	if (g_ptNameHead == NULL)
	{
		return NULL;
	}
	else
	{
		ptCur = g_ptNameHead;
		//�������ÿһ������ֺ�Ҫɾ���������Ա�����ֱȽϣ������ͬ���ҵ�Ҫɾ���Ľṹ��
		do {
			if (strcmp(ptCur->name, name) == 0)
				return ptCur;
			else
				ptCur = ptCur->next;
		}while (ptCur);
	}
	return NULL;
}
//ɾ�������Ա
void del_name(PT_Name ptDel)
{
	PT_Name ptCur;	
	PT_Name ptPre;	
	PT_Name ptNext;	
	
	if (g_ptNameHead == ptDel)
	{
		g_ptNameHead = ptDel->next;
		/* �ͷ� */
		return;
	}
	else
	{
		ptCur = g_ptNameHead->next;
		while (ptCur)
		{
			if (ptCur == ptDel)
			{
				//ptCur->preΪ��һ����Ա����ʼ��ַ��ptCur->nextΪ��һ����Ա����ʼ��ַ
				ptPre  = ptCur->pre;
				ptNext = ptCur->next;
				//��������ɾ��һ��ʱ������һ���ṹ���Ա��next��Աָ����һ���ṹ�����ʼ��ַ
				//                    ����һ���ṹ���Ա��pre��Աָ����һ���ṹ�����ʼ��ַ
				//                    ��һ���ṹ���Ա����һ���ṹ���Ա����Ҫɾ���Ľṹ��
				ptPre->next = ptNext;
				if (ptNext)
				{
					ptNext->pre = ptPre;
				}
				break;
			}
			else
			{
				ptCur = ptCur->next;
			}
		}
	}
  //�ͷ������Ա�Ŀռ�
	free(ptDel->name);
	free(ptDel);
}
//////////////////////////��ʾ����ĳ�Ա//////////////////////////////////////


void list_all_name(int data, int heartbeat)
{
	PT_Name ptCur;
	int i = 1;
	ptCur = g_ptNameHead;
	if (!g_ptNameHead){
		printf("No Client Online!\n");
		return;
	}
	while (ptCur)
	{
		printf("%02d : %s\t nPort_data : %d\t nPort_heartbeat : %d\t\n", i++, ptCur->name,data,heartbeat);
		ptCur = ptCur->next;
	}
}

PT_Name GetNameHead(){	//�������ͷ
	return g_ptNameHead;
}

/////////////////////////����������ӷ��͵��ַ����ͽ��յ��ַ���///////////////////////////////////////
char* get_receive_buf(char* name){
	PT_Name ptFind;
	ptFind = get_name(name);
	if (ptFind == NULL)
	{
		printf("do not have this name\n");
		return NULL;
	}
	return ptFind->receive_buf;
}

void add_send_buf(char* name, char* buf){
	PT_Name ptFind;
	ptFind = get_name(name);
	if (ptFind == NULL)
	{
		printf("do not have this name\n");
		return ;
	}
	ptFind->send_buf = buf;
}





