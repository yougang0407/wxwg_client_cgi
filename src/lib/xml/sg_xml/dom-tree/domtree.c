/* add NODE_TEXT & NODE_CDATA_SECTION in child_element_exist */
/* add gb2312 support main see gb.c & gb.h file and this file */
/* correct text format show also see child_element_exist() */
/* parseError->reason use static string, do NOT need to release */

/* add legal childNodes check in _DOMNode_appendChild() "switch" */
/* fix one bug code in _DOMNode_removeChild() to test length == 0 */
/* smart output fromat of __get_xml() function */
/* add child_element_exist(node) to test wether node has child */

/* add checkDTD method in XMLDOMDocument */
/* in handle length string, we use dynamic buff instead static */
/* domtree.c [_DOMDoc_releaseNode() last two line need modify] */
/* domtree.c [ReleaseObject() release doc->parseError NOT fully] */
/* domtree.c [Free XML_Parser before quit to Fix Memory leak bug] */
/* domtree.c [free node->childNodes in _DOMDoc_releaseNode()] */

#include <stdio.h>
#include <string.h>
#include <stdlib.h> /*malloc*/
#include <stdarg.h>
#include <setjmp.h>

#include "xmlparse.h"
#include "domlib.h"
#include "domlog.h"
#include "dtd_chk.h"

#include "sg_domtree.h"
#include "sg_domapi.h"

#include "gb.h"

#include "../../libxml2/include/libxml/xmlmemory.h"
#include "../../libxml2/include/libxml/parser.h"

/* Smart adjust flag
 * Added for SoftDev
 */
unsigned char g_smart_adjust = 1;

//static FILE* fp;	/*从文件读取*/

struct nodetype {
	XMLDOMNodeType nType;
	char* strType;
};

static struct nodetype nt_tbl[] = {
	{NODE_ELEMENT			,"element"},
	{NODE_ATTRIBUTE			,"attribute"},
	{NODE_TEXT				,"text"},
	{NODE_CDATA_SECTION		,"cdatasection"},
	{NODE_ENTITY_REFERENCE	,"entityreference"},
	{NODE_ENTITY			,"entity"},
	{NODE_PROCESSING_INSTRUCTION,"processinginstruction"},
	{NODE_COMMENT			,"comment"},
	{NODE_DOCUMENT			,"document"},
	{NODE_DOCUMENT_TYPE		,"documenttype"},
	{NODE_DOCUMENT_FRAGMENT,"documentfragment"},
	{NODE_NOTATION			,"notation"}
};


typedef struct stack_node {
	XMLDOMNodeType type;
	char* name;
	XMLDOMNode* addr;
	struct stack_node* pre_node;
} stack_node;


typedef struct userData {
	int depth; 				/*zero=root element*/
	stack_node* stk_node;	/*element stack*/
	XMLDOMDocument* doc;	/*dom document*/
	int error;				/* error during proc hndl */
	jmp_buf jmp_env;		/* if error jump to */
	struct userData* next;
} userData;

/* 保存所有DOM对象的连表*/
static userData* domlist = NULL;
static int domlistnum = 0;

#if 1 /*DOM func define*/

/*出错信息*/
char*	(*get_srcText)(void);

/*读取和存储xml*/
static BOOL _DOMDoc_load(XMLDOMDocument* this, const char* url);
static BOOL _DOMDoc_loadXML(XMLDOMDocument* this, const char* xmldoc);
static BOOL _DOMDoc_save(char* url);
static BOOL _DOMDoc_get_xml(XMLDOMDocument* doc, char** ps);

/*创建释放接点*/

static XMLDOMNode* _DOMDoc_createNode(XMLDOMNodeType type,
							const char* name,char* namespaceURI);
static void _DOMDoc_releaseNode(XMLDOMNode** pnode);
static BOOL _DOMDoc_checkDTD(XMLDOMDocument* doc);

static BOOL _DOMNode_get_xml(XMLDOMNode* node,char** ps);
static XMLDOMNode*		_DOMNode_get_firstChild(XMLDOMNode* node);
static XMLDOMNode*		_DOMNode_get_lastChild(XMLDOMNode* node);
static XMLDOMNode*		_DOMNode_get_previousSibling(XMLDOMNode* node);
static XMLDOMNode*		_DOMNode_get_nextSibling(XMLDOMNode* node);

XMLDOMDocument*	(*get_ownerDocument)(void);

/*处理接点*/
static XMLDOMNode*	_DOMNode_appendChild(XMLDOMNode* pre,XMLDOMNode* node);
static BOOL	_DOMNode_replaceChild(XMLDOMNode **old, XMLDOMNode *nw);
static BOOL	_DOMNode_removeChild(XMLDOMNode* node);
static BOOL	_DOMNode_insertBefore(XMLDOMNode *ref, XMLDOMNode *node);
static XMLDOMNode* _DOMNode_duplicateChild(XMLDOMNode* ref, XMLDOMNode* parent);
static XMLDOMNodeList* _DOMNode_createNodeList(void);

/*处理接点连表*/
static XMLDOMNode* _DOMNodeList_get_item(XMLDOMNode* parent,long index);
static XMLDOMNode* _DOMNodeList_nextNode(XMLDOMNodeList* list);
static int _DOMNodeList_reset(XMLDOMNodeList* list);

#endif

#if (1) /* local function */

static void _log(int level, const char* fmt, ...) {
	va_list ap;
	va_start(ap,fmt);
	dom_log(level, (const char*) fmt, ap);
	va_end(ap);
}

/* for gb2312 support */
int is_gb2312_name(char* name)
{
  static const char prefixL[] = ("gb2312");
  static const char prefixU[] = ("GB2312");
  int i;

  if(name == NULL)
  	return 0;

  for (i = 0; prefixU[i]; i++)
    if (name[i] != prefixU[i] && name[i] != prefixL[i])
      return 0;
  return 1;
}

/* add for gb2312 support */
int is_gb2312_doc(XMLDOMDocument* doc)
{
	if(doc == NULL || doc->pInstruction == NULL)
		return 0;

	if(!is_gb2312_name(doc->pInstruction->encoding))
		return 0;

	return 1;
}


/* recurse replace child node's ownerDocument
   */
static int recurse_set_node(XMLDOMNode* node, int flags, void** v)
{
	XMLDOMNode* n;
	XMLDOMNodeList* list;

	#define set_by_flags(flags, n, v)\
	do {\
		switch(flags) {\
		case NODE_ATTRIBUTE_OWNERDOCUMENT:\
			n->ownerDocument = (XMLDOMDocument*) v;\
			break;\
		default:\
			break;\
		}\
	} while(0)

	set_by_flags(flags, node, v);
	if(node->hasChildNodes==TRUE) {
		list = node->childNodes;
		list->reset(list);
		while((n = list->nextNode(list)) != (XMLDOMNode*)NULL) {
			set_by_flags(flags, n, v);
			recurse_set_node(n, flags, v);
		}
	}

	return 1;
}


