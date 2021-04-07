#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "AuthServiceSoapBinding.nsmap"
#include "soapStub.h"
#include "GetSignature.h"

#include "libxml/parser.h"
#include "libxml/tree.h"
#include "libxml/xmlmemory.h"

struct sec_assert_msg_hdr {
	unsigned short type;
	unsigned short len;
};

int sec_assert_msg_send(unsigned int gw_ip, unsigned int gw_port, char *msg, int len)
{
	int fd = -1;
	int n;
	struct sockaddr_in server;
	struct sec_assert_msg_hdr msg_hdr;
	
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		printf("socket create failed\n");
		return 1;
	}
	
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = gw_ip;
	server.sin_port = htons(gw_port);
	if (connect(fd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) < 0) {
		printf("socket connect failed\n");
		close(fd);
		return 1;
	}
	
	n = send(fd, msg, len, 0);
	if (n != len) {
		printf("send sec assert msg failed\n");
		close(fd);
		return 1;
	}
	
	n = recv(fd, &msg_hdr, sizeof(msg_hdr), 0);
	if (n != sizeof(msg_hdr)) {
		printf("recv sec assert result msg failed\n");
		close(fd);
		return 1;
	}
	
	if (ntohs(msg_hdr.type) != 3) {
		printf("recv sec assert result: failed\n");
		close(fd);
		return 1;
	}
	
	close(fd);
	return 0;
}

