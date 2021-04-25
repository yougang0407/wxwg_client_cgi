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
#include "./soap/nac_soap.h"

#define NACD_BUF	512

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
	char send_buf[NACD_BUF] = {0,};
	user_info_ptr_hdr *user_info_ptr_data_hdr = &(session->user_info_data.user_info_hdr);

	switch (session->state) {
		case NACD_CONNECT:
			srvdata = nacd_connect(session->fd, &session->nacd_connec);
			WWC_DEBUG("SEND NACD_CONNECT, srvdata[%s]\n", srvdata);
			if (!srvdata) {
				retval = NACD_CONNECTION_SERVER;
				goto end;
			}
			break;

		case NACD_SEND_USER_PASSWD:
			WWC_DEBUG("SEND NACD_USER_PASSWD\n");
			user_info_ptr_data_hdr->type = NACD_USER_PASSWD_AUTH;
			user_info_ptr_data_hdr->len = sizeof(session->user_info_data);
			WWC_DEBUG("SEND NACD_USER_PASSWD user_info_ptr_data_hdr->len[%d]\n", user_info_ptr_data_hdr->len);
			user_info_ptr_data_hdr->type = htons(user_info_ptr_data_hdr->type);
			user_info_ptr_data_hdr->len = htons(user_info_ptr_data_hdr->len);
			WWC_DEBUG("SEND NACD_USER_PASSWD user_info_ptr_data_hdr->len[%d]\n", user_info_ptr_data_hdr->len);
			WWC_DEBUG("session->nacd_config_msg_data.use_ssl[%d]\n", session->nacd_config_msg_data.use_ssl);
			if (session->nacd_config_msg_data.use_ssl) {
				retval = ssl_nacd_send(session->fd, (char *)(user_info_ptr_data_hdr), \
						 sizeof(session->user_info_data));
			} else {
				retval = nacd_send(session->fd, (char *)(user_info_ptr_data_hdr), \
						 sizeof(session->user_info_data));
			}
			if (retval == -1) {
				WWC_ERROR("nacd send user and passwd error\n");
				retval = NACD_SYSTEM_ERR;
				goto end;
			}
			break;

		case NACD_ADD_SEC_ASSERT:
			WWC_DEBUG("ADD NACD_SEC_ASSERT\n");
			session->sec_assert_info_data.user_info_hdr.type = NACD_SEC_ASSERT_AAD;
			session->sec_assert_info_data.user_info_hdr.len = sizeof(session->sec_assert_info_data);
			WWC_DEBUG("SEND NACD_ADD_SEC_ASSERT session->sec_assert_info_data.user_info_hdr.len[%d]\n", session->sec_assert_info_data.user_info_hdr.len);
			session->sec_assert_info_data.user_info_hdr.type = htons(session->sec_assert_info_data.user_info_hdr.type);
			session->sec_assert_info_data.user_info_hdr.len = htons(session->sec_assert_info_data.user_info_hdr.len);
			WWC_DEBUG("SEND NACD_ADD_SEC_ASSERT session->sec_assert_info_data.user_info_hdr.len[%d]\n", session->sec_assert_info_data.user_info_hdr.len);
			WWC_DEBUG("session->nacd_config_msg_data.use_ssl[%d]\n", session->nacd_config_msg_data.use_ssl);
			if (session->nacd_config_msg_data.use_ssl) {
				retval = ssl_nacd_send(session->fd, (char *)(&session->sec_assert_info_data.user_info_hdr), \
						 sizeof(session->sec_assert_info_data));
			} else {
				retval = nacd_send(session->fd, (char *)(&session->sec_assert_info_data.user_info_hdr), \
						 sizeof(session->sec_assert_info_data));
			}
			if (retval == -1) {
				WWC_ERROR("nacd add sec assert error\n");
				retval = NACD_SYSTEM_ERR;
				goto end;
			}
			break;

		case NACD_DEL_SEC_ASSERT:
			WWC_DEBUG("DEL NACD_SEC_ASSERT\n");
			session->sec_assert_info_data.user_info_hdr.type = NACD_SEC_ASSERT_DEL;
			session->sec_assert_info_data.user_info_hdr.len = sizeof(session->sec_assert_info_data);
			WWC_DEBUG("SEND NACD_DEL_SEC_ASSERT session->sec_assert_info_data.user_info_hdr.len[%d]\n", session->sec_assert_info_data.user_info_hdr.len);
			session->sec_assert_info_data.user_info_hdr.type = htons(session->sec_assert_info_data.user_info_hdr.type);
			session->sec_assert_info_data.user_info_hdr.len = htons(session->sec_assert_info_data.user_info_hdr.len);
			WWC_DEBUG("SEND NACD_DEL_SEC_ASSERT session->sec_assert_info_data.user_info_hdr.len[%d]\n", session->sec_assert_info_data.user_info_hdr.len);
			WWC_DEBUG("session->nacd_config_msg_data.use_ssl[%d]\n", session->nacd_config_msg_data.use_ssl);
			if (session->nacd_config_msg_data.use_ssl) {
				retval = ssl_nacd_send(session->fd, (char *)(&session->sec_assert_info_data.user_info_hdr), \
						 sizeof(session->sec_assert_info_data));
			} else {
				retval = nacd_send(session->fd, (char *)(&session->sec_assert_info_data.user_info_hdr), \
						 sizeof(session->sec_assert_info_data));
			}
			if (retval == -1) {
				WWC_ERROR("nacd del sec assert error\n");
				retval = NACD_SYSTEM_ERR;
				goto end;
			}
			break;

		case NACD_QUIT:
			snprintf(send_buf, NACD_BUF, "QUIT\r\n");
			WWC_DEBUG("SEND NACD_QUIT[%s]\n", send_buf);
			if (session->nacd_config_msg_data.use_ssl) {
				retval = ssl_nacd_send(session->fd, (char *)send_buf, strlen(send_buf));
			} else {
				retval = nacd_send(session->fd, (char *)send_buf, strlen(send_buf));
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
	int retval = -1;
	char buf[NACD_BUF] = {0,};

	WWC_DEBUG("recv_nacd_message, use_ssl[%d]\n", session->nacd_config_msg_data.use_ssl);
	if (session->nacd_config_msg_data.use_ssl) {
		ssl_nacd_recv(session->fd, buf, NACD_BUF);
	} else {
		nacd_recv(session->fd, buf, NACD_BUF);
	}

	user_info_ptr_hdr *user_info_ptr_data_hdr = NULL;
	switch (session->state) {
		case NACD_CONNECT:
			WWC_DEBUG("RECV NACD_CONNECT\n\n");
			if (nacd_error(buf)) {
				retval = NACD_CONN_ERR;
				goto end;
			}
			session->state = NACD_SEND_USER_PASSWD;
			break;

		case NACD_SEND_USER_PASSWD:
			WWC_DEBUG("RECV NACD_SEND_USER_PASSWD\n\n");
			if (nacd_error(buf)) {
				retval = NACD_USER_PASSWD_AUTH_ERR;
				goto end;
			}
			user_info_ptr_data_hdr = (user_info_ptr_hdr *)buf;
			user_info_ptr_data_hdr->type = ntohs(user_info_ptr_data_hdr->type);
			WWC_DEBUG("user_info_ptr_data_hdr->type[%d]\n", user_info_ptr_data_hdr->type);
			if (user_info_ptr_data_hdr->type == NACD_USER_PASSWD_AUTH_SUCCESS) {
				WWC_DEBUG("NACD_USER_PASSWD_AUTH_SUCCESS\n");
			} else if (user_info_ptr_data_hdr->type == NACD_USER_PASSWD_AUTH_FAILED) {
				WWC_DEBUG("NACD_USER_PASSWD_AUTH_FAILED\n");
			}
			session->state = NACD_QUIT;
			break;

		case NACD_ADD_SEC_ASSERT:
			WWC_DEBUG("RECV NACD_ADD_SEC_ASSERT\n\n");
			if (nacd_error(buf)) {
				retval = NACD_SEC_ASSERT_ADD_ERR;
				goto end;
			}
			user_info_ptr_data_hdr = (user_info_ptr_hdr *)buf;
			user_info_ptr_data_hdr->type = ntohs(user_info_ptr_data_hdr->type);
			WWC_DEBUG("user_info_ptr_data_hdr->type[%d]\n", user_info_ptr_data_hdr->type);
			if (user_info_ptr_data_hdr->type == NACD_SEC_ASSERT_AAD_SUCCESS) {
				WWC_DEBUG("NACD_SEC_ASSERT_AAD_SUCCESS\n");
			} else if (user_info_ptr_data_hdr->type == NACD_SEC_ASSERT_AAD_FAILED) {
				WWC_DEBUG("NACD_SEC_ASSERT_AAD_FAILED\n");
			}
			session->state = NACD_QUIT;
			break;

		case NACD_DEL_SEC_ASSERT:
			WWC_DEBUG("RECV NACD_DEL_SEC_ASSERT\n\n");
			if (nacd_error(buf)) {
				retval = NACD_SEC_ASSERT_DEL_ERR;
				goto end;
			}
			user_info_ptr_data_hdr = (user_info_ptr_hdr *)buf;
			user_info_ptr_data_hdr->type = ntohs(user_info_ptr_data_hdr->type);
			WWC_DEBUG("user_info_ptr_data_hdr->type[%d]\n", user_info_ptr_data_hdr->type);
			if (user_info_ptr_data_hdr->type == NACD_SEC_ASSERT_DEL_SUCCESS) {
				WWC_DEBUG("NACD_SEC_ASSERT_DEL_SUCCESS\n");
			} else if (user_info_ptr_data_hdr->type == NACD_SEC_ASSERT_DEL_FAILED) {
				WWC_DEBUG("NACD_SEC_ASSERT_DEL_FAILED\n");
			}
			session->state = NACD_QUIT;
			break;

		case NACD_QUIT:
			WWC_DEBUG("RECV NACD_QUIT\n\n");
			if (nacd_error(buf)) {
				retval = NACD_DATA_ERR;
				goto end;
			}
			session->state = NACD_STATE_MAX;
			break;
	}
	retval = NACD_SUCCESS;

end:
	return retval;
}

void session_drop(nacd_session_data * session)
{
	_session_drop(session);
}

int nacd_close_session(nacd_session_data *session, int result)
{
	session_drop(session);
	return result;
}

void nacd_msg_process(int fd, short ev, void *arg)
{
	int retval = NACD_SUCCESS;
	struct timeval tv;
	nacd_session_data *session = (nacd_session_data *)arg;

	if (ev & EV_TIMEOUT) {
		WWC_DEBUG("[EV_TIMEOUT]\n");
		nacd_close_session(session, NACD_EV_TIMEOUT);
	} else if (ev & EV_READ) {
		WWC_DEBUG("[EV_READ]\n");
		retval = nacd_recv_message(session);
		WWC_DEBUG("session->state[%d] retval[%d]\n", session->state, retval);
		if (retval != NACD_SUCCESS) {
			WWC_DEBUG("[NACD_EV_READ_ERR]\n");
			nacd_close_session(session, NACD_EV_READ_ERR);
		} else {
			if (session->state == NACD_STATE_MAX) {
				WWC_DEBUG("[NACD_SUCCESS]\n");
				nacd_close_session(session, NACD_SUCCESS);
			} else {
				WWC_DEBUG("[add EV_WRITE | EV_TIMEOUT]\n");
				event_set(&session->ev, session->fd->sock, EV_WRITE|EV_TIMEOUT, \
						  nacd_msg_process, (void *)session);
				tv.tv_sec = session->nacd_config_msg_data.timeout;
				tv.tv_usec = 0;
				event_add(&session->ev, &tv);
			}
		}
	} else if (ev & EV_WRITE) {
		WWC_DEBUG("[EV_WRITE]\n");
		retval = nacd_send_message(session);
		WWC_DEBUG("session->state[%d] retval[%d]\n", session->state, retval);
		if (retval != NACD_SUCCESS) {
			WWC_DEBUG("[NACD_EV_WRITE_ERR]\n");
			nacd_close_session(session, NACD_EV_WRITE_ERR);
		}

		WWC_DEBUG("[add EV_READ | EV_TIMEOUT]\n");
		event_set(&session->ev, session->fd->sock, EV_READ|EV_TIMEOUT, \
				  nacd_msg_process, (void *)session);
		tv.tv_sec = session->nacd_config_msg_data.timeout;
		tv.tv_usec = 0;
		event_add(&session->ev, &tv);
	}
}

int nacd_open_session(nacd_session_data *session)
{
	WWC_DEBUG("open nacd session\n");
	int retval = NACD_SUCCESS;
	session->state = NACD_CONNECT;
	retval = nacd_send_message(session);
	if (retval != NACD_SUCCESS) {
		WWC_ERROR("open nacd session failed. retval[%d]\n", retval);
		return retval;
	}

	return NACD_SUCCESS;
}

int nacd_process_task_session(nacd_session_data *session, int nacd_state)
{
	WWC_DEBUG("process nacd task session\n");
	struct timeval tv;
	int retval = NACD_SUCCESS;

	session->state = nacd_state;
	retval = nacd_send_message(session);
	if (retval != NACD_SUCCESS) {
		return retval;
	}

	event_set(&session->ev, session->fd->sock, EV_READ|EV_TIMEOUT, nacd_msg_process, \
			  (void *)session);
	tv.tv_sec = session->nacd_config_msg_data.timeout;
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


nacdsock* nacd_prepare(nacd_config_msg *nacd_cfg_ptr, struct sockaddr_in *connection)
{
	WWC_DEBUG("nacd prepare start..\n");
	nacdsock *fd;

	memset((char*)connection, 0, sizeof(struct sockaddr_in));

	connection->sin_family = AF_INET;
	connection->sin_addr.s_addr = inet_addr(nacd_cfg_ptr->nacd_server_ip);
	connection->sin_port = htons(nacd_cfg_ptr->nacd_server_port);

	if (nacd_cfg_ptr->use_ssl) {
		connection->sin_port = htons(nacd_cfg_ptr->nacd_server_port != 995 ? \
			(unsigned short int)nacd_cfg_ptr->nacd_server_port : (unsigned short int)995);
	} else {
		connection->sin_port = htons(nacd_cfg_ptr->nacd_server_port != 110 ? \
			(unsigned short int)nacd_cfg_ptr->nacd_server_port : (unsigned short int)110);
	}

	fd = ssl_prepare(nacd_cfg_ptr->use_ssl);
	if (NULL == fd) {
		WWC_ERROR("nacd tcp client sock error\n");
	}

	return fd;
}

int nacd_handle_user_passwd_func(nacd_config_msg *nacd_cfg_ptr, user_info *user_info_ptr, int nacd_state)
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
	session->fd = nacd_prepare(nacd_cfg_ptr, &nacd_connec);
	if (NULL == session->fd) {
		WWC_ERROR("network wrong.. please check network..\n");
		retval = NACD_SYSTEM_ERR;
		goto end;
	}
	session->nacd_config_msg_data.timeout = nacd_cfg_ptr->timeout;
	session->nacd_config_msg_data.use_ssl = nacd_cfg_ptr->use_ssl;
	memcpy(&session->nacd_connec, &nacd_connec, sizeof(nacd_connec));
	strncpy(session->nacd_config_msg_data.nacd_server_ip, nacd_cfg_ptr->nacd_server_ip, \
			sizeof(session->nacd_config_msg_data.nacd_server_ip));
	session->nacd_config_msg_data.nacd_server_port = nacd_cfg_ptr->nacd_server_port;
	strncpy(session->user_info_data.user_name, user_info_ptr->user_name, \
			sizeof(session->user_info_data.user_name));
	strncpy(session->user_info_data.user_passwd, user_info_ptr->user_passwd, \
			sizeof(session->user_info_data.user_passwd));
	nacd_open_session(session);
	nacd_process_task_session(session, nacd_state);

end:
	return retval;
}

int nacd_handle_sec_assert_func(nacd_config_msg *nacd_cfg_ptr, char *auth_sec_assert_msg, int nacd_state)
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
	session->fd = nacd_prepare(nacd_cfg_ptr, &nacd_connec);
	if (NULL == session->fd) {
		WWC_ERROR("network wrong.. please check network..\n");
		retval = NACD_SYSTEM_ERR;
		goto end;
	}
	session->nacd_config_msg_data.timeout = nacd_cfg_ptr->timeout;
	session->nacd_config_msg_data.use_ssl = nacd_cfg_ptr->use_ssl;
	memcpy(&session->nacd_connec, &nacd_connec, sizeof(nacd_connec));
	strncpy(session->nacd_config_msg_data.nacd_server_ip, nacd_cfg_ptr->nacd_server_ip, \
			sizeof(session->nacd_config_msg_data.nacd_server_ip));
	session->nacd_config_msg_data.nacd_server_port = nacd_cfg_ptr->nacd_server_port;
	strncpy(session->sec_assert_info_data.auth_sec_assert_msg, auth_sec_assert_msg, \
			sizeof(session->sec_assert_info_data.auth_sec_assert_msg));

	nacd_open_session(session);
	nacd_process_task_session(session, nacd_state);

end:
	return retval;
}

