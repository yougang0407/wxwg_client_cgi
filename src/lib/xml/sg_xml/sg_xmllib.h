/******************************************************************************
	Copyright (C), 2012-2020, legendsec Tech. Co., Ltd.
	File name:  sg_xmllib.h
	Author: liyz
	Version:1.0
	Date:2012-03-06
	Description: declare xml functions.

	Others:
	Function List:

	History:
		1.  Date:2012-03-06
			Author:liyz
			Modification:create sg_xmllib.h add fnctions

 ******************************************************************************/

#ifndef __XML_XMLLIB_H__
#define __XML_XMLLIB_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sg_domtree.h"
#include "sg_domapi.h"

/*********ctld struct end******/
#define XML_ANY "Any"
#define XML_DEFAULT "Default"
#define XML_PERMIT "permit"
#define XML_DENY "deny"
#define XML_AUTH "auth"
#define XML_TRUE "true"
#define XML_FALSE "false"
#define XML_YES "yes"
#define XML_NO "no"
#define XML_ENABLE "enable"
#define XML_DISABLE "disable"

#define IPV4LISTLENGTH 1280


extern XMLDOMNode *sg_create_element_node(char *ele_name, char *ele_textn);
extern int sg_write_xmldata_to_file(char *temp_path, XMLDOMDocument *doc);

extern char* lTrimStr( char *Str );
extern char* rTrimStr( char *Str );

#ifndef sg_append_attr
#define sg_append_attr(Element, AttrName, AttrVal)\
    sg_append_attr_ex(Element, 1, AttrName, AttrVal)
#endif
XMLDOMNode *sg_append_attr_ex(XMLDOMNode *element, int attr_num, ...);

#ifndef sg_append_element
#define sg_append_element(Element, NodeName, NodeVal)\
    sg_append_element_ex(Element, 1, NodeName, NodeVal)
#endif
XMLDOMNode *sg_append_element_ex(XMLDOMNode *element, int ele_num, ...);

#ifndef sg_append_node
#define sg_append_node(Element, ChildElement) \
    sg_append_node_ex(Element, 1, ChildElement)
#endif
XMLDOMNode *sg_append_node_ex(XMLDOMNode *element, int ele_num, ...);

XMLDOMNode *sg_create_attr_node(char *element_name, int attr_num, ...);

XMLDOMNode *sg_create_element_node_ex(char *element_name, int num, ...);

XMLDOMNode *sg_create_child_std(char *child_name, char *value);
#ifndef sg_append_list
#define sg_append_list(List, ChildName, ValueList)\
    sg_append_list_ex(List, ChildName, ValueList, 0)
#endif
XMLDOMNode *sg_append_list_ex(XMLDOMNode *list, char *child_name, char *value_list, int num, ...);

#ifndef sg_create_list
#define sg_create_list(ElementName, ChildName, ValueList, Flg)\
	    sg_create_list_ex(ElementName, ChildName, ValueList, Flg, 0)
#endif
XMLDOMNode *sg_create_list_ex(char *ele_name, char *child_name, char *value_list, int flg, int num, ...);

#ifndef sg_find_rule
#define sg_find_node(List, ChildName, AttrName, Attr_val)\
	sg_find_node_ex(List, ChildName, 1, AttrName, Attr_val)
#define sg_find_rule(List, AttrVal)\
	sg_find_node(List, "Item", "ID", AttrVal)
#endif
XMLDOMNode *sg_find_node_ex(XMLDOMNode *list, char *child_name, int num, ...);

int sg_add_noden( int seq, XMLDOMNode *root, XMLDOMNode *tmp_node );

#ifndef sg_move_rule
#define sg_move_rule(Lst, RuleId, Seq)\
	sg_move_item(Lst, "Item", "ID", RuleId, Seq)
#endif
XMLDOMNode *sg_move_item(XMLDOMNode *list, char *child_name,char *attr_name, char *item_id, int seq);

#ifndef sg_set_node_val
#define XML_SET_ERROR_MOD		0	//default
#define XML_SET_CREATE_ATTR	1
#define XML_SET_CREATE_ELEMENT	2
#define XML_SET_ZERO_MOD		3
#define sg_set_node_val(Node, Path, Value) \
    sg_set_node_val_ex(Node, XML_SET_ERROR_MOD, 1, Path, Value)
#endif
int sg_set_node_val_ex(XMLDOMNode *node, int flg, int num, ...);

#ifndef sg_del_child_node
#define sg_del_child_node(List, Child)\
    sg_del_child_node_ex(List, Child, NULL, NULL)
#endif
int sg_del_child_node_ex(XMLDOMNode *list, char *child_item, char *attr_name, char *attr_val);

char *sg_get_node_val(XMLDOMNode *node, char *path);

#endif