/* only use in expat_xxx_hndl() */
static XMLDOMNode*  create_add_node(XMLDOMNode* pp, XMLDOMNodeType type,
									const char* name, const char* text)
{
	XMLDOMNode* p = NULL;

    if((p = _DOMDoc_createNode(type ,(const char*) name,NULL)) == NULL)
    	goto error;
    if((_DOMNode_appendChild(pp,p)) == NULL)
    	goto error;

#if (0000)
    if(text != NULL)
	  	if((p->text = estrdup(text)) == NULL)
    		goto error;
#else
    if(text != NULL) {
		/* for gb2312 support */
    	char* t = (char*) text;
		if(is_gb2312_doc(pp->ownerDocument)) {
			struct __utf_2gb_mngr u2g;
			char* o;
			o = code_hndl_utf_2gb(&u2g, text, strlen(text));
			if(o != NULL)
				t = o;
		}
		if((p->text = estrdup(t)) == NULL)
    		goto error;
   	}
#endif
  	return p;

 error:
	if(p != NULL) {
		if(text != NULL)
			if(p->text != NULL)
				free(p->text);
		free(p);
	}
	return NULL;
}

static int child_element_exist(XMLDOMNode* node)
{
	XMLDOMNode* t;
	XMLDOMNodeList* list;
	if(node == NULL)
		return 0;

	if(node->hasChildNodes == TRUE && node->childNodes != NULL) {
		list = node->childNodes;
		list->reset(list);
		while((t = list->nextNode(list)) != NULL)
		{
			/* add NODE_TEXT & NODE_CDATA_SECTION type
				for text NOT show bug */
			if(t->nodeType == NODE_ELEMENT ||
				t->nodeType == NODE_TEXT ||
				t->nodeType == NODE_CDATA_SECTION)
				return 1;
		}
	}
	return 0;
}

#endif

#if (1) //stack_func

BOOL node_stack_push(stack_node** ptop, XMLDOMNode* node);
BOOL node_stack_pop(stack_node** ptop);

/*free string in node*/
BOOL node_stack_pop(stack_node** ptop)
{
	stack_node* pt;
	stack_node* top;

	pt = *ptop;
	top = pt->pre_node;
	free(pt->name);
	free(pt);

	*ptop = top;
	return TRUE;
}

/*make a duplicated copy for string in node*/
BOOL node_stack_push(stack_node** ptop, XMLDOMNode* node)
{
	stack_node* nd;
	char* name = NULL;

	nd = (stack_node*) malloc(sizeof(stack_node));
	if(nd==NULL)
		return FALSE;

	name = estrdup(node->nodeName);
	if(name == NULL && node->nodeName != NULL) {
		free(nd); nd = NULL;
		return FALSE;
	}

	nd->name = name;
	nd->type = node->nodeType;
	nd->addr = node;
	nd->pre_node = *ptop;

	*ptop = nd;

	return TRUE;
}

#endif

#if (1) //domlist mngr

int add_domlist(userData* dom)
{
	userData* list = NULL;
	if(domlist==(userData*)NULL)
		domlist = dom;
	else {
		list = domlist;
		while(list->next != (userData*)NULL)
			list = list->next;
		list->next = dom;
	}
	domlistnum ++;
	return 1;
}


userData* get_domlist(XMLDOMDocument* doc)
{
  userData* p = domlist;
  while (p != (userData*)NULL)  {
  	if(p->doc == doc)
  		break;
  	p = p->next;
  }
  return p;
}

int free_domlist(XMLDOMDocument* doc)
{
	userData *pre, *p = domlist;
	pre = NULL;
	while(p != NULL) {
		if(p->doc == doc) {
			if(pre == NULL)
				domlist = p->next;
			else
				pre->next = p->next;

			/* free stack node list */
			if(p->stk_node != NULL) {
				stack_node *pt, *top;
				do {
					pt = p->stk_node;
					top = pt->pre_node;
					free(pt->name);
					free(pt);
					p->stk_node = top;
				} while(p->stk_node != NULL);
			}

			free(p);
			domlistnum --;
			return 1;
		}
		pre = p;
		p = p->next;
	}
	return 0;
}

#endif


#if 1 /*Expat func define*/


#define expat_error_return(this)\
do {\
	this->error = TRUE;\
	longjmp(this->jmp_env, 0);\
} while(0)

static void expat_default_hndl(void *data, const char *s, int len) {
  userData* this = (userData*)data;
  XMLDOMNode* ppre = this->stk_node->addr;
  char *cstr, *spool;

  spool = (char*) malloc(len+1);
  memcpy(spool,s,len);
  spool[len] = '\0';

  cstr = eltrim(spool);
  if(strlen(cstr) == 0)
  	goto done;

  /* collection DTD data */
  if(this->doc->docType != NULL &&
  	this->doc->docType->flag == DTD_START) {
  	#define is_first_string (this->doc->docType->dtd == NULL)
	build_string((is_first_string)?1:0,&(this->doc->docType->dtd),cstr);
	build_string(0,&(this->doc->docType->dtd)," ");
	if(cstr[strlen(cstr)-1]=='>' ||cstr[strlen(cstr)-1]=='[')
		build_string(0,&(this->doc->docType->dtd),"\n");
  }

  /* add entity collection */
  if( cstr[0] == '&' && cstr[strlen(cstr)-1] == ';') {
  	/* NODE_ENTITY_REFERENCE
	The node represents a reference to an entity in the XML document
	(its nodeTypeString property is "entityreference"). This applies to
	all entities, including character entity references. An EntityReference
	node can have the following child node types: Element,
	ProcessingInstruction, Comment, Text, CDATASection, and
	EntityReference. The EntityReference node can appear as the
	child of the Attribute, DocumentFragment, Element, and
	EntityReference nodes.
	*/
	if(!create_add_node(ppre, NODE_ENTITY_REFERENCE, "#entityreference", cstr))
		expat_error_return(this);
  }

done:
  if(spool != NULL)
	free(spool);
}

static
void expat_comment_hndl(void* data, const char* cmt)
{
  userData* this = (userData*)data;
  XMLDOMNode* ppre = this->stk_node->addr;

	/* move comment collection from default handle
	to here */

	/* NODE_COMMENT
	The node represents a comment in the XML document
	(its nodeTypeString property is "comment"). A Comment
	node cannot have any child nodes. The Comment node
	can appear as the child of the Document, DocumentFragment,
	Element, and EntityReference nodes.
	*/
	if(!create_add_node(ppre, NODE_COMMENT, "#comment", cmt))
		expat_error_return(this);
}