nac_user *get_user_attr_by_name(char *username, char *g_websoap_user_url)
{
	int ret = 0;
	nac_user *user = NULL;

	char tmpreq[1024];
	char *tmpvalue = NULL;

	struct soap mysoap;
	char save_challenge[64];

	XMLDOMDocument *doc 			 = NULL;
	XMLDOMNode	   *root			 = NULL;
	XMLDOMNode	   *result_node 	 = NULL;

	XMLDOMNode *chan_node = NULL;

	XMLDOMNode	   *userlist_node	 = NULL;
	XMLDOMNode	   *user_node		 = NULL;
	XMLDOMNode	   *property_node	 = NULL;

	struct ns1__challenge soap_chan_req;
	struct ns1__challengeResponse soap_chan_resp;
	struct ns1__userListGet soap_user_req;
	struct ns1__userListGetResponse soap_user_resp;

	if (!g_websoap_user_url[0]) {
		WWC_DEBUG("soap user url not set\n");
		return NULL;
	}

	soap_init(&mysoap);
	soap_set_mode(&mysoap, SOAP_C_UTFSTRING);
	mysoap.mode |= SOAP_C_UTFSTRING;

	// 1. get challenge
	snprintf(tmpreq,1024,"<?xml version=\"1.0\" encoding=\"UTF-8\"?><Chanllenge><EncCert></EncCert></Chanllenge>");
	soap_chan_req.arg0 = tmpreq;

	ret = soap_call___ns1__challenge(&mysoap, g_websoap_user_url, NULL, &soap_chan_req, \
									 &soap_chan_resp);
	if (ret) {
		WWC_DEBUG("call soap_call___ns1__challenge failed, ret = %d\n", ret);
		goto cleanup;
	}

	doc = sg_dom_xmldom_create();
	if(NULL == doc){
		WWC_DEBUG("sg_dom_xmldom_create failed\n");
		goto cleanup;
	}

	if (!doc->loadXML(doc, soap_chan_resp.return_)) {
		WWC_DEBUG("UserListGet: parse challenge resp, xml parse failed\n");
		goto cleanup;
	}

	if ((root = sg_dom_rootdocnode(doc)) == NULL) {
		WWC_DEBUG("UserListGet: challenge resp xml get root node failed\n");
		goto cleanup;
	}

	result_node = sg_dom_get_xmlnode(root, "Result");
	if (!result_node) {
		WWC_DEBUG("UserListGet: challenge resp xml no Result node\n");
		goto cleanup;
	}

	chan_node = sg_dom_get_xmlnode(result_node, "Challenge");
	if (!result_node) {
		WWC_DEBUG("UserListGet: challenge resp xml no Challenge node\n");
		goto cleanup;
	}

	tmpvalue=sg_get_node_val(chan_node, ".");

	strncpy(save_challenge, tmpvalue, sizeof(save_challenge));

	sg_dom_xmldom_release(doc);
	doc = NULL;

	// 2. get user
	WWC_DEBUG("get user %s\n", username);
	snprintf(tmpreq,1024,"<?xml version=\"1.0\" encoding=\"UTF-8\"?><UserListGet><SearchInfo startNum=\"1\" maxNum=\"1\" challenge=\"%s\">"
		"<Item><Condition name=\"登录名\" relation=\"等于\" value=\"%s\" /></Item></SearchInfo>"
		"<Signature signerID=\"legendsec\" type=\"0\" algorithm=\"MD5\">12345678901234567890123456789012</Signature></UserListGet>",
		save_challenge, username);

	/*snprintf(tmpreq,1024,"<?xml version=\"1.0\" encoding=\"UTF-8\"?><UserListGet><SearchInfo startNum=\"1\" maxNum=\"1\" challenge=\"%s\">"
		"<Item><Condition name=\"姓名\" relation=\"等于\" value=\"%s\" /></Item></SearchInfo>"
		"<Signature signerID=\"legendsec\" type=\"0\" algorithm=\"MD5\">12345678901234567890123456789012</Signature></UserListGet>",
		save_challenge,username);*/

	soap_user_req.arg0 = tmpreq;
	ret = soap_call___ns1__userListGet(&mysoap, g_websoap_user_url, NULL, &soap_user_req, \
									   &soap_user_resp);
	if (ret) {
		WWC_DEBUG("call soap_call___ns1__userListGet failed, ret=%d\n", ret);
		goto cleanup;
	}

	WWC_DEBUG("%s\n",soap_user_resp.return_);

	doc = sg_dom_xmldom_create();
	if (NULL == doc) {
		goto cleanup;
	}
	if (!doc->loadXML(doc, soap_user_resp.return_)) {
		WWC_DEBUG("UserListGet resp xml parse failed\n");
		goto cleanup;
	}
	if ((root = sg_dom_rootdocnode(doc)) == NULL) {
		WWC_DEBUG("UserListGet resp xml get root node failed\n");
		goto cleanup;
	}

	result_node = sg_dom_get_xmlnode(root, "Result");
	if (!result_node) {
		WWC_DEBUG("UserListGet resp xml no Result node\n");
		goto cleanup;
	}
	userlist_node = sg_dom_get_xmlnode(result_node, "UserList");
	if (!userlist_node) {
		WWC_DEBUG("UserListGet resp xml no UserList node\n");
		goto cleanup;
	}

	user_node = sg_dom_get_xmlnode(userlist_node, "User");
	if (!user_node) {
		WWC_DEBUG("UserListGet resp xml no user node\n");
		goto cleanup;
	}

	user = malloc(sizeof(*user));
	if (!user) {
		WWC_DEBUG("malloc user memory failed\n");
		goto cleanup;
	}
	memset(user, 0, sizeof(*user));

	property_node = sg_dom_firstchild(user_node);
	while (property_node) {
		char *name, *value;

		name = sg_get_node_val(property_node, "name");
		value = sg_get_node_val(property_node, "value");

		if (!strcmp(name, "登录名")) {
			strncpy(user->login_name, value, sizeof(user->login_name));
		} else if (!strcmp(name, "姓名")) {
			strncpy(user->name, value, sizeof(user->name));
		} else if (!strcmp(name, "身份证号")) {
			strncpy(user->id_card_num, value, sizeof(user->id_card_num));
		} else if (!strcmp(name, "民族")) {
			strncpy(user->ethnic, value, sizeof(user->ethnic));
		} else if (!strcmp(name, "单位")) {
			strncpy(user->unit, value, sizeof(user->unit));
		} else if (!strcmp(name, "单位全称")) {
			strncpy(user->unit_name, value, sizeof(user->unit_name));
		} else if (!strcmp(name, "职务")) {
			strncpy(user->position, value, sizeof(user->position));
		} else if (!strcmp(name, "性别")) {
			strncpy(user->gender, value, sizeof(user->gender));
		} else if (!strcmp(name, "军队证件号")) {
			strncpy(user->identity_no, value, sizeof(user->identity_no));
		} else if (!strcmp(name, "保障卡号")) {
			strncpy(user->soldier_security_no, value, sizeof(user->soldier_security_no));
		} else if (!strcmp(name, "出生时间")) {
			strncpy(user->birthday, value, sizeof(user->birthday));
		} else if (!strcmp(name, "联系方式")) {
			strncpy(user->contact_way, value, sizeof(user->contact_way));
		} else if (!strcmp(name, "血型")) {
			strncpy(user->blood_type, value, sizeof(user->blood_type));
		} else if (!strcmp(name, "职务层级")) {
			strncpy(user->rank, value, sizeof(user->rank));
		} else if (!strcmp(name, "军衔")) {
			strncpy(user->militray_rank, value, sizeof(user->militray_rank));
		} else if (!strcmp(name, "军兵种")) {
			strncpy(user->army_service, value, sizeof(user->army_service));
		}

		property_node = sg_dom_nextsibling(property_node);
	}

cleanup:
	if (doc != NULL) {
		sg_dom_xmldom_release(doc);
		doc = NULL;
	}

	soap_destroy(&mysoap);
	soap_end(&mysoap);
	soap_done(&mysoap);

	return user;
}

