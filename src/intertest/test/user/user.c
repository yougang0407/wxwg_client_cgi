#include <stdio.h>
#include "../../../include/log.h"

#if 0
#include "RNServiceSoapBinding.nsmap"
#include "soapStub.h"

#include "libxml/parser.h"
#include "libxml/tree.h"
#include "libxml/xmlmemory.h"

#include "sg_domtree.h"
#include "sg_xmllib.h"
#include "sg_domapi.h"
#endif

#if 0
nac_user *get_user_attr_by_name(char *username)
{
	int ret = 0;
	nac_user *user = NULL;

	char tmpreq[1024];
	char *tmpvalue=NULL;

	struct soap mysoap;
	char save_challenge[64];

	XMLDOMDocument *doc 			 = NULL;
	XMLDOMNode	   *root			 = NULL;
	XMLDOMNode	   *result_node 	 = NULL;

	XMLDOMNode *chan_node=NULL;

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
	soap_chan_req.arg0=tmpreq;

	ret = soap_call___ns1__challenge(&mysoap,g_websoap_user_url,NULL,&soap_chan_req,&soap_chan_resp);
	if (ret) {
		WWC_DEBUG("call soap_call___ns1__challenge failed, ret=%d\n", ret);
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

	tmpvalue=sg_get_node_val(chan_node,".");

	strlcpy(save_challenge,tmpvalue,sizeof(save_challenge));

	sg_dom_xmldom_release(doc);
	doc=NULL;
	
	// 2. get user
	WWC_DEBUG("get user %s", username);
	snprintf(tmpreq,1024,"<?xml version=\"1.0\" encoding=\"UTF-8\"?><UserListGet><SearchInfo startNum=\"1\" maxNum=\"1\" challenge=\"%s\">"
		"<Item><Condition name=\"登录名\" relation=\"等于\" value=\"%s\" /></Item></SearchInfo>"
		"<Signature signerID=\"legendsec\" type=\"0\" algorithm=\"MD5\">12345678901234567890123456789012</Signature></UserListGet>",
		save_challenge,username);	
	
	/*snprintf(tmpreq,1024,"<?xml version=\"1.0\" encoding=\"UTF-8\"?><UserListGet><SearchInfo startNum=\"1\" maxNum=\"1\" challenge=\"%s\">"
		"<Item><Condition name=\"姓名\" relation=\"等于\" value=\"%s\" /></Item></SearchInfo>"
		"<Signature signerID=\"legendsec\" type=\"0\" algorithm=\"MD5\">12345678901234567890123456789012</Signature></UserListGet>",
		save_challenge,username);*/
	
	soap_user_req.arg0 = tmpreq;
	ret = soap_call___ns1__userListGet(&mysoap,g_websoap_user_url,NULL,&soap_user_req,&soap_user_resp);
	if (ret) {
		WWC_DEBUG("call soap_call___ns1__userListGet failed, ret=%d\n", ret);
		goto cleanup;
	}

	doc = sg_dom_xmldom_create();
	if(NULL == doc){
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
			strlcpy(user->login_name, value, sizeof(user->login_name));
		} else if (!strcmp(name, "姓名")) {
			strlcpy(user->name, value, sizeof(user->name));
		} else if (!strcmp(name, "身份证号")) {
			strlcpy(user->id_card_num, value, sizeof(user->id_card_num));
		} else if (!strcmp(name, "民族")) {
			strlcpy(user->ethnic, value, sizeof(user->ethnic));
		} else if (!strcmp(name, "单位")) {
			strlcpy(user->unit, value, sizeof(user->unit));
		} else if (!strcmp(name, "单位全称")) {
			strlcpy(user->unit_name, value, sizeof(user->unit_name));
				} else if (!strcmp(name, "职务")) {
			strlcpy(user->position, value, sizeof(user->position));
		} else if (!strcmp(name, "性别")) {
			strlcpy(user->gender, value, sizeof(user->gender));
		} else if (!strcmp(name, "军队证件号")) {
			strlcpy(user->identity_no, value, sizeof(user->identity_no));
		} else if (!strcmp(name, "保障卡号")) {
			strlcpy(user->soldier_security_no, value, sizeof(user->soldier_security_no));
		} else if (!strcmp(name, "出生时间")) {
			strlcpy(user->birthday, value, sizeof(user->birthday));
		} else if (!strcmp(name, "联系方式")) {
			strlcpy(user->contact_way, value, sizeof(user->contact_way));
		} else if (!strcmp(name, "血型")) {
			strlcpy(user->blood_type, value, sizeof(user->blood_type));
		} else if (!strcmp(name, "职务层级")) {
			strlcpy(user->rank, value, sizeof(user->rank));
		} else if (!strcmp(name, "军衔")) {
			strlcpy(user->militray_rank, value, sizeof(user->militray_rank));
		} else if (!strcmp(name, "军兵种")) {
			strlcpy(user->army_service, value, sizeof(user->army_service));
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
