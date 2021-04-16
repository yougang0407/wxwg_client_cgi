/* debug check_with_dtd() memory leak */
/* update value in create_raw_node() */
/* getXmlNode()/setXmlNode() support NODE_TEXT */
/* getXmlNode() add "name[index]" in search path */
/* xml_check_with_dtd() restore origianl dtd after check */
/* xml_check_with_dtd() restore origianl dtd after check */
/* xml_set_dtd() change root can NULL while type is NULL */
/* add "" between dtd publicID and url in xml_set_dtd() */
/* add getXmlNode() function to get node by path */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sg_domtree.h"
#include "sg_domapi.h"

#include "domlib.h"
#include "dtd_chk.h"

int __sg_xml_get_num__ = 0;

/* version info */
static const char* version = "1.0";
static const char* lastupdate = "2011-01-10";
static const char* signature = "fOOLfISH 2011-2012";
static const char* product = "foolfish xmldom 1.1";
static const char* changelog = "\n"\
"Add getXmlNode() setXmlNode() \n"
"Change getversionfinof() interface \n"
"Fix several memory leak bug, so NO memory leak now\n"
"support gb2312 encoding \n"
"xml_set_dtd() change root can NULL while type is NULL\n"
"xml_check_with_dtd() restore origianl dtd after check\n"
"chk_dtd.c->match_node() change right to locate Element\n"
"add \"name[index]\" path attribute in get/setXmlNode\n"
"support NODE_TEXT in getXmlNode() and setXmlNode()\n"
"debug xml_check_with_dtd() memory leak \n"
"modify the declaration of all functions and macros in tts_domapi.h \n"
;


#if (1) /* version info */
void getversioninfo(unsigned char **pversion,
				unsigned char **plastupdate,
				unsigned char **psignature,
				unsigned char **pproduct,
				unsigned char **pchangelog)
{
	if(pversion != NULL)		*pversion = (unsigned char*) version;
	if(plastupdate != NULL)		*plastupdate = (unsigned char*) lastupdate;
	if(psignature != NULL)		*psignature = (unsigned char*) signature;
	if(pproduct != NULL)		*pproduct = (unsigned char*) product;
	if(pchangelog != NULL)		*pchangelog = (unsigned char*) changelog;
}
#endif

#if (100) /* xml output */

extern char *dom_proc_escape(char *text);
char *dom_get_nodetext_escape(XMLDOMNode *text_node)
{
	XMLDOMNode *temp = NULL;
	int total_len = 0;
	XMLDOMNode *first = NULL;
	XMLDOMNode *prev = NULL;
	XMLDOMNode *next = NULL;
	XMLDOMNode *esc_node = NULL;
	char *esc = NULL;
	char *text_node_all_content = NULL;
	int n = 0;

	if(text_node == NULL){
		//fprintf(stderr, "text_node recvd is null\n");
		return NULL;
	}

	/* Judge para's type, only process textnode */
	if(!sg_dom_istext(text_node)){
		return text_node->text;
	}
	
	/* Get the all textnodes' length */
	temp = sg_dom_searchfirst_textn(text_node);
	if(temp == NULL)
		return NULL;
	do{
		total_len += strlen(temp->text);
		temp = sg_dom_nextsibling(temp);
	}while(temp != NULL);

	/* 
	 * If text_node only have one childnode, 
	 * return it's text_node->text 
	 */
	first = sg_dom_searchfirst_textn(text_node);
	// first is text_node
	if(sg_dom_nextsibling(first) == NULL){
		return text_node->text;
	}

	/* 
	 * Alloc space for text_node_all_content
	 * More childnodes exist
	 */
	text_node_all_content = malloc(total_len + 1);
	if(text_node_all_content == NULL){
		//fprintf(stderr, "malloc fail, not enough space\n");
		return NULL;
	}
	memset(text_node_all_content, 0, total_len + 1);

#if 00
	/* 
	 * Get text_node 's all childnode's content , 
	 * and store the in one string 
	 */
	if(first->text != NULL){
		strcpy(&text_node_all_content[n], first->text);
		n += strlen(first->text);
	}
	next = first;
	if(next != NULL){
		prev = next;
		while((next = sg_dom_nextsibling(next)) != NULL){
			if(next->text != NULL){
				strcpy(&text_node_all_content[n], next->text);
				n += strlen(next->text);
			}
			//sg_dom_remove_node(prev);
			prev = next;
		}
		//sg_dom_remove_node(prev);
	}

	return text_node_all_content;
#else
	/* 
	 * Get text_node's all childnode's content, 
	 * and store them in one string 
	 */
	if(first->text != NULL){
		strcpy(&text_node_all_content[n], first->text);
		n += strlen(first->text);
	}
	next = first; // node_text
	if(next != NULL){
		prev = next;
		while((next = sg_dom_nextsibling(next)) != NULL){
			if(next->text != NULL){
				strcpy(&text_node_all_content[n], next->text);
				n += strlen(next->text);
			}
			if( prev != text_node )
				sg_dom_remove_node(prev);
			prev = next;
		}
		sg_dom_remove_node(prev);
	}

	/* 
	 * change text_node_all_content to string 
	 * including Esc character 
	 */
	esc = dom_proc_escape(text_node_all_content);
	if(esc == NULL){
		free(text_node_all_content);
		return NULL;
	}

	esc_node = sg_dom_create_textn((char *)esc);
	sg_dom_replace_node(text_node, esc_node);
	//sg_dom_replace_node(first, esc_node);
	
	if(esc != NULL)	
		free(esc);

	/* 
	 * Just for this function!
	 * if node'type is not text_node, 
	 * esc_node->text is string not including escape character, 
	 * and the string is what we want;
	 * if node'type is text_node, 
	 * esc_node->text is string including escape character, 
	 * and the string is not what we want, so we do the following
	 */
	free(esc_node->text);
	esc_node->text = text_node_all_content;
	
	return esc_node->text;
#endif
}