static void expat_start_hndl(void *data, const char *el, const char **attr) {
  userData* this = (userData*)data;
  XMLDOMNode* ppre = this->stk_node->addr;
  XMLDOMNode* pnew;
  int i;

	if(!(pnew = create_add_node(ppre, NODE_ELEMENT, el, NULL)))
		expat_error_return(this);

  if(!node_stack_push(&(this->stk_node), pnew)) {
  	  _log(L_ERR,"push one element to stack failure\n");
	  expat_error_return(this);
  }

  /* add attribute node */
  ppre = this->stk_node->addr;
  for (i = 0; attr[i]; i += 2) {
	/* create and add one node */
	if(!create_add_node(ppre, NODE_ATTRIBUTE, attr[i], attr[i+1]))
		expat_error_return(this);
  }

  this->depth++;
}

static void expat_end_hndl(void *data, const char *el)
{
  userData* this = (userData*)data;
  this->depth--;
  node_stack_pop(&(this->stk_node));
}

static void expat_char_hndl(void *data, const char *txt, int txtlen)
{
  userData* this = (userData*)data;
  XMLDOMNode* ppre = this->stk_node->addr;
  char *spool,*out;

  spool = (char*) malloc(txtlen+1);

  memcpy(spool,txt,txtlen);
  spool[txtlen]='\0';

  if(this->doc->preserveWhiteSpace==TRUE ||
  	ppre->nodeType==NODE_CDATA_SECTION) {
	out = &spool[0];
  } else {
	  out = eltrim(spool);
	  if(strlen(out) == 0)
		goto done;
  }

  /* create and add one node */
  if(!create_add_node(ppre, NODE_TEXT, "#text", out))
	expat_error_return(this);

done:
	if(spool != NULL)
		free(spool);
	return;
}

static void expat_proc_hndl(void *data, const char *target, const char *pidata)
{
  userData* this = (userData*)data;
  XMLDOMNode* ppre = this->stk_node->addr;

  /* create and add one new node */
  if(!create_add_node(ppre, NODE_PROCESSING_INSTRUCTION, target, pidata))
		expat_error_return(this);
}

static void expat_cdata_start_hndl(void *data)
{
  userData* this = (userData*)data;
  XMLDOMNode* ppre = this->stk_node->addr;
  XMLDOMNode* pnew;

  if(!(pnew = create_add_node(ppre, NODE_CDATA_SECTION, "#cdata-section", data)))
	expat_error_return(this);

  if(!node_stack_push(&(this->stk_node), pnew)) {
	  _log(L_ERR,"push one element to stack failure!\n");
	  expat_error_return(this);
  }

  this->depth++;
}

static void expat_cdata_end_hndl(void *data)
{
  userData* this = (userData*)data;
  this->depth--;
  node_stack_pop(&(this->stk_node));
}

/* add code to mark docType infomation */
static void expat_doctype_start_hndl(void *data, const char* name)
{
  userData* this = (userData*)data;
  XMLDOMDocumentType* dtd = NULL;
  char* n = NULL;

  if(this->doc->docType != NULL) {
	 _log(L_ERR,"multicast DOCTYPE defination found\n");
	 expat_error_return(this);
  }

  if((dtd = (XMLDOMDocumentType*) malloc(sizeof(*dtd))) == NULL) {
	  _log(L_ERR,"malloc for XMLDOMDocumentType error\n");
	  expat_error_return(this);
  }

  if((n = estrdup(name)) == NULL) {
  	if(dtd != NULL)
	  	free(dtd);
	expat_error_return(this);
  }

  dtd->dtd_exist = TRUE;
  dtd->name = n;
  dtd->flag = DTD_START;
  dtd->dtd = NULL;

  this->doc->docType = dtd;
}

static void expat_doctype_end_hndl(void *data)
{
  userData* this = (userData*)data;
  if(this->doc->docType == NULL) {
  	_log(L_ERR,"match DTD end but no head\n");
	expat_error_return(this);
  }

  this->doc->docType->flag = DTD_END;
}

/* add for gb2312 support */
int expat_unknown_encode_hndl(void *data,
                              const char* name,
                              XML_Encoding *info)
{
  userData* this = (userData*)data;

#if (0000)
typedef struct {
  int map[256];
  void *data;
  int (*convert)(void *data, const char *s);
  void (*release)(void *data);
} XML_Encoding;
#endif

  if(!is_gb2312_name((char*) name))
  	return 0;

  /* save encoding type into struct XMLDOMPInstruction [head] */
  do {
	  if(this->doc->pInstruction == NULL) {
		XMLDOMPInstruction* pi;
		#define __encoding (this->doc->pInstruction->encoding)
		if((pi = (XMLDOMPInstruction*) malloc(sizeof(*pi))) == NULL) {
			_log(L_ERR,"malloc for XMLDOMPInstruction error\n");
			expat_error_return(this);
		}
		memset(pi, 0 , sizeof(*pi));
		this->doc->pInstruction = pi;
	  }
	  if(__encoding != NULL) {
	  	free(__encoding);
	  	__encoding = NULL;
	  }
	  __encoding = estrdup(name);
	  if(__encoding == NULL)
		  _log(L_NOTE,"can NOT alloc memory for encoding[%s]\n",name);
  } while(0);
  /* save encoding type into struct XMLDOMPInstruction  [tail] */

  code_hndl_gbmap_init(info->map);
  code_hndl_gbtab_init();

  info->convert = code_hndl_gb_2unicot;
  info->data = NULL;
  info->release = NULL;

  return 1;
}


#define set_process_handler()\
do {\
XML_SetUserData(p,this);\
XML_SetCommentHandler(p, expat_comment_hndl);\
XML_SetElementHandler(p, expat_start_hndl, expat_end_hndl);\
XML_SetCharacterDataHandler(p, expat_char_hndl);\
XML_SetProcessingInstructionHandler(p, expat_proc_hndl);\
XML_SetCdataSectionHandler(p, expat_cdata_start_hndl, expat_cdata_end_hndl);\
XML_SetDoctypeDeclHandler(p, expat_doctype_start_hndl, expat_doctype_end_hndl);\
/*XML_SetExternalEntityRefHandler(p, expat_external_entity_hndl);*/\
XML_SetDefaultHandler(p, expat_default_hndl);\
/* add for gb2312 support */\
XML_SetUnknownEncodingHandler(p,expat_unknown_encode_hndl, this);\
} while(0)

static void expat_parser_error(XML_Parser p, XMLDOMDocument* doc)
{
  	XMLDOMParseError* error = doc->parseError;
    error->errorCode = XML_GetErrorCode(p);
    error->filepos = XML_GetCurrentByteIndex(p);
    error->line = XML_GetCurrentLineNumber(p);
    error->linepos = XML_GetCurrentByteCount(p);

	/* NOTE: at here reason use static string,
	 do NOT need release */
    /* error->reason = estrdup(XML_ErrorString(error->errorCode)); */
    error->reason = (char*) XML_ErrorString(error->errorCode);
}

