#include "clienttool.h"

#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <pthread.h>

typedef struct ThreadArgs{
	char *msg;
	char *nick_name;
	char *to_name;
	MSG *head;
	int sockfd;
	int *mode;
	int *msg_num;
	SAI *server_addr_ptr;
} ThreadArgs;	
char msg[MAXLINE];
/*
 * message thread for waiting for server mesasge
 */
void *msg_thread(void * ta);
/*
 * check user command and proccess it with different command
 * cmd : user input message
 * sockfd : client sockfd
 * servadd : pointer for server sock_addr
 * nickname : nickname for client user
 * toname : private chat object
 * mode : chat mode
 * msg_num : client message view list size
 * rt : message queue head pointer
 */
int cmd_prs(char *cmd, int sockfd, SAI* servadd, char *nickname, char *toname, int *mode, int *msg_num, MSG *rt);

int main(int argc, char *argv[])
{
    int sockfd, tp;
	char buf[MAXLINE] = "new Hello, world!";
	int user_cmd;
	char nick_name[MAXLINE] = "游客";
	char ch;
	int ch_input;
	int msg_num = 8;
	int mode = 1;
	char to_name[MAXLINE];
    SAI server_addr; 
	MSG *head = msg_queue_init();
    if (2 == argc) 
	{
        perror("usage: client hostIp nickName\n");
        exit(1);
    }
    if (2 < argc)
	{
		strcpy(nick_name, argv[2]);
	}
	/*
	 * init the socket
	 */
    server_init(&sockfd, &server_addr, argv[1]);
	/*
	 * message receive thread
	 */
	pthread_t *thread = (pthread_t *) malloc( sizeof(pthread_t) );
	ThreadArgs ta;
	ta.head = head;
	ta.msg = msg;
	ta.nick_name = nick_name;
	ta.to_name = to_name;
	ta.mode = &mode;
	ta.msg_num = &msg_num;
	ta.sockfd = sockfd;
	ta.server_addr_ptr = &server_addr;
	tp = pthread_create(thread, NULL, msg_thread, &ta );
	if (0 != tp) 
	{
		printf("message thread create failed!\n");	
		exit(1);
	}
	
	while ( 1 )
	{
		/*user input*/
		ch_input = 0;
		msg[0] = 0;
		while ('\n' != (ch=getch()))
		{
			msg[ch_input++] = ch;
			msg[ch_input] = 0;
			putchar(ch);
			fflush(stdout);
			if (MAXLINE/2 == ch_input) break;
		}
		user_cmd = cmd_prs(msg, sockfd, &server_addr, nick_name, to_name, &mode, &msg_num, head);
		/*
		 * general message without user command
		 */
		if (0 == user_cmd)
		{
			if (ALL == mode)
			{
				strcpy(buf,"all");
				send_msg(sockfd, &server_addr, append_msg(buf, nick_name, get_time(), msg));
			}
			else if (PRI == mode)
			{
				buf[0] = 0;
				strcat(buf, to_name);
				send_msg(sockfd, &server_addr, append_msg(buf, nick_name, get_time(), msg));
			}
		}
		else if (1 == user_cmd) display(head, nick_name, mode, to_name, NULL, msg_num);
	}
	pthread_join(*thread, NULL);
	close(sockfd);
	return 0;
}

void *msg_thread(void * ta)
{
	ThreadArgs *t = (ThreadArgs *) ta;
	int numbytes;
	socklen_t sin_size;
	char buf[MAXLINE];
	char name[30];
	char tme[30];
	char ip[30];
	char tmsg[MAXLINE];
		
	/*
	 * send a "new client" message to server 
	 */
	buf[0] = 0;
	strcpy(buf,"new");
	send_msg(t->sockfd, t->server_addr_ptr, append_msg(buf, t->nick_name, get_time(),"Join"));
	//sleep(1);
	msg[0] = 0;
	display(t->head, t->nick_name, *t->mode, t->to_name, msg, *t->msg_num);
	while ( 1 )
	{
		numbytes = recv_msg(t->sockfd, t->server_addr_ptr, &sin_size, buf);
		if (0 == numbytes)
		{
			perror("no message received.");
		}
		else
		{	/*
			 * check nickName avaliable by message from server
			 */
			msg_prs(buf, ip, name, tme, tmsg);
			if (0 == strcmp("error", name))
			{
				if (-1 == system("clear"))
				{
					perror("system clear error.");
				}
				perror("\nNickName Already In Use, Try Another One.");
				exit(1);
			}
			/*
			 * receive online userlist from server
			 */
			else if (0 == strcmp("userlist", name))
			{
				buf[0] = 0;
				strcat(buf, "Online Users : ");
				strcat(buf, tmsg);
				strcpy(tmsg, buf);
				buf[0] = 0;
				strcat(buf, ip);
				append_msg(buf, "System", tme, tmsg);
			}
			/*
			 * client timeout check
			 */
			else if (0 == strcmp("timeout", name))
			{
				sprintf (buf, "online %s %s %s is online.",t->nick_name, get_time(), t->nick_name );
				send_msg(t->sockfd, t->server_addr_ptr, buf);
				continue;
			}
			msg_queue_ins(t->head, buf);
			display(t->head, t->nick_name, *t->mode, t->to_name, msg, *t->msg_num);
		}
	}
}

int cmd_prs(char *cmd, int sockfd, SAI* servadd, char *nickname, char *toname, int *mode, int *msg_num, MSG *rt)
{
	if (NULL == cmd) return 0;
	char *p = cmd;
	char tc[MAXLINE], *q = tc;
	while (*p && (' ' == *p)) ++p;
	/*
	 * user command mode
	 */
	if (*p && ('-' == *p))
	{
		if ('-' == *p)
		{
			/*
			 * get user command
			 */
			++p;
			while (*p && (' ' != *p)) *q++ = *p++;
			*q = 0;
			/*
			 * command -user or -u
			 * client query online users from server
			 */
			if (0 == strcmp("user", tc) || 0 == strcmp("u", tc))
			{
				printf("Begin to query user list.\n");
				sprintf(tc, "getuser %s %s user check", nickname, get_time());
				send_msg(sockfd, servadd, tc);
				return 2;
			} 
			/*
			 * command -all or -a
			 * set client chat mode to everyone
			 */
			else if (0 == strcmp("all", tc) || 0 == strcmp("a", tc))
			{
				*mode = ALL;
				return 1;
			}
			/*
			 * command -clearl or -c
			 * clear message queue
			 */
			else if (0 == strcmp("clear", tc) || 0 == strcmp("c", tc))
			{
				rt->next=NULL;
				return 1;
			}
			/*
			 * command -private user_nick_name or -p user_nick_name
			 * set client chat mode to private chat to user_nick_name
			 */
			else if (0 == strcmp("private", tc) || 0 == strcmp("p", tc))
			{
				*mode = PRI;
				while (*p && (' ' == *p)) ++p;
				q = toname;
				while (*p && (' ' != *p)) *q++ = *p++;
				*q = 0;
				return 1;
			}
			/*
			 * command -quit or -q
			 * client quit from server normally
			 */
			else if (0 == strcmp("quit", tc) || 0 == strcmp("q", tc))
			{
				sprintf(tc, "quit %s %s user check", nickname, get_time());
				send_msg(sockfd, servadd, tc);
				puts("");
				exit(0);
			}
			/*
			 * command -list num or -l num
			 * set client message list size to num
			 */
			else if (0 == strcmp("list", tc) || 0 == strcmp("l", tc))
			{
				while (*p && (' ' == *p)) ++p;
				q = tc;
				while (*p && (' ' != *p)) *q++ = *p++;
				*q = 0;
				int num = atoi(tc);
				*msg_num = num;
				return 1;
			}
			/*
			 * command -helpl or -h
			 * open help document for user
			 */
			else if (0 == strcmp("help", tc) || 0 == strcmp("h", tc))
			{
				if (-1 == system("gedit readme.txt"))
				{
					perror("system get readme.txt error.");
				}
				return 1;
			}
			q = tc;
		}
		
		return 1;
	}
	/*
	 * client send message to someone
	 */
	else if(*p && ('/' == *p))
	{
		++ p;
		q = tc;
		while (*p && (' ' != *p)) *q++ = *p++;
		*q = 0;
		send_msg(sockfd, servadd, append_msg(tc, nickname, get_time(), p));
		return 1;
	}
	else return 0;
}
