/******************************************************************************
	Copyright (C), 2012-2020, legendsec Tech. Co., Ltd.
	File name:  sg_domtree.h
	Author: liyz
	Version:1.0
	Date:2012-03-06
	Description: define DOM tree struct 

	Others:
	Function List:

	History:    
		1.  Date:2012-03-06
			Author:liyz
			Modification:create sg_domtree.h add fnctions

 ******************************************************************************/

#ifndef __SG_DOMTREE_H__
#define __SG_DOMTREE_H__

#ifndef TEST
#define TEST()\
	fprintf(stderr,"%s[%d]\n",__FILE__,__LINE__)
#endif

#ifdef WIN32
typedef enum {
	FALSE 	= 0 ,
	TRUE 	= !FALSE
} BOOL;
#else
typedef int BOOL;
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#endif

typedef struct {
	long	errorCode;
	long	filepos;
	long	line;
	long	linepos;
	const char*	reason;
	char*	(*get_srcText)(void);
} XMLDOMParseError;

typedef enum {
	NODE_ELEMENT 					= 0,
	NODE_ATTRIBUTE  				= 1+NODE_ELEMENT,
	NODE_TEXT 					= 1+NODE_ATTRIBUTE,
	NODE_CDATA_SECTION 				= 1+NODE_TEXT,
	NODE_ENTITY_REFERENCE 				= 1+NODE_CDATA_SECTION,
	NODE_ENTITY 					= 1+NODE_ENTITY_REFERENCE,
	NODE_PROCESSING_INSTRUCTION 			= 1+NODE_ENTITY,
	NODE_COMMENT 					= 1+NODE_PROCESSING_INSTRUCTION,
	NODE_DOCUMENT 					= 1+NODE_COMMENT,
	NODE_DOCUMENT_TYPE 				= 1+NODE_DOCUMENT,
	NODE_DOCUMENT_FRAGMENT 				= 1+NODE_DOCUMENT_TYPE,
	NODE_NOTATION 					= 1+NODE_DOCUMENT_FRAGMENT
} XMLDOMNodeType;

typedef struct XMLDOMNode XMLDOMNode;
typedef struct XMLDOMNodeList XMLDOMNodeList;
typedef struct XMLDOMPInstruction XMLDOMPInstruction;
typedef struct XMLDOMDocument XMLDOMDocument;
typedef struct XMLDOMDocumentType XMLDOMDocumentType; 


struct XMLDOMNode {
	/* 获得接点信息*/
	BOOL	hasChildNodes;
	char*	namespaceURI;
	char*	nodeName;
	XMLDOMNodeType 	nodeType;
	char*	nodeTypeString;
	
	/*Indicates whether the node (usually an attribute) is
	  explicitly specified or derived from a default value in 
	  the document type definition (DTD) or schema. 
	  Boolean. The property is read-only. Returns True if 
	  the attribute is explicitly specified in the element. 
	  Returns False if the attribute value comes from the 
	  DTD or schema. (other types nodes return True) */
	BOOL	specified;
	
	BOOL	(*get_xml)(XMLDOMNode* node,char** ps);

	/* 取得和设置接点值*/
	char*	text;

	XMLDOMNode*		parentNode;
	XMLDOMNodeList*	childNodes;
	XMLDOMNode*		(*get_firstChild)(XMLDOMNode* node);
	XMLDOMNode*		(*get_lastChild)(XMLDOMNode* node );
	XMLDOMNode*		(*get_previousSibling)(XMLDOMNode* node);
	XMLDOMNode*		(*get_nextSibling)(XMLDOMNode* node);
	XMLDOMDocument*	ownerDocument;

	XMLDOMNode*	(*appendChild)(XMLDOMNode* pre,XMLDOMNode* node);
	BOOL	(*replaceChild)(XMLDOMNode **old, XMLDOMNode *nw);
	BOOL	(*removeChild)(XMLDOMNode* node);
	BOOL	(*insertBefore)(XMLDOMNode *ref, XMLDOMNode *node);
	XMLDOMNode* (*duplicateChild)(XMLDOMNode* ref, XMLDOMNode* parent);

	/* internal interface */
	XMLDOMNode* nextNode;
	XMLDOMNode* prevNode;
};


struct XMLDOMNodeList {
	/* interface */
	long length;
	XMLDOMNode* (*get_item)(XMLDOMNode* parent,long index);
	XMLDOMNode* (*nextNode)(XMLDOMNodeList* list);
	int (*reset)(XMLDOMNodeList* list);

	XMLDOMNode* headNode;
	XMLDOMNode* curNode;
};

/* add for gb2312 support */
struct XMLDOMPInstruction {
	char* version;
	char* encoding;
	char* standalone; 
};

/* DOM文档定义*/
struct XMLDOMDocument{

	/* 读取和存储xml */
	BOOL	(*load)(XMLDOMDocument* th, const char* url);
	BOOL	(*loadXML)(XMLDOMDocument* th, const char* xmldoc);
	int		(*save)(char* url);
	BOOL	(*get_xml)(XMLDOMDocument* doc,char** ps);

	/* 获得和设置parser flag*/
	BOOL	async;
	BOOL	validateOnParse;
	BOOL	resolveExternals;
	BOOL	preserveWhiteSpace;

	/* document info */
	XMLDOMParseError*	parseError;
	long				readyState;
	char*				url;
	XMLDOMDocumentType* docType;
	/* add for gb2312 support */
	XMLDOMPInstruction* pInstruction;

	XMLDOMNode*	documentElement;

	/* create and delete node */
	XMLDOMNode*	(*createNode)
			(XMLDOMNodeType Type,const char* name,char* namespaceURI);
	void (*releaseNode)(XMLDOMNode** pnode);

	BOOL (*checkDTD)(XMLDOMDocument* doc);
};


/* 
 * create this node only for say DOCTYPE exist ! 
 * if  DTD exist, dtd_exist == TRUE
 * I did't according the MSDOM standard in docType attribute
 */
enum __dtd_flags__ {
	DTD_START = 0x01,
	DTD_END = 0x00,
};

struct XMLDOMDocumentType {
	BOOL dtd_exist;		//DTD字段是否存在
	unsigned int flag;	//开始，结束DTD标志
	char* name;			//DTD根接点名称
	char* dtd;			//DTD部分内容
};


enum __dom_attribute__ {
	NODE_ATTRIBUTE_OWNERDOCUMENT = 1,
};

#endif