int get_random_func(char *ip, int port, char *res_random, int res_random_len)
{
	int ret = 0;
	struct soap mysoap;
	char requrl[1024];

	XMLDOMDocument *doc              = NULL;
	XMLDOMNode     *root             = NULL;
	XMLDOMNode *resultnode = NULL;
	XMLDOMNode *rndnode = NULL;

	struct ns1__random soap_random_req;
	struct ns1__randomResponse soap_random_resp;

	char *tmpvalue = NULL;

	snprintf(requrl, sizeof(requrl), "http://%s:%d/wserver_war/app/services/AuthService", \
			 ip, port);
	WWC_DEBUG("request url:%s\n", requrl);

	soap_init(&mysoap);
	soap_set_mode(&mysoap, SOAP_C_UTFSTRING);
	mysoap.mode |= SOAP_C_UTFSTRING;

	WWC_DEBUG("call soap_call___ns1__random\n");
	ret = soap_call___ns1__random(&mysoap, requrl, NULL, &soap_random_req, &soap_random_resp);
	WWC_DEBUG("soap_call___ns1__random return %d\n", ret);
	if (ret) {
		goto cleanup;
	}

	WWC_DEBUG("%s\n", soap_random_resp.return_);

	doc = sg_dom_xmldom_create();
	if (!doc->loadXML(doc, soap_random_resp.return_)) {
		WWC_DEBUG("xml failed\n");
		goto cleanup;
	}

	root = sg_dom_rootdocnode(doc);
	WWC_DEBUG("root = %p\n", root);

	resultnode = sg_dom_get_xmlnode(root, "Result");
	WWC_DEBUG("Result = %p\n", resultnode);
	rndnode = sg_dom_get_xmlnode(resultnode, "Random");
	WWC_DEBUG("rndnode = %p\n", rndnode);
	tmpvalue = sg_get_node_val(rndnode, ".");

	strncpy(res_random, tmpvalue, res_random_len);

	WWC_DEBUG("res_random = %s\n", res_random);

	sg_dom_xmldom_release(doc);

cleanup:
	soap_destroy(&mysoap);
	soap_end(&mysoap);
	soap_done(&mysoap);
	return ret;
}