XMLDOMNode* get_root(XMLDOMDocument* doc) 
{
	XMLDOMNode *n, *m;
	if(doc == NULL || (n = doc->documentElement) == NULL)
		return NULL;

	if(sg_dom_haschild(n)) {
		for(m = sg_dom_firstchild(n); !sg_dom_isnull(m); m = sg_dom_nextsibling(m))
			if(sg_dom_iselement(m)) return m;
	}			

	return NULL;
}	

int xml_get_num(void)
{
	return __sg_xml_get_num__;
}

void free_xml(char *xml)
{
	free(xml);
	__sg_xml_get_num__--;
}

char* get_xml(XMLDOMNode* n)
{
	char* xml = NULL;
	if(n != NULL)
		n->get_xml(n,&xml);
	
	__sg_xml_get_num__++;
	return xml;
}

void free_docxml(char *docxml)
{
	__sg_xml_get_num__--;
	free(docxml);
}

char* get_docxml(XMLDOMDocument* doc)
{
	char* xml = NULL;
	if(doc != NULL)
		doc->get_xml(doc,&xml);

	__sg_xml_get_num__++;
	return xml;
}

#endif


#if (1) //node create

BOOL release_raw_node(XMLDOMNode** n)
{
	XMLDOMDocument* doc = NULL;
	//XMLDOMNode* node = NULL;
	if(n == NULL || *n == NULL)
		return FALSE;	
	if((doc = sg_dom_xmldom_create()) == NULL)
		return FALSE;
	(void) doc->releaseNode(n);
	sg_dom_xmldom_release(doc);
	return TRUE;
}

XMLDOMNode* create_raw_node(int type, char* name, char* value)
{
	XMLDOMDocument* doc = NULL;
	XMLDOMNode* node = NULL;	
	if((doc = sg_dom_xmldom_create()) == NULL)
		return NULL;

	switch(type) {
	case NODE_ELEMENT:
		node = doc->createNode(NODE_ELEMENT,name,NULL);	
		break;
	case NODE_ATTRIBUTE:
		node = doc->createNode(NODE_ATTRIBUTE,name,NULL);
		break;
	case NODE_TEXT:
		node = doc->createNode(NODE_TEXT,"#text",NULL);
		break;
	case NODE_CDATA_SECTION:
		node = doc->createNode(NODE_CDATA_SECTION,"#cdata-section",NULL);
		break;
	case NODE_COMMENT:
		node = doc->createNode(NODE_COMMENT,"#comment",NULL);
		break;
	default:
		break;
	}

	if(node != NULL && value != NULL)
		node->text = estrdup(value);

	sg_dom_xmldom_release(doc);
	return node;
}

#endif

