#include <unistd.h>
#include "httpd.h"
#include "./include/log.h"

//初始化
int web_http_server_init(const char* ip, int port)
{
	assert(ip);

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		print_log("socket failed!  ",FATAL);
		return 1;
	}

	int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = inet_addr(ip);
	socklen_t len = sizeof(local);

	if (bind(sock, (struct sockaddr*)&local, len) < 0) {
		print_log("bind_failed!  ",FATAL);
		return 2;
	}

	if (listen(sock,5) < 0) {
		print_log("listen failed!  ",FATAL);
		return 3;
	}
	return sock;
}

void Usage(const char* proc)
{
	WWC_DEBUG("%s [local_ip] [local_port]", proc);
}

void* accept_request(void* arg)
{
	int sock = *(int* )arg;
	pthread_detach(pthread_self());
	int ret = http_request_handle_func(sock);
	WWC_DEBUG("ret = %d \n", ret);
	return NULL;
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		Usage(argv[0]);
	}
	char local_ip[32] = {"\0"};
	int local_port = 8080;
	snprintf(local_ip, sizeof(local_ip), "%s", argv[1]);
	local_port = atoi(argv[2]);
	int listen_sock = web_http_server_init(local_ip, local_port);

	while (1) {
		struct sockaddr_in client;
		socklen_t len = sizeof(client);
		int sock = accept(listen_sock, (struct sockaddr* )&client, &len);
		if (sock < 0) {
			print_log("accept failed!", FATAL);
			continue;
		}

		pthread_t tid;
		pthread_create(&tid, NULL, accept_request, &sock);
		usleep(50000);
	}
	close(listen_sock);
	return 0;
}
