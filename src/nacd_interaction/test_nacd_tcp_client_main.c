#include <log.h>
#include <nacd_tcp_client.h>

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event-internal.h>

#include "./soap/soapStub.h"
#include "./soap/stdsoap2.h"

#include "../lib/xml/sg_xml/include/libxml/parser.h"
#include "../lib/xml/sg_xml/include/libxml/tree.h"
#include "../lib/xml/sg_xml/include/libxml/xmlmemory.h"

#include "../lib/xml/sg_xml/sg_domtree.h"
#include "../lib/xml/sg_xml/sg_xmllib.h"
#include "../lib/xml/sg_xml/sg_domapi.h"


struct event_base *sg_current_base = NULL;

nacd_config_msg *nacd_cfg_ptr = NULL;
user_info *user_info_ptr = NULL;


void getuserlist(char *ip,int port)
{
	char requrl[1024];
	struct soap mysoap;
	struct ns1__challenge soap_chan_req;
	struct ns1__challengeResponse soap_chan_resp;
	struct ns1__userListGet soap_user_req;
	struct ns1__userListGetResponse soap_user_resp;
	struct ns1__accessUnitGet soap_unit_req;
	struct ns1__accessUnitGetResponse soap_unit_resp;

	XMLDOMDocument *doc 			 = NULL;
	XMLDOMNode	   *root			 = NULL;
	XMLDOMNode *resultnode=NULL;
	XMLDOMNode *channode=NULL;

	char save_chan[32];
	
	char tmpreq[1024];
	char *tmpvalue=NULL;
	int iRet;

	sprintf(requrl,"http://%s:%d/wserver_war/app/services/RNService",ip,port);

	

	soap_init(&mysoap);
	soap_set_mode(&mysoap, SOAP_C_UTFSTRING);
	   mysoap.mode |= SOAP_C_UTFSTRING;

	snprintf(tmpreq,1024,"<?xml version=\"1.0\" encoding=\"UTF-8\"?><Chanllenge><EncCert></EncCert></Chanllenge>");

		WWC_DEBUG("call soap_call___ns1__challenge\n");
	 soap_chan_req.arg0=tmpreq;
		iRet = soap_call___ns1__challenge(&mysoap,requrl,NULL,&soap_chan_req,&soap_chan_resp);
		WWC_DEBUG("soap_call___ns1__challenge return %d\n",iRet);

		if (iRet){
				goto cleanup;
		}
	 WWC_DEBUG("%s\n",soap_chan_resp.return_);

	 doc = sg_dom_xmldom_create();
	 if (!doc->loadXML(doc, soap_chan_resp.return_)) {
		WWC_DEBUG("xml failed\n");
		goto cleanup;
	 }

	 root = sg_dom_rootdocnode(doc);
	 WWC_DEBUG("root=%p\n",root);

	 resultnode = sg_dom_get_xmlnode(root, "Result");
	 WWC_DEBUG("Result=%p\n",resultnode);
	 channode = sg_dom_get_xmlnode(resultnode, "Challenge");
	 WWC_DEBUG("channode=%p\n",channode);
	 tmpvalue=sg_get_node_val(channode,".");
	 

	 strncpy(save_chan,tmpvalue,sizeof(save_chan));

	 WWC_DEBUG("challenge=%s\n",save_chan);

	sg_dom_xmldom_release(doc);


	snprintf(tmpreq,1024,"<?xml version=\"1.0\" encoding=\"UTF-8\"?><UserListGet><SearchInfo startNum=\"%d\" maxNum=\"%d\" challenge=\"%s\"></SearchInfo>"
		"<Signature signerID=\"legendsec\" type=\"0\" algorithm=\"MD5\">12345678901234567890123456789012</Signature></UserListGet>",
		0,5,save_chan);
	
	
	soap_user_req.arg0 = tmpreq;
	WWC_DEBUG("call soap_call___ns1__userListGet\n");
	iRet = soap_call___ns1__userListGet(&mysoap,requrl,NULL,&soap_user_req,&soap_user_resp);
	WWC_DEBUG("soap_call___ns1__userListGet return %d\n",iRet);

	if (iRet){
		goto cleanup;
	}

	WWC_DEBUG("%s\n",soap_user_resp.return_);

	doc = sg_dom_xmldom_create();
	 if (!doc->loadXML(doc, soap_chan_resp.return_)) {
		WWC_DEBUG("xml failed\n");
		goto cleanup;
	 }

	 
	sg_dom_xmldom_release(doc);

	snprintf(tmpreq,1024,"<?xml version=\"1.0\" encoding=\"UTF-8\"?><AccessUnitGet><Applicant ip=\"192.168.5.119\" challenge=\"%s\" />"
		"<Signature signerID=\"legendsec\" type=\"0\" algorithm=\"MD5\">12345678901234567890123456789012</Signature>"
		"<SignerCert>xxxxxx</SignerCert></AccessUnitGet>",
		save_chan);
	soap_unit_req.arg0=tmpreq;
	WWC_DEBUG("soap_call___ns1__accessUnitGet\n");
	iRet = soap_call___ns1__accessUnitGet(&mysoap,requrl,NULL,&soap_unit_req,&soap_unit_resp);
	WWC_DEBUG("soap_call___ns1__accessUnitGet return %d\n",iRet);
	if (iRet){
		goto cleanup;
	}
	WWC_DEBUG("%s\n",soap_unit_resp.return_);
	
	doc = sg_dom_xmldom_create();
	 if (!doc->loadXML(doc, soap_chan_resp.return_)) {
		WWC_DEBUG("xml failed\n");
		goto cleanup;
	 }

	 
	sg_dom_xmldom_release(doc);
	

	
cleanup:
		soap_destroy(&mysoap);
		soap_end(&mysoap);
		soap_done(&mysoap);

		return ;

}