#if (1) //local search function
static XMLDOMNode* get_raw_node(int type, XMLDOMNode* p, const char* value)
{
	XMLDOMNode* n;
	if(sg_dom_isnull(p)) return NULL;	

	for(n = p; n != NULL; n = sg_dom_nextsibling(n)) {
		if(n->nodeType != type)
			continue;
		
		switch(type) {
		case NODE_ELEMENT:
			if(sg_dom_isnull(value)) return NULL;
			if(strcmp(n->nodeName,value)==0)
				return n;
			break;
		case NODE_ATTRIBUTE:
		case NODE_TEXT:
		case NODE_CDATA_SECTION:
		case NODE_COMMENT:
			return n;
		default:
			return NULL;
		}
	}
	return NULL;
}


XMLDOMNode* get_raw_child(int type, XMLDOMNode* p, const char* value)
{
	if(sg_dom_isnull(p) || !sg_dom_haschild(p)) return NULL;
	return get_raw_node(type, sg_dom_firstchild(p), value);
}

XMLDOMNode* get_raw_next(int type, XMLDOMNode* p, const char* value)
{
	if(sg_dom_isnull(p)) return NULL;
	return get_raw_node(type, sg_dom_nextsibling(p), value);
}

#endif

#if (1) //recurse search function

XMLDOMNode* search_raw_node(int type, XMLDOMNode* p, const char* n)
{
	XMLDOMNode* t;
	if(sg_dom_isnull(p)) return NULL;

	/* seach cur */
	if(p->nodeType == type) {
		switch(type) {
		case NODE_ELEMENT:
			if(sg_dom_isnull(n)) return NULL;
			if(strcmp(p->nodeName,n)==0)
				return p;
			break;
		case NODE_ATTRIBUTE:
		case NODE_TEXT:
		case NODE_CDATA_SECTION:
		case NODE_COMMENT:
			return p;
		default:
			return NULL;
		}
	}
	
	/* search child */
	{
	XMLDOMNode* c;
	c = sg_dom_firstchild(p);
	while(!sg_dom_isnull(c)) {
		t = search_raw_node(type, c, n);
		if(!sg_dom_isnull(t))
			return t;
		c = sg_dom_nextsibling(c);
	}
	}

	return NULL;
}

XMLDOMNode* search_raw_next(int type, XMLDOMNode* p, 
							const char* n, XMLDOMNode* stop)
{
	XMLDOMNode* c;

	/* ignore cur p */
	if(sg_dom_isnull(p))
		return NULL;
	
	/* search child */
	c = sg_dom_firstchild(p);
	while(!sg_dom_isnull(c)) {
		XMLDOMNode* t;
		t = search_raw_node(type, c, n);
		if(!sg_dom_isnull(t))
			return t;
		c = sg_dom_nextsibling(c);
	}

	/* search next sbiling */
	c = sg_dom_nextsibling(p);
	while(!sg_dom_isnull(c)) {
		XMLDOMNode* t;
		t = search_raw_node(type, c, n);
		if(!sg_dom_isnull(t))
			return t;
		c = sg_dom_nextsibling(c);
	}

	/* search parent next */	
	p = sg_dom_parentnode(p);
	while(!sg_dom_isnull(p) && p != stop) {
		c = sg_dom_nextsibling(p);
		while(!sg_dom_isnull(c)) {
			XMLDOMNode* t;
			t = search_raw_node(type, c, n);
			if(!sg_dom_isnull(t))
				return t;
			c = sg_dom_nextsibling(c);
		}
		p = sg_dom_parentnode(p);
	}

	return NULL;
}

#endif

#if (1)
int xml_check_dtd(XMLDOMDocument* doc)
{
	extern int check_DTD(void* doc);
	return (check_DTD((void*) doc)==1)?TRUE:FALSE;	
}

