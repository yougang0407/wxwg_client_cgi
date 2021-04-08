
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event-internal.h>

#include <web_server.h>
#include <httpd.h>
#include <log.h>


int wxwgc_web_server_fd = -1;


void wxwgc_web_server_read(int sockfd, short event, void *arg)
{
	wxwgc_web_client *client = (wxwgc_web_client *)arg;
	wxwgc_web_client_info wxwgc_web_client_msg;

	if (event & EV_TIMEOUT) {
		WWC_DEBUG("wxwgc web server read data from client(fd:%d) timeout\n", client->fd);
		goto exit;
	}

	int ret = http_request_handle_func(client);
	WWC_DEBUG("web server(fd:%d) recv msg from client(fd:%d). ret = %d \n", sockfd, client->fd, ret);

exit:
	close(client->fd);
	event_del(&client->ev);
	free(client);
	return;
}

void wxwgc_web_server_accept(int sockfd, short event, void *arg)
{
	struct sockaddr_in addr;
	unsigned int addr_len = sizeof(addr);
	int fd;
	int sockctl;
	wxwgc_web_client *client = NULL;
	struct timeval timeout;
	
	if ((fd = accept(wxwgc_web_server_fd, (struct sockaddr *)&addr, &addr_len)) < 0) {
		return;
	}

	sockctl = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, sockctl | O_NONBLOCK);

	client = (wxwgc_web_client *)malloc(sizeof(wxwgc_web_client));
	if (!client) {
		WWC_ERROR("wxwgc web client malloc failed\n");
		close(fd);
		return;
	}
	memset(client, 0, sizeof(wxwgc_web_client));
	client->fd = fd;
	memcpy(&client->addr, &addr, sizeof(client->addr));

	WWC_DEBUG("accept new client from %s fd=%d\n", inet_ntoa(addr.sin_addr), fd);

	timeout.tv_sec = 0;
	timeout.tv_usec = 50000;
	event_set(&client->ev, fd, EV_READ | EV_PERSIST, wxwgc_web_server_read, (void *)client);
	event_add(&client->ev, &timeout);

	return;
}

int wxwgc_web_server_start(const char* ip, int port)
{
	struct sockaddr_in local;
	int sockctl;
	struct event *ev = NULL;

	wxwgc_web_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (wxwgc_web_server_fd < 0) {
		WWC_ERROR("wxwgc web server socket create failed\n");
		return -1;
	}

	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = inet_addr(ip);
	//local.sin_addr.s_addr = INADDR_ANY;
	socklen_t len = sizeof(local);
	if (bind(wxwgc_web_server_fd, (struct sockaddr*)&local, len) < 0) {
		WWC_ERROR("wxwgc web server socket bind failed\n");
		return -1;
	}

	sockctl = fcntl(wxwgc_web_server_fd, F_GETFL, 0);
	fcntl(wxwgc_web_server_fd, F_SETFL, sockctl | O_NONBLOCK);

	if (listen(wxwgc_web_server_fd, 5) < 0) {
		WWC_ERROR("wxwgc web server socket listen failed\n");
		return -1;
	}

	ev = event_new(NULL, wxwgc_web_server_fd, EV_READ | EV_PERSIST, wxwgc_web_server_accept, NULL);
	if (!ev) {
		WWC_ERROR("wxwgc web server event create failed\n");
		return -1;
	}
	event_add(ev, NULL);

	return 0;
}