BOOL expat_parser_file(XMLDOMDocument* doc, FILE* fp)
{
  XMLDOMNode* pnew = NULL;
  userData* this;
  BOOL result = TRUE;

  /*
   * change for parse larget whole line file
   * so an XML file's whole line must less than it 2002-02-04
   */
  char Buff[32*1024];
  XML_Parser p;

  if ( fp <= 0 ) {
    _log(L_ERR,"open a file first\n");
    return FALSE;
  }

  if((this = get_domlist(doc)) == NULL) {
  	_log(L_ERR,"domlist empty!\n");
  	return FALSE;
  }

  if(!(p = XML_ParserCreate(NULL))) {
    _log(L_ERR,"couldn't allocate memory for parser\n");
    return FALSE;
  }

  /* 设置各种handler    */
  set_process_handler();

  /* create add and root node */
  if((pnew = _DOMDoc_createNode(NODE_DOCUMENT,"#document",NULL)) == NULL) {
  	_log(L_ERR,"create node error\n");
	XML_ParserFree(p);
	return FALSE;
  }
  this->doc->documentElement = pnew;
  pnew->ownerDocument = this->doc;

  if(!node_stack_push(&(this->stk_node), pnew)) {
	  _log(L_ERR,"push one element to stack failure!\n");
	  XML_ParserFree(p);
	  return FALSE;
  }

  for (;;) {
    int done, len;
	memset(Buff,0,sizeof(Buff));
    fgets(Buff, sizeof(Buff), fp);
    len = strlen(Buff);
    if (ferror(fp)) {
      _log(L_ERR,"read file error\n");
      result = FALSE;
      break;
    }

    done = feof(fp);

	/* during parsing using expat , any error will call
	longjmp() then, expat_parser_error() will run */
    if (setjmp(this->jmp_env) || !XML_Parse(p, Buff, len, done)) {
		expat_parser_error(p, this->doc);
		result = FALSE;
		break;
    }

    if (done)
      break;
  }

  if(this->error == TRUE) {
	XML_ParserFree(p);
	return FALSE;
  }

  if(!node_stack_pop(&(this->stk_node))) {
	  _log(L_ERR,"pop one element from stack failure!\n");
	  XML_ParserFree(p);
	  return FALSE;
  }

  XML_ParserFree(p);
  return result;
}


BOOL expat_parser_doc(XMLDOMDocument* doc, const char* xmldoc)
{
  XMLDOMNode* pnew = NULL;
  userData* this;
  BOOL result = TRUE;
  XML_Parser p;

  if((this = get_domlist(doc)) == NULL) {
  	_log(L_ERR,"domlist empty!\n");
  	return FALSE;
  }

  if(!(p = XML_ParserCreate(NULL))) {
    _log(L_ERR,"couldn't allocate memory for parser\n");
    return FALSE;
  }

  /* 设置各种handler    */
  set_process_handler();

  /* create and add root node */
  if((pnew = _DOMDoc_createNode(NODE_DOCUMENT,"#document",NULL)) == NULL) {
  	_log(L_ERR,"create node error\n");
	XML_ParserFree(p);
	return FALSE;
  }
  this->doc->documentElement = pnew;
  pnew->ownerDocument = this->doc;

  if(!node_stack_push(&(this->stk_node), pnew)) {
	  _log(L_ERR,"push one element to stack failure!\n");
	  XML_ParserFree(p);
	  return FALSE;
  }

  /* during parsing using expat , any error will call
  longjmp() then, expat_parser_error() will run */
  if (setjmp(this->jmp_env) || !XML_Parse(p, xmldoc, strlen(xmldoc), FALSE)) {
	expat_parser_error(p, this->doc);
	result = FALSE;
  }

  if(!node_stack_pop(&(this->stk_node))) {
	  _log(L_ERR,"pop one element from stack failure!\n");
	  XML_ParserFree(p);
	  return FALSE;
  }

  if(this->error == TRUE) {
	XML_ParserFree(p);
	return FALSE;
  }


  XML_ParserFree(p);
  return result;
}


#endif

#if 1 /*Register func*/
void* CreateObject(char* obj)
{
	XMLDOMDocument* doc = NULL;
	XMLDOMParseError* err = NULL;
	userData* dom = (userData*)NULL;

	if(strcmp(obj,"foolfish.xmldom")!=0 && strcmp(obj,"FOOLFISH.XMLDOM")!=0) {
		_log(L_ERR,"create object using [%s] not support\n",obj);
		goto error;
	}

	if((doc = (XMLDOMDocument*) malloc(sizeof(XMLDOMDocument))) == NULL) {
		_log(L_ERR,"malloc memory for XMLDOMDocument failure\n");
		goto error;
	}
	memset(doc,0,sizeof(*doc));

	doc->load 		= _DOMDoc_load;
	doc->loadXML 	= _DOMDoc_loadXML;
	doc->save 		= _DOMDoc_save;
	doc->get_xml 	= _DOMDoc_get_xml;
	doc->checkDTD	= _DOMDoc_checkDTD;

	doc->async 				= TRUE;
	doc->validateOnParse 	= TRUE;
	doc->resolveExternals 	= TRUE;
	doc->preserveWhiteSpace	= FALSE;

	if((err = (XMLDOMParseError*) malloc(sizeof(XMLDOMParseError))) == NULL) {
		_log(L_ERR,"malloc XMLDOMParseError memory error\n");
		goto error;
	}
	memset(err,0,sizeof(*err));

	doc->parseError = err;
	doc->readyState = (long)NULL;
	doc->url = (char*)NULL;
	doc->docType = (XMLDOMDocumentType*) NULL;
	/* add for gb2312 support */
	doc->pInstruction = (XMLDOMPInstruction*) NULL;

	doc->documentElement = (XMLDOMNode*)NULL;
	doc->createNode = _DOMDoc_createNode;
	doc->releaseNode = _DOMDoc_releaseNode;

	/* 初始化设置用户数据,
	 * 将新dom对象追加到domlist末尾*/
	dom = (userData*) malloc(sizeof(userData));
	if(dom == NULL) {
		_log(L_ERR,"malloc mem for userData fail\n");
		goto error;
	}
	memset(dom,0,sizeof(*dom));

	dom->depth = 0;
	dom->doc = doc;
	dom->stk_node = (stack_node*)NULL;
	dom->error = FALSE;
	/* Avoid warning */
	//dom->jmp_env;
	dom->next = (userData*)NULL;

	add_domlist(dom);

	__sg_xml_get_num__++;
	return doc;

error:

	if(dom != NULL)		free(dom);
	if(err != NULL)		free(err);
	if(doc != NULL)		free(doc);

	return NULL;
}

