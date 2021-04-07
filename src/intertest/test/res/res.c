#include <stdio.h>

#include "RmsWebServiceSoapBinding.nsmap"
#include "soapStub.h"

int main(int argc,char *argv[])
{
	int iRet;
	struct soap mysoap;
	char xmlstr[8192];
	char *devicelist;
	char *softlist;
	char *datalist;
	char requrl[1024];

	if (argc!=3) {
		printf("Usage %s <ip> <port>\n",argv[0]);
		return 1;
	}

	

	sprintf(requrl,"http://%s:%d/app/services/RmsWebService",argv[1],atoi(argv[2]));
	printf("requesturl:%s\n",requrl);


	strcpy(xmlstr,"<?xml version='1.0' encoding='UTF-8'?><DeviceListGet><SearchInfo startNum=\"0\" maxNum=\"10\"></SearchInfo><Signature signerID=\"legendsec\" type=\"0\" algorithm=\"MD5\">741e05a117850988ac58f121674fbc7c</Signature></DeviceListGet>");



	soap_init(&mysoap);
	soap_set_mode(&mysoap, SOAP_C_UTFSTRING);
	mysoap.mode |= SOAP_C_UTFSTRING;

	printf("\ncall soap_call_ns1__DeviceListGet\n");
	iRet=soap_call_ns1__DeviceListGet(&mysoap,requrl,NULL,xmlstr,&devicelist);
	if (iRet){
		printf("getdevicelist iRet=%d\n",iRet);
		return 2;
	}
	
	printf("return:%s\n\n",devicelist);
	
	
	strcpy(xmlstr,"<?xml version='1.0' encoding='UTF-8'?><SoftListGet><SearchInfo startNum=\"0\" maxNum=\"10\"></SearchInfo><Signature signerID=\"legendsec\" type=\"0\" algorithm=\"MD5\">741e05a117850988ac58f121674fbc7c</Signature></SoftListGet>");

	printf("\ncall soap_call_ns1__SoftListGet\n");
	iRet=soap_call_ns1__SoftListGet(&mysoap,requrl,NULL,xmlstr, &softlist);
	if (iRet) {
		printf("getsoftlist iRet=%d\n",iRet);
		return 2;
	}
	printf("return:%s\n\n",softlist);
	

	

	strcpy(xmlstr,"<?xml version='1.0' encoding='UTF-8'?><DataListGet><SearchInfo startNum=\"0\" maxNum=\"10\"></SearchInfo><Signature signerID=\"legendsec\" type=\"0\" algorithm=\"MD5\">741e05a117850988ac58f121674fbc7c</Signature></DataListGet>");

	printf("\ncall soap_call_ns1__DataListGet\n");
	iRet=soap_call_ns1__DataListGet(&mysoap,requrl,NULL,xmlstr, &datalist);
	if (iRet){
		printf("getdatalist iRet=%d\n",iRet);
		return 2;
	}
	printf("return:%s\n\n",datalist);
	
	soap_destroy(&mysoap);
	soap_end(&mysoap);
	soap_done(&mysoap);
	return iRet;
}