int test_nacd_init_event(void)
{
	sg_current_base = event_init();
	if (NULL == sg_current_base) {
		return -1;
	}

	return 0;
}

int main()
{
#if 1
	char *soapip="172.24.227.34";
	int soapport=8080;
	getuserlist(soapip, soapport);
#endif
	test_nacd_init_event();

	user_info *user_msg = (user_info *)malloc(sizeof(user_info));
	nacd_config_msg *nacd_cfg = (nacd_config_msg *)malloc(sizeof(nacd_config_msg));;

	snprintf(user_msg->user_name, sizeof(user_msg->user_name), "admin");
	snprintf(user_msg->user_passwd, sizeof(user_msg->user_passwd), "123456");

	snprintf(nacd_cfg->nacd_server_ip, sizeof(nacd_cfg->nacd_server_ip), "0.0.0.0");
	nacd_cfg->nacd_server_port = 8859;
	nacd_cfg->timeout = 1;
	nacd_cfg->use_ssl = 0;

	int ret = 0;
	//int ret = nacd_handle_user_passwd_func(nacd_cfg, user_msg, NACD_USER_PASSWD_AUTH);

#if 1
	char g_websoap_user_url[256];
	strncpy(g_websoap_user_url, \
			"http://172.24.227.34:8080/wserver_war/app/services/RNService", \
			sizeof(g_websoap_user_url));
	strncpy(user_msg->user_name, "555666198808089999@ZS", sizeof(user_msg->user_name));
	nac_user *user;
	user = get_user_attr_by_name(user_msg->user_name, g_websoap_user_url);
	WWC_DEBUG("user:%p \n\
			\r user->name:%s \n\
			\r user->login_name:%s \n\
			\r user->id_card_num:%s \n\
			\r \n", user, user->name, user->login_name, user->id_card_num);
	if (!user) {
		WWC_ERROR("get_user_attr_by_name error..");
		return -1;
	}

	char res_random[32];
	ret = get_random_func(soapip, soapport, res_random, sizeof(res_random));
	if (ret) {
		WWC_ERROR("get_random_func error..");
		return -1;
	}

	//get_auth_sec_assert_msg_func(user, res_random);
	//WWC_DEBUG("auth_sec_assert_msg: %s\n", auth_sec_assert_msg);

#endif

	event_dispatch();
	return ret;
}
