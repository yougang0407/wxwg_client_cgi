/******************************************************************************
	Copyright (C), 2012-2020, legendsec Tech. Co., Ltd.
	File name:  sg_xmllib.c
	Author: liyz
	Version:1.0
	Date:2012-03-06
	Description: Libs based on DOM API

	Others:
	Function List:

	History:
		1.  Date:2012-03-06
			Author:liyz
			Modification:create sg_xmllib.c add fnctions

 ******************************************************************************/

#ifdef __cplusplus
extern "C"{
#endif
#include "stdarg.h"

#include "sg_xmllib.h"
//#include <cp/sg_cmd_lib.h>
//#include <cp/sg_err.h>

#define TAG 0

#define XML_DEBUG(tag, fmtx...)  \
	do{ \
		if( tag ){ \
			fprintf(stderr, "%s[%d]", __FILE__, __LINE__); \
			fprintf(stderr, fmtx); \
		} \
	} while( 0 )


char* lTrimStr( char *Str )
{
	char *pStr = NULL;
	char buf[1024] = {0,};

	if( Str == NULL )
		return NULL;

	if (strlen(Str) < sizeof(buf))
		strcpy(buf, Str);
	else
		return NULL;

	pStr = buf;
	while( *pStr != '\0' && ( *pStr == ' ' || *pStr == '\t' ) ){
		pStr ++;
	}

	strcpy(Str, pStr);

	return pStr;
}

char* rTrimStr( char *Str )
{
	char *pStr = Str;

	if( pStr == NULL )
		return NULL;

	pStr = Str + strlen( Str );

	while( pStr >= Str
			&& ( *pStr == ' ' || *pStr == '\t' || *pStr == '\0' ) ){
		pStr --;
	}

	*( pStr + 1 ) = '\0';

	return Str;
}


/*
 * Description: write doc's content to a file
 * 	     input: path, doc
 */
int sg_write_xmldata_to_file(char *temp_path,
		                  XMLDOMDocument *doc)
{
	FILE *fp = NULL;
	char *buf = NULL;
	int len = 0;

	if(temp_path == NULL || doc == NULL){
		printf("receive null parameters\n");
		goto err;
	}

	if((fp = fopen(temp_path, "w+")) == NULL){
		printf("can not open file--------- %s\n", temp_path);
		goto err;
	}

	buf = sg_dom_get_docxml(doc);
	len = fwrite(buf, 1, strlen(buf), fp);
	if(len == 0){
		printf("do not write buf to file\n");
		fclose(fp);
		goto err;
	}

	fclose(fp);

	if(buf != NULL){
		sg_dom_free_docxml(buf);
		buf = NULL;
	}
	return 0;
err:
	if(buf != NULL){
		sg_dom_free_docxml(buf);
		buf = NULL;
	}
	return -1;
}


/*
 * append more than one attr node into element node one time.
 * example:
 *  change
 *      <element/>
 *    to
 *      <element attr="attr_val" attr1="attr1_val"/>
 *  Just use one function
 *  In this example, attr_num=2
 */

#ifndef sg_append_attr
#define sg_append_attr(Element, AttrName, AttrVal)\
	sg_append_attr_ex(Element, 1, AttrName, AttrVal)
#endif
XMLDOMNode *sg_append_attr_ex(XMLDOMNode *element, int attr_num, ...)
{
	XMLDOMNode *attr = NULL;
	va_list va_attr;
	char *attr_name = NULL;
	char *attr_value = NULL;
	int i = 0;

	if (element == NULL) {
		return NULL;
	}
	if (sg_dom_iselement(element) == FALSE) {
		return NULL;
	}
	va_start(va_attr, attr_num);
	for (i = 0; i < attr_num; i++) {

		attr_name = va_arg(va_attr, char*);
		if (attr_name == NULL) {
			goto err;
		}
		attr_value = va_arg(va_attr, char*);
		if (attr_value == NULL) {
// printf("NULL VAL[%s]", attr_name);
			continue;
		}
		if ((attr = sg_dom_create_attributen(attr_name, attr_value)) == NULL) {
			goto err;
		}

		if (sg_dom_add_childnode(element, attr) == FALSE) {
			sg_dom_release_noden(attr);
			goto err;
		}
	}
	va_end(va_attr);

	return element;
err:
	va_end(va_attr);
	return NULL;
}


/*
 * 通用XML节点及属性处理函数。
 */
/*
 *  Description: create an element, like <Checked>true</Checked>
 *	      Input: element's name; element's content
 *	     Output: element NODE
 */
XMLDOMNode* sg_create_element_node(char *ele_name, char *ele_textn)
{
	XMLDOMNode *ele = NULL;
	XMLDOMNode *ele_txt_node = NULL;

	if(ele_name == NULL){
		fprintf(stderr, "receive null parameters\n");
		goto err;
	}

	if(ele_textn != NULL){
		ele_txt_node = sg_dom_create_textn(ele_textn);
		if(ele_txt_node == NULL){
			fprintf(stderr, "sg_dom_create_textn fail\n");
			goto err;
		}
		ele = sg_dom_create_elementn(ele_name);
		if(ele == NULL){
			fprintf(stderr, "sg_dom_create_elementn fail\n");
			goto err;
		}
		ele_txt_node = sg_dom_add_childnode(ele, ele_txt_node);
	}else{
		ele = sg_dom_create_elementn(ele_name);
	}

	return ele;
err:
	return NULL;
}


/*
 * add ele_num element node one time, return last added child
 * ... ,give me element node names.
 *  use sg_append_element(element, "hello", "hello_val")
 *  change
 *
 *   <element/>
 *
 *  into
 *
 *   <element>
 *    <hello>hello_val</hello>
 *   </element>
 *
 *  use sg_append_element_ex(element, 2,
 *                  "hello1", "hello1_val",
 *                  "hello2", "hello2_val")
 *  change
 *   <element/>
 *
 *  into
 *
 *   <element>
 *    <hello1>hello1_val</hello>
 *    <hello2>hello2_val</hello>
 *   <element>
 *
 */
#ifndef sg_append_element
#define sg_append_element(Element, NodeName, NodeVal)\
	sg_append_element_ex(Element, 1, NodeName, NodeVal)
#endif
XMLDOMNode *sg_append_element_ex(XMLDOMNode *element, int ele_num, ...)
{
	XMLDOMNode *child = NULL;
	char *child_name = NULL;
	char *child_val = NULL;
	va_list ele;
	int i = 0;
	if (element == NULL) {
		return NULL;
	}
	if (sg_dom_iselement(element) == FALSE) {
		return NULL;
	}
	va_start(ele, ele_num);
	for (i = 0; i < ele_num; i++) {
		child_name = va_arg(ele, char*);
		if (child_name == NULL) {
			goto err;
		}
		child_val = va_arg(ele, char*);
		if (child_val == NULL) {
			if ((child = sg_dom_create_elementn(child_name)) == NULL) {
				goto err;
			}
		} else {
			if ((child = sg_create_element_node(child_name, child_val)) == NULL) {
				goto err;
			}
		}
		if (sg_dom_add_childnode(element, child) == FALSE) {
			sg_dom_release_noden(child);
			goto err;
		}
	}
	va_end(ele);
	return element;
err:
	va_end(ele);
	return NULL;
}

/*
 * add child element node , add many child one time.
 *
 *    <element/>
 *
 *  sg_append_node_ex(element, 3, child1, child2, child3);
 *
 *    <element>
 *  	<child1 attr="attrval"/>
 *  	<child2>text_val</child2>
 *  	<child3>
 *  	  <hello/>
 *  	</child3>
 *    </element>
 *
 *
 *   use sg_append_node(element, child1)
 *   change <element/> into
 *   <element>
 *   	<child1>
 *   <element>
 */
#ifndef sg_append_node
#define sg_append_node(Element, ChildElement) \
	sg_append_node_ex(Element, 1, ChildElement)
#endif
XMLDOMNode *sg_append_node_ex(XMLDOMNode *element, int ele_num, ...)
{
	XMLDOMNode *child = NULL;
	va_list ele;
	int i = 0;

	if (element == NULL) {
		return NULL;
	}
	if (sg_dom_iselement(element) == FALSE) {
		return NULL;
	}
	va_start(ele, ele_num);
	for (i = 0; i < ele_num; i++) {

		child = va_arg(ele, XMLDOMNode*);
		if (child == NULL) {
			goto err;
		}
		if (sg_dom_iselement(child) == FALSE) {
			goto err;
		}
		if (sg_dom_add_childnode(element, child) == FALSE) {
			goto err;
		}

	}
	va_end(ele);
	return element;
err:
	va_end(ele);
	return NULL;
}

/*
 * create xml node like
 *
 *  use sg_create_attr_node("element_name", 1, "attr1", "attr1_val")
 *  create
 *  	<element_name attr1="attr1_val"/>
 *  and use sg_create_attr_node("element_name", 2, "attr1", "attr1_val", "attr2", "attr2_val")
 *  	<element_name attr1="attr1_val" attr2="attr2_val"/>
 *
 *  BUG: if attr_num > real attr num, may segment fault!
 *
 *  attr_num can be 0. then it same with sg_dom_create_elementn().
 */
XMLDOMNode *sg_create_attr_node(char *element_name, int attr_num, ...)
{
	XMLDOMNode *element = NULL;
	va_list attr;
	char *attr_name = NULL;
	char *attr_value = NULL;
	int i = 0;

	if ((element = sg_dom_create_elementn(element_name)) == NULL) {
		return NULL;
	}
	va_start(attr, attr_num);
	for (i = 0; i < attr_num; i++) {

		attr_name = va_arg(attr, char*);
		if (attr_name == NULL) {
			goto err;
		}
		attr_value = va_arg(attr, char*);

		if (sg_append_attr(element, attr_name, attr_value) == NULL) {
			goto err;
		}
	}
	va_end(attr);

	return element;
err:
	va_end(attr);
	sg_dom_release_noden(element);
	return NULL;
}

/*
 * create ele_num child element node one time, return created element.
 * ... ,give me element node names.
 *  use sg_create_element_node_ex("element", 1, "hello", "hello_val")
 *  create
 *   <element>
 *    <hello>hello_val</hello>
 *   </element>
 *
 *   use sg_create_element_node_ex("element", 2, "hello1", "hello1_val", "hello2", "hello2_val")
 *  create
 *   <element>
 *    <hello1>hello1_val</hello>
 *    <hello2>hello2_val</hello>
 *   <element>
 */
XMLDOMNode *sg_create_element_node_ex(char *element_name, int num, ...)
{
	XMLDOMNode *element = NULL;
	va_list attr;
	char *name = NULL;
	char *value = NULL;
	int i = 0;

	if ((element = sg_dom_create_elementn(element_name)) == NULL) {
		return NULL;
	}
	va_start(attr, num);
	for (i = 0; i < num; i++) {

		name = va_arg(attr, char*);
		if (name == NULL) {
			goto err;
		}
		value = va_arg(attr, char*);
		if (value == NULL) {
			value = "";
		}

		if (sg_append_element(element, name, value) == NULL) {
			goto err;
		}
	}
	va_end(attr);

	return element;
err:
	va_end(attr);
	sg_dom_release_noden(element);
	return NULL;
}

/*
 * Addr/Proto List operate lib
 *  sg_create_child_std("IP", "10.1.1.1-10.1.2.1")
 *   create <IP Start="10.1.1.1" End="10.1.2.1"/>
 *
 *  sg_create_child_std("IP", "10.1.1.1")
 *   create <IP Start="10.1.1.1"/>
 */
XMLDOMNode *sg_create_child_std(char *child_name, char *value)
{
	char *start = value;
	char *end = value;

	if (start == NULL) {
		return NULL;
	}
	while(end[0] != '\0') {
		if (end[0] == '-') {
			end[0] = '\0';
			end++;
			break;
		}
		end++;
	}
	if (end[0] == '\0') {
		return sg_create_attr_node(child_name, 1, "Start", start);
	} else {
		return sg_create_attr_node(child_name, 2, "Start", start, "End", end);
	}
}

/*
 * We call a list is a XML NODE like
 * <Lst>
 *      <IP Start="start_val1" End="end_val1"/>
 *      <IP Start="start_val2" End="end_val2"/>
 *      ... ...
 * </Lst>
 *
 * Function sg_append_list_ex receive arg of child_name just like
 *    "IP" listed. so , if we want to create that Lst, we call this
 *    function with args:
 *   sg_append_list_ex(list, "IP", "start_val1-end_val1,start_val2-end_val2", 0)
 *
 * arg Num used to create special list form like
 * <Lst>
 *      <IP Start="start_val1" End="end_val1" Special_node="Special_val"/>
 *      <IP Start="start_val2" End="end_val2" Special_node="Special_val"/>
 *      ... ...
 * </Lst>
 * 	We create this list use:
 * 	sg_append_list_ex(list, "IP", "start_val1-end_val1,start_val2-end_val2", 1,
 * 	                          "Special_node", "Special_val")
 *
 */
#ifndef sg_append_list
#define sg_append_list(List, ChildName, ValueList)\
	sg_append_list_ex(List, ChildName, ValueList, 0)
#endif
XMLDOMNode *sg_append_list_ex(XMLDOMNode *list, char *child_name,
		char *value_list, int num, ...)
{
	XMLDOMNode *child_element = NULL;

	va_list attr;

	char values[IPV4LISTLENGTH];
	char *start = NULL;
	char *end = NULL;
	char *attr_name[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
	char *attr_val[6] = {NULL, NULL, NULL, NULL, NULL, NULL};

	int i = 0;

	va_start(attr, num);
	for (i = 0; i < num; i++) {
		attr_name[i] = va_arg(attr, char*);
		if (attr_name[i] == NULL) {
			va_end(attr);
			return NULL;
		}
		attr_val[i] = va_arg(attr, char*);
		if (attr_val[i] == NULL) {
			attr_val[i] = "";
		}
	}
	va_end(attr);

	if (list == NULL || child_name == NULL || value_list == NULL) {
		return NULL;
	}

	memset(values, 0, IPV4LISTLENGTH);
	if (strlen(value_list) > (IPV4LISTLENGTH - 1)) {
		goto err;
	}
	memcpy(values, value_list, strlen(value_list) + 1);

	start = values;
	end = values;
	while (end[0] != 0) {
		if (end[0] == ',') {
			end[0] = '\0';
			if ((child_element = sg_create_child_std(child_name, start)) == NULL) {
				goto err;
			}
			if (sg_dom_add_childnode(list, child_element) == FALSE) {
				goto err;
			}
			for (i = 0; i < num; i ++) {
				if (sg_append_attr(child_element, attr_name[i], attr_val[i]) == NULL) {
					goto err;
				}
			}
			start = end + 1;
		}
		end++;
	}
	if ((child_element = sg_create_child_std(child_name, start)) == NULL) {
		goto err;
	}
	if (sg_dom_add_childnode(list, child_element) == FALSE) {
		goto err;
	}
	for (i = 0; i < num; i ++) {
		if (sg_append_attr(child_element, attr_name[i], attr_val[i]) == NULL) {
			goto err;
		}
	}

	return list;
err:
	if (child_element != NULL) {
		sg_dom_release_noden(child_element);
	}
	return NULL;
}

/*
 * Create a list node
 *  use sg_create_list("Sip", "IP", "10.1.1.1,10.1.2.1-10.1.3.1", 1)
 *  <Sip Any="false">
 *  	<IP Start="10.1.1.1"/>
 *  	<IP Start="10.1.2.1" End="10.1.3.1"/>
 *  </Sip>
 *  when flg == 1, we create Any node, if flg == 0
 *  we just create
 *  <Sip>
 *  	<IP Start="10.1.1.1"/>
 *  	<IP Start="10.1.2.1" End="10.1.3.1"/>
 *  </Sip>
 *  without Any node
 *
 *  Arg of num used to create special attr node, same with function
 *   sg_append_list_ex()
 */
#ifndef sg_create_list
#define sg_create_list(ElementName, ChildName, ValueList, Flg)\
	sg_create_list_ex(ElementName, ChildName, ValueList, Flg, 0)
#endif
XMLDOMNode *sg_create_list_ex(char *ele_name, char *child_name,
		char *value_list, int flg, int num, ...)
{
	XMLDOMNode *element = NULL;
	XMLDOMNode *child_element = NULL;

	va_list attr;

	char values[IPV4LISTLENGTH];
	char *start = NULL;
	char *end = NULL;
	char *attr_name[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
	char *attr_val[6] = {NULL, NULL, NULL, NULL, NULL, NULL};

	int i = 0;

	va_start(attr, num);
	for (i = 0; i < num; i++) {
		attr_name[i] = va_arg(attr, char*);
		if (attr_name[i] == NULL) {
			XML_DEBUG( TAG, "\n" );
			va_end(attr);
			return NULL;
		}
		attr_val[i] = va_arg(attr, char*);
		if (attr_val[i] == NULL) {
			attr_val[i] = "";
		}
	}
	va_end(attr);

	if (ele_name == NULL || child_name == NULL
			|| (flg != 0 && flg != 1)) {
		XML_DEBUG( TAG, "\n" );
		return NULL;
	}
	//if (strstr(value_list, XML_ANY) != NULL) {
	if (value_list == NULL
			|| strncmp(value_list, XML_ANY, strlen(value_list)) == 0) {
		element = sg_create_attr_node(ele_name, 1, "Any", "true");
		return element;
	} else if (flg == 1) {
		if ((element = sg_create_attr_node(ele_name, 1,
						"Any", "false")) == NULL) {
			XML_DEBUG( TAG, "\n" );
			return NULL;
		}
	} else if (flg == 0) {
		if ((element = sg_create_attr_node(ele_name, 0)) == NULL) {
			XML_DEBUG( TAG, "\n" );
			return NULL;
		}
	}

	if (value_list == NULL) {
		XML_DEBUG( TAG, "\n" );
		return element;
	}
	memset(values, 0, IPV4LISTLENGTH);
	if (strlen(value_list) > (IPV4LISTLENGTH - 1)) {
		XML_DEBUG( TAG, "\n" );
		goto err;
	}
	memcpy(values, value_list, strlen(value_list) + 1);

	start = values;
	end = values;
	while (end[0] != 0) {
		if (end[0] == ',') {
			end[0] = '\0';
			if ((child_element = sg_create_child_std(child_name, start)) == NULL) {
				XML_DEBUG( TAG, "\n" );
				goto err;
			}
			if (sg_dom_add_childnode(element, child_element) == FALSE) {
				XML_DEBUG( TAG, "\n" );
				goto err;
			}
			for (i = 0; i < num; i ++) {
				if (sg_append_attr(child_element, attr_name[i], attr_val[i]) == NULL) {
					XML_DEBUG( TAG, "\n" );
					goto err;
				}
			}
			start = end + 1;
		}
		end++;
	}
	if ((child_element = sg_create_child_std(child_name, start)) == NULL) {
		XML_DEBUG( TAG, "\n" );
		goto err;
	}
	if (sg_dom_add_childnode(element, child_element) == FALSE) {
		XML_DEBUG( TAG, "\n" );
		goto err;
	}
	for (i = 0; i < num; i ++) {
		if (sg_append_attr(child_element, attr_name[i], attr_val[i]) == NULL) {
			XML_DEBUG( TAG, "\n" );
			goto err;
		}
	}

	return element;
err:
	if (element != NULL) {
		sg_dom_release_noden(element);
	}
	if (child_element != NULL) {
		sg_dom_release_noden(child_element);
	}
	return NULL;
}

/*
 * Find child node from a node list.
 *  search
 * 	<list>
 * 		<user id='1'/>
 * 		<user id='2'/>
 * 		<user id='3'/>
 * 	</list>
 * 	We can use sg_find_node(list, "user", "id", "2")
 *
 * 	search
 * 	 <list>
 * 	    <user>
 * 	      <id>2</id>
 * 	    </user>
 * 	 </list>
 * 	We can use sg_find_node(list, "user", "id", "2")
 * 	to get node
 * 	            <user>
 * 	               <id>2</id>
 * 	            </user>
 *
 * 	 search
 * 	 	<list>
 * 	 		<user>1</user>
 * 	 		<user>2</user>
 * 	 		<user>3</user>
 * 	 	</list>
 * 	 We can use sg_find_node(list, "user", ".", "2")
 * 	 to get node <user>2</user>
 *
 *   /// extend function use va_list, more than one match condition.
 *   example
 *     search
 *         <list>
 *              <user id="1">tangxg</user>
 *              <user id="2">duhk</user>
 *         </list>
 *       We can use sg_find_node(list, "user", 2, ".", "duhk", "id", "2")
 *       to get node <user id="2">duhk</user>
 */
#ifndef sg_find_rule
#define sg_find_node(List, ChildName, AttrName, Attr_val)\
	sg_find_node_ex(List, ChildName, 1, AttrName, Attr_val)
#define sg_find_rule(List, AttrVal)\
	sg_find_node(List, "Item", "ID", AttrVal)
#endif
XMLDOMNode *sg_find_node_ex(XMLDOMNode *list, char *child_name, int num, ...)
{
	XMLDOMNode *child_node = NULL;
	XMLDOMNode *attr_node = NULL;
	XMLDOMNode *text_node = NULL;
	char *attr_name[6] = {NULL,};
	char *attr_val[6] = {NULL,};
	char *val = NULL;
	va_list attr_list;
	int i = 0;

	if (list == NULL || num > 6 || num < 0 || child_name == NULL) {
		return NULL;
	}

	va_start(attr_list, num);
	for (i = 0; i < num; i ++) {
		attr_name[i] = va_arg(attr_list, char*);
		if (attr_name[i] == NULL) {
			va_end(attr_list);
			return NULL;
		}
		attr_val[i] = va_arg(attr_list, char*);
		if (attr_val[i] == NULL) {
			attr_val[i] = "";
		}
	}
	va_end(attr_list);

	child_node = sg_dom_get_xmlnode(list, child_name);
	while(child_node != NULL) {
		for (i = 0; i < num; i ++) {
			attr_node = sg_dom_get_xmlnode(child_node, attr_name[i]);
			if (attr_node == NULL) {
				break;
			}
			if (sg_dom_isattribute(attr_node) == FALSE) {
				text_node = sg_dom_getchild_textn(attr_node);
				val = sg_dom_nodetext(text_node);
			} else {
				val = sg_dom_nodetext(attr_node);
			}
			lTrimStr( val );
			rTrimStr( val );
			if (val != NULL && strcmp(val, attr_val[i]) == 0) {
				if (i == num - 1) {
					return child_node;
				} else {
					continue;
				}
			} else {
				break;
			}
		}
		child_node = sg_dom_getnext_elementn(child_node, child_name);
	}

	return NULL;
}

/*
 * We want move match child node to seq, use this function
 *  example:
 *   <user_list>
 *     <user id="1">tangxg</user>
 *     <user id="2">duhk</user>
 *     <user id="3">tangxg1</user>
 *   </user_list>
 *
 *   use sg_move_item(list, "user", "id", "2", 3) or
 *       sg_move_item(list, "user", ".", "duhk", 3)
 *   to get:
 *
 *   <user_list>
 *     <user id="1">tangxg</user>
 *     <user id="3">tangxg1</user>
 *     <user id="2">duhk</user>
 *   </user_list>
 *
 */

/*
 * Add node according to the sequence
 * Return: 0 success, else failure
 */
int sg_add_noden( int seq, XMLDOMNode *root, XMLDOMNode *tmp_node )
{
	XMLDOMNode *tmp_node1 = NULL;
	char *name = NULL;

	int ret = 0;
	int i = 1;

	if (seq == 0) {
		seq = 1;
	}

	name = sg_dom_nodename( tmp_node );
	for( tmp_node1 = sg_dom_getchild_elementn( root, name );
			tmp_node1 != NULL;
			tmp_node1 = sg_dom_getnext_elementn( tmp_node1, name ) ){
		if (i == seq) {
			break;
		}
		i++;
	}

	//XML_DEBUG( TAG, "i[%d], seq[%d], name[%s]\n", i, seq, name );

	if (tmp_node1 == NULL) {
		if (sg_dom_add_childnode(root, tmp_node) == NULL) {
			ret = -1;
			goto err;
		}
	} else {
		if (sg_dom_add_prevnode(tmp_node1, tmp_node) == FALSE) {
			ret = -1;
			goto err;
		}
	}

err:
	return ret;
}

#ifndef sg_move_rule
#define sg_move_rule(Lst, RuleId, Seq)\
	sg_move_item(Lst, "Item", "ID", RuleId, Seq)
#endif
XMLDOMNode *sg_move_item(XMLDOMNode *list, char *child_name,
		char *attr_name, char *item_id, int seq)
{
	XMLDOMNode *item = NULL;
	XMLDOMNode *item1 = NULL;

	item = sg_find_node(list, child_name, attr_name, item_id);
	if (item == NULL) {
		return NULL;
	}
	if ((item1 = sg_dom_duplicate_node(item, NULL)) == NULL) {
		return NULL;
	}
	if (sg_dom_remove_node(item) == FALSE){
		goto err;
	}
	if (sg_add_noden(seq, list, item1) != 0) {
		goto err;
	}
	return item1;
err:
	if (item1 != NULL) {
		sg_dom_release_noden(item1);
	}
	return NULL;
}

/*
 * Path, Insist, you'd better use just one level path, error may occur
 * 	when use muti level path and with a flag 1 or 2.
 * if node/path == NULL
 *   flag == 0 return -1;
 *   flag == 1 create path="value"
 *   flag == 2 create <path>value</path>
 *   flag == 3 just return 0
 *
 *	<node attr1="hello"/>
 *	 use sg_set_node_val(node, "attr", "goodbye");
 *	 get: <node attr1="goodbye"/>
 *
 *	<node/>
 *	 when use sg_set_node_val(node, "attr", "goodbye") it
 *	return a error code.
 *	 we can use sg_set_node_val_ex(node, "attr", "goodbye", 1) to
 *	change <node/> to <node attr="goodbye"/>
 *	 or use sg_set_node_val_ex(node, "attr", "goodbye", 2) to
 *	change <node/> to <node> <attr>goodbye</attr></node>
 *	 or use sg_set_node_val_ex(node, "attr", "goodbye", 3) to
 *	leave <node/>, and return 0.
 *
 */
#ifndef sg_set_node_val
#define XML_SET_ERROR_MOD      0   //default
#define XML_SET_CREATE_ATTR    1
#define XML_SET_CREATE_ELEMENT 2
#define XML_SET_ZERO_MOD       3
#define sg_set_node_val(Node, Path, Value) \
	sg_set_node_val_ex(Node, XML_SET_ERROR_MOD, 1, Path, Value)
#endif
int sg_set_node_val_ex(XMLDOMNode *node, int flg, int num, ...)
{
	XMLDOMNode *child_node = NULL;
	XMLDOMNode *text_node = NULL;

	va_list attr;

	char *path = NULL;
	char *value = NULL;

	int i = 0;
	int ret = 0;

	if (node == NULL) {
		return -1;
	}
	va_start(attr, num);
	for (i = 0; i < num; i++) {
		path = va_arg(attr, char *);
		if (path == NULL) {
			ret =  -1;
			goto err;
		}
		value = va_arg(attr, char *);
		if (value == NULL) {
			value = "";
		}
		child_node = sg_dom_get_xmlnode(node, path);
		if (child_node == NULL) {
			if (flg == 1) { // create attr node
				if ((child_node = sg_append_attr(node, path, value)) == NULL) {
					ret =  -1;
					goto err;
				}
				continue;
			} else if (flg == 2) { // create ele node
				if ((child_node = sg_create_element_node(path, value)) == NULL) {
					ret = -1;
					goto err;
				}
				if (sg_dom_add_childnode(node, child_node) == FALSE) {
					sg_dom_release_noden(child_node);
					ret = -1;
					goto err;
				}
				continue;
			} else if (flg == 3) {
				continue;
			} else {
				ret = -1;
				goto err;
			}
		}

		text_node = child_node;
		if (sg_dom_isattribute(child_node) == FALSE) {
			text_node = sg_dom_getchild_textn(child_node);
			if (sg_dom_istext(text_node) == FALSE) {
				ret = -1;
				goto err;
			}
		}
		if (sg_dom_set_xmlnode(text_node, ".", value) == NULL) {
			ret = -1;
			goto err;
		}
	}
	va_end(attr);

	return 0;
err:
	va_end(attr);
	return ret;
}

/*
 * Delete
 * 	return 0: succeed
 * 	   other: error code.
 *	<a b="b"/> or
 *	<a>
 *	 <b/>
 *	</a>
 *	use sg_del_child_node(list, "b");
 *
 * 	<a>
 * 	 <b c="a"/b>
 * 	 <b c="b"/b>
 * 	 <b c="c"/b>
 * 	</a> or
 * 	<a>
 * 	 <b>a</b>
 * 	 <b>b</b>
 * 	 <b>c</b>
 * 	</a>
 * 	use sg_del_child_node_ex(list, "b", ".", "b");
 * 	<a>
 * 	 <b>
 * 	  <c>c</c>
 * 	 </b>
 * 	 <b>
 * 	  <c>d</c>
 * 	 </b>
 * 	</a>
 * 	use sg_del_child_node_ex(list, "b", "c", "d"); to del the second <b>.
 */
#ifndef sg_del_child_node
#define sg_del_child_node(List, Child)\
	sg_del_child_node_ex(List, Child, NULL, NULL)
#endif
int sg_del_child_node_ex(XMLDOMNode *list, char *child_item,
		char *attr_name, char *attr_val)
{
	XMLDOMNode *child_node = NULL;

	if (list == NULL || child_item == NULL) {
		return -1;
	}
	if (attr_name == NULL && attr_val == NULL) {
		if ((child_node = sg_dom_get_xmlnode(list, child_item)) == NULL) {
			return -1;
		}
	} else if (attr_name != NULL && attr_val != NULL) {
		child_node = sg_find_node(list, child_item, attr_name, attr_val);
		if (child_node == NULL) {
			return -1;
		}
	} else {
		return -1;
	}
	if (sg_dom_remove_node(child_node) == FALSE){
		return -1;
	}
	return 0;
}

/*
 * Get node value.
 *  <node path="val"/>
 *  or
 *  <node>
 *  	<path>val</path>
 *  </node>
 *  Path can be muti level.
 * or <node>val</node>
 * Path is "."
 */
char *sg_get_node_val(XMLDOMNode *node, char *path)
{
	XMLDOMNode *child_node = NULL;
	XMLDOMNode *text_node = NULL;

	if ((child_node = sg_dom_get_xmlnode(node, path)) == NULL) {
		return NULL;
	}
	if (sg_dom_isattribute(child_node) == TRUE) {
		return sg_dom_nodetext(child_node);
	} else {
		if ((text_node = sg_dom_getchild_textn(child_node)) == NULL) {
			return NULL;
		}
		if (sg_dom_istext(text_node) == TRUE) {
			return sg_dom_nodetext(text_node);
		}
	}

	return NULL;
}

#ifdef __cplusplus
}
#endif