//modify the arg2's type from void** to XMLDOMDocument**
//for gcc version 3.3.4
//void ReleaseObject(char* type, void** obj)
void ReleaseObject(char* type, XMLDOMDocument** obj)
{
	XMLDOMDocument* doc;

	if(strcmp(type,"foolfish.xmldom")!=0 &&
	   strcmp(type,"FOOLFISH.XMLDOM")!=0) {
		_log(L_ERR,"create object using [%s] not support\n",obj);
		return;
	}

	doc = (XMLDOMDocument*)*obj;

	/* release in domlist */
	free_domlist(doc);

	if(doc->url !=NULL)
		free(doc->url);

	if(doc->documentElement != NULL)
		_DOMDoc_releaseNode(&doc->documentElement);

	if(doc->parseError !=NULL) {
		/* NOTE: at here reason use static string,
		 it do NOT need release */
		#if (0000)
		if(doc->parseError->reason != NULL)
			free(doc->parseError->reason);
		#endif
		free(doc->parseError);
	}

	if(doc->docType != NULL) {
		if(doc->docType->name != NULL)
			free(doc->docType->name);
		if(doc->docType->dtd != NULL)
			free(doc->docType->dtd);
		free(doc->docType);
	}

	/* add for gb2312 support */
	if(doc->pInstruction != NULL) {
		if(doc->pInstruction->version != NULL)
			free(doc->pInstruction->version);
		if(doc->pInstruction->encoding != NULL)
			free(doc->pInstruction->encoding);
		if(doc->pInstruction->standalone != NULL)
			free(doc->pInstruction->standalone);
		free(doc->pInstruction);
	}


	free(doc);
	*obj = NULL;
	__sg_xml_get_num__--;
}

#endif


void treeConstruct(XMLDOMNode *parent,xmlNodePtr cur)
{
	XMLDOMNode *attri=NULL;
	XMLDOMNode *element=NULL;
	XMLDOMNode *text=NULL;

	if(cur!=NULL)
	{
		element = sg_dom_create_elementn((char *)cur->name);
		sg_dom_add_childnode(parent,element);

		//according cur fill child
		xmlAttrPtr attrPtr = cur->properties;

		while (attrPtr != NULL)
		{
			xmlChar* szAttr;szAttr = xmlNodeGetContent(attrPtr->children);
			attri=sg_dom_create_attributen((char *)attrPtr->name,(char *)szAttr);
			sg_dom_add_childnode(element,attri);
			xmlFree(szAttr);
			attrPtr = attrPtr->next;
		}
		cur = cur->xmlChildrenNode;
		while(cur!=NULL)
		{
			if(cur->type == XML_ELEMENT_NODE)
				treeConstruct(element,cur);
																																							            if(cur->type == XML_TEXT_NODE )
			{
				text = sg_dom_create_textn((char *)cur->content);
				sg_dom_add_childnode(element,text);
			}

			cur=cur->next;
		}
	}
}


#if 1 /*domDoc methods*/
static BOOL _DOMDoc_loadXML(XMLDOMDocument* this, const char* xmldoc)
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	XMLDOMNode* pnew = NULL;
	xmlNodePtr pnode;
	XMLDOMNode* comment = NULL;

	xmlKeepBlanksDefault (0);
	doc = xmlReadMemory( xmldoc, strlen(xmldoc), "noname.xml", NULL, 0);

	if (doc == NULL )
	{
		fprintf(stderr,"Document not parsed successfully. \n");
		return FALSE;
    }

	if((pnew = this->createNode(NODE_DOCUMENT,"#document",NULL)) == NULL)
	{
		printf("create node error\n");
		return FALSE;
	}

	this->documentElement = pnew;
	pnew->ownerDocument = this;

	//consturct tree
	cur = xmlDocGetRootElement(doc);

	for(pnode=cur->prev;pnode!=NULL;pnode=pnode->prev)
	{
		if(pnode->type==XML_COMMENT_NODE)
		{
			comment=sg_dom_create_commentn((char *)pnode->content);
			sg_dom_add_childnode(pnew,comment);
		}
	}

	if(cur->type == XML_ELEMENT_NODE)
		treeConstruct(pnew,cur);

	xmlFreeDoc(doc);
	return TRUE;
}


static BOOL _DOMDoc_load(XMLDOMDocument* this, const char* url)
{
    xmlDocPtr doc;
	xmlNodePtr cur;
	XMLDOMNode* pnew = NULL;
	xmlNodePtr pnode;
	XMLDOMNode* comment = NULL;
    xmlKeepBlanksDefault (0);
    this->url= estrdup(url);
	doc = xmlParseFile(url);

	if (doc == NULL ) {
		fprintf(stderr,"Document not parsed successfully. \n");
		return FALSE;
	}

	if((pnew = this->createNode(NODE_DOCUMENT,"#document",NULL)) == NULL) {
	printf("create node error\n");
	return FALSE;
	}

	this->documentElement = pnew;
	pnew->ownerDocument = this;

	//consturct tree
	cur = xmlDocGetRootElement(doc);
	for(pnode=cur->prev;pnode!=NULL;pnode=pnode->prev)
	{
		if(pnode->type==XML_COMMENT_NODE)
		{
			comment=sg_dom_create_commentn((char *)pnode->content);
			sg_dom_add_childnode(pnew,comment);
		}
	}

	if(cur->type == XML_ELEMENT_NODE)
		treeConstruct(pnew,cur);
	xmlFreeDoc(doc);
	return TRUE;
}

static BOOL _DOMDoc_save(char* url)
{
	return FALSE;
}

static BOOL _DOMDoc_get_xml(XMLDOMDocument* doc, char** ps)
{
	#define say(s)\
	do {\
		build_string((*ps==NULL)?1:0,ps,s);\
	} while(0)

	if(doc == NULL)
		return FALSE;

/* add for gb2312 support */
#if (1)
say("<?xml ");
{
XMLDOMPInstruction* pi = doc->pInstruction;
char* version = "1.0";
char* encoding = "UTF-8";
if(pi == NULL) {
	say("version=\""); say(version); say("\"");
	say(" encoding=\""); say(encoding); say("\"");
	//say("version=\'"); say(version); say("\'");
	//say(" encoding=\'"); say(encoding); say("\'");
}
else {
	if(pi->version != NULL)
		version = pi->version;
	if(pi->encoding != NULL)
		encoding = pi->encoding;
	/* here we do NOT deal with standalone */
	say("version=\""); say(version); say("\"");
	say(" encoding=\""); say(encoding); say("\"");
}
}
say("?>");
#else
	say("<?xml version=\"1.0\"?>");
#endif

	/* display dtd */
	if(doc->docType != NULL && doc->docType->dtd != NULL) {
		_log(L_INFO,"dtd[%s]\n",doc->docType->dtd);
		say("\n<!DOCTYPE "); say(doc->docType->dtd); say(">");
	}

	return _DOMNode_get_xml(doc->documentElement,ps);
}