char auth_sec_assert_msg[2048];

void get_auth_sec_assert_msg_func(nac_user *user, char *res_random)
{
	char sec_assert_msg[1024] = {0,};
	char *client_ip = "192.168.5.212";
	int userAuthType = 1;
	snprintf(sec_assert_msg, 1024, \
			 "<SecAssert ID=\"a3ef32bc@RNS1.NB\"> "
				"<Info timestamp=\"20141031 120000\" "
				"		validityFrom=\"20141031 120015\" "
				"		validityTo=\"20141031 200015\">"
				"	<UserInfo userAuthID=\"CN=wangliang,OU=cdjq,O=MILCA\" "
				"		userAuthType=\"%d\" authSecLevel=\"3\" userIP=\"%s\">"
				"		<UserID>wangliang@RNS1.NB</UserID>"
				"		<Property name=\"登录名\" value=\"%s\"/>"
				"		<Property name=\"身份证号\" value=\"%s\"/>"
				"		<Property name=\"姓名\" value=\"%s\"/>"
				"		<Property name=\"单位\" value=\"%s\"/>"
				"	</UserInfo>"
				"</Info>"
				"<Signature signerID=\"CN=rsrf001,OU=secdev,O=MILCA\" "
				"			type=\"1\" algorithm=\"SM2-SHA\">xxxxxxxxx "
				"</Signature>"
			 "</SecAssert>", userAuthType, client_ip, user->login_name, user->id_card_num, \
			 user->name, user->unit);
	WWC_DEBUG("sec_assert_msg: %s\n\n", sec_assert_msg);
	//char *auth_sec_assert_msg = (char *)malloc(2048);
	snprintf(auth_sec_assert_msg, 2048, \
			 "<AuthSecAssert> "
			 "	<Random>%s</Random> "
			 "	%s "
			 "	<Signature signerId=\"xxx\"  type=\"1\">xxxxxx</Signature> "
			 "	<SignerCert>xxxx</SignerCert> "
			 "</AuthSecAssert> ", res_random, sec_assert_msg);
	WWC_DEBUG("auth_sec_assert_msg: %s\n", auth_sec_assert_msg);

	return;
}

