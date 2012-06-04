#include "servtool.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
typedef struct ThreadArgs{
	ClientList *head;
}ThreadArgs;
int sockfd;
/*
 * proccess message msg with command for client addr
 */
void service (SA *addr, SA *servaddr, socklen_t len, char *msg, ClientList *rt);
/*
 * thread for check client timeout 
 */
void *thread_timeout(void *ta);
/*
 * check user onlineable
 */
int check_user_online(ClientList *rt, Client *t);

int main (int argc, char *argv[] ) 
{
	int retval;
	socklen_t sin_size;
	char buf[MAXLINE];
	SAI host_addr, new_addr;
	ClientList *head = client_init();
	ThreadArgs ta;
	ta.head = head;
	pthread_t * thread;
	int tp = 0;
	if (-1 == im_init(&sockfd, &host_addr))
	{
		exit(1);
	}
	
	thread = (pthread_t *)malloc(sizeof(pthread_t));
	tp = pthread_create(thread, NULL, thread_timeout, &ta);
	if (0 != tp) 
	{
		printf("message thread create failed!\n");	
		exit(1);
	}
	puts("service start.");
	while ( 1 )
	{
		retval = recvfrom(sockfd, buf, MAXLINE, 0, (SA*)&new_addr, &sin_size);
		if (0 == retval)
		{
			perror("accept error!");
			continue;
		}
		buf[retval] = 0;
		service((SA*)&new_addr, (SA *)&host_addr, sin_size, buf, head);
	}
	pthread_join(*thread, NULL);
	return 0;
}
void *thread_timeout(void *ta)
{
	ThreadArgs *t = (ThreadArgs *) ta;
	ClientList *rt = t->head->next;
	ClientList *ttr, *head = t->head;
	Client ct;
	char str[MAXLINE];
	sprintf(str, "0.0.0.0 timeout %s timeout check.", get_time());
	/*
	 * check users timeout 
	 */
	while (1)
	{
		sleep(10);
		//printf("start timeout check.\n");
		rt = head->next;
		while (NULL != rt)
		{
			rt->client.status = OFFLINE;
			sprintf(str, "0.0.0.0 timeout %s timeout check.", get_time());
			sendto(sockfd, str, MAXLINE, 0, &(rt->client.sock_addr), sizeof(SA));
			//printf("now check user %s\n", rt->client.nick_name);
			sleep(1);
			if (OFFLINE == rt->client.status)
			{
				//printf("\tuser %s is offline.\n", rt->client.nick_name);
				ct.sock_addr = rt->client.sock_addr;
				strcpy(ct.nick_name, rt->client.nick_name);
				ttr = rt->next;
				if (-1 == clinet_del(head, &ct))
				{
					perror("Client quit error!");
				}
				printf("%s has disconnected.\n", rt->client.nick_name);
				/*
				* notify all online user that user nick_name has disconnected from server
				*/
				sprintf(str, "%s System %s %s has disconnected.", (char *)inet_ntoa(((SAI*)&(rt->client.sock_addr))->sin_addr), get_time(), rt->client.nick_name);
				rt = head->next;
				while (NULL != rt)
				{
					sendto (sockfd, str, MAXLINE, 0, &(rt->client.sock_addr), sizeof(SA));
					rt = rt->next;
				}
				rt = ttr;
			}
			else rt = rt->next;
		}
		//printf("end timeout check.\n");
	}	
}
int check_user_online(ClientList *rt, Client *t)
{
	ClientList *f = client_find(rt, t);
	if (NULL != f) return 0;
	else return -1;
}
void service (SA *addr, SA *servaddr, socklen_t len, char *msg, ClientList *rt)
{
	char str[MAXLINE];
	char cmd[MAXLINE];
	char tme[30];
	char ip[MAXLINE];
	char nick_name[MAXLINE];
	char *p = msg, *q = cmd;
	Client t;
	ClientList *head = rt;
	int flag;
	/*
	 * get command from msg to cmd
	 */
	while (*p && (' ' == *p)) ++p;
	while (*p && (' ' != *p))
	{
		*q++ = *p++;
	}
	*q = 0;
	if (*p)++p;
	/*
	 * store the remain message to str
	 */
	q = str;
	while (*p)
	{
		*q++ = *p++;
	}
	*q = 0;
	
	/*
	 * append client ip in front of message str
	 * with format "ip nick_name time message"
	 * the result is stored to str
	 */
	strcpy(ip, (char*)inet_ntoa(((SAI *)addr)->sin_addr));
	strcat(ip, " ");
	strcat(ip, str);
	strcpy(str, ip);
	/*
	 * if client is'nt a new client, check it avaliable.
	 */
	if (0 != strcmp(cmd, "new"))
	{
		msg_prs(str, NULL, nick_name, tme, NULL);
		strcpy(t.nick_name,nick_name);
		int f = check_user_online(head, &t);
		if (-1 == f)
		{
			sprintf(str, "%s System %s You had offline from server, please reconnected.", (char*)inet_ntoa(((SAI *)servaddr)->sin_addr), get_time());
			sendto(sockfd, str, MAXLINE, 0, addr, len);
			return;
		}
	}
	/*
	 * client send message to all online user
	 */
	if (0 == strcmp(cmd, "all"))
	{
		rt = rt->next;
		while (NULL != rt)
		{
			sendto (sockfd, str, MAXLINE, 0, &(rt->client.sock_addr), len);
			rt = rt->next;
		}
	}
	/*
	 * a new client connected to server
	 */
	else if (0 == strcmp(cmd, "new"))
	{
		/*
		 * get client queue element with t
		 */
		msg_prs(str, NULL, nick_name, tme, NULL);
		strcpy(t.nick_name,nick_name);
		t.sock_addr = *addr;
		t.status = ONLINE;
		if (-1 == (flag=client_ins(rt, &t)))
		{
			perror("Clinet insert error!");
		}
		else if (1 == flag)
		{
			printf("A invalidate client connected.\n");
			sprintf(str, "%s error %s NickName Error.", (char*)inet_ntoa(((SAI *)servaddr)->sin_addr), get_time());
			sendto(sockfd, str, MAXLINE, 0, addr, len);
			return;
		}/*user already on server*/
		
		printf ("Got a new conection from %s.\n", ip);	
		/*
		 * send new user join message to all online user
		 */
		sprintf(str, "%s System %s %s has connected.",(char*)inet_ntoa(((SAI *)servaddr)->sin_addr),  get_time(), nick_name);
		rt = head->next;
		while (NULL != rt)
		{
			sendto (sockfd, str, MAXLINE, 0, &(rt->client.sock_addr), len);
			rt = rt->next;
		}
		
	}
	/*
	 * client quit from server normally
	 */
	else if (0 == strcmp(cmd, "quit"))
	{
		msg_prs(str, ip, nick_name, NULL, NULL);
		t.sock_addr = *addr;
		strcpy(t.nick_name, nick_name);
		if (-1 == clinet_del(rt, &t))
		{
			perror("Client quit error!");
		}
		printf("%s has disconnected.\n", nick_name);
		/*
		 * notify all online user that user nick_name has disconnected from server
		 */
		sprintf(str, "%s System %s %s has disconnected.", ip, get_time(), nick_name);
		rt = rt->next;
		while (NULL != rt)
		{
			sendto (sockfd, str, MAXLINE, 0, &(rt->client.sock_addr), len);
			rt = rt->next;
		}
	}
	/*
	 * client query online users from server
	 */
	else if (0 == strcmp(cmd, "getuser"))
	{
		ClientList *t = rt->next;
		str[0] = 0;
		strcat(str, (char*)inet_ntoa(((SAI *)addr)->sin_addr));
		strcat(str, " ");
		strcat(str, "userlist");
		strcat(str, " ");
		strcat(str, get_time());
		while ( NULL != t )
		{
			strcat(str, " ");
			strcat(str, t->client.nick_name);
			t = t->next;
		}
		sendto(sockfd, str, MAXLINE, 0, addr, len);
	}
	/*
	 * client query online users from server
	 */
	else if (0 == strcmp(cmd, "online"))
	{
		msg_prs(str, ip, nick_name, NULL, NULL);
		t.sock_addr = *addr;
		strcpy(t.nick_name, nick_name);
		t.status = ONLINE;
		ClientList * find = client_find(head, &t);
		find->client.status = ONLINE;
		//printf("find client %s is online.\n", find->client.nick_name);
		return;
	}
	/*
	 * client send message to private user
	 */
	else
	{
		ClientList *t = rt->next;
		while ( NULL != t )
		{
			if ( 0 == strcmp(t->client.nick_name, cmd))
			{
				msg_prs(str, ip, nick_name, tme, cmd);
				strcpy(str, "to ");
				strcat(str, t->client.nick_name);
				strcat(str, ">> ");
				strcat(str, cmd);
				cmd[0] = 0;
				append(cmd, ip, nick_name, tme, str);
				sendto(sockfd, cmd, MAXLINE, 0, &(t->client.sock_addr), len);
				sendto(sockfd, cmd, MAXLINE, 0, addr, len);
				return;
			}
			t = t->next;
		}
		sprintf(str, "%s System %s Wrong user nickname, please try another one.", (char*)inet_ntoa(((SAI *)servaddr)->sin_addr), get_time());
		sendto(sockfd, str, MAXLINE, 0, addr, len);
	}
}