static XMLDOMNode* _DOMDoc_createNode(XMLDOMNodeType type,
						const char* name,char* namespaceURI)
{
	XMLDOMNode*	node;
	char _name[256];

	memset(_name,'\0',sizeof(_name));
	strcpy(_name,name);

	switch(type)
	{
	case NODE_ELEMENT:
	case NODE_ATTRIBUTE:
		break;
	case NODE_TEXT:
		strcpy(_name,"#text");
		break;
	case NODE_CDATA_SECTION:
		strcpy(_name,"#cdata-section");
		break;
	case NODE_ENTITY_REFERENCE:
	case NODE_ENTITY:
	case NODE_PROCESSING_INSTRUCTION:
		break;
	case NODE_COMMENT:
		strcpy(_name,"#comment");
		break;
	case NODE_DOCUMENT:
		strcpy(_name,"#document");
		break;
	case NODE_DOCUMENT_TYPE:
	case NODE_DOCUMENT_FRAGMENT:
	case NODE_NOTATION:
		break;
	default:
		_log(L_ERR,"error 'type' parameter at createNode\n");
		return NULL;
	}

	node = (XMLDOMNode*) malloc(sizeof(XMLDOMNode));
	if(node == (XMLDOMNode*)NULL) {
		_log(L_ERR,"malloc new node failure!\n");
		return NULL;
	}

	node->hasChildNodes = FALSE;
	node->namespaceURI = estrdup(namespaceURI);

	node->ownerDocument = (XMLDOMDocument*)NULL;

#if (1)
    {
   	char* t = _name;
	if(is_gb2312_doc(node->ownerDocument)) {
		struct __utf_2gb_mngr u2g;
		char* o;
		o = code_hndl_utf_2gb(&u2g, _name, strlen(_name));
		if(o != NULL)
			t = o;
	}
	node->nodeName = estrdup(t);
   	}
#else
	node->nodeName = estrdup(_name);
#endif

	node->nodeType = type;
	node->nodeTypeString = nt_tbl[type].strType;
	node->specified = (type==NODE_ATTRIBUTE)?
						(name==(char*)NULL?FALSE:TRUE):TRUE;
	node->get_xml = _DOMNode_get_xml;

	node->text = (char*)NULL;

	node->parentNode = (XMLDOMNode*)NULL;
	node->childNodes = (XMLDOMNodeList*)NULL;

	node->get_firstChild = _DOMNode_get_firstChild;
	node->get_lastChild = _DOMNode_get_lastChild;
	node->get_previousSibling = _DOMNode_get_previousSibling;
	node->get_nextSibling = _DOMNode_get_nextSibling;

	node->appendChild = _DOMNode_appendChild;
	node->replaceChild = _DOMNode_replaceChild;
	node->removeChild = _DOMNode_removeChild;
	node->insertBefore = _DOMNode_insertBefore;
	node->duplicateChild = _DOMNode_duplicateChild;

	node->nextNode = (XMLDOMNode*)NULL;
	node->prevNode = (XMLDOMNode*)NULL;

	return node;
}


static void _DOMDoc_releaseNode(XMLDOMNode** pnode)
{
	XMLDOMNodeList* list;
	XMLDOMNode* child;
	XMLDOMNode* node;
	node = *pnode;

	if(node->namespaceURI!=NULL)
		free(node->namespaceURI);

	if(node->nodeName!=NULL)
		free(node->nodeName);

	if(node->text != NULL)
		free(node->text);

	if(node->hasChildNodes==TRUE) {
		list = node->childNodes;
		list->reset(list);
		while((child = list->nextNode(list)) != NULL)
		{
			_DOMDoc_releaseNode(&child);
		}

		free(node->childNodes);
		node->childNodes = NULL;
		node->hasChildNodes = FALSE;
	}

	free(*pnode);
	*pnode = (XMLDOMNode*)NULL;
}


/* this method need dtd_chk.c module */
static BOOL _DOMDoc_checkDTD(XMLDOMDocument* doc)
{
	extern int check_DTD(void* doc);
	return (check_DTD((void*) doc)==1)?TRUE:FALSE;
}


#endif

/* Only support adjust smart flag base process, can't adjust one DOM.
 * Added for SoftDev
 */
void dom_smart_adjust( unsigned char flg )
{
	g_smart_adjust = flg;
}

#if 1 /*domNode methods*/
char *dom_proc_escape(char *text)
{
	int len = 0;
	int i = 0;
	int j = 0;
	int total_len = 0;
	char *buf = NULL;
	char *tmp = text;

	if(text == NULL){
		return NULL;
	}

	len = strlen(text);
	total_len = len * 6;
	buf = malloc(total_len + 1);
	if(buf == NULL){
		return NULL;
	}

	memset(buf, 0, total_len + 1);
#if 10
	for(i = 0; i < len; i ++){
		switch(tmp[i]){
		case '<':
			if(j + 4 > total_len)
			goto err;
			strcpy(&buf[j], "&lt;");
			j += 4;
			break;
		case '>':
		    if(j + 4 > total_len)
			goto err;
		    strcpy(&buf[j], "&gt;");
		    j += 4;
		    break;
		case '\'':
		    if(j + 6 > total_len)
			goto err;
		    strcpy(&buf[j], "&apos;");
		    j += 6;
		    break;
		case '\"':
		    if(j + 6 > total_len)
			goto err;
		    strcpy(&buf[j], "&quot;");
		    j += 6;
		    break;
		case '&':
		    if(j + 5 > total_len)
			goto err;
		    strcpy(&buf[j], "&amp;");
		    j += 5;
		    break;
		default:
		    buf[j] = tmp[i];
		    j += 1;
		    break;
		}
	}
#else
	strcpy( buf, text );
#endif
	return buf;
err:
	if( buf != NULL ){
		free( buf );
	}
	return NULL;
}

