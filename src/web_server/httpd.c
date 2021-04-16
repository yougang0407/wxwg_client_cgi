#include <httpd.h>
#include <log.h>
#include "../intertest/test/user/user.h"
#include "../nacd_interaction/include/nacd_tcp_client.h"

#define DEBUG

//打印错误信息
void print_log(const char* log_msg, int level)
{
	char* err_list[5] = {
		"NORMAL",
		"WARNING",
		"FATAL",
	};

//#ifdef DEBUG
//#define DEBUG
	char* log_path = "/var/wxwg_client/log/file.log";
	int fd = open(log_path, O_WRONLY|O_APPEND|O_CREAT, 0644);

	char buf[BUFF_SIZE];
	memset(buf, 0, sizeof(buf));
	strncpy(buf, err_list[level], strlen(err_list[level]));	//错误等级和提示
	strncat(buf, ": ", strlen(": "));
	strncat(buf, log_msg, strlen(log_msg));	//错误信息
	time_t t;
	char ti[30];
	ti[0] = ' ';
	time(&t);
	strcpy(ti+1, ctime(&t));
	strcat(buf, ti);
	write(fd, buf, strlen(buf));
	close(fd);
//#endif
}

//按行读取内容
static int get_line(int sock, char * buf, int len)
{
	assert(buf);
	char ch='\0';
	int i = 0;
	while (i < len -1 && ch != '\n' ) {
		if (recv(sock, &ch, 1, 0) > 0) {
			if (ch == '\r') {
				//考虑下一个字符是否为\n
				if ( recv(sock, &ch, 1, MSG_PEEK) > 0 && ch == '\n' )
					recv(sock, &ch, 1, 0);
				else
					ch = '\n';
			}
			buf[i++] = ch;
		}
	}
	buf[i] = '\0';
	return i;
}

static int clear_header(int sock)
{
	char line[BUFF_SIZE];
	int ret = -1;
	do {
		ret = get_line(sock, line, BUFF_SIZE);
	} while (ret != 1 && strcmp(line, "\n") != 0);

	return ret;
}

static int http_response_func(int sock, const char* path, ssize_t size)
{
	int ret = 0;
	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		print_log("open failed!  ",FATAL);
		echo_errno(sock,503);
		ret = 8;
	}

	char line[BUFF_SIZE];
	snprintf(line, sizeof(line), "HTTP/1.1 200 OK\r\n");
	send(sock, line, strlen(line), 0);
	send(sock, "\r\n", 2, 0);
	if (sendfile(sock, fd, NULL, size) < 0) {
		print_log("sendfile is failed!  ", FATAL);
		echo_errno(sock,503);
		ret = 9;
	}
	close(fd);
	return ret;
}

//server应答错误页面
void bad_request(const char* path, const char* head, int sock)
{
	struct stat st;
	if (stat(path, &st) < 0) {
		return;
	}
	int fd = open(path, O_RDONLY);

	//响应报头+空行
	const char* status_line = head;
	send(sock,status_line,strlen(status_line),0);
	const char* content_type = "Content-Type:text/html;charset=ISO-8859-1\r\n";
	send(sock,content_type,strlen(content_type),0);
	send(sock, "\r\n", 2, 0);

	sendfile(sock, fd, NULL,st.st_size);
	close(fd);
}

void echo_errno(int sock, int err_code)
{
	switch (err_code) {
		case 404:
			bad_request("webui/404.html", "HTTP 404 Not Found\r\n", sock);
			break;
		case 503:
			bad_request("webui/503.html","HTTP 503 Server Unavailable\r\n",sock);
			break;
		default:
			break;
	}
}

