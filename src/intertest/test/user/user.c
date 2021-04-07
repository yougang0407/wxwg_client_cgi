#include <stdio.h>
#include "../../../include/log.h"
#if 0
#include "RNServiceSoapBinding.nsmap"
#include "soapStub.h"

#include "libxml/parser.h"
#include "libxml/tree.h"
#include "libxml/xmlmemory.h"
#endif

int get_user_identity_info(char* user_name, char* id_manage_server_ip, \
						   int id_manage_server_port, void* res_user_info)
{
	WWC_DEBUG("=== okay === \n");
	return 0;
}

#if 0
int main(int argc, char *argv[])
{
	int iRet;
	int nlen = 0;
	struct soap mysoap;
	char *chanllenge = "";
	char *chanreturn = NULL;
	
	xmlDocPtr doc = NULL;
	xmlNodePtr cur;
	xmlChar *key = NULL;
	char xmlstr[8192];
	char *userlist;

	char requrl[1024];

	if (argc!=3) {
		printf("Usage %s <ip> <port>\n", argv[0]);
		return 1;
	}

	sprintf(requrl, "http://%s:%d/app/services/RNService", argv[1], atoi(argv[2]));
	printf("requesturl:%s\n", requrl);

	soap_init(&mysoap);
	soap_set_mode(&mysoap, SOAP_C_UTFSTRING);
	mysoap.mode |= SOAP_C_UTFSTRING;

	printf("\ncall soap_call_ns2__Challenge_\n");
	iRet=soap_call_ns1__Challenge(&mysoap, requrl, NULL, chanllenge, &chanreturn);
	/*if (iRet){
		printf("soap_call_ns2__Challenge_=%d\n",iRet);
		return 2;
	}*/

	printf("return:%s\n\n", chanreturn);

	if (chanreturn) {
		nlen=strlen(chanreturn);
		doc = xmlParseMemory(chanreturn, nlen);
		if (!doc) {
			printf("xml parse failed\n");
			goto cleanup;
		}

		cur = xmlDocGetRootElement(doc);
		if (!cur) {
			printf("root empty");
			goto cleanup;
		}

		cur = cur->xmlChildrenNode;
		cur = cur->xmlChildrenNode;
		cur = cur->next;
		key=xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		printf("random:%s\n", key);
	}

	strcpy(xmlstr, "<UserListGet><SearchInfo startNum=\"0\" maxNum=\"10\" challenge=\"");
	if (key) {
		strcat(xmlstr, (char *)key);
	}
	strcat(xmlstr, "\"></SearchInfo><Signature signerID=\"legendsec\" type=\"0\" algorithm=\"MD5\">12345678901234567890123456789012</Signature></UserListGet>");

	printf("\ncall soap_call_ns1__UserListGet\n");
	iRet= soap_call_ns1__UserListGet(&mysoap, requrl, NULL, xmlstr, &userlist);
	if (iRet) {
		printf("getuserlist iRet=%d\n", iRet);
		goto cleanup;
	}

	printf("return:%s\n", userlist);

cleanup:
	if (doc) xmlFreeDoc(doc);
	soap_destroy(&mysoap);
	soap_end(&mysoap);
	soap_done(&mysoap);
	return iRet;
}
#endif
