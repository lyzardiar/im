#include "servtool.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

int im_init(int *sockfd, SAI *host_addr)
{
	/*	
	 *init socket
	 */
	if (-1 == (*sockfd = socket(AF_INET, SOCK_DGRAM, 0)))
	{
		perror("socket error!");
		return -1;
	}
	/*
	 * init host information
	 */
	memset (host_addr, 0, sizeof(SAI));
	host_addr->sin_family = AF_INET;
	host_addr->sin_port = htons(MYPORT);
	host_addr->sin_addr.s_addr = INADDR_ANY;
	/*
	 * bind
	 */
	if (-1 == bind(*sockfd, (SA *)host_addr, sizeof(SA)))
	{
		perror("bind error!");
		return -1;
	}
	
	return 0;
}

ClientList * client_init()
{
	ClientList *rt = (ClientList*)malloc(sizeof(ClientList));
	if (NULL == rt) return NULL;
	else 
	{
		rt->syn = 0;
		rt->next = NULL;
		return rt;
	}
}
int client_ins (ClientList* rt, Client *c) 
{
	if (NULL != client_find(rt, c)) return 1;
	ClientList *temp = (ClientList*)malloc(sizeof(ClientList));
	if (NULL == temp) return -1;
	else 
	{
		temp->client = *c;
		temp->next = rt->next;
		rt->next = temp;
		temp->pre = rt;
		if (NULL != temp->next)
		{
			temp->next->pre = temp;
		}
		return 0;
	}
}
int clinet_del (ClientList* rt, Client *c) 
{
	ClientList *p = client_find(rt, c);
	if (NULL == p) return -1;
	else 
	{
		p->pre->next = p->next;
		if (NULL != p->next)
		{
			p->next->pre = p->pre;
		}
		free(p);
		p = NULL;
	}
	return 0;
}
ClientList * client_find (ClientList* rt, Client *c)
{
	if (0 == strcmp("error", c->nick_name)) return rt;
	if (0 == strcmp("system", c->nick_name)) return rt;
	if (0 == strcmp("all", c->nick_name)) return rt;
	if (0 == strcmp("quit", c->nick_name)) return rt;
	if (0 == strcmp("System", c->nick_name)) return rt;
	if (0 == strcmp("getuser", c->nick_name)) return rt;
	
	ClientList *p = rt->next;
	while ( NULL != p )
	{
		if (0 == strcmp(p->client.nick_name, c->nick_name)) return p;
		p = p->next;
	}
	return NULL;
}
char tme[30];
char *get_time()
{
	time_t now;
	struct tm * timenow;
	time(&now);
	timenow = localtime(&now);
	sprintf (tme, "%02d:%02d", timenow->tm_hour,timenow->tm_min);
	tme[5] = 0;
	return tme;//asctime(timenow);
}
void msg_prs(char *msg, char *ip, char *nick_name, char *time, char *user_msg)
{
	if (NULL == msg) return;
	while (*msg && ' ' == *msg) ++msg;
	while (*msg && ' ' != *msg) 
	{
		if (NULL != ip) *ip++ = *msg++;
		else ++msg;
	}
	if (NULL != ip) *ip = 0;
	while (*msg && ' ' == *msg) ++msg;
	while (*msg && ' ' != *msg) 
	{
		if (NULL != nick_name) *nick_name++ = *msg++;
		else ++msg;
	}
	if (NULL != nick_name) *nick_name = 0;
	while (*msg && ' ' == *msg) ++msg;
	while (*msg && ' ' != *msg) 
	{
		if (NULL != time) *time++ = *msg++;
		else ++msg;
	}
	if (NULL != time) *time = 0;
	while (*msg && ' ' == *msg) ++msg;
	while (*msg)
	{
		if(NULL != user_msg) *user_msg++ = *msg++;
		else ++msg;
	}
	if (NULL != user_msg) *user_msg = 0;
}

char * append(char *msg, char *ip, char *nick_name, char *time, char *user_msg)
{
	strcat(msg, " ");
	strcat(msg, ip);
	strcat(msg, " ");
	strcat(msg, nick_name);
	strcat(msg, " ");
	strcat(msg, time);
	strcat(msg, " ");
	strcat(msg, user_msg);
	return msg;
}