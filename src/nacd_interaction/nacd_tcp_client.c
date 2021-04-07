#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<assert.h>
#include<string.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#include "../include/log.h"

int nacd_tcp_client()
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(sockfd != -1);
	struct sockaddr_in ser, cli;
	memset(&ser, 0, sizeof(ser));
	ser.sin_family = AF_INET;
	ser.sin_port = htons(8859);
	ser.sin_addr.s_addr = inet_addr("0.0.0.0");
	int res = connect(sockfd, (struct sockaddr*)&ser, sizeof(ser));
	assert(res != -1);
	while (1) {
		WWC_DEBUG("please input msg_type(请输入1/2。1为添加断言; 2是删除断言):");
		fflush(stdout);
		char buff[128]={0};
		fgets(buff,128,stdin);
		buff[strlen(buff)-1]=0;
		if (strcmp(buff,"1")&&strcmp(buff, "2")) {
			WWC_DEBUG("error!!! exit!!!\n");
			break;
		}
		send(sockfd, buff, strlen(buff), 0);
		char recvbuff[128] = {0};
		recv(sockfd, recvbuff, 127, 0);
		WWC_DEBUG("%s\n", recvbuff);
	}
	close(sockfd);
}