static int cgi_exec_func(int sock, const char* method, char* path, const char* query_string, ssize_t size)
{
	int ret = 0;
	int content_len = -1;

	if (strcasecmp(method, "GET") == 0) {
		ret = clear_header(sock);
		ret = http_response_func(sock, path, size);
	} else if (strcasecmp(method, "POST") == 0) {
		//POST方法需要读取参数，而读多少字节能读完是依靠Content-Length所提示的大小知道的
		char line[BUFF_SIZE];
		do {
			ret = get_line(sock, line, sizeof(line));
			WWC_DEBUG("ret = %d \n",ret);

			//获得请求消息的大小
			if (ret > 0 && strncasecmp(line, "Content-Length: ", 16) == 0) {
				content_len = atoi(line + 16);	//取得content-length的大小
			}
		} while (ret != 1 && strcmp(line, "\n") != 0);

		if (content_len < 0) {
			print_log("have no arguments!\n",FATAL);
			echo_errno(sock,404);
			return 10;
		}
	} else {
		return -1;
	}

	//响应报头+空行
	const char* status_line = "HTTP/1.1 200 OK\r\n";
	send(sock, status_line, strlen(status_line), 0);
	const char* content_type = "Content-Type:text/html;charset=ISO-8859-1\r\n";
	send(sock, content_type, strlen(content_type), 0);
	send(sock, "\r\n", 2, 0);

#if 0
	//创建管道
	int input[2];
	int output[2];
	pipe(input);
	pipe(output);

	//fork子进程
	pid_t id;
	id = fork();
	if (id == 0) {	//child
		//1.关闭相应文件描述符
		//2.对文件描述符进行重定向
		//3.通过环境变量传递参数(method,path,query_string,content_len)
		//4.进行exec程序替换
		close(input[1]);
		close(output[0]);
		//close(sock);
		dup2(input[0],0);//将标准输入重定向到子进程
		dup2(output[1],1);//将子进程的输出重定向到标准输出---->浏览器

		char method_env[BUFF_SIZE/8];
		char query_string_env[BUFF_SIZE/4];
		char content_len_env[BUFF_SIZE/8];
		sprintf(method_env, "METHOD=%s", method);
		putenv(method_env);
		//GET:传递query_string     POST：传递content_len
		if (strcasecmp(method, "GET") == 0) {
			sprintf(query_string_env, "QUERY-STRING=%s", query_string);
			putenv(query_string_env);
		} else {
			sprintf(content_len_env, "CONTENT-LENGTH=%d", content_len);
			putenv(content_len_env);
		}

		execl(path, path, NULL);
		exit(1);
	} else {	//father
		//1.关闭适当的文件描述符
		//2.方法决定读写顺序(POST：需要继续读取数据，直到读完POST方法的参数;GET：可以直接从子进程读取结果)
		//3.将数据和方法全部交给子进程后等待子进程的结果

		close(input[0]);
		close(output[1]);

		//POST
		char c = '\0';
		if (strcasecmp(method, "POST") == 0) {
			int i = 0;
			//一个一个字符读取并通过管道传递给子进程
			for (; i<content_len; i++) {
				recv(sock, &c, 1, 0);//先sock获取数据
				write(input[1],&c, 1);//通过管道传给子进程
			}
		}

		//GET
		while (read(output[0], &c, 1) > 0) {
			send(sock, &c, 1, 0);
		}

		close(input[1]);
		close(output[0]);
		waitpid(id, NULL, 0);
	}
#endif
}