typedef void (*timer_handle)(int fd, short event, void *arg);

int timer_is_work(struct event *timer)
{
#if 00
	int ret = 0;

	ret = event_pending(timer, EV_TIMEOUT, NULL);

	return ret & EV_TIMEOUT;
#endif
	return 1;
}

void nacd_close_timer(struct event *timer)
{
	if (timer_is_work(timer)) {
		evtimer_del(timer);
	}
}

void nacd_set_timer(struct event *timer,
	unsigned long expires, timer_handle func, void *arg)
{
	struct timeval time;

	time.tv_sec = 0;
	time.tv_usec = expires;

	evtimer_set(timer, func, arg);
	evtimer_add(timer, &time);
}

void nacd_reset_timer(struct event *timer,
	unsigned long expires, timer_handle func, void *arg)
{
	nacd_close_timer(timer);
	nacd_set_timer(timer, expires, func, arg);
}

struct event nacd_breath_out;
#define NACD_SYNC_BREATH_TIME		(500 * 1000)
#define NACD_SYNC_BREATH_TIMEOUT	(3 * NACD_SYNC_BREATH_TIME)

void nacd_Heartbeat_timeout_func(int fd, short event, void *arg)
{
	WWC_DEBUG("nacd_Heartbeat_timeout...\n");
	// to do ...
	#if 0
	int res = nacd_handle_sec_assert_func(nacd_cfg_ptr, auth_sec_assert_msg, \
											 NACD_DEL_SEC_ASSERT);
	if (res) {
		WWC_ERROR("username: %s  nacd_add_sec_assert_func error[%d] \n", \
				  user_info_ptr->user_name, res);
	}
	#endif

	nacd_reset_timer(&nacd_breath_out,
		NACD_SYNC_BREATH_TIMEOUT, nacd_Heartbeat_timeout_func, NULL);
}

void nacd_set_Heartbeat_timer(void)
{
	nacd_set_timer(&nacd_breath_out,
		NACD_SYNC_BREATH_TIMEOUT, nacd_Heartbeat_timeout_func, NULL);
}


