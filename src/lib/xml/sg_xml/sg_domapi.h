/******************************************************************************
	Copyright (C), 2012-2020, legendsec Tech. Co., Ltd.
	File name:  sg_domapi.h
	Author: liyz
	Version:1.0
	Date:2012-03-06
	Description:  DOM parser api of XML 

	Others:
	Function List:

	History:    
		1.  Date:2012-03-06
			Author:liyz
			Modification:create sg_domapi.h add fnctions

 ******************************************************************************/

#ifndef __SG_DOM_H__
#define __SG_DOM_H__
#include "sg_domtree.h"
#ifdef __cplusplus
extern "C" {
#endif

extern int __sg_xml_get_num__;
XMLDOMNode* get_root(XMLDOMDocument* doc);
int xml_get_num(void);
void free_xml(char *xml);
char* get_xml(XMLDOMNode* n);
void free_docxml(char *docxml);
char* get_docxml(XMLDOMDocument* doc);
extern char *dom_proc_escape(char *text);
void* CreateObject(char* obj);
void ReleaseObject(char* type, XMLDOMDocument** obj);

XMLDOMNode* create_raw_node(int type, char* name, char* value);
BOOL release_raw_node(XMLDOMNode** n);

/* search child node */
XMLDOMNode* get_raw_child(int type, XMLDOMNode* p, const char* value);
XMLDOMNode* get_raw_next(int type, XMLDOMNode* p, const char* value);

/* recursive search */
XMLDOMNode* search_raw_node(int type, XMLDOMNode* p, const char* n);
XMLDOMNode* search_raw_next(int type, XMLDOMNode* p, const char* n, \
		XMLDOMNode* stop);

/* dtd manipulate */
int xml_check_dtd(XMLDOMDocument* doc);
int xml_check_with_dtd(XMLDOMDocument* doc, const char* url);
int xml_set_dtd(XMLDOMDocument* doc, 
				const char* root,const char* type,
				const char* publidID,const char* url,
				const char* dtd);

void get_versioninfo(unsigned char **pversion,
					unsigned char **plastupdate,
					unsigned char **psignature,
					unsigned char **pproduct,
					unsigned char **changelog);

XMLDOMNode* getXmlNode(XMLDOMNode* cur, 
						const char* path);
XMLDOMNode* setXmlNode(XMLDOMNode* cur,
						const char* path,
						char* text);


char *dom_get_nodetext_escape(XMLDOMNode *text_node);
void dom_smart_adjust(unsigned char flg );

#ifdef __cplusplus
}
#endif

#define sg_dom_isnull(n) ((n) == NULL)

/* node type decision */
#define sg_dom_iselement(n) ((n)->nodeType == NODE_ELEMENT)
#define sg_dom_isattribute(n) ((n)->nodeType == NODE_ATTRIBUTE)
#define sg_dom_istext(n) ((n)->nodeType == NODE_TEXT)
#define sg_dom_iscdata(n) ((n)->nodeType == NODE_CDATA_SECTION)
#define sg_dom_iscomment(n) ((n)->nodeType == NODE_COMMENT)

/* child node operation */
#define sg_dom_parentnode(n) ((n)->parentNode)
#define sg_dom_haschild(n) ((n)->hasChildNodes && (n)->childNodes)
#define sg_dom_firstchild(n) ((n)->get_firstChild(n))
#define sg_dom_lastchild(n) ((n)->get_lastChild(n))
#define sg_dom_prevsibling(n) ((n)->get_previousSibling(n))
#define sg_dom_nextsibling(n) ((n)->get_nextSibling(n))

/* other node operation */
#define sg_dom_nodename(n) ((n)->nodeName)

#define sg_dom_nodetext(n) (dom_get_nodetext_escape(n))
//#define sg_dom_nodetext(n) ((n)->text)

#define sg_dom_nodedocument(n) ((n)->ownerDocument)
#define sg_dom_rootdocument(n) ((n)->documentElement)
#define sg_dom_rootnode(n) (get_root(sg_dom_nodedocument(n)))
#define sg_dom_rootdocnode(n) (get_root(n))

/* recursive outputting tree */
#define sg_dom_get_root( docx )  get_root(docx )
#define sg_dom_xml_get_num() xml_get_num()
#define sg_dom_get_xml( nodex ) get_xml( nodex )
#define sg_dom_free_xml( xml ) free_xml( xml )
#define sg_dom_get_docxml( docx ) get_docxml( docx )
#define sg_dom_free_docxml( docxml ) free_docxml( docxml )

/* added for SoftDev */
#define sg_dom_smart_adjust( flgx ) dom_smart_adjust( flgx )

/* node operation */
#define sg_dom_add_childnode(n,m) ((XMLDOMNode*) (n)->appendChild((n),(m)))
#define sg_dom_add_prevnode(n,m) ((BOOL) (n)->insertBefore((n),(m)))
#define sg_dom_remove_node(n) ((BOOL) (n)->removeChild(n))
#define sg_dom_replace_node(n,m) ((BOOL) (n)->replaceChild((&n),(m)))
#define sg_dom_duplicate_node(n,p) ((XMLDOMNode*) (n)->duplicateChild((n),(p)))

/* create xmlDOM */
#define sg_dom_xmldom_create() ((XMLDOMDocument*) CreateObject("FOOLFISH.XMLDOM"))
#define sg_dom_xmldom_release(doc) ReleaseObject("FOOLFISH.XMLDOM",&(doc))
//#define sg_dom_xmldom_release(doc) ReleaseObject("FOOLFISH.XMLDOM",(void**)&(doc))
//modify the arg2's type from void** to XMLDOMDocument** 
//for gcc version 3.3.4

#define sg_dom_create_raw_node( typex, namex, valuex ) \
    create_raw_node( typex, namex, valuex )
#define sg_dom_release_raw_node( nodex ) \
    release_raw_node( nodex )

#define sg_dom_release_noden(n) release_raw_node(&n)
#define sg_dom_create_elementn(n) create_raw_node(NODE_ELEMENT,n,NULL)
#define sg_dom_create_attributen(n,v) create_raw_node(NODE_ATTRIBUTE,n,v)
#define sg_dom_create_textn(v) create_raw_node(NODE_TEXT,NULL,v)
#define sg_dom_create_cdatan(v) create_raw_node(NODE_CDATA_SECTION,NULL,v)
#define sg_dom_create_commentn(v) create_raw_node(NODE_COMMENT,NULL,v)

#define sg_dom_get_raw_child( typex, px, valuex ) \
    get_raw_child( typex, px, valuex )
#define sg_dom_get_raw_next( typex, px, valuex ) \
    get_raw_next( typex, px, valuex )

#define sg_dom_getchild_elementn(p,n) get_raw_child(NODE_ELEMENT, (p), (n))
#define sg_dom_getchild_attributen(p) get_raw_child(NODE_ATTRIBUTE, (p), NULL)
#define sg_dom_getchild_textn(p) get_raw_child(NODE_TEXT, (p), NULL)
#define sg_dom_getchild_cdatan(p) get_raw_child(NODE_CDATA_SECTION, (p), NULL)
#define sg_dom_getchild_commentn(p) get_raw_child(NODE_COMMENT, (p), NULL)

#define sg_dom_getnext_elementn(c,n) get_raw_next(NODE_ELEMENT,(c), (n))
#define sg_dom_getnext_attributen(c) get_raw_next(NODE_ATTRIBUTE,(c), NULL)
#define sg_dom_getnext_textn(c) get_raw_next(NODE_TEXT,(c), NULL)
#define sg_dom_getnext_cdatan(c) get_raw_next(NODE_CDATA_SECTION,(c), NULL)
#define sg_dom_getnext_commentn(c) get_raw_next(NODE_COMMENT,(c), NULL)

#define sg_dom_search_raw_node( typex, px, nx ) \
    search_raw_node( typex, px, nx )
#define sg_dom_search_raw_next( typex, px, nx, stopx ) \
    search_raw_next( typex, px, nx, stopx )

#define sg_dom_searchfirst_elementn(p, n) search_raw_node(NODE_ELEMENT, p, n)
#define sg_dom_searchfirst_attributen(p) search_raw_node(NODE_ATTRIBUTE, p, NULL)
#define sg_dom_searchfirst_textn(p) search_raw_node(NODE_TEXT, p, NULL)
#define sg_dom_searchfirst_cdatan(p) search_raw_node(NODE_CDATA_SECTION, p, NULL)
#define sg_dom_searchfirst_commentn(p) search_raw_node(NODE_COMMNET, p, NULL)
#define sg_dom_searchnext_elementn(p, n, stop) search_raw_next(NODE_ELEMENT, p, n, stop)
#define sg_dom_searchnext_attributen(p, stop) search_raw_next(NODE_ATTRIBUTE, p, NULL, stop)
#define sg_dom_searchnext_textn(p, stop) search_raw_next(NODE_TEXT, p, NULL, stop)
#define sg_dom_searchnext_cdatan(p, stop) search_raw_next(NODE_CDATA_SECTION, p, NULL, stop)
#define sg_dom_searchnext_commentn(p, stop) search_raw_next(NODE_COMMENT, p, NULL, stop)

#define sg_dom_xml_check_dtd( docx ) \
    xml_check_dtd( docx )
#define sg_dom_xml_check_with_dtd( docx, urlx ) \
    xml_check_with_dtd( docx, urlx )
#define sg_dom_xml_set_dtd( docx,rootx, typex, publicIDx, urlx, dtdx ) \
    xml_set_dtd( docx,rootx, typex, publicIDx, urlx, dtdx );

#define sg_dom_get_versioninfo( pversionx, plastupdatex, psignaturex, pproductx, changelogx ) \
    getversioninfo( pversionx, plastupdatex, psignaturex, pproductx, changelogx )

XMLDOMNode* get_xmlnode(XMLDOMNode* cur, const char* path);
XMLDOMNode* set_xmlnode(XMLDOMNode* cur, const char* path, char* text);
#define sg_dom_get_xmlnode( curx, pathx ) \
    getXmlNode( curx, pathx )
#define sg_dom_set_xmlnode( curx, pathx, textx ) \
    setXmlNode( curx, pathx, textx )

#endif
