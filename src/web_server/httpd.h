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

#include "web_server.h"

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
#define CONTENT_SIZE 256
#define INFO_SIZE 128


void Usage(const char* proc);
void print_log(const char* log_msg, int level);
int http_request_handle_func(wxwgc_web_client *client);
void echo_errno();

#endif
