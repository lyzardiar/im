#ifndef __CLIENTTOOL_H__
#define __CLIENTTOOL_H__

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define ALL 1
#define PRI 2
#define GETUSER 4
#define SETALL 8
#define SETPRI 16
#define MAXLINE 1024
#define MYPORT 8888 

typedef struct sockaddr SA;
typedef struct sockaddr_in SAI;
/*
 * message queue
 */
typedef struct MSG{
	char nick_name[30];
	char ip[16];
	char time[30];
	char msg[MAXLINE];
	struct MSG *next;
}MSG;
/*
 * get user input without input cache
 */
int getch();
/*
 * init socket of server
 * return 0 for success
 * return -1 for error
 */
int server_init(int *sfd, SAI *serv, char *hostip);
/*
 * append args string nick_name, time, msg with token " " follow to string cmd
 * return pointer to cmd
 */
char *append_msg(char *cmd, char *nick_name, char *time, char *msg);
/*
 * decoration for message send
 * return 0 for success
 * return -1 for error
 */
int send_msg(int fd, SAI* to, char *msg);
/*
 * decoration for message receive
 * return 0 for error
 * return size of message for success
 */
int recv_msg(int fd, SAI* from, socklen_t *sin_size, char *msg);
/*
 * init message queue
 * return head of queue for success
 * return NULL for error
 */
MSG* msg_queue_init();
/*
 * insert message msg to message queue
 * return 0 for success
 * return -1 for error
 */
int msg_queue_ins(MSG* rt, char *msg);
/*
 * display latest message from message queue for user with length num
 */
void msg_display(MSG *rt, int deep, int num);
/*
 * read args string ip, nick_name, time, user_msg with token " " from string msg
 */
void msg_prs(char *msg, char *ip, char *nick_name, char *time, char *user_msg);
/*
 * display client user inerface 
 */
void display(MSG *rt, char *nick_name, int mode, char *to_name, char *msg, int num);
/*
 * return current time string with format hh:mm 
 */
char *get_time();

#endif
