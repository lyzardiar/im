#ifndef __SERVTOOL_H__
#define __SERVTOOL_H__

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define MAXLINE 1024
#define MYPORT 8888 
#define BACKLOG 10
#define ONLINE 1
#define OFFLINE 0
#define saifd(x) (((SAI *)(x))->sin_addr.s_addr)

typedef struct sockaddr SA;
typedef struct sockaddr_in SAI;

/*
 * element of client queue 
 */
typedef struct Client {
	SA sock_addr;
	char nick_name[30];
	int status;
}Client;

/*
 * client queue
 */
typedef struct ClientList{
	Client client;
	int syn;
	struct ClientList * next;	
	struct ClientList * pre;
}ClientList;

/*
 * Init link
 */
int im_init(int *fd, SAI *sa);

/*
 * Init list
 * Success	: return the head of list
 * False	: return NULL
 */
ClientList * client_init();

/*
 * Insert element into list
 * arg 1:the head of list, arg2:the inserted element
 * Success	: return 0
 * False	: return -1
 */
int client_ins (ClientList* rt, Client *c);

/*
 * Delete element from list
 * arg 1:the head of list, arg2:the deleted element
 * Success	: return 0
 * False	: return -1
 */
int clinet_del (ClientList* rt, Client *c);

/*
 * Find element in list
 * arg 1:the head of list, arg2:the finding element
 * Success	: return pointer of element in list
 * False	: return NULL
 */
ClientList * client_find (ClientList* rt, Client *c);

/*
 * return current time string with format hh:mm 
 */
char *get_time();

/*
 * read args string ip, nick_name, time, user_msg with token " " from string msg
 */
void msg_prs(char *msg, char *ip, char *nick_name, char *time, char *user_msg);

/*
 * append args string ip, nick_name, time, user_msg with token " " follow to string msg
 * return pointer to msg
 */
char * append(char *msg, char *ip, char *nick_name, char *time, char *user_msg);

#endif