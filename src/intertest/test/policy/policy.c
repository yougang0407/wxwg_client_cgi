#include <stdio.h>

#include "AuthorizeServiceSoapBinding.nsmap"
#include "soapStub.h"

#include "libxml/parser.h"
#include "libxml/tree.h"
#include "libxml/xmlmemory.h"


int soap_call_ns1_judgePermission(char *requrl,char* idno, char* host_Port)
{
	char xmlstr[8192];
	char *xmlresp=NULL;
	int iRet;
	int nlen=0;
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlChar *key;
	FILE *fp;

	struct soap mysoap;

	soap_init(&mysoap);
	soap_set_mode(&mysoap, SOAP_C_UTFSTRING);
	mysoap.mode |= SOAP_C_UTFSTRING;


	
	snprintf(xmlstr,8192,"<Data><IdNo>%s</IdNo><Resource>%s</Resource></Data>",idno,host_Port);
	

	printf("\nsoap_call_ns1__judgePermission with %s %s\n",idno,host_Port);
	iRet=soap_call_ns1__judgePermission(&mysoap,requrl,NULL,xmlstr,&xmlresp);
	
	if(iRet) {
		printf("judgePermission iret=%d\n",iRet);
		return 2;
	}
	
	printf("return:%s\n\n",xmlresp);

	fp=fopen("/tmp/permission.out","a+");
	fprintf(fp,"call with %s %s\n",idno,host_Port);
	fwrite(xmlresp,1,strlen(xmlresp),fp);
	fprintf(fp,"\n\n");
	fclose(fp);

	nlen=strlen(xmlresp);
	doc = xmlParseMemory(xmlresp,nlen);
	if (!doc){
		printf("xml parse failed\n");
		goto cleanup;
	}

	cur = xmlDocGetRootElement(doc);
	if (!cur) {
		printf("root empty");
		goto cleanup;
	}

	key=xmlNodeListGetString(doc,cur->xmlChildrenNode,1);
	printf("judge:%s\n",key);
	if(0==strcmp((char *)key,"false"))
	{
		return 0;
	}
	else if(0==strcmp((char *)key,"true"))
	{
		return 1;
	}

cleanup:
	if (doc) xmlFreeDoc(doc);
	return 2;
}


int main(int argc,char *argv[])
{
	int iRet;
	int nlen=0;
	
	struct soap mysoap;
	char xmlstr[8192];
	char *xmlresp=NULL;
	char requrl[1024];

	if (argc!=7) {
		printf("Usage %s <ip> <port> <userno> <deviceid> <usernojudge> <resourcejudge>\n",argv[0]);
		return 1;
	}

	

	sprintf(requrl,"http://%s:%d/app/services/AuthorizeService",argv[1],atoi(argv[2]));
	printf("requesturl:%s\n",requrl);
	
	

	soap_init(&mysoap);
	soap_set_mode(&mysoap, SOAP_C_UTFSTRING);
	mysoap.mode |= SOAP_C_UTFSTRING;
	
	strcpy(xmlstr,"<Data><IdNo>");
	strcat(xmlstr,argv[3]);
	strcat(xmlstr,"</IdNo></Data>");
	
	printf("\nsoap_call_ns1__getPermissionsByUserId with %s\n",argv[3]);
	iRet=soap_call_ns1__getPermissionsByUserId(&mysoap,requrl,NULL,xmlstr,&xmlresp);
	if(iRet)
	{
		printf("getPermissionsByUserId iret=%d\n",iRet);
		return iRet;
	}
	
	printf("return:%s\n\n",xmlresp);
	
	strcpy(xmlstr,"<Data><DeviceId>");
	strcat(xmlstr,argv[4]);
	strcat(xmlstr,"</DeviceId></Data>");
	
	printf("\nsoap_call_ns1__getPermissionsByDeviceId with %s\n",argv[4]);
	iRet=soap_call_ns1__getPermissionsByDeviceId(&mysoap,requrl,NULL,xmlstr,&xmlresp);
	if(iRet)
	{
		printf("soap_call_ns1__getPermissionsByDeviceId iret=%d\n",iRet);
		return iRet;
	}
	printf("return:%s\n\n",xmlresp);


	iRet=soap_call_ns1_judgePermission(requrl,argv[5],argv[6]);


	soap_destroy(&mysoap);
	soap_end(&mysoap);
	soap_done(&mysoap);
	
	return iRet;
}
