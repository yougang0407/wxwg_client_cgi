#include <stdio.h>
#include "../../../include/log.h"
#include "user.h"

#if 0
#include "RNServiceSoapBinding.nsmap"
#include "soapStub.h"

#include "libxml/parser.h"
#include "libxml/tree.h"
#include "libxml/xmlmemory.h"
#endif


int main(int argc, char *argv[])
{
	WWC_DEBUG("=== test_userd okay === \n");

#if 0
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
		WWC_DEBUG("Usage %s <ip> <port>\n", argv[0]);
		return 1;
	}

	sWWC_DEBUG(requrl, "http://%s:%d/app/services/RNService", argv[1], atoi(argv[2]));
	WWC_DEBUG("requesturl:%s\n", requrl);

	soap_init(&mysoap);
	soap_set_mode(&mysoap, SOAP_C_UTFSTRING);
	mysoap.mode |= SOAP_C_UTFSTRING;

	WWC_DEBUG("\ncall soap_call_ns2__Challenge_\n");
	iRet=soap_call_ns1__Challenge(&mysoap, requrl, NULL, chanllenge, &chanreturn);
	/*if (iRet){
		WWC_DEBUG("soap_call_ns2__Challenge_=%d\n",iRet);
		return 2;
	}*/

	WWC_DEBUG("return:%s\n\n", chanreturn);

	if (chanreturn) {
		nlen=strlen(chanreturn);
		doc = xmlParseMemory(chanreturn, nlen);
		if (!doc) {
			WWC_DEBUG("xml parse failed\n");
			goto cleanup;
		}

		cur = xmlDocGetRootElement(doc);
		if (!cur) {
			WWC_DEBUG("root empty");
			goto cleanup;
		}

		cur = cur->xmlChildrenNode;
		cur = cur->xmlChildrenNode;
		cur = cur->next;
		key=xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		WWC_DEBUG("random:%s\n", key);
	}

	strcpy(xmlstr, "<UserListGet><SearchInfo startNum=\"0\" maxNum=\"10\" challenge=\"");
	if (key) {
		strcat(xmlstr, (char *)key);
	}
	strcat(xmlstr, "\"></SearchInfo><Signature signerID=\"legendsec\" type=\"0\" algorithm=\"MD5\">12345678901234567890123456789012</Signature></UserListGet>");

	WWC_DEBUG("\ncall soap_call_ns1__UserListGet\n");
	iRet= soap_call_ns1__UserListGet(&mysoap, requrl, NULL, xmlstr, &userlist);
	if (iRet) {
		WWC_DEBUG("getuserlist iRet=%d\n", iRet);
		goto cleanup;
	}

	WWC_DEBUG("return:%s\n", userlist);

cleanup:
	if (doc) xmlFreeDoc(doc);
	soap_destroy(&mysoap);
	soap_end(&mysoap);
	soap_done(&mysoap);
	return iRet;

#endif
}

