#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <log.h>

#include "./include/nacd_tcp_client.h"
#include "./include/nacd_types.h"


#define NACD_BUF 512


enum nacd_state_e {
	NACD_CONNECT,
	NACD_SEND_USER,
	NACD_SEND_PASS,
	NACD_QUIT,
	NACD_STATE_MAX,
};

static int SSL_UP = 0;

int nacd_error(char *string) {
	/* returns 1 if nacd server error reply (i.e : -ERR ...) */
	return (string ? (!strncmp(string, "-ERR", (size_t)4)) : 1);
}

int ssl_nacd_recv(nacdsock *fd, char *buf, int len) {
	return fd->ssl ? SSL_read(fd->ssl, buf, len) : recv(fd->sock, buf, len, 0);
}

int ssl_nacd_send(nacdsock *fd, char *buf, int len) {
	return fd->ssl ? SSL_write(fd->ssl, buf, len) : send(fd->sock, buf, len, 0);
}

int nacd_recv(nacdsock *fd, char *buf, int len) {
	return recv(fd->sock, buf, len, 0);
}

int nacd_send(nacdsock *fd, char *buf, int len) {
	return send(fd->sock, buf, len, 0);
}

char *nacd_connect(nacdsock *fd, struct sockaddr_in *connection)
{
	WWC_DEBUG("nacd connect\n");
	int ret = 0;
	int flags = 0;

	if (!fd->ssl) {
		flags = fcntl(fd->sock, F_GETFL, 0);
		fcntl(fd->sock, F_SETFL, flags | O_NONBLOCK);
	}
	ret = connect(fd->sock, (struct sockaddr *)connection, sizeof(struct sockaddr));
	if (!fd->ssl) {
		WWC_DEBUG("nacd no use ssl\n");
		if (ret == -1 && EINPROGRESS != errno) {
			close(fd->sock);
			return NULL;
		} else {
			return "OK";
		}
	} else {
		WWC_DEBUG("nacd use ssl\n");
		if (ret == -1) {
			close(fd->sock);
			return NULL;
		} else {
			WWC_DEBUG("before ssl handshark --- sock %d, ssl %p, ctx %p, ret %d\n", \
					   fd->sock, fd->ssl, fd->ctx, ret);
			if ((ret = SSL_connect(fd->ssl)) == -1) {
				WWC_ERROR("nacd SSL_connect error\n");
				return NULL;
			}
			WWC_DEBUG("after ssl handshark --- sock %d, ssl %p, ctx %p, ret %d\n", \
					   fd->sock, fd->ssl, fd->ctx, ret);
			return "OK";
		}
	}
}

static int nacd_send_message(nacd_session_data *session)
{
	WWC_DEBUG("send nacd message\n");
	int retval = NACD_SUCCESS;
	char *srvdata = NULL;
	char query[NACD_BUF] = {0,};

	switch (session->state) {
		case NACD_CONNECT:
			srvdata = nacd_connect(session->fd, &session->nacd_connec);
			WWC_DEBUG("SEND NACD_CONNECT, srvdata[%s]\n", srvdata);
			if (!srvdata) {
				retval = NACD_CONNECTION_SERVER;
				goto end;
			}
			break;

		case NACD_SEND_USER:
			snprintf(query, NACD_BUF, "USER %s\r\n", session->user_name);
			WWC_DEBUG("SEND NACD_USER[%s]\n", query);
			if (session->use_ssl) {
				retval = ssl_nacd_send(session->fd, (char *)query, strlen(query));
			} else {
				retval = nacd_send(session->fd, (char *)query, strlen(query));
			}
			if (retval == -1) {
				WWC_ERROR("nacd send user error\n");
				retval = NACD_SYSTEM_ERR;
				goto end;
			}
			break;
			
		case NACD_SEND_PASS:
			snprintf(query, NACD_BUF, "PASS %s\r\n", session->user_passwd);
			WWC_DEBUG("SEND NACD_PASS[%s]\n", query);
			if (session->use_ssl) {
				retval = ssl_nacd_send(session->fd, (char *)query, strlen(query));
			} else {
				retval = nacd_send(session->fd, (char *)query, strlen(query));
			}
			if (retval == -1) {
				WWC_ERROR("nacd send pass error\n");
				retval = NACD_SYSTEM_ERR;
				goto end;
			}
			break;
			
		case NACD_QUIT:
			snprintf(query, NACD_BUF, "QUIT\r\n");
			WWC_DEBUG("SEND NACD_QUIT[%s]\n", query);
			if (session->use_ssl) {
				retval = ssl_nacd_send(session->fd, (char *)query, strlen(query));
			} else {
				retval = nacd_send(session->fd, (char *)query, strlen(query));
			}
			if (retval == -1) {
				retval = NACD_SYSTEM_ERR;
				goto end;
			}
			break;
	}
	retval = NACD_SUCCESS;

end:
	return retval;
}