int main(int argc,char *argv[])
{
	int iRet = 1;
	int nlen=0;
	FILE *fp;
	struct soap mysoap;
	char *chanreturn;
	char requrl[1024];
	xmlDocPtr doc = NULL;
	xmlNodePtr cur = NULL;
	xmlChar *key = NULL;

	char *assert_sig_xml=NULL;
	unsigned int xmllen=0;

	char *assert_out=NULL;
	xmlChar *assert_out_code=NULL;

	unsigned int auth_ip;
	unsigned int auth_port;
	unsigned int gw_ip;
	unsigned int gw_port;
	//int fd = -1;
	//struct sockaddr_in server;
	//struct sockaddr_in client;
	//unsigned int addr_len = sizeof(struct sockaddr_in);
	//int n;
	int slen;
	char *sbuf = NULL;
	struct sec_assert_msg_hdr *msg_hdr = NULL;

	if (argc!=5) {
		printf("Usage: %s <auth_ip> <auth_port> <gw_ip> <gw_port>\n",argv[0]);
		return 1;
	}

	auth_ip = inet_addr(argv[1]);
	if (auth_ip == INADDR_NONE) {
		printf("Usage: %s <auth_ip> <auth_port> <gw_ip> <gw_port>\n",argv[0]);
		return 1;
	}
	auth_port = atoi(argv[2]);
	if (auth_port > 65535) {
		printf("Usage: %s <auth_ip> <auth_port> <gw_ip> <gw_port>\n",argv[0]);
		return 1;
	}
	
	gw_ip = inet_addr(argv[3]);
	if (gw_ip == INADDR_NONE) {
		printf("Usage: %s <auth_ip> <auth_port> <gw_ip> <gw_port>\n",argv[0]);
		return 1;
	}
	gw_port = atoi(argv[4]);
	if (gw_port > 65535) {
		printf("Usage: %s <auth_ip> <auth_port> <gw_ip> <gw_port>\n",argv[0]);
		return 1;
	}

	sprintf(requrl,"http://%s:%d/app/services/AuthService",argv[1],atoi(argv[2]));
	printf("requesturl:%s\n",requrl);

	soap_init(&mysoap);
	soap_set_mode(&mysoap, SOAP_C_UTFSTRING);
	mysoap.mode |= SOAP_C_UTFSTRING;


	printf("\ncall soap_call_ns1__Random\n");
	iRet=soap_call_ns1__Random(&mysoap,requrl,NULL,&chanreturn);
	
	if (iRet){
		printf("soap_call_ns1__Random=%d\n",iRet);
		goto cleanup;
	}
	printf("return:%s\n\n",chanreturn);

	nlen=strlen(chanreturn);
	doc = xmlParseMemory(chanreturn,nlen);
	if (!doc){
		printf("xml parse failed\n");
		iRet = 1;
		goto cleanup;
	}

	cur = xmlDocGetRootElement(doc);
	if (!cur) {
		printf("root empty\n");
		iRet = 1;
		goto cleanup;
	}
	printf("%s->",cur->name);

	cur = cur->xmlChildrenNode;
	printf("%s->",cur->name);
	cur = cur->xmlChildrenNode;
	printf("%s->",cur->name);
	cur = cur->next;
	printf("%s->",cur->name);
	key=xmlNodeListGetString(doc,cur->xmlChildrenNode,1);
	printf("\nrandom %s\n",key);

	printf("\ncall api:GetSignatureAndAssertXR\n");
	/*iRet=GetSignatureAndAssertXR(&assert_sig_xml,&xmllen,(const char *)key);	
	if (iRet) {
		printf("GetSignatureAndAssertXR=%d\n");
		goto cleanup;
	}*/
	
	char buf[2048];
	memset(buf, 0, sizeof(buf));
	fp = fopen("./UserInfo.xml", "r");
	if (!fp) {
		printf("GetSignatureAndAssertXR failed\n");
		goto cleanup;
	}
	fread(buf,sizeof(buf)-1,1,fp);
	fclose(fp);
	
	char buf2[2048];
	sprintf(buf2,"<AuthSecAssert><Random>%s</Random>", key);
	strcat(buf2,buf),
	strcat(buf2,"<Signature signerID=\"\" type=\"1\">64373837613035383333666465356438646537356534353763656261353766346564366638633764</Signature><Cert></Cert></AuthSecAssert>");
	
	assert_sig_xml = buf2;
	xmllen = strlen(buf2);
	printf("return assertxml=%s\n\n",assert_sig_xml);
	
	xmlFreeDoc(doc);
	doc = NULL;

	printf("\ncall soap_call_ns1__AuthSecAssert\n");
	iRet=soap_call_ns1__AuthSecAssert(&mysoap,requrl,NULL,assert_sig_xml,&assert_out);
	printf("AuthSecAssert=%d\n",iRet);
	if (iRet) {
		goto cleanup;
	}

	printf("return:%s\n",assert_out);
	
	nlen=strlen(assert_out);
	doc = xmlParseMemory(assert_out,nlen);
	if (!doc){
		printf("xml parse failed\n");
		iRet = 1;
		goto cleanup;
	}
	cur = xmlDocGetRootElement(doc);
	if (!cur) {
		printf("root empty\n");
		iRet = 1;
		goto cleanup;
	}
	cur = cur->xmlChildrenNode;
	if (!cur) {
		printf("children empty\n");
		iRet = 1;
		goto cleanup;
	}
	cur = cur->xmlChildrenNode;
	if (!cur) {
		printf("children empty\n");
		iRet = 1;
		goto cleanup;
	}
	if (xmlStrcmp(cur->name, BAD_CAST("ReturnInfo"))) {
		printf("get ReturnInfo node error\n");
		iRet = 1;
		goto cleanup;
	}
	assert_out_code = xmlGetProp(cur, BAD_CAST("code"));
	if (!assert_out_code || atoi((char*)assert_out_code)) {
		printf("sec assert auth failed, return code=%d\n", atoi((char*)assert_out_code));
		iRet = 1;
		goto cleanup;
	}

	slen = sizeof(struct sec_assert_msg_hdr) + xmllen;
	sbuf = (char*)malloc(slen);
	if (!sbuf) {
		printf("malloc send buf failed\n");
		iRet = 1;
		goto cleanup;
	}
	msg_hdr = (struct sec_assert_msg_hdr *)sbuf;
	msg_hdr->type = htons(1);
	msg_hdr->len = htons(slen);
	memcpy(msg_hdr+1, assert_sig_xml, xmllen);
	
	iRet = sec_assert_msg_send(gw_ip, gw_port, sbuf, slen);
	if (iRet) {
		printf("send sec assert add msg failed\n");
		goto cleanup;
	}
	
	printf("press any key to quit:");
	getchar();
	
	msg_hdr->type = htons(2);
	
	iRet = sec_assert_msg_send(gw_ip, gw_port, sbuf, slen);
	if (iRet) {
		printf("send sec assert del msg failed\n");
		goto cleanup;
	}	
	
	iRet = 0;
cleanup:
	if (doc) xmlFreeDoc(doc);
	soap_destroy(&mysoap);
	soap_end(&mysoap);
	soap_done(&mysoap);
	
	if(sbuf)
		free(sbuf);
	
	return iRet;
}