int http_request_handle_func(wxwgc_web_client *client)
{
	int ret = 0;
	int flag = 0;

	if (get_line(client->fd, client->buf, sizeof(client->buf)) <= 0) {
		print_log("get_line error!  ", FATAL);
		WWC_ERROR("get_line error!\n");
		echo_errno(client->fd, 404);
		ret = 5;
		goto end;
	}
	WWC_DEBUG("get_line buf: %s \n", client->buf);

	char *token = NULL;
	token = strtok(client->buf, " ");
	char method[METHOD_SIZE];
	snprintf(method, sizeof(method), "%s", token);
	WWC_DEBUG("get_line method: %s \n", method);

	if (strcasecmp(method, "GET") && strcasecmp(method, "POST")) {
		echo_errno(client->fd, 404);
		ret = 6;
		goto end;
	}

	int cgi_mode_flag = CGI_MODE_DISABLE;
	if (strcasecmp(method, "POST") == 0) {
		cgi_mode_flag = CGI_MODE_ENABLE;
	}

	token = strtok(NULL, " ");
	char url[URL_SIZE];
	snprintf(url, sizeof(url), "%s", token);
	WWC_DEBUG("get_line url: %s \n", url);

	token = strtok(url, "?");
	char path[PATH_SIZE];
	snprintf(path, sizeof(path), "%s", token);
	WWC_DEBUG("get_line path: %s \n", path);

	char http_get_content[CONTENT_SIZE] = {0,};
	user_info *user_info_ptr = (user_info *)malloc(sizeof(user_info));
	nacd_config_msg *nacd_cfg_ptr = (nacd_config_msg *)malloc(sizeof(nacd_config_msg));;
	snprintf(nacd_cfg_ptr->nacd_server_ip, sizeof(nacd_cfg_ptr->nacd_server_ip), "0.0.0.0");
	nacd_cfg_ptr->nacd_server_port = 8859;
	nacd_cfg_ptr->timeout = 1;
	nacd_cfg_ptr->use_ssl = 0;
	sec_assert_msg *sec_assert_msg_ptr = (sec_assert_msg *)malloc(sizeof(sec_assert_msg));;

	if (strcasecmp("GET", method) == 0) {
		token = strtok(NULL, "?");
		snprintf(http_get_content, sizeof(http_get_content), "%s", token);
		WWC_DEBUG("get_line http_get_content: %s \n", http_get_content);

		if (strstr(http_get_content, "&") != NULL) {
			token = strtok(http_get_content, "&");
			snprintf(user_info_ptr->user_name, sizeof(user_info_ptr->user_name), "%s", token);
			token = strtok(NULL, "&");
			snprintf(user_info_ptr->user_passwd, sizeof(user_info_ptr->user_passwd), "%s", token);
			
			token = strtok(user_info_ptr->user_name, "=");
			token = strtok(NULL, "=");
			snprintf(user_info_ptr->user_name, sizeof(user_info_ptr->user_name), "%s", token);
			WWC_DEBUG("get_line user_info_ptr->user_name: %s \n", user_info_ptr->user_name);
			
			token = strtok(user_info_ptr->user_passwd, "=");
			token = strtok(NULL, "=");
			snprintf(user_info_ptr->user_passwd, sizeof(user_info_ptr->user_passwd), "%s", token);
			WWC_DEBUG("get_line user_info_ptr->user_passwd: %s \n", user_info_ptr->user_passwd);
		}

		// to do authenticate the third ID manage system
		char id_manage_server_ip[16] = "172.30.202.81";
		int id_manage_server_port = 8086;
		void* res_user_info = NULL;
		int retval = 0;

		nac_user *nac_user_ptr = NULL;
		nac_user_ptr = get_user_attr_by_name(user_info_ptr->user_name);
		if (nac_user_ptr != NULL) {
			WWC_DEBUG("username[%s] get_user_attr_by_name success.\n", \
					  user_info_ptr->user_name);
			// to do get random and create SecAssert ...
			// send sec assert to nacd
			retval = nacd_handle_sec_assert_func(nacd_cfg_ptr, sec_assert_msg_ptr, \
												 NACD_ADD_SEC_ASSERT);
			if (!retval) {
				WWC_ERROR("username: %s  nacd_add_sec_assert_func error[%d] \n", \
						  user_info_ptr->user_name, retval);
			}
			flag = 1;
		} else {
			retval = nacd_handle_user_passwd_func(nacd_cfg_ptr, user_info_ptr, \
												  NACD_USER_PASSWD_AUTH);
			if (!retval) {
				WWC_ERROR("username: %s  nacd_tcp_client_handle_func error[%d] \n", \
						  user_info_ptr->user_name, retval);
			}
			flag = 1;
		}
		if (strstr(url, "?") != NULL)
			cgi_mode_flag = CGI_MODE_ENABLE;
	}


	char http_response_path[PATH_SIZE];

	if (strcasecmp("/", path) == 0) {
		snprintf(http_response_path, sizeof(http_response_path), \
				 "webui/index.html");
	} else if (strcasecmp("/cgi_bin/cgi_select", path) == 0) {
		#ifndef DEBUG
		if (strcasecmp("admin", user_info_ptr->user_name) == 0 \
			&& strcasecmp("123456", user_info_ptr->user_passwd) == 0) {
			snprintf(http_response_path, sizeof(http_response_path), \
					 "webui/login_success.html");
		} else {
			snprintf(http_response_path, sizeof(http_response_path), \
					 "webui/login_failed.html");
		}
		#endif
		if (flag) {
			snprintf(http_response_path, sizeof(http_response_path), \
					 "webui/login_success.html");
		} else {
			snprintf(http_response_path, sizeof(http_response_path), \
					 "webui/login_failed.html");
			int retval = nacd_handle_sec_assert_func(nacd_cfg_ptr, sec_assert_msg_ptr, NACD_DEL_SEC_ASSERT);
			if (!retval) {
				WWC_ERROR("username: %s  nacd_add_sec_assert_func error[%d] \n", \
						  user_info_ptr->user_name, retval);
			}
		}
	}

	//检查webui资源是否存在
	struct stat st;
	if (stat(http_response_path, &st) < 0) {
		print_log("file is not exist!  ",WARNING);
		clear_header(client->fd);
		echo_errno(client->fd, 404);
		ret = 7;
		goto end;
	} else {
		if (S_ISREG(st.st_mode)) {
			if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH)) {
				cgi_mode_flag = CGI_MODE_ENABLE;
			}
		} else if (S_ISDIR(st.st_mode)) {
			snprintf(http_response_path, sizeof(http_response_path), \
					 "webui/index.html");
		}
	}

	WWC_DEBUG("http_response_path: %s \n", http_response_path);
	if(cgi_mode_flag == CGI_MODE_ENABLE) {
		ret = cgi_exec_func(client->fd, method, http_response_path, http_get_content, st.st_size);
	} else {
		ret = clear_header(client->fd);
		ret = http_response_func(client->fd, http_response_path, st.st_size);
	}

end:
	close(client->fd);
	free(user_info_ptr);
	free(nacd_cfg_ptr);
	free(sec_assert_msg_ptr);
	return ret;
}
