#ifndef __NACD_TCP_CLIENT_H__
#define __NACD_TCP_CLIENT_H__


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <event2/event.h>

#include <openssl/ssl.h>


#define INFO_SIZE 128


typedef struct user_info_t {
	char name[INFO_SIZE];
	char passwd[INFO_SIZE];
} user_info;

typedef struct nacd_config_msg_t {
	char nacd_server_ip[32];
	unsigned short nacd_server_port;
	unsigned short timeout;
	unsigned short ssl;
} nacd_config_msg;

typedef struct nacdsock_t {
	int sock;
	SSL *ssl;
	SSL_CTX *ctx;
} nacdsock;

typedef struct nacd_session_data_t {
	struct event ev;
	nacdsock *fd;
	struct sockaddr_in nacd_connec;
	char nacd_server_ip[32];
	unsigned short nacd_server_port;
	char user_name[INFO_SIZE];
	char user_passwd[INFO_SIZE];
	int state;
	int timeout;
	unsigned int use_ssl;
} nacd_session_data;


int nacd_tcp_client_handle_func(nacd_config_msg *nacd_cfg, user_info *user_msg);


#endif
