#ifndef __HTTPD__
#define __HTTPD__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>

#define NORMAL 0
#define WARNING 1
#define FATAL 2

#define CGI_MODE_ENABLE 1
#define CGI_MODE_DISABLE 0

#define SIZE 1024
#define BUFF_SIZE 1024
#define METHOD_SIZE 8
#define URL_SIZE 512
#define PATH_SIZE 256
#define HTTP_GET_CONTENT_SIZE 256

void Usage(const char* proc);
void print_log(const char* log_msg, int level);
//int startup(const char* ip, int port);
//static int get_line(int sock, char* buf, int sz);
//void* accept_request(void* arg);
int http_request_handle_func(int sock);
void echo_errno();

#endif