static int nacd_recv_message(nacd_session_data *session)
{
	WWC_DEBUG("recv nacd message\n");
	int retval;
	char buf[NACD_BUF] = {0,};

	if (session->use_ssl) {
		ssl_nacd_recv(session->fd, buf, NACD_BUF);
	} else {
		nacd_recv(session->fd, buf, NACD_BUF);
	}
	WWC_DEBUG("recv_nacd_message, buf[%s]\n", buf);
	switch (session->state) {
		case NACD_CONNECT:
			WWC_DEBUG("RECV NACD_CONNECT\n\n");
			if (nacd_error(buf)) {
				retval = NACD_USER_UNKNOWN;
				goto end;
			}
			session->state = NACD_SEND_USER;
			break;
			
		case NACD_SEND_USER:
			WWC_DEBUG("RECV NACD_SEND_USER\n\n");
			if (nacd_error(buf)) {
				retval = NACD_USER_UNKNOWN;
				goto end;
			}
			session->state = NACD_SEND_PASS;
			break;
			
		case NACD_SEND_PASS:
			WWC_DEBUG("RECV NACD_SEND_PASS\n\n");
			if (nacd_error(buf)) {
				retval = NACD_PASSWORD_ERROR;
				goto end;
			}
			session->state = NACD_QUIT;
			break;
			
		case NACD_QUIT:
			WWC_DEBUG("RECV NACD_QUIT\n\n");
			if (nacd_error(buf)) {
				retval = NACD_PASSWORD_ERROR;
				goto end;
			}
			session->state = NACD_STATE_MAX;
			break;
	}
	retval = NACD_SUCCESS;

end:
	return retval;
}

void nacd_msg_process(int fd, short ev, void *arg)
{
	int retval = NACD_SUCCESS;
	struct timeval tv;
	nacd_session_data *session = (nacd_session_data *)arg;

	if (ev & EV_TIMEOUT) {
		WWC_DEBUG("[EV_TIMEOUT]\n");
	} else if (ev & EV_READ) {
		WWC_DEBUG("[EV_READ]\n");
		retval = nacd_recv_message(session);
		if (retval != NACD_SUCCESS) {
			WWC_DEBUG("[NACD_AUTH_ERR]\n");
		} else {
			if (session->state == NACD_STATE_MAX) {
				WWC_DEBUG("[NACD_SUCCESS]\n");
			} else {
				WWC_DEBUG("[add EV_WRITE | EV_TIMEOUT]\n");
				event_set(&session->ev, session->fd->sock, EV_WRITE|EV_TIMEOUT, nacd_msg_process, (void *)session);
				tv.tv_sec = session->timeout;
				tv.tv_usec = 0;
				event_add(&session->ev, &tv);
			}
		}
	} else if (ev & EV_WRITE) {
		WWC_DEBUG("[EV_WRITE]\n");
		retval = nacd_send_message(session);
		if (retval != NACD_SUCCESS) {
			return;
		}

		WWC_DEBUG("[add EV_READ | EV_TIMEOUT]\n");
		event_set(&session->ev, session->fd->sock, EV_READ|EV_TIMEOUT, nacd_msg_process, (void *)session);
		tv.tv_sec = session->timeout;
		tv.tv_usec = 0;
		event_add(&session->ev, &tv);
	}
}