int xml_check_with_dtd(XMLDOMDocument* doc, const char* url)
{
	XMLDOMNode* n;
	XMLDOMDocumentType *pre;
	int result;
	
	result = 0;
	if(doc == NULL || url == NULL)
		return 0;
	if((n = sg_dom_rootdocnode(doc)) == NULL || n->nodeName == NULL)
		return 0;

	pre = NULL;

	/* ----------------- */
	/* save original dtd */
	/* ----------------- */
	if(doc->docType != NULL) {
		pre = (XMLDOMDocumentType*) malloc(sizeof(*pre));
		if(pre == NULL)
			goto done;
		memcpy(pre,doc->docType,sizeof(*pre));
		if(doc->docType->name != NULL) {
			pre->name = estrdup(doc->docType->name);

			if(pre->name == NULL)
				goto done;
		}
		if(doc->docType->dtd != NULL) {
			pre->dtd = estrdup(doc->docType->dtd);
			if(pre->dtd == NULL)
				goto done;
		}
	}	


	if(xml_set_dtd(doc, n->nodeName,"SYSTEM",NULL,url,NULL) == 0)
		goto done;

	result = xml_check_dtd(doc);

	/* -------------------- */
	/* restore original dtd */
	/* -------------------- */
	{
	if(xml_set_dtd(doc, NULL, NULL, NULL, NULL, NULL) != 1)
		goto done;
	doc->docType = pre;
	pre = NULL;
	}


done:
	if(pre != NULL) {
		if(pre->name != NULL)
			free(pre->name);
		if(pre->dtd != NULL)
			free(pre->dtd);
		free(pre);
	}
	return result;
}

int xml_set_dtd(XMLDOMDocument* doc, 
				const char* root,const char* type,
				const char* publicID,const char* url,const char* dtd)
{
	struct XMLDOMDocumentType* dt;
	
	if(doc == NULL)							
		return 0;
	if(root == NULL && type != NULL)
		return 0;
	#if (0000)
	if(type == NULL && dtd == NULL)
		return 0;
	#endif
	if(type != NULL && strcmp(type,"SYSTEM")==0 && url == NULL)
		return 0;
	if(type != NULL && strcmp(type,"PUBLIC")==0 && 
							(publicID == NULL || url == NULL)) 
		return 0;

	/* ---------------- */
	/* wether clean dtd */
	/* ---------------- */
	dt = NULL;
	if(root == NULL && type == NULL && dtd == NULL)
		goto clean;

	if((dt = (struct XMLDOMDocumentType*) malloc(sizeof(*dt))) == NULL)
		return 0;
	memset(dt,0,sizeof(*dt));
	
	dt->flag = DTD_START;
	dt->name = estrdup(root);
	build_string(1, &(dt->dtd), (char*) root);
	build_string(0,&(dt->dtd)," ");
	if(type == NULL) {
		build_string(0,&(dt->dtd),"[");
		build_string(0,&(dt->dtd),(char*) dtd);
		build_string(0,&(dt->dtd),"]");
	}
	else {
		if(strcmp(type,"SYSTEM")==0) {
			build_string(0,&(dt->dtd),"SYSTEM");
			build_string(0,&(dt->dtd)," ");
			build_string(0,&(dt->dtd),"\"");
			build_string(0,&(dt->dtd),(char*) url);
			build_string(0,&(dt->dtd),"\"");
		}
		else { //PUBLIC
			build_string(0,&(dt->dtd),"PUBLIC");
			build_string(0,&(dt->dtd)," ");
			build_string(0,&(dt->dtd),"\"");
			build_string(0,&(dt->dtd),(char*) publicID);
			build_string(0,&(dt->dtd),"\"");
			build_string(0,&(dt->dtd)," ");
			build_string(0,&(dt->dtd),"\"");
			build_string(0,&(dt->dtd),(char*) url);
			build_string(0,&(dt->dtd),"\"");
		}
		if(dtd != NULL) {
			build_string(0,&(dt->dtd)," ");
			build_string(0,&(dt->dtd),"[");
			build_string(0,&(dt->dtd),(char*) dtd);
			build_string(0,&(dt->dtd),"]");
		}
	}
		
	dt->flag = DTD_END;
	dt->dtd_exist = TRUE;

clean:

	/* ------------------------- */
	/* free original dtd at here */
	/* ------------------------- */
	if(doc->docType != NULL) {
		if(doc->docType->name != NULL)
			free(doc->docType->name);
		if(doc->docType->dtd != NULL)
			free(doc->docType->dtd);
		free(doc->docType);
	}	

	/* link new dtd to doc */
	doc->docType = dt;

	return 1;
}

#endif