static BOOL __get_xml(XMLDOMNode* node,char** ps,int depth)
{
	XMLDOMNode* son;
	XMLDOMNodeList* list;

	char *ptr = NULL;
	int count = 0;

	#define __say(s)\
		build_string((*ps==NULL)?1:0,ps,s);

	#define __say_space(n)\
	do {\
		int i;\
		for(i = 0; i < n; i++)\
			__say("    ");\
	} while(0)

	switch(node->nodeType)
	{
	case NODE_DOCUMENT:
		if(node->hasChildNodes==TRUE) {
			list = node->childNodes;
			list->reset(list);
			while((son = list->nextNode(list))!= (XMLDOMNode*)NULL)
			{
				switch(son->nodeType)
				{
				case NODE_PROCESSING_INSTRUCTION:
					__get_xml(son, ps, 0 /*depth*/);
					break;
				case NODE_ELEMENT:
				case NODE_COMMENT:
				case NODE_CDATA_SECTION:
					__get_xml(son, ps, depth);
					break;
				default:
					break;
				}
			}
		}
		break;
	case NODE_ELEMENT:

		if(!(*ps == NULL || *ps[0] == '\0'/*strlen(*ps) == 0*/))
			if( g_smart_adjust )
				__say("\n");
		if( g_smart_adjust )
			__say_space(depth);
		__say("<");
		__say(node->nodeName);
		if(node->hasChildNodes==TRUE) { /* 登记属性接点*/
			list = node->childNodes;
			list->reset(list);
			while((son = list->nextNode(list)) != (XMLDOMNode*)NULL)
				if(son->nodeType==NODE_ATTRIBUTE) {
					__say(" ");
					__say(son->nodeName);
					__say("=\"");
					//__say("=\'");

				  	ptr = dom_proc_escape(son->text);
					if(ptr == NULL){
						return FALSE;
					}
					__say(ptr);
					free(ptr);
					//__say(son->text);

					__say("\"");
					//__say("\'");
				}
		}
		if(child_element_exist(node)) {
			__say(">");
			list = node->childNodes;
			list->reset(list);
			count = 0;
			while((son = list->nextNode(list)) != (XMLDOMNode*)NULL)
				if(son->nodeType != NODE_ATTRIBUTE){
					__get_xml(son, ps, depth + 1);
					count ++;
			};

			if( g_smart_adjust && count > 1 ){
				//__say("\n");
				//__say_space(depth);
			}

			__say("</");
			__say(node->nodeName);
			__say(">");

			/*if( g_smart_adjust && count > 1 ){
				__say("\n");
				__say_space(depth);
			}*/
		}
		else {
			/* short over string */
			__say("/>");
			//if( g_smart_adjust && count > 1 ){
			if( g_smart_adjust ){
				__say("\n");
				__say_space((( depth > 0 ) ? depth - 1 : depth ));
			}
		}
		break;
	case NODE_ATTRIBUTE: /* 已经在NODE_ELEMENT中登记*/
		break;
	case NODE_TEXT:

		/* correct text show format */
		/*if( g_smart_adjust ){
			__say("\n");
			__say_space(depth);
		}*/

	  	ptr = dom_proc_escape(node->text);
		if(ptr == NULL){
			return FALSE;
		}
		__say(ptr);
		free(ptr);
		//__say(node->text);

		break;
	case NODE_CDATA_SECTION:
		__say_space(depth);
		__say("\n");
		__say("<![CDATA[");
		if(node->hasChildNodes==TRUE) { /* 登记子接点*/
			list = node->childNodes;
			list->reset(list);
			while ((son = list->nextNode(list)) != (XMLDOMNode *)NULL) {
				if (son->nodeType == NODE_TEXT) {
					// To CDATA, needn't process escape char
				  	/*ptr = dom_proc_escape(son->text);
					if(ptr == NULL){
						return FALSE;
					}
					__say(ptr);
					free(ptr);*/
					__say(son->text);
				}
			}
		}
		__say("]]>");
		break;
	case NODE_ENTITY_REFERENCE:
	  	ptr = dom_proc_escape(node->text);
		if(ptr == NULL){
			return FALSE;
		}
		__say(ptr);
		free(ptr);
		//__say(node->text);
		break;
	case NODE_ENTITY:
		break;
	case NODE_PROCESSING_INSTRUCTION:
		__say_space(depth);
		__say("\n");
		__say("<?");
		__say(node->nodeName);
		__say(" ");
	  	ptr = dom_proc_escape(node->text);
		if(ptr == NULL){
			return FALSE;
		}
		__say(ptr);
		free(ptr);
		//__say(node->text);
		__say("?>");
		break;
	case NODE_COMMENT:
		__say_space(depth);
		__say("\n");
		__say("<!--");
		// Don't transfer COMMENT node
	  	/*ptr = dom_proc_escape(node->text);
		if(ptr == NULL){
			return FALSE;
		}
		__say(ptr);
		free(ptr);*/
		__say(node->text);
		__say("-->");
		break;
	case NODE_DOCUMENT_TYPE:
	case NODE_DOCUMENT_FRAGMENT:
	case NODE_NOTATION:
	default:
		_log(L_ERR,"error node type at _get_xml\n");
		return FALSE;
	}

	return TRUE;
}

static BOOL _DOMNode_get_xml(XMLDOMNode* node,char** ps)
{
	return __get_xml(node, ps, 0);
}

static XMLDOMNode*	_DOMNode_get_firstChild(XMLDOMNode* node) {
	if(node && node->hasChildNodes==TRUE)
		return node->childNodes->get_item(node,0);
	return NULL;
}

static XMLDOMNode*	_DOMNode_get_lastChild(XMLDOMNode* node) {
	if(node && node->hasChildNodes==TRUE)
		return node->childNodes->get_item(node,node->childNodes->length-1);
	return NULL;
}

static XMLDOMNode*	_DOMNode_get_previousSibling(XMLDOMNode* node) {
	return (node == NULL)?node:node->prevNode;
}

static XMLDOMNode*	_DOMNode_get_nextSibling(XMLDOMNode* node) {
	return (node == NULL)?node:node->nextNode;
}

/* append "node" as "pre" 's child */
static XMLDOMNode*	_DOMNode_appendChild(XMLDOMNode* parent,XMLDOMNode* node)
{
	if(parent== (XMLDOMNode*)NULL)
		return NULL;

	node->parentNode = parent;

	/*
	   if you want to check node->childNodes legal
	   open the following code, else don't do it
	*/
	if (0000) {
	/* add legal childNodes check */
	if(parent->childNodes == NULL) {
		parent->hasChildNodes = FALSE;
	}
	else {
		if(parent->childNodes->length == 0 ||
			parent->childNodes->headNode == NULL) {
			free(parent->childNodes);
			parent->childNodes = NULL;
			parent->hasChildNodes = FALSE;
		}
		else {
			parent->hasChildNodes = TRUE;
		}
	}
	}


	if(parent->hasChildNodes != TRUE) {
		XMLDOMNodeList* list = _DOMNode_createNodeList();
		list->length = 1;
		list->headNode = node;
 		list->curNode = NULL;
		parent->childNodes = list;
		parent->hasChildNodes = TRUE;
  	} else {
		XMLDOMNode* cnode;
		/* TODO: here should force NULL check */
  		cnode = parent->childNodes->headNode;
		while(cnode ->nextNode != (XMLDOMNode*)NULL)
			cnode  = cnode ->nextNode;
		cnode ->nextNode = node;
  		node->prevNode = cnode ;
		parent->childNodes->length++;
	}

	recurse_set_node(node,
					NODE_ATTRIBUTE_OWNERDOCUMENT,
					(void*) parent->ownerDocument);

	return node;
}


