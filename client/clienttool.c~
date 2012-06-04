#include "clienttool.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

int server_init(int *sfd, SAI *serv, char *hostip)
{
	if ((*sfd=socket(AF_INET,SOCK_DGRAM,0))==-1) 
	{
        perror("socket error!");
        return -1;
    }
    memset(serv, 0, sizeof(SAI));
    serv->sin_family = AF_INET;
    serv->sin_port = htons(MYPORT);
    if (-1 == (inet_pton(AF_INET, hostip, &(serv->sin_addr))))
	{
		printf("inet_pton error for %s\n", hostip);
		return -1;
	}
	return 0;
}

char *append_msg(char *cmd, char *nick_name, char *time, char *msg)
{
	strcat (cmd, " ");
	strcat (cmd, nick_name);
	strcat (cmd, " ");
	strcat (cmd, time);
	strcat (cmd, " ");
	strcat (cmd, msg);
	return cmd;
}

int send_msg(int fd, SAI* to, char *msg)
{
	int numbytes = 0;
	if (-1 == (numbytes = sendto(fd, msg, MAXLINE, 0,
                           (SA *)to,sizeof((*to))))) 
	{
        perror("send to");
		return -1;
	}
	return 0;
}

int recv_msg(int fd, SAI* from, socklen_t *sin_size, char *msg)
{
	int numbytes = 0;
	if (-1 == (numbytes = recvfrom(fd, msg, MAXLINE, 0,
								(SA *)from, sin_size))) 
	{
        perror("receive from");
		return 0;
	}
	msg[numbytes] = '\0';
	return numbytes;
}

MSG* msg_queue_init()
{
	MSG *rt = (MSG *)malloc(sizeof(MSG));
	if (NULL == rt) return NULL;
	rt->next = NULL;
	return rt;
}
int msg_queue_ins(MSG* rt, char *msg)
{
	MSG *t = (MSG *)malloc(sizeof(MSG));
	char nick_name[30], ip[20], time[30], buf[MAXLINE];
	if (NULL == t)
	{
		perror("msg insert error!!");
		return -1;
	}
	t->next = rt->next;
	msg_prs(msg, ip, nick_name, time, buf);
	strcpy(t->nick_name, nick_name);
	strcpy(t->ip, ip);
	strcpy(t->time, time);
	strcpy(t->msg, buf);
	rt->next = t;
	return 0;
}

void msg_display(MSG *rt, int deep, int num)
{
	static int id = 0;
	if (NULL == rt && num == deep) 
	{
		perror("No Message.");
		id = 0;
		while (id++ < deep) puts ("");
		return ;
	}
	if (deep > 0 && NULL != rt)
	{
		msg_display(rt->next, deep-1, num);
		if (0 == strcmp("System", rt->nick_name))
		{
			printf("\t%d: %s: %s[%s]\n", ++id, rt->nick_name, rt->msg, rt->time);
		}
		else printf("\t%d: %s(%s): %s[%s]\n", ++id, rt->nick_name, rt->ip, rt->msg, rt->time);
		if (num == deep) while (id++ < deep) puts ("");
	}
	else
	{
		id = 0;
	}
}

void display(MSG *rt, char *nick_name, int mode, char *to_name, char *msg, int num)
{
	char buf[MAXLINE] = {0};
	if (-1 == system("clear"))
	{
		perror("system clear error.");
	}
	if (0 == num) num = 8;
	printf("\n\t\t\tWelcome~!\n");
	printf("Recent %d message:\n", num);
	msg_display(rt->next, num, num);
	sprintf(buf, "\n$%s >>>", nick_name);
	if (PRI == mode)
	{
		strcat(buf, "(");
		strcat(buf, to_name);
		strcat(buf, ")");
	}
	if (NULL != msg)
	{
		strcat(buf, msg);
	}
	printf (buf);
	fflush(stdout);
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

int getch()
{
    char ch;
    struct termios save, ne;
    ioctl(0, TCGETS, &save);
    ioctl(0, TCGETS, &ne);
    ne.c_lflag &= ~(ECHO | ICANON);
    ioctl(0, TCSETS, &ne);
    if (-1 == read(0, &ch, 1))
	{
		return -1;
	}
    ioctl(0, TCSETS, &save);
    return ch;
}

char tme[30];
char *get_time()
{
	time_t now;
	struct tm * timenow;
	time(&now);
	//return ctime(&now);
	timenow = localtime(&now);
	sprintf (tme, "%02d:%02d", timenow->tm_hour,timenow->tm_min);
	tme[5] = 0;
	return tme;//asctime(timenow);
}