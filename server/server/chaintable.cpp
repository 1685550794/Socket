#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chaintable.h"
static PT_Name g_ptNameHead;


/////////////////////////////////从链表中增加一项////////////////////////////////////////////////

void add_one_name(char name[])
{
	PT_Name ptNew;
	char *str;
	//char name[128];
	
	//printf("enter the name:");
	//scanf("%s", name);
  
	str  = (char *)malloc(strlen(name) + 1);
	strcpy(str, name);
	//链表中增加一项时，先分配空间，后设置结构体的成员
	ptNew = (PT_Name)malloc(sizeof(T_Name));
	ptNew->name = str;
	ptNew->receive_buf = NULL;
	ptNew->send_buf = NULL;
	  //双向链表pre指向上一个结构体起始地址，next指向下一个结构体起始地址
	ptNew->pre  = NULL;
	ptNew->next = NULL;
  //设置结构体的成员后，把此结构体添加到链表结尾
	add_name(ptNew);
}
void add_name(PT_Name ptNew)
{
	PT_Name ptCur;
	//如果链表头为空，表面链表中无结构体成员
	if (g_ptNameHead == NULL)
	{
		g_ptNameHead = ptNew;
	}
	//否则找到链表的最后一项
	else
	{
		ptCur = g_ptNameHead;
		while (ptCur->next)
		{
			ptCur = ptCur->next;
		}
		//链表中添加一项时，将链表倒数第2项的next成员指向新成员的起始地址
		//                  将链表的新成员的pre成员指向倒数第2项的起始地址
		ptCur->next = ptNew;
		ptNew->pre  = ptCur;
	}
}

/////////////////////////////////从链表中删除一项////////////////////////////////////////////////
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
//获得要删除的链表成员
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
		//将链表的每一项的名字和要删除的链表成员的名字比较，如果相同则找到要删除的结构体
		do {
			if (strcmp(ptCur->name, name) == 0)
				return ptCur;
			else
				ptCur = ptCur->next;
		}while (ptCur);
	}
	return NULL;
}
//删除链表成员
void del_name(PT_Name ptDel)
{
	PT_Name ptCur;	
	PT_Name ptPre;	
	PT_Name ptNext;	
	
	if (g_ptNameHead == ptDel)
	{
		g_ptNameHead = ptDel->next;
		/* 释放 */
		return;
	}
	else
	{
		ptCur = g_ptNameHead->next;
		while (ptCur)
		{
			if (ptCur == ptDel)
			{
				//ptCur->pre为上一个成员的起始地址，ptCur->next为下一个成员的起始地址
				ptPre  = ptCur->pre;
				ptNext = ptCur->next;
				//从链表中删除一项时，将上一个结构体成员的next成员指向下一个结构体的起始地址
				//                    将下一个结构体成员的pre成员指向上一个结构体的起始地址
				//                    上一个结构体成员和下一个结构体成员间有要删除的结构体
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
  //释放链表成员的空间
	free(ptDel->name);
	free(ptDel);
}
//////////////////////////显示链表的成员//////////////////////////////////////


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

PT_Name GetNameHead(){	//获得链表头
	return g_ptNameHead;
}

/////////////////////////在链表中添加发送的字符串和接收的字符串///////////////////////////////////////
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