/* remove node from his parent and free node and his children */
static BOOL	_DOMNode_removeChild(XMLDOMNode* c)
{
	XMLDOMNode* parent;

	if(c==NULL)
		return FALSE;

	if(c->nextNode != NULL)
		c->nextNode->prevNode = c->prevNode;
	if(c->prevNode != NULL)
		c->prevNode->nextNode = c->nextNode;

	parent = c->parentNode;
	if(parent != NULL && parent->hasChildNodes == TRUE) {
		if(parent->childNodes->headNode == c)
			parent->childNodes->headNode = c->nextNode;
		if(parent->childNodes->curNode == c)
			parent->childNodes->curNode = c->nextNode;
		parent->childNodes->length --;

		/*
		   right code is test "length == 0"
		*/
		/* if(parent->childNodes == 0) { */
		if(parent->childNodes->length == 0) {
			free(parent->childNodes);
			parent->childNodes = NULL;
			parent->hasChildNodes = FALSE;
		}
	}

	/* release node and it's child nodes */
	//c->ownerDocument->releaseNode(&c);
	if( c->ownerDocument != NULL ){
		c->ownerDocument->releaseNode( &c );
	}else{
		release_raw_node( &c );
	}

	return TRUE;
}

static BOOL	_DOMNode_insertBefore(XMLDOMNode *ref, XMLDOMNode *node)
{
	if(ref==(XMLDOMNode*)NULL)
		return FALSE;

	/* can NOT add before root or document node */
	if(ref->parentNode==(XMLDOMNode*)NULL ||
		ref->parentNode->nodeType==NODE_DOCUMENT)
		return FALSE;
	node->parentNode = ref->parentNode;
	node->prevNode = ref->prevNode;
	node->nextNode = ref;

	recurse_set_node(node,
					NODE_ATTRIBUTE_OWNERDOCUMENT,
					(void*) ref->ownerDocument);

	if(ref->prevNode==(XMLDOMNode*)NULL)
		ref->parentNode->childNodes->headNode = node;
	else
		ref->prevNode->nextNode = node;
	ref->prevNode = node;

	node->parentNode->childNodes->length ++;

	return TRUE;
}

/* change first parameter from XMLDOMNode* to XMLDOMNode** */
static BOOL	_DOMNode_replaceChild(XMLDOMNode **po, XMLDOMNode *n)
{
	#define o (*po)

	if(o==(XMLDOMNode*)NULL)
		return FALSE;

	#if (0000)
	/* method one */
	_DOMNode_insertBefore(o,n);
	_DOMNode_removeChild(o);
	o = n;
	#else
	/* method two */
	n->parentNode = o->parentNode;
	n->prevNode = o->prevNode;
	n->nextNode = o->nextNode;

	if(o->prevNode != NULL)
		o->prevNode->nextNode = n;
	if(o->nextNode != NULL)
		o->nextNode->prevNode = n;

	/* update node o's parent info */
	if(o->parentNode != NULL && o->parentNode->hasChildNodes) {
		if(o->parentNode->childNodes->headNode == o)
			o->parentNode->childNodes->headNode = n;
		if(o->parentNode->childNodes->curNode == o)
			o->parentNode->childNodes->curNode = n;
	}

	recurse_set_node(n,
					NODE_ATTRIBUTE_OWNERDOCUMENT,
					(void*) o->ownerDocument);

	_DOMDoc_releaseNode(&o);
	o = n;
	#endif

	#undef o

	return TRUE;
}

static XMLDOMNode* _DOMNode_duplicateChild(XMLDOMNode* r, XMLDOMNode* parent)
{
	XMLDOMNode* n;

	if(r == (XMLDOMNode*) NULL)
		return NULL;

	if((n = (XMLDOMNode*) malloc(sizeof(XMLDOMNode))) == NULL) {
		_log(L_ERR,"malloc new node failure in duplicateChild\n");
		return NULL;
	}

	memcpy(n, r, sizeof(*n));

	/* n->ownerDocument = r->ownerDocument
	here we simply let new node's ownerDocument
	equal to older */
	n->ownerDocument = (parent == NULL)?NULL:parent->ownerDocument;

	n->nextNode = NULL;
	n->prevNode = NULL;
	n->parentNode = parent;

	/* duplicate child nodes */
	{
	XMLDOMNode* ln;
	XMLDOMNodeList* list, *nlist;

	if(r->hasChildNodes==TRUE) {
		list = r->childNodes;

		/* duplicate node list */
		if((nlist = _DOMNode_createNodeList()) == NULL) {
			_log(L_ERR,"error on malloc nodelist\n");
			return NULL;
		}

		memcpy(nlist, list, sizeof(*nlist));
		nlist->headNode = NULL;
		nlist->curNode = NULL;

		list->reset(list);
		while((ln = list->nextNode(list)) != (XMLDOMNode*)NULL) {
			XMLDOMNode *c, *nw = _DOMNode_duplicateChild(ln, n);
			if(nlist->headNode == NULL)
				nlist->headNode = nw;
			else {
				for(c = nlist->headNode; c->nextNode != NULL; )
					c = c->nextNode;
				c->nextNode = nw;
				nw->prevNode = c;
			}
		}

		n->childNodes = nlist;
	}
	}

	n->namespaceURI = estrdup(r->namespaceURI);
	n->nodeName = estrdup(r->nodeName);
	n->text = estrdup(r->text);

	return n;
}


#endif

#if 1 /*nodeList methods*/
static XMLDOMNodeList* _DOMNode_createNodeList(void)
{
	XMLDOMNodeList* pn;
  	if((pn = (XMLDOMNodeList*) malloc(sizeof(XMLDOMNodeList))) == NULL)
  		return NULL;

  	pn->length = 0;
  	pn->get_item = _DOMNodeList_get_item;
  	pn->nextNode = _DOMNodeList_nextNode;
  	pn->reset = _DOMNodeList_reset;
  	pn->headNode = (XMLDOMNode*)NULL;
  	pn->curNode = (XMLDOMNode*)NULL;

  	return pn;
}

static XMLDOMNode* _DOMNodeList_get_item(XMLDOMNode* parent,long index)
{
	XMLDOMNode* node;

	if(index<0||index>=parent->childNodes->length)
		return (XMLDOMNode*)NULL;

	node = parent->childNodes->headNode;
	while(index--)
		node = node->nextNode;
	return node;
}

static XMLDOMNode* _DOMNodeList_nextNode(XMLDOMNodeList* list)
{
	XMLDOMNode* node;

	if(list == (XMLDOMNodeList*)NULL)
		return (XMLDOMNode*)NULL;

	node = list->curNode;
	if(node != (XMLDOMNode*)NULL)
		list->curNode = list->curNode->nextNode;
	return node;
}

static int _DOMNodeList_reset(XMLDOMNodeList* list)
{
	if(list == (XMLDOMNodeList*)NULL)
		return TRUE;

	list->curNode = list->headNode;
	return TRUE;
}

#endif