int nacd_open_session(nacd_session_data *session)
{
	WWC_DEBUG("open nacd session\n");
	struct timeval tv;
	int retval = NACD_SUCCESS;

	retval = nacd_send_message(session);
	if (retval != NACD_SUCCESS) {
		return retval;
	}

	event_set(&session->ev, session->fd->sock, EV_READ|EV_TIMEOUT, nacd_msg_process, (void *)session);
	tv.tv_sec = session->timeout;
	tv.tv_usec = 0;
	event_add(&session->ev, &tv);

	return NACD_INCOMPLETE;
}

nacdsock* ssl_prepare(unsigned short use_ssl)
{
	WWC_DEBUG("ssl prepare start..\n");
	nacdsock *fd;
	fd = (nacdsock *)malloc(sizeof(nacdsock));
	fd->sock = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == fd->sock) {
		WWC_ERROR("socket error\n");
		free(fd);
		return NULL;
	}

	if (use_ssl) {
		if (0 == SSL_UP) {
			SSL_UP = SSL_library_init();
		}
		if (1 != SSL_UP) {
			close(fd->sock);
			free(fd);
			WWC_ERROR("Error initializing SSL library\n");
			return NULL;
		}
		SSL_load_error_strings();
		fd->ctx = SSL_CTX_new(SSLv23_client_method());
		if (NULL == fd->ctx) {
			close(fd->sock);
			free(fd);
			WWC_ERROR("SSL v23 not available\n");
			return NULL;
		}
		fd->ssl = SSL_new(fd->ctx);
		if (NULL == fd->ssl) {
			close(fd->sock);
			SSL_CTX_free(fd->ctx);
			free(fd);
			WWC_ERROR("Error creating SSL context\n");
			return NULL;
		}
		SSL_set_fd(fd->ssl, fd->sock);
	} else {
		fd->ssl = NULL;
		fd->ctx = NULL;
	}

	return fd;
}


nacdsock* nacd_prepare(nacd_config_msg *nacd_cfg, struct sockaddr_in *connection)
{
	WWC_DEBUG("nacd prepare start..\n");
	nacdsock *fd;

	memset((char*)connection, 0, sizeof(struct sockaddr_in));

	connection->sin_family = AF_INET;
	connection->sin_addr.s_addr = inet_addr(nacd_cfg->nacd_server_ip);
	connection->sin_port = htons(nacd_cfg->nacd_server_port);

	if (nacd_cfg->ssl) {
		connection->sin_port = htons(nacd_cfg->nacd_server_port != 995 ? \
			(unsigned short int)nacd_cfg->nacd_server_port : (unsigned short int)995);
	} else {
		connection->sin_port = htons(nacd_cfg->nacd_server_port != 110 ? \
			(unsigned short int)nacd_cfg->nacd_server_port : (unsigned short int)110);
	}

	fd = ssl_prepare(nacd_cfg->ssl);
	if (NULL == fd) {
		WWC_ERROR("nacd tcp client sock error\n");
	}

	return fd;
}

int nacd_tcp_client_handle_func(nacd_config_msg *nacd_cfg, user_info *user_msg)
{
	int retval = NACD_SUCCESS;
	nacd_session_data *session;
	struct sockaddr_in nacd_connec;

	session = (nacd_session_data *)malloc(sizeof(*session));
	memset(session, 0, sizeof(*session));
	if (NULL == session) {
		WWC_ERROR("init session failure\n");
		retval = NACD_BUF_ERR;
		goto end;
	}
	session->fd = nacd_prepare(nacd_cfg, &nacd_connec);
	if (NULL == session->fd) {
		WWC_ERROR("network wrong.. please check network..\n");
		retval = NACD_SYSTEM_ERR;
		goto end;
	}
	session->timeout = nacd_cfg->timeout;
	session->state = NACD_CONNECT;
	session->use_ssl = nacd_cfg->ssl;
	memcpy(&session->nacd_connec, &nacd_connec, sizeof(nacd_connec));
	strncpy(session->nacd_server_ip, nacd_cfg->nacd_server_ip, sizeof(session->nacd_server_ip));
	session->nacd_server_port = nacd_cfg->nacd_server_port;
	strncpy(session->user_name, user_msg->name, sizeof(session->user_name));
	strncpy(session->user_passwd, user_msg->passwd, sizeof(session->user_passwd));
	retval = nacd_open_session(session);

end:
	return retval;
}
