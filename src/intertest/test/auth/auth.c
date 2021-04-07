#include <stdio.h>
#include "AuthServiceSoapBinding.nsmap"
#include "soapStub.h"
#include "GetSignature.h"


#include "libxml/parser.h"
#include "libxml/tree.h"
#include "libxml/xmlmemory.h"

int main(int argc,char *argv[])
{
	int iRet;
	int nlen=0;
	FILE *fp;
	struct soap mysoap;
	char *chanreturn;
	char requrl[1024];
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlChar *key;

	char *assert_sig_xml=NULL;
	unsigned int xmllen=0;

	char *assert_out=NULL;


	if (argc!=3) {
		printf("Usage %s <ip> <port>\n",argv[0]);
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
		return 1;
	}
	printf("return:%s\n\n",chanreturn);

	nlen=strlen(chanreturn);
	doc = xmlParseMemory(chanreturn,nlen);
	if (!doc){
		printf("xml parse failed\n");
		goto cleanup;
	}

	cur = xmlDocGetRootElement(doc);
	if (!cur) {
		printf("root empty");
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
	iRet=GetSignatureAndAssertXR(&assert_sig_xml,&xmllen,(const char *)key);
	

	if (iRet) {
		printf("GetSignatureAndAssertXR=%d\n");
		goto cleanup;
	}

	printf("return assertxml=%s\n\n",assert_sig_xml);
	

	printf("\ncall soap_call_ns1__AuthSecAssert\n");
	iRet=soap_call_ns1__AuthSecAssert(&mysoap,requrl,NULL,assert_sig_xml,&assert_out);
	printf("AuthSecAssert=%d\n",iRet);
	if (iRet) {
		goto cleanup;
	}

	printf("return:%s\n",assert_out);

	
cleanup:
	if (doc) xmlFreeDoc(doc);
	soap_destroy(&mysoap);
	soap_end(&mysoap);
	soap_done(&mysoap);
	return iRet;
}
