#ifndef __WEB_SERVER_H__
#define __WEB_SERVER_H__

#include <event2/event_struct.h>

#define HTTP_GET_USER_NAME_SIZE 128
#define HTTP_GET_USER_PASSWD_SIZE 256


typedef struct wxwgc_web_client_t {
	int fd;
	struct event ev;
	struct sockaddr_in addr;
	int rlen;
	char buf[2048];
} wxwgc_web_client;

typedef struct wxwgc_web_client_info_t {
	char user_name[HTTP_GET_USER_NAME_SIZE];
	char user_passwd[HTTP_GET_USER_PASSWD_SIZE];
} wxwgc_web_client_info;


void wxwgc_web_server_read(int sockfd, short event, void *arg);
void wxwgc_web_server_accept(int sockfd, short event, void *arg);
int wxwgc_web_server_start(const char* ip, int port);

#endif