#if (1)
XMLDOMNode* getXmlNode(XMLDOMNode* cur, 
						const char* path)
{
	char* dir;
	XMLDOMNode* node = NULL;
	int target_exist;

#define __WETHER_LEGAL_DOT_MODE(str,quit)\
do { int i; \
	for(i = 0; i < strlen((char *)t); i++)\
		if(t[i] != '.') goto quit;\
} while(0)


	if(cur == NULL || path == NULL)
		return NULL;

	if((dir = strdup((char *)path)) == NULL)
		return NULL;

	/* wether start with root Node */	
	node = (path[0]=='/') ? sg_dom_parentnode(sg_dom_rootnode(cur)) : cur;

	target_exist = 0;

	{
	unsigned char* t;
	t = (unsigned char*) strtok(dir,"/");
	do {
		int node_found = 0;

		if(t == NULL)
			break;
#if (0000)
		fprintf(stderr,"analysis path [%s] on Node name [%s]\n",
						t,sg_dom_nodename(node));
#endif

		/* ------------------- */
		/* search dot mode tag */
		/* ------------------- */
		if(t[0] == '.') {
			int cnt;
			__WETHER_LEGAL_DOT_MODE((char *)t,done);
		
			if(strlen((char *)t)==1) //cur Node
				node_found = 1;
			else if(node == (XMLDOMNode*)sg_dom_parentnode(sg_dom_rootnode(node))) //cur is RootNode
				node_found = 1;
			else {
				for(cnt = 0; cnt < strlen((char *)t) - 1; cnt++) {
					node = sg_dom_parentnode(node);	
					if(node == NULL)
						goto done;
					node_found = 1;

					if(node == sg_dom_parentnode(sg_dom_rootnode(node)))
						break; //only back to RootNode
				}
			}
		}

		/* ----------------------- */
		/* search NON dot mode tag */
		/* ----------------------- */
		else {
			XMLDOMNode* c;
			int REQUIRED_CHILD_INDEX;
			int current_child_index;
			char *__BEGIN, *__END;

			/* ------------------------ */
			/* wether have name[i] mode */
			/* ------------------------ */
			REQUIRED_CHILD_INDEX = 1;
			if((__BEGIN = strstr((char *)t,"[")) != NULL) {
				if((__END = strstr(__BEGIN,"]")) != NULL) {
					if((__END - 1) - __BEGIN > 0) {	
						*__END = '\0';
						REQUIRED_CHILD_INDEX = atoi(__BEGIN+1);
						*__BEGIN = '\0';
					}
				}
			}

			current_child_index = 0;
			for(c = sg_dom_firstchild(node); 
							c != NULL; c = sg_dom_nextsibling(c))
			{
				/* support NODE_TEXT */
				if(!(sg_dom_iselement(c) || sg_dom_isattribute(c) || sg_dom_istext(c)))
					continue;

#if (0000)
printf("compare node [%s] with target [%s]\n", sg_dom_nodename(c),t);
#endif
				if(strcmp(sg_dom_nodename(c),(char *)t) == 0) {
					current_child_index ++;
					if(current_child_index == REQUIRED_CHILD_INDEX) {
						node = c; node_found = 1; 
						break;
					}
				}
			}

			if(node_found != 1) break;
		}

		/* ------------------ */
		/* get NEXT path name */
		/* ------------------ */
		t = (unsigned char*) strtok(NULL,"/");

		/* ------------------ */
		/* wether over search */
		/* ------------------ */
		if(t == NULL) {
			if(node_found == 1) 
				target_exist = 1;
			break;
		}

	} while(1);
	}

done:
	if(dir != NULL)
		free(dir);

	return (target_exist==1) ? node : NULL;
}

XMLDOMNode* setXmlNode(XMLDOMNode* cur, 
						const char* path,
						char* text)
{
	unsigned char* dir;
	XMLDOMNode* node;

	if(cur == NULL || text == NULL)
		return NULL;

	dir = (path == NULL) ? ((unsigned char*)".") : 
					(unsigned char*) path;

	node = getXmlNode(cur,(char *)dir);
	if(node == NULL)
		return NULL;

	#if (0000)
	/* do NOT check at here 
	   for getXmlNode() already check */
	if(!(sg_dom_iselement(node) || sg_dom_isattribute(node) || sg_dom_istext(node)))
		return NULL;
	#endif

	/* replace old with new */
	{
	char* t;
	t = strdup((char *)text);
	if(t == NULL)
		return NULL;
	if(node->text != NULL)
		free(node->text);
	node->text = t;
	}

	return node;
}

#endif
