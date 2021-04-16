
/* change to != NODE_ELEMENT */
/* NOTE that reason use static string, it do NOT need release */

/* release_node_elmnt() to fix release element memory leak */
/* almost rewrite get_external_dtd() to debug memory leak */
/* add NOTATION_ITEM release code */

/* NULL leak fixed in func. _XMLDOM_prepare_dtd() */
/* add "" special deal for strtok("" a) will return a NOT "" */
/* change type "== NODE_COMMENT" to "!= NODE_ELEMENT" */
/* change from L_ERR to L_NOTE at match_node() MODE_ARRAY */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <stdarg.h>

#include "dtd_chk.h"

#include "domlog.h"
#include "domlib.h"
#include "sg_domtree.h"
#include "sg_domapi.h"


static void* create_node(int type, char* name);
static void release_node(int type, void** pv);

static struct dtd_count prepare_hash_tbl(char* dtd, 
			struct dtd_node* tbl[], 
			struct dtd_attnode* att_tbl[],  
			struct dtd_entity* ent_tbl[],
			struct dtd_notation* not_tbl[],
			int hashi);

static int parse_hash_tbl(struct dtd_node* tbl[], int hashi);
static int parse_att_hash_tbl(struct dtd_attnode* tbl[], int hashi);
static int hash_t(char* seed, int hashi);
static enum __action__ get_action(char* de, long* pstart, long* pend);
static struct dtd_node* get_child_list(char* de, long start, long end);
static void* get_entity(int flags, void* v);
static int check_hash_tbl(struct dtd_mngr* dm, int hashi);
static int check_ent_hash_tbl(struct dtd_mngr* dm, int hashi);
static int check_att_hash_tbl(struct dtd_mngr* dm, int hashi);
static int check_not_hash_tbl(struct dtd_mngr* dm, int hashi);
static int check_node(struct dtd_node* node, struct dtd_node* tbl[], int hashi, int in);
static int att_node_exist(struct dtd_attnode* node, struct dtd_node* tbl[], int hashi);
static int check_att_node(struct dtd_attnode* node, struct dtd_mngr* dm, struct dtd_node* tbl[], int hashi, char* pos);
static int check_att_dupnode(struct dtd_attnode* n);
static int check_ent_sub(struct dtd_entity* tbl[], int hashi);
static int check_attlist_sub(struct dtd_mngr* dm, void* ptr, int hashi);
static int check_attlist_all(struct dtd_mngr* dm, void* ptr, int hashi);
static int check_dtd_sub(struct dtd_mngr* dm, void* ptr, int hashi);
static void free_hash_tbl(int type, void* tbl[], int hashi);
static int add_entity_sub(struct dtd_mngr* dm, void* ptr, int hashi);
static int match_node(struct dtd_node* dn, XMLDOMNode* p, int* pstart);
static int match_child(struct dtd_node* dn, XMLDOMNode* p, int* pstart);
static struct dtd_entity* match_raw_entity(char* rawent, struct dtd_mngr* dm, int hashi);
static struct dtd_entity* match_entity(const char* ent, struct dtd_mngr* dm, int hashi);
static int match_attlist(const char* name, struct dtd_mngr* dm, int hashi,
					struct dtd_attnode** ppe, struct dtd_attnode** ppi);
static int match_attr(struct dtd_attnode* n, char* attr, struct dtd_attnode* t);
static int match_e_value(struct e_value* pev, char* v);
static int match_notation(struct dtd_mngr* dm, struct dtd_attnode* node, struct e_value* ev);
static int meet_end(int start, XMLDOMNode* p);
static BOOL get_external_dtd(struct dtd_mngr* dm, char** dtd);


#if (0001) //准接口函数
int _XMLDOM_prepare_dtd(struct dtd_mngr **pdm, char* root, const char* dtd);
int _XMLDOM_add_entity(struct dtd_mngr* dm, void* ptr);
int _XMLDOM_match_attlist(struct dtd_mngr* dm, void* ptr);
int _XMLDOM_check_dtd(struct dtd_mngr* dm, void* ptr);
int _XMLDOM_release_dtd(struct dtd_mngr** pdm);
#endif

#if (0001) //子函数

static jmp_buf jmp_env;
static void _err(const char* fmt, ...) {
	va_list ap;
	va_start(ap,fmt);
	dom_log(L_ERR, (const char*) fmt, ap);
	va_end(ap);
	longjmp(jmp_env, 0);
}

static void _log(int level, const char* fmt, ...) {
	va_list ap;
	va_start(ap,fmt);
	dom_log(level, (const char*) fmt, ap);
	va_end(ap);
}

#endif

#if (10001) //模块函数

static void* create_node(int type, char* name)
{
	struct dtd_node* elmn;
	struct dtd_attnode* att;
	struct dtd_entity* ent;
	struct dtd_notation* not;

	if(name == NULL)
		return NULL;

	switch(type) {
	case ELEMENT_ITEM:
		elmn = (struct dtd_node*) malloc(sizeof(struct dtd_node));
		if( elmn != NULL ) 
			memset(elmn, 0, sizeof(*elmn));
		return (void*) elmn;
	case ATTLIST_ITEM:
		att = (struct dtd_attnode*) malloc(sizeof(struct dtd_attnode));
		if( att != NULL ) 
			memset(att, 0, sizeof(*att));
		return (void*) att;
	case ENTITY_ITEM:
		ent = (struct dtd_entity*) malloc(sizeof(struct dtd_entity));
		if( ent != NULL ) 
			memset(ent, 0, sizeof(*ent));
		return (void*) ent;		
	case NOTATION_ITEM:
		not = (struct dtd_notation*) malloc(sizeof(struct dtd_notation));
		if( not != NULL ) 
			memset(not, 0, sizeof(*not));
		return (void*) not;
	default:
		_err("invalid dtd item meet");
	};

	return NULL;
}

/* 
add this function to fix release element node 
memory leak  
*/
static void release_node_elmnt(struct dtd_node** pn, int delnext)
{
	if((*pn)->name != NULL) 
		free((*pn)->name);

	if((*pn)->define != NULL)
		free((*pn)->define);
		
	if((*pn)->child != NULL) 
		release_node_elmnt(&((*pn)->child), 1);

	if(delnext == 1) {
		if((*pn)->next != NULL) {
			struct dtd_node *t,*tt;
			for(t = (*pn)->next; t != NULL; ) {
				tt = t->next;
				release_node_elmnt(&t, 0);
				t = tt;	
			}
			(*pn)->next = NULL;
		}
	}
	
	free(*pn);
	*pn = NULL;
}

static void release_node(int type, void** pv)
{
	if(*pv == NULL)
		return;

	switch(type) {
	case ELEMENT_ITEM:
		{
		struct dtd_node** pn = (struct dtd_node**) pv;
		if((*pn)->name != NULL) 
			free((*pn)->name);

		if((*pn)->define != NULL)
			free((*pn)->define);
		
		/* rewrite element node release 
		 */
		if((*pn)->child != NULL) 
			release_node_elmnt(&((*pn)->child), 1);

		if (0000) {	
		/* 
		NOTE we do NOT need to release here
		for in root element next MUST be NULL
		*/
		if((*pn)->next != NULL) {
			struct dtd_node *t,*tt;
			for(t = (*pn)->next; t != NULL; ) {
				tt = t->next;
				release_node_elmnt(&t, 0);
				t = tt;	
			}
			(*pn)->next = NULL;
		}
		}
		
		free(*pn);
		*pn = NULL;
		}
		break;
	case ATTLIST_ITEM:
		{
		struct dtd_attnode** pn = (struct dtd_attnode**) pv;
		if((*pn)->name != NULL) 
			free((*pn)->name);
		
		if((*pn)->define != NULL) 
			free((*pn)->define);
		
		if((*pn)->attr != NULL) 
			free((*pn)->attr);

		if((*pn)->e_value != NULL) {
			struct e_value* ev, *nt;
			for(ev = (*pn)->e_value; ev != NULL; ) {
				nt = ev->next;
				if(ev->value != NULL)
					free(ev->value);
				free(ev);
				ev = nt;
			}
		}
		
		if((*pn)->defaut != NULL) 
			free((*pn)->defaut);
		
		if((*pn)->sibling != NULL) {
			struct dtd_attnode* ev, *nt;
			for(ev = (*pn)->sibling; ev != NULL; ) {
				nt = ev->next;
				release_node(ATTLIST_ITEM, (void**) &ev);				
				ev = nt;
			}
		}
		
		free(*pn);
		*pn = NULL;
		}
		break;
	case ENTITY_ITEM:
		{
		struct dtd_entity** pn = (struct dtd_entity**) pv;
		if((*pn)->name != NULL) 
			free((*pn)->name);

		if((*pn)->define != NULL)
			free((*pn)->define);

		if((*pn)->entity != NULL)
			free((*pn)->entity);

		switch((*pn)->data.type) {
		case ENT_DATA_XML_WHOLE:
		case ENT_DATA_XML_PIECE:
	  		ReleaseObject("FOOLFISH.XMLDOM",/* (void**) */( XMLDOMDocument** ) &((*pn)->data.ptr));
			break;
		case ENT_DATA_UNKNOWN:
			/* also release data.ptr data maybe it a gif file
			 */
			if((*pn)->data.ptr != NULL)
				free((*pn)->data.ptr);
			break;
		case ENT_DATA_INVALID:
		default:
			_err("unknown or invalid ent data meet\n");
		}
		
		free(*pn);
		*pn = NULL;
		}
		break;
	case NOTATION_ITEM:
		/* add notation item release */
		{
		struct dtd_notation** pn = (struct dtd_notation**) pv;
		if((*pn)->name != NULL)		
			free((*pn)->name);

		if((*pn)->define != NULL)
			free((*pn)->define);

		if((*pn)->publicID != NULL)	
			free((*pn)->publicID);
		if((*pn)->url != NULL)
			free((*pn)->url);
		
		free(*pn);
		*pn = NULL;
		}
		break;
	default:
		return;
	}
	
	return;
}


static struct dtd_count prepare_hash_tbl(char* dtd, 
						struct dtd_node* tbl[], 
						struct dtd_attnode* att_tbl[], 
						struct dtd_entity* ent_tbl[], 
						struct dtd_notation* not_tbl[],
						int hashi)
{
	char* t =NULL;
	char *ptr;
	int HQ;
	struct dtd_count count;
	enum __dtd_item__ item_t;
	
	memset(&count, 0 , sizeof(count));

	if(dtd == NULL || tbl == NULL)
		return count;
	
	ptr = estrdup(dtd);
	
	for(t = strtok(ptr, " "); t != NULL; t = strtok(NULL, " "))
	{
		while(*t == '\t' || *t == ' ' || *t == '\r' || *t == '\n')
			t++;

		item_t = INVALID_ITEM;
		if(strcmp(t,"<!ELEMENT") == 0) 
			item_t = ELEMENT_ITEM;
		else if(strcmp(t,"<!ATTLIST")==0) 
			item_t = ATTLIST_ITEM;
		else if(strcmp(t,"<!NOTATION")==0)
			item_t = NOTATION_ITEM;
		else if(strcmp(t,"<!ENTITY")==0) 
			item_t = ENTITY_ITEM;
		else
			continue;

		switch(item_t) {
		case ELEMENT_ITEM:			
			{
			struct dtd_node* pn;
			pn = (struct dtd_node*) create_node(ELEMENT_ITEM, t);
			if(pn == NULL) 
				_err("malloc for node fail\n");

			memset(pn, 0, sizeof(*pn));		
			pn->name = strdup(strtok(NULL," "));
			pn->define = strdup(strtok(NULL,">"));
			pn->flag = FLAG_NORMAL;
			pn->hash_nt = NULL;

			HQ = hash_t(pn->name, hashi);
			if(tbl[HQ] == NULL) 
				tbl[HQ] = pn;
			else {
				struct dtd_node* p;
				for(p = tbl[HQ]; p != NULL; p = p->hash_nt) {
					if(strcmp(p->name,pn->name)==0) 
						_err("same element describe");				
					if(p->hash_nt == NULL) {
						p->hash_nt = pn;
						break;
					}
				}
			}			
			count.elmn ++;
			}
			break;
		case ATTLIST_ITEM:
			{
			struct dtd_attnode* patt;
			patt = (struct dtd_attnode*) create_node(ATTLIST_ITEM, t);
			if(patt == NULL) 
				_err("malloc for node fail\n");

			patt->name = strdup(strtok(NULL," "));
			patt->define = strdup(strtok(NULL,">"));
			patt->flag = FLAG_NORMAL;
			patt->hash_nt = NULL;

			HQ = hash_t(patt->name, hashi);
			if(att_tbl[HQ] == NULL) 
				att_tbl[HQ] = patt;
			else {
				struct dtd_attnode* p;
				for(p = att_tbl[HQ]; p != NULL; p = p->hash_nt) {
					if(strcmp(p->name,patt->name)==0) {
						struct dtd_attnode* n;						
						for(n = p; n->next != NULL; n = n->next)
							;
						n->next = patt;
						break;
					}
					if(p->hash_nt == NULL) {
						p->hash_nt = patt;
						break;
					}
				}
			}
			count.attlist ++;
			}
			break;		
		case ENTITY_ITEM:
			{ //add entoty parse 
			struct dtd_entity* pn;
			char* s;
			
			pn = (struct dtd_entity*) create_node(ENTITY_ITEM, t);
			if(pn == NULL) 
				_err("malloc for entity node fail size[%d]\n",sizeof(*pn));
			
			memset(pn, 0, sizeof(*pn));

			s = strtok(NULL," ");			
			pn->name = strdup(s);

			{ /* get entity value between two "" */
			char *ep, *sp = s+strlen(pn->name)+1;
			int ct = 0;
			for(ep = sp; *ep != '\0'; ep++) {
				if(*ep == '"') ct ++;
				if(ct == 2) {
					char* tt = (char*) malloc(ep-sp+1+1);
					memset(tt, 0, ep-sp+1+1);
					memcpy(tt, sp, ep-sp+1);
					pn->define = tt;

					t = strtok(ep, ">");					
					break;
				}
			}
			if(pn->define == NULL)
				_err("entity [%s]'s defination NOT found\n",pn->name);
			}
			
			pn->flag = FLAG_NORMAL;
			pn->hash_nt = NULL;

			HQ = hash_t(pn->name, hashi);
			if(ent_tbl[HQ] == NULL) 
				ent_tbl[HQ] = pn;
			else {
				struct dtd_entity* p;
				for(p = ent_tbl[HQ]; p != NULL; p = p->hash_nt) {
					if(strcmp(p->name,pn->name)==0) {
						_log(L_NOTE, "same ENTITY [%s] describe [%s] meet, "
							"ignore the 2nd one\n",pn->name, pn->define);
						count.entity_ign ++;
						break;
					}
					if(p->hash_nt == NULL) {
						p->hash_nt = pn;
						break;
					}
				}
			}			
			count.entity ++;
			}
			break;
		case NOTATION_ITEM:
			{ //add notation parse 
			struct dtd_notation* pn;
			char* s;
			
			pn = (struct dtd_notation*) create_node(NOTATION_ITEM, t);
			if(pn == NULL) 
				_err("malloc for notation node fail size[%d]\n",sizeof(*pn));
			
			memset(pn, 0, sizeof(*pn));

			s = strtok(NULL," ");			
			pn->name = strdup(s);

			pn->define = strdup(strtok(NULL,">"));
			if(pn->define == NULL)
				_err("notation [%s]'s defination NOT found\n",pn->name);
			
			pn->flag = FLAG_NORMAL;
			pn->hash_nt = NULL;

			HQ = hash_t(pn->name, hashi);
			if(not_tbl[HQ] == NULL) 
				not_tbl[HQ] = pn;
			else {
				struct dtd_notation* p;
				for(p = not_tbl[HQ]; p != NULL; p = p->hash_nt) {
					if(strcmp(p->name,pn->name)==0) {
						_log(L_NOTE, "same NOTATION [%s] describe [%s] meet, "
							"ignore the 2nd one\n",pn->name, pn->define);
						count.notation_ign ++;
						break;
					}
					if(p->hash_nt == NULL) {
						p->hash_nt = pn;
						break;
					}
				}
			}			
			count.notation ++;
			}			
			break;
		default:
			break;
		}
	}

	if(ptr != NULL)
		free(ptr);	

	return count;
}

/* 
 * 解析struct dtd_node* tbl[HASH]中的数据生成数据结构
 */
static int parse_hash_tbl(struct dtd_node* tbl[], int hashi)
{
	int i;
	struct dtd_node* node;

	if(tbl == NULL || hashi <=0)
		return 0;

	_log(L_DBG, "=== parse hash tbl ===\n");

	for(i = 0; i < hashi; i++) {
		for(node = tbl[i]; node!=NULL; node = node->hash_nt) {
			long start, end;
			
			_log(L_DBG, "i[%d]\tnode[%x]\tname[%s]\tdefine[%s]\n",
				i,node,node->name,node->define);

			/* ====== KEY CODE ====== */
			node->flag = FLAG_NORMAL;			
			node->mode = MODE_ARRAY;						
			node->action = get_action(node->define, &start, &end); //*,+,?,one
			switch(node->action) {
			case ACTION_INVALID:
				_err("invalid action meet");				
			default:
				
				/* Searching ROOT elements' defination...
				 * if found ANY or EMPTY then mark it
				 * else call get_child_list to parse ot
				 */
				if(memcmp((char*)(start+1),"ANY",(end-start-1))==0) {
					node->action = ACTION_ANY;
					node->child = (struct dtd_node*) NULL;
				}
				else if (memcmp((char*)(start+1),"EMPTY",(end-start-1))==0) {
					node->action = ACTION_EMPTY;
					node->child = (struct dtd_node*) NULL;					
				}
				else
					node->child = get_child_list(node->define, start, end);
				
				break;
			}
			node->prev = NULL;
			node->next = NULL;
			/* ====== KEY CODE ====== */			
		}
	}

	return 1;
}


/* 解析struct dtd_attnode* tbl[HASH]中的数据生成数据结构
 */
static int parse_att_hash_tbl(struct dtd_attnode* tbl[], int hashi)
{
	int newa = 0, i;
	struct dtd_attnode* node;	
	#define LOCAL_SEP " \r\n\t"

	if(tbl == NULL || hashi <=0)
		return 0;

	_log(L_DBG,"=== parse attlist hash tbl ===\n");
	
	for(i = 0; i < hashi; i++) {
		for(node = tbl[i]; node!=NULL; node = node->hash_nt) {
			struct dtd_attnode* n;
			for(n = node; n != NULL; n = n->next) {
				struct dtd_attnode* pre = n;
				char *str = strdup(n->define);
				char *c, *nt = NULL;
				
				_log(L_DBG,"i[%d]\tnode[%x]\tname[%s]\tdefine[%s]\n",
					i,n,n->name,n->define);

				c = strtok(str, LOCAL_SEP);

sibling:
				n->flag = FLAG_NORMAL;
				n->type = ATT_TYPE_INVALID;
				n->attr = strdup(c);
				n->defaut = NULL;

				/* ==================== */
				/* seeking attribute 10 types here */
				/* ==================== */

				c = strtok(NULL,LOCAL_SEP);
				if(strcmp(c,"CDATA")==0)
					n->type = ATT_TYPE_CDATA;
				else if(strcmp(c,"ID")==0)
					n->type = ATT_TYPE_ID;
				else if(strcmp(c,"IDREF")==0)
					n->type = ATT_TYPE_IDREF;
				else if(strcmp(c,"IDREFS")==0)
					n->type = ATT_TYPE_IDREFS;
				else if(strcmp(c,"ENTITY")==0)
					n->type = ATT_TYPE_ENTITY;
				else if(strcmp(c,"ENTITIES")==0)
					n->type = ATT_TYPE_ENTITIES;
				else if(strcmp(c,"NMTOKEN")==0)
					n->type = ATT_TYPE_NMTOKEN;
				else if(strcmp(c,"NMTOKENS")==0)
					n->type = ATT_TYPE_NMTOKENS;				
				else if(strcmp(c,"NOTATION")==0)
					n->type = ATT_TYPE_NOTATION;				
				else if(strncmp(c,"(",1)==0)
					n->type = ATT_TYPE_ENUM;
				else
					n->type = ATT_TYPE_INVALID;

				if(n->type == ATT_TYPE_INVALID)
					_err("invalid att type meet");

				/* relocation current string pointer */
				nt = c + strlen(c) + 1;

				/* cut notation left '(' at here */
				if(n->type == ATT_TYPE_NOTATION) {
					char* et;
					if((et = strchr(nt,'(')) == NULL)
						_err("invalid notation format");
					nt = et + 1;
				}

				if(n->type == ATT_TYPE_ENUM || n->type == ATT_TYPE_NOTATION) {
					char* et, *ett;
					struct e_value* ev;
				
					/* process Enumberated here */
					//et = strdup((nt = strtok(NULL, ")")));
					et = strdup((nt = strtok(nt, ")")));
					if(et == NULL)
						_err("empty or invalid enum value meet");
					nt = nt + strlen(nt) + 1;

					for(ett = strtok(et,"|"); ett != NULL; ett = strtok(NULL, "|")) {
						ev = (struct e_value*) malloc(sizeof(*ev));
						if(ev == NULL)
							_err("alloc memory fail");
						ev->next = NULL;
						ev->value = strdup(ett);
						
						/* process Enumberated here */
						if(n->e_value == NULL)
							n->e_value = ev;
						else {
							struct e_value* ttt;
							for(ttt = n->e_value; ttt->next != NULL; ttt =ttt->next)
								;
							ttt->next = ev;
						}
					}	
					free(et);
					et = NULL;					
				}
				else {
					nt = c + strlen(c) + 1;
				}
			

				/* ========================= */
				/* seeking attribute DEFAULT-MODE here */
				/* ========================= */
				{
				char* t;
				for(t = nt; t != NULL ; t++) {
					if(*t == ' ' || *t == '\t' || *t == '\r' || *t == '\n')
						continue;

					if(*t == '#') {
						c = strtok(t, LOCAL_SEP);
						if(strcmp(c,"#REQUIRED")==0)
							n->mode = ATT_MODE_REQUIRED;
						else if(strcmp(c,"#IMPLIED")==0)
							n->mode = ATT_MODE_IMPLIED;
						else if(strcmp(c,"#FIXED")==0)
							n->mode = ATT_MODE_FIXED;
						else
							_err("invalid default type");
						nt = c + strlen(c) + 1;
						break;
					}
					else if(*t == '\"') {
						n->mode = ATT_MODE_DEFAULT;
						nt = t;
						break;
					}
				}
				}

				/* get fixed or default value */
				if(n->mode == ATT_MODE_FIXED ||
				  n->mode == ATT_MODE_DEFAULT) {
					/* add "" special deal  
					for strtok("" a) will return a NOT "" */
					#define __empty_string__ "\"\""
					if(strncmp(nt,__empty_string__,
							strlen(__empty_string__))==0) {
						n->defaut = strdup("");
						nt = nt + strlen(__empty_string__);
					}
					/* add end */
					else {
						c = strtok(nt, "\"");
						n->defaut = strdup(c);
						nt = c + strlen(c) + 1;
					}
				}

				/* found next att defination */
				if((c = strtok(nt, LOCAL_SEP)) != NULL) {
					struct dtd_attnode* patt;
					patt = (struct dtd_attnode*) create_node(ATTLIST_ITEM, n->name);
					if(patt == NULL) 
						_err("malloc for node fail\n");

					patt->name = strdup(n->name);
					patt->define = NULL;
					patt->flag = FLAG_NORMAL;
					patt->hash_nt = NULL;

					/* link to sibling list */
					if(n->sibling == NULL) {
						n->sibling = patt;
						newa ++;
					}
					
					/* the following may never been touched 
					  for we recurse add sibling node in new
					  sibling node */
					#if (0000)
					else {
						struct dtd_attnode* ttt;
						for(ttt = n->sibling; ttt->next != NULL; ttt= ttt->next)
							;
						ttt->next = patt;
					}
					#endif

					n = patt;					
					goto sibling;
				}

				free(str);
				str = NULL;

				n = pre;
			}
		}
	}

	return newa;
}

static int parse_ent_hash_tbl(struct dtd_entity* tbl[], int hashi)
{
	int i;
	struct dtd_entity* node;

	if(tbl == NULL || hashi <=0)
		return 0;

	_log(L_DBG,"=== parse entity hash tbl ===\n");

	for(i = 0; i < hashi; i++) {
		for(node = tbl[i]; node!=NULL; node = node->hash_nt) {
			char *p, *t;
			//int param_entity = 0;
			
			_log(L_DBG, "i[%d]\tnode[%x]\tname[%s]\tdefine[%s]\n",
				i,node,node->name,node->define);

			/* ====== KEY CODE ====== */
			p = strdup(node->define);

			/* not this parser NOT suport param entity 
			  so default it's type is common entity 
			   */

			t = strtok(p, " \t\r\n");
			if(strcmp(t,"SYSTEM")==0) {
				node->type = ENT_TYPE_COMMON_EXTERNAL;
				t = strtok(NULL, "\"");
				if(t == NULL) 
					_err("null meet at entity %s\n",node->name);
				node->entity = strdup(t);
			}
			else {
				char* tt = strdup(node->define);
				node->type = ENT_TYPE_COMMON_INTERNAL;
				t = strtok(tt, "\"");
				if(t == NULL) 
					_err("null meet at entity %s\n",node->name);
				node->entity = strdup(t);
				free(tt);
			}
			
			free(p);
			/* ====== KEY CODE ====== */			
		}
	}

	return 1;
}

static int parse_not_hash_tbl(struct dtd_notation* tbl[], int hashi)
{
	int i;
	struct dtd_notation* node;

	if(tbl == NULL || hashi <=0)
		return 0;

	_log(L_DBG,"=== parse notation hash tbl ===\n");

	for(i = 0; i < hashi; i++) {
		for(node = tbl[i]; node!=NULL; node = node->hash_nt) {
			char *p, *t;
			//int param_entity = 0;
			
			_log(L_DBG, "i[%d]\tnode[%x]\tname[%s]\tdefine[%s]\n",
				i,node,node->name,node->define);

			/* ====== KEY CODE ====== */
			p = strdup(node->define);

			t = strtok(p, " \t\r\n");
			if(strcmp(t,"SYSTEM")==0) {
				node->type = TYPE_SYSTEM;
				t = strtok(NULL, "\"");
				if(t == NULL) 
					_err("null meet at notation %s\n",node->name);
				node->publicID = strdup(t);
			}
			else if(strcmp(t,"PUBLIC")==0) {
				char* tt = strdup(node->define);
				node->type = TYPE_PUBLIC;
				strtok(tt, "\"");
				t = strtok(NULL, "\"");
				if(t == NULL) 
					_err("null meet at notation %s\n",node->name);
				node->publicID = strdup(t);
				strtok(NULL, "\"");
				t = strtok(NULL, "\"");
				if(t == NULL) 
					_err("null meet at notation %s\n",node->name);
				node->url = strdup(t);
				free(tt);
			}
			else {
				_log(L_ERR, "notation[%s] define[%s] must start");
				_log(L_ERR, " as \"SYSTEM\" or \"PUBLIC\"\n",
							node->name,node->define);
			}
			
			free(p);
			/* ====== KEY CODE ====== */			
		}
	}

	return 1;
}

static int hash_t(char* seed, int hashi)
{
	char* t = NULL;
	int sum = 0;
	
	for(t = seed; *t != '\0'; t++)
		sum += *t;		

	return (sum % hashi);
}

static enum __action__ get_action(char* de, long* pstart, long* pend)
{
	enum __action__ act;
	char* p;
	
	if(de == NULL)
		return ACTION_INVALID;

	*pstart = *pend = 0;
	act = ACTION_INVALID;

	for(p = de ; p <= de+strlen(de)-1; p++)
	{
		if(*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')	
			continue;

		if(*p == '(') {
			*pstart = (long) p;
			break;
		}

		/* Do NOT check ANY & EMPTY @ here 
		 * or We maybe confused when use this function in child nodes
		 * for child nodes can be ANY and EMPTY */
		/*
		if(strncmp(p, "ANY",strlen("ANY"))==0) 
			act = ACTION_ANY;
		else if(strncmp(p,"EMPTY",strlen("EMPTY"))==0)
			act = ACTION_EMPTY;
		*/
		
		*pstart = (long) p-1;
		break;
	}

	if(*pstart == 0)
		return ACTION_INVALID;

	for(p = de+strlen(de)-1 ; p >= de ; p--)
	{
		if(*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')	
			continue;

		if(*p == '*' || *p == '?' || *p == '+') {
			if(*p == '*') act = ACTION_MULTI;
			if(*p == '?') act = ACTION_QUESTION;
			if(*p == '+') act = ACTION_PLUS;
			continue;
		}

		if(act == ACTION_INVALID) 
			act = ACTION_ONE;

		if(*p == ')')			
			*pend = (long) p;
		else
			*pend = (long) p+1;
		
		break;
	}

	if(*pend == 0)
		return ACTION_INVALID;

	return act;
}

static struct dtd_node* get_child_list(char* de, long start, long end)
{
	struct dtd_node* child = NULL;
	char* p;
	int BRACKET_COUNT;
	enum __mode__ CURRENT_MODE;
	long es,ee;

	BRACKET_COUNT = 0;
	CURRENT_MODE = MODE_ARRAY;
	es = start+1;
	ee = end-1;

	for( p = (char*) start+1; p <= (char*) end; p++)
	{	
		if(p == (char*) end)
			;
		else {
			if (*p == '(') {
				BRACKET_COUNT ++; continue;
			}		
			else if (*p == ')') {
				BRACKET_COUNT --; continue;
			}
		}

		if ((*p == ',' || *p == '|' || p==(char*)end) && BRACKET_COUNT == 0) 
		{
			if(p == (char*) end)
				;
			else {
				if(*p == ',') 
					CURRENT_MODE = MODE_ARRAY;
				else if(*p == '|') 
					CURRENT_MODE = MODE_OR;
			}

			ee = (long) p-1;			
			
			/* add one node here */
			{
			struct dtd_node* pn;
			char* name;
			long lstart,lend;
			
			if(ee < es) 
				_err("ee < es");
			if(CURRENT_MODE == MODE_INVALID)
				_err("invalid mode found");

			//get name
			name = (char*) malloc(ee-es+1+1);
			memset(name, 0, ee-es+1+1);
			memcpy(name, (const void*) es, ee-es+1);

			pn = (struct dtd_node*) create_node(ELEMENT_ITEM, name);
			if(pn == NULL) 
				_err("malloc for node fail");
	
			pn->name = NULL;
			pn->define = name;
			pn->flag = FLAG_NORMAL;
			pn->mode = CURRENT_MODE;
			
			pn->action = get_action(pn->define, 
									&lstart, &lend); //*,+,?,one
			switch(pn->action) {
			case ACTION_INVALID:
				_err("invalid action meet");
				break;

			default:
				/* 如果是单个元素，直接返回 */
				if(strpbrk(name, ",|")==NULL) {
					char* ln;
					long ss, se;

					ln = (char*) malloc(lend-lstart);
					memset(ln, 0, lend-lstart);
					memcpy(ln, (const void*) (lstart+1), lend-lstart-1);

					if(alltrim(ln, &ss, &se)) {
						char* p = (char*) malloc(se-ss+1+1);
						if(p != NULL) {
							memset(p, 0, se-ss+1+1);
							memcpy(p, (const void*)ss, se-ss+1);
							pn->name = p;
						}
					}

					if(pn->name == NULL)
						pn->name = estrdup(ln);

					free(ln);

					pn->child = (struct dtd_node*) NULL;
				}
				else {
					/* 递归取得child 元素 */
					pn->name = estrdup("#GROUP");
					pn->child = get_child_list(name, lstart, lend);
				}
				break;
			}

			_log(L_DBG,"\tnode[%x]\tname[%s]\tdefine[%s]\n",
						pn,pn->name,pn->define);

			if(child == NULL) {
				pn->prev = NULL;
				child = pn;
			}
			else {
				struct dtd_node* t;
				for(t = child; t->next != NULL; t = t->next)
					;
				pn->prev = t;
				t->next = pn;
			}

			pn->next = NULL;			
			pn->hash_nt = NULL;			
			
			es = (long) p+1; ee = 0;
			}
		}		
	}
	
	return child;
}


static void* get_entity(int flags, void* v)
{
	void* ret = NULL;

	if(v == NULL)
		return NULL;

	switch(flags) {
	case ENT_DATA_XML_WHOLE:
	case ENT_DATA_XML_PIECE:
		{
		XMLDOMDocument* doc = (XMLDOMDocument*) v;
		XMLDOMNodeList* list;
		XMLDOMNode* en;
		
		if(doc->documentElement->hasChildNodes==TRUE) {
			list = doc->documentElement->childNodes;
			list->reset(list);
			while((en = list->nextNode(list)) != NULL) 
			{
				switch(flags) {
				case ENT_DATA_XML_WHOLE:
					if(en->nodeType == NODE_ELEMENT)
						return (void*) en;
					break;
				case ENT_DATA_XML_PIECE:
					if(en->nodeType == NODE_ELEMENT) {					
						if(strcmp(en->nodeName, "foolfish")==0 &&
							en->hasChildNodes == TRUE )
							return (void*) en->childNodes->headNode;
						else
							_err("here should be foolfish node and not empty\n");
					}
					break;
				default:
					_err("invalid type meet\n");
				}
			}
		}
		
		}
		break;
	default:
		ret = NULL;
	}

	return ret;
}

static int check_hash_tbl(struct dtd_mngr* dm, int hashi)
{
	int i;
	struct dtd_node* node;

	if(dm == NULL || hashi <=0 )
		return 0;

	_log(L_INFO,"checking ELEMENT in external DTD\n");
	for(i = 0; i < hashi; i++) {
		for(node = dm->ex_tbl[i]; node!=NULL; node = node->hash_nt) {
			if(!check_node(node, dm->ex_tbl, hashi, 0))
				goto error;
		}
	}
	
	_log(L_INFO,"checking ELEMENT in internal DTD\n");
	for(i = 0; i < hashi; i++) {
		for(node = dm->in_tbl[i]; node!=NULL; node = node->hash_nt) {
			if(!check_node(node, dm->in_tbl, hashi, 1)) {
				if(!check_node(node, dm->ex_tbl, hashi, 0))
					goto error;
			}
		}
	}

	return 1;
	
error:
	_err("validate ELEMENT in DTD fault\n");
	return 0;
}

static int check_att_hash_tbl(struct dtd_mngr* dm, int hashi)
{
	int i;
	struct dtd_attnode* n;

	if(dm == NULL || hashi <=0 )
		return 0;


	_log(L_INFO,"checking ATTLIST existence\n");
	for(i = 0; i < hashi; i++) {
		for(n = dm->att_ex_tbl[i]; n!=NULL; n = n->hash_nt) {
			if(!att_node_exist(n, dm->ex_tbl, hashi)) {
				_log(L_ERR,"DTD error: attlist[%s]'s element [%s] "
						"NOT define in external DTD",n->attr,n->name);
				goto error;
			}
		}
	}
	for(i = 0; i < hashi; i++) {
		for(n = dm->att_in_tbl[i]; n!=NULL; n = n->hash_nt) {
			if(att_node_exist(n, dm->in_tbl, hashi))
				continue;
			if(!att_node_exist(n, dm->ex_tbl, hashi)) {
				_log(L_ERR,"DTD error: attlist[%s]'s element [%s] "
					"NOT define in internal or external DTD",n->attr,n->name);
				goto error;
			}
		}
	}

	_log(L_INFO,"checking external ATTLIST defination\n");
	for(i = 0; i < hashi; i++) {
		for(n = dm->att_ex_tbl[i]; n!=NULL; n = n->hash_nt) {
			if(!check_att_node(n, dm, dm->ex_tbl, hashi, "internal"))
				goto error;
		}
	}
	_log(L_INFO,"checking internal ATTLIST defination\n");
	for(i = 0; i < hashi; i++) {
		for(n = dm->att_in_tbl[i]; n!=NULL; n = n->hash_nt) {
			if(!check_att_node(n, dm, dm->in_tbl, hashi, "internal"))
				goto error;
		}
	}
	

	/* TODO: delete duplicate attlist in internal and external attlist 
	 */
	_log(L_INFO,"checking external ATTLIST duplicate atts\n");
	for(i = 0; i < hashi; i++) {
		for(n = dm->att_ex_tbl[i]; n!=NULL; n = n->hash_nt) {
			int num = check_att_dupnode(n);
			if(num > 0) {
				_log(L_INFO,"Total [%d] duplicate attributes in element [%s] found\n",
						num, n->name);
				dm->eye.ex.attlist_ign += num;
			}
		}
	}
	
	_log(L_INFO,"checking internal ATTLIST duplicate atts\n");
	for(i = 0; i < hashi; i++) {
		for(n = dm->att_in_tbl[i]; n!=NULL; n = n->hash_nt) {
			int num = check_att_dupnode(n);
			if(num > 0) {
				_log(L_NOTE, "Total [%d] duplicate attributes in element [%s] found\n",
						num, n->name);
				dm->eye.in.attlist_ign += num;
			}
		}
	}

	/* if exist external node sample as internal node, delete
	the exteranl node, for interanl node is priority to exteranl 
	we what to mask these external node */
	_log(L_INFO,"checking external ATTLIST node same with interanl\n");
	for(i = 0; i < hashi; i++) {
		for(n = dm->att_ex_tbl[i]; n!=NULL; n = n->hash_nt) {
			struct dtd_attnode *t1,*t2;
			for(t1 = n; t1 != NULL; t1 = t1->next) {
				for(t2 = t1; t2 != NULL; t2 = t2->sibling) {
					if(t2->flag == FLAG_NOUSE)
						continue;
					
					/* seek attr in internal attlist */
					{
					struct dtd_attnode* m;
					int k, found = 0;
					for(k = 0; k < hashi; k++) {
						for(m = dm->att_in_tbl[k]; m != NULL; m = m->hash_nt) {
							if(strcmp(m->name, t2->name)==0) {
								if(match_attr(m,t2->attr,NULL)) 
									found = 1;
								goto cont;
							}
						}
					}						
cont:
					if(found == 1) {
						t2->flag = FLAG_NOUSE;
						_log(L_NOTE,"external element[%s].att[%s].define ALREADY exist in internal. ignore this\n",
							t2->name, t2->attr);
						dm->eye.ex.attlist_ign += 1;
					}
					}
				}
			}
		}
	}	

	return 1;
	
error:
	_err("validate ATTLIST element fault !\n");	
	return 0;
}

static int check_ent_hash_tbl(struct dtd_mngr* dm, int hashi)
{
	int ret = 0;

	_log(L_INFO,"checking external ENTITY defination\n");
	ret = check_ent_sub(dm->ent_ex_tbl, hashi);
	
	_log(L_INFO,"checking internal ENTITY defination\n");
	ret = check_ent_sub(dm->ent_in_tbl, hashi);

	return ret;
}


static int check_not_hash_tbl(struct dtd_mngr* dm, int hashi)
{
	struct dtd_notation *dn,*dn2;
	int i,j, ret = 0;

	_log(L_INFO,"checking external NOTATION defination\n");
	_log(L_INFO,"checking internal NOTATION defination\n");

	/* find wether exteranl have same NOTATION with internal DTD defination */
	for(i = 0; i < hashi; i++) {
		for(dn = dm->not_ex_tbl[i]; dn != NULL; dn = dn->hash_nt) {
			for(j = 0; j < hashi; j++) {
				for(dn2 = dm->not_in_tbl[i]; dn2 != NULL; dn2 = dn2->hash_nt) {
					if(strcmp(dn->name, dn2->name)==0) {
						_log(L_NOTE,"external DTD have same notation[%s] with internal, ignore it\n",
								dn2->name);
						dn2->flag = FLAG_NOUSE;
						dm->eye.ex.notation_ign ++;
					}
				}
			}
		}
	}	

	return ret;
}


static int check_node(struct dtd_node* node, struct dtd_node* tbl[], int hashi, int position)
{				
	int i, found = 0;
	struct dtd_node* n;
	
	if(node == NULL || tbl == NULL || hashi <= 0)
		return 0;

	/* check wether ELEMENT's leaf nodes has been 
	   define in other leaf ELEMENT */
	
	if(node->child == NULL) {
		switch(node->action) 
		{
		case ACTION_ANY:
		case ACTION_EMPTY:
			break;	//Do NOT need to check ANY & EMPTY node
		case ACTION_ONE:
		case ACTION_PLUS:
		case ACTION_QUESTION:
		case ACTION_MULTI:
			for(i = 0; !found && i < hashi ; i++) {
				for(n = tbl[i]; !found && n!=NULL; n = n->hash_nt) 
					if(strcmp(n->name,node->name)==0) 
						found = 1;
			}
			if(!found && !(strcmp(node->name, "#PCDATA")==0)) {
				if(position == 1)
					_log(L_NOTE,"DTD note: element '%s' is undefined in internal DTD\n",node->name); 
				else {
					_log(L_ERR,"DTD error: element '%s' is undefined in external DTD\n",node->name);
					return 0;
				}
			}
			break;
		case ACTION_INVALID:
			return 0;
		default:
			return 0;
		}
	}	

	if(node->child != NULL)
		if(!check_node(node->child, tbl, hashi, position))
			return 0;

	if(node->next != NULL)
		if(!check_node(node->next, tbl, hashi, position))
			return 0;

	return 1;
}



static int att_node_exist(struct dtd_attnode* node, struct dtd_node* tbl[], int hashi)
{				
	int i;
	//int found = 0;
	struct dtd_node* n;
	
	if(node == NULL || tbl == NULL || hashi <= 0)
		return 0;

	/* check wether attlist's ELEMENT name has been define */	
	for(i = 0; i < hashi ; i++) {
		for(n = tbl[i]; n!=NULL; n = n->hash_nt) {
			if(strcmp(n->name,node->name)==0)
				return 1;
		}
	}
	
	return 0;	
}


static int check_att_node(struct dtd_attnode* node, struct dtd_mngr* dm, struct dtd_node* tbl[], int hashi, char* pos)
{				
	struct dtd_attnode *t1, *t2;	
	
	if(node == NULL || tbl == NULL || hashi <= 0)
		return 0;

	/* trim e_value front and end space character 
	 */
	for(t1 = node; t1 != NULL; t1 = t1->next) {
		for(t2 = t1; t2 != NULL; t2 = t2->sibling) {
			struct e_value* pv;
			if(!(t2->type == ATT_TYPE_ENUM ||
				t2->type == ATT_TYPE_NOTATION))
				continue;
			for(pv = t2->e_value; pv != NULL; pv = pv->next) {
				long ss, se;
				if(alltrim(pv->value, &ss, &se)==1) {
					char* p = (char*) malloc(se-ss+1+1);
					if(p == NULL)
						continue;
					memset(p, 0, se-ss+1+1);
					memcpy(p, (const void*) ss, se-ss+1);
					free(pv->value);
					pv->value = p;
				}
			}
		}
	}

	/* wether default value in enum set 
	and check wether value in enum is valid */
	for(t1 = node; t1 != NULL; t1 = t1->next) {
		for(t2 = t1; t2 != NULL; t2 = t2->sibling) {
			if(!(t2->type == ATT_TYPE_ENUM ||
				t2->type == ATT_TYPE_NOTATION))
				continue;

			/* wether notation value in enum is valid */
			if(t2->type == ATT_TYPE_NOTATION)
				if(!match_notation(dm,t2,t2->e_value))
					return 0;							
			
			if(t2->mode != ATT_MODE_DEFAULT && 
				t2->mode != ATT_MODE_FIXED)
				continue;
			if(!match_e_value(t2->e_value, t2->defaut)) {
				_log(L_ERR,"DTD error: default value [%s] NOT found in Enumbered set\n",t2->defaut);
				return 0;
			}
		}
	}


	/* 
	 * due to expat already check this, 
	 * these code can NOT be access 
	 */
	/* check ID type wether defined as #FIXED or default value,
	if so error.  */
	for(t1 = node; t1 != NULL; t1 = t1->next) {
		for(t2 = t1; t2 != NULL; t2 = t2->sibling) {
			if(t2->type != ATT_TYPE_ID)
				continue;
			if(t2->mode == ATT_MODE_FIXED || t2->mode == ATT_MODE_DEFAULT) {
				_log(L_ERR,"DTD error: element[%s]'s ID attribute[%s] must be either #REQUIRED or #IMPLIED\n",
							t2->name, t2->attr);
				return 0;
			}
		}
	}

	/* check wether att node only have one ID attribute 
	 */
	{
	BOOL has_ID = FALSE;
	for(t1 = node; t1 != NULL; t1 = t1->next) {
		for(t2 = t1; t2 != NULL; t2 = t2->sibling) {
			if(t2->type != ATT_TYPE_ID)
				continue;
			if(has_ID == TRUE) {
				_log(L_ERR,"DTD error: element[%s] may only have one ID attribute\n",t2->name);
				return 0;
			}
			has_ID = TRUE;
		}
	}
	}

	/* check IDREF|IDREEFS|NMTOKEN|NMTOKENS|ENTITY|ENTITIES 
	#FIXED or #DEFAULT value wether is legal */
	for(t1 = node; t1 != NULL; t1 = t1->next) {
		for(t2 = t1; t2 != NULL; t2 = t2->sibling) {

			/* pass #REQUIRED and #IMPLIED mode */
			if(t2->mode != ATT_MODE_DEFAULT && t2->mode != ATT_MODE_FIXED)
				continue;
			
			switch(t2->type) {
			case ATT_TYPE_IDREF:
				if(!isvalidatexmlname(t2->defaut)) {
					_log(L_ERR,"DTD error: element[%s]'s attribute[%s]'s type[%s] default"
							" value [%s] NOT an validate xml name\n",
							t2->name, t2->attr, attrTypeString(t2->type), t2->defaut);
					return 0;
				}
				break;
			case ATT_TYPE_IDREFS:
				{
				char *p, *idrefs = estrdup(t2->defaut);
				if(idrefs == NULL)
					_err("not enough memory on ifrefs\n");				
				p = strtok(idrefs, " ");
				while(p != NULL) {
					if(!isvalidatexmlname(p)) {
						_log(L_ERR,"DTD error: one of element[%s]'s attribute[%s]'s type[%s] default[%s]"
								" value[%s] NOT an validate xml name\n",
								t2->name, t2->attr, attrTypeString(t2->type), t2->defaut, p);
						free(idrefs);
						return 0;
					}
					p = strtok(NULL, " ");
				}				
				free(idrefs);
				}
				break;
			case ATT_TYPE_NMTOKEN:
				if(!isxmlname(t2->defaut)) {
					_log(L_ERR,"DTD error: element[%s]'s attribute[%s]'s type[%s] default"
								" value[%s] NOT an xml name\n",
								t2->name, t2->attr, attrTypeString(t2->type), t2->defaut);
					return 0;
				}
				break;
			case ATT_TYPE_NMTOKENS:
				{
				char *p, *nmtokens = estrdup(t2->defaut);
				if(nmtokens == NULL)
					_err("not enough memory on nmtokens\n");				
				p = strtok(nmtokens, " ");
				while(p != NULL) {
					if(!isxmlname(p)) {
						_log(L_ERR,"DTD error: one of element[%s]'s attribute[%s]'s type[%s] default[%s]"
									" value [%s] NOT an validate xml name\n",
									t2->name, t2->attr, attrTypeString(t2->type), t2->defaut, p);
						free(nmtokens);
						return 0;
					}
					p = strtok(NULL, " ");
				}				
				free(nmtokens);
				}
				break;
			case ATT_TYPE_ENTITY:
				/* add entity check  
				check wether entity in default value defined in DTD */
				if(match_entity(t2->defaut, dm, hashi) == NULL) {
					_log(L_ERR,"DTD error: element[%s]'s attribute[%s]'s type[%s] default"
								" value[%s] NOT an entity\n",
								t2->name, t2->attr, attrTypeString(t2->type), t2->defaut);
					return 0;
				}
				break;
			case ATT_TYPE_ENTITIES:
				/* add entity check  
				check wether entity in default value defined in DTD */				{
				char *p, *entities = estrdup(t2->defaut);
				if(entities == NULL)
					_err("not enough memory on entities\n");				
				p = strtok(entities, " ");
				while(p != NULL) {
					if(match_entity(p, dm, hashi) == NULL) {
						_log(L_ERR,"DTD error: one of element[%s]'s attribute[%s]'s type[%s] default[%s]"
								" value[%s] NOT an entity\n",
								t2->name, t2->attr, attrTypeString(t2->type), t2->defaut, p);
						free(entities);
						return 0;
					}
					p = strtok(NULL, " ");
				}				
				free(entities);
				}
				break;
			default:
				break;
			}
		}
	}	

	return 1;
}

static int check_att_dupnode(struct dtd_attnode* n)
{
	int delnodes = 0;
	struct dtd_attnode *t1, *t2;
	
	if(n == NULL)
		return 0;

	for(t1 = n; t1 != NULL; t1 = t1->next) {
		for(t2 = t1; t2 != NULL; t2 = t2->sibling) {
			if(match_attr(n, t2->attr, t2)) {
				_log(L_NOTE,"element[%s].attr[%s].define has a duplicate one. ignore this\n" ,
					t2->name, t2->attr, t2->define);
				t2->flag = FLAG_NOUSE;
				delnodes ++;
			}
		}
	}

	return delnodes;
}



#define parse_error_msg(e)\
do {\
	_log(L_ERR,"errorCode=%d\nfilepos=%d\nline=%d\nlinepos=%d\nreason=%s\n",\
	 	e->errorCode, e->filepos,e->line, e->linepos,e->reason);\
} while(0)

static int check_ent_sub(struct dtd_entity* tbl[], int hashi)
{
	struct dtd_entity* de;
	int i;
	
	/* seek every entity and detect those type, 
	  and parse entity's document into struct 
	  */

	/* find entity in internal DTD defination */
	for(i = 0; i < hashi; i++) {
		for(de = tbl[i]; de != NULL; de = de->hash_nt) {
			char* entity = NULL;
			
			/* parse entity to domtree object : the way is 
			first parse raw entity doc with domtree parser, if failed, then
			add entity doc in an temporary xml file's root element then
			reparse it, if failed return failed. if parse ok , replace entity
			node with new sub domtree which is parsed just now */

			/* get entity's content */
			if(de->type == ENT_TYPE_COMMON_INTERNAL)
				entity = de->entity;
			else if(de->type == ENT_TYPE_COMMON_EXTERNAL) {
				FILE* fp = fopen(de->entity, "r");
				char* read_buf[100+1];
				int read;
				
				if(fp == NULL) {
					_log(L_ERR,"can NOT open external entity [%s]'s file [%s]\n",
						de->name, de->entity);
					goto error;
				}

				/* store file fp's content to entity */
				build_string(1, (char**) &(entity), "");
				do {
					memset(read_buf, 0, sizeof(read_buf));
					read = fread(read_buf, 100, 1, fp);
					build_string(0, (char**) &(entity), (char*) read_buf);
				} while(read > 0);

		 		fclose(fp);			 		
			}
			else {
				_log(L_NOTE,"not support entity [%s]'s type [%d] meet\n",
					de->name, de->type);
				goto error;
			}	

			if(entity == NULL) {
				_log(L_ERR,"can NOT get entity[%s]'s value\n",de->name);
				goto error;
			}


			/* if entity is common internal , directly parse it
			with foolfish root element envolpment. or directly
			parse it may return NULL result use expat
			*/
			if(de->type == ENT_TYPE_COMMON_INTERNAL)
				goto xmlpiece;
			else if(de->type == ENT_TYPE_COMMON_EXTERNAL)
				goto xmlwhole;
			else {
				_log(L_ERR,"unknown entity [%s] type [%d] meet\n",de->name, de->type);
				goto error;
			}

#define de_type_string(type)\
	(type== ENT_TYPE_COMMON_INTERNAL)?\
	"common_in":((type==ENT_TYPE_COMMON_EXTERNAL)?\
	"common_ex":"unknown now")

xmlwhole:
	
			/* parse it with domtree parser */
			{
	 		XMLDOMDocument* doc;				
			if((doc = (XMLDOMDocument*) CreateObject("FOOLFISH.XMLDOM")) == NULL) {
				_log(L_INFO,"create domtree for parse entity [%s]failed\n", de->name);
				goto error;
			}

			/* OPEN DTD validate on parse here
			 */
			/* doc->validateOnParse = TRUE; */ //default is TRUE
			if(doc->loadXML(doc, entity) == TRUE) {
				_log(L_INFO,"try parse origianl [%s] entity[%s] ok\n",
							de_type_string(de->type), de->name);
				de->data.type = ENT_DATA_XML_WHOLE;
				/*
				(XMLDOMDocument*) de->data.ptr = doc;*/
				de->data.ptr = doc;
				
				/* note : here we do NOT release doc object, instead
				we store it into struct dtd_entity in DTD struct */
				goto done;
			}
			else {
				_log(L_INFO,"try parse origianl [%s] entity[%s] failed\n",
							de_type_string(de->type), de->name);
				/* free no used domtree object */	
		  		ReleaseObject("FOOLFISH.XMLDOM",/*(void**)*/( XMLDOMDocument** )&doc);
			}
			}
			
xmlpiece:

			/* reparse it with root envolpment */
			{
			char* tmpxml = NULL;
	 		XMLDOMDocument* doc;				
			if((doc = (XMLDOMDocument*) CreateObject("FOOLFISH.XMLDOM")) == NULL) {
				_log(L_ERR,"create domtree for parse entity [%s] failed\n",de->name);
				goto error;
			}

			build_string(1, (char**) &(tmpxml), "<foolfish>");
			build_string(0, (char**) &(tmpxml), entity);
			build_string(0, (char**) &(tmpxml), "</foolfish>");
			
			/* for there is NO dtd here, so no
			need to change validateOnParse var
			 */
			/* doc->validateOnParse = FALSE; */
			if(doc->loadXML(doc, tmpxml) == TRUE) {
				_log(L_INFO,"try parse envolpped [%s] entity[%s] ok\n",
						de_type_string(de->type), de->name);
				de->data.type = ENT_DATA_XML_PIECE;
				/*(XMLDOMDocument*) de->data.ptr = doc;*/
				de->data.ptr = doc;
				
				/* note : here we do NOT release doc object, instead
				we store it into struct dtd_entity in DTD struct */
				
				free(tmpxml);
		  		goto done;
			}
			else {
				/* if entity can NOT parse, mark it UNKNOWN type 
				 */				
				_log(L_NOTE,"try parse envolpped [%s] entity[%s] failed, it's NOT an xml\n",
							de_type_string(de->type), de->name);
		  		ReleaseObject("FOOLFISH.XMLDOM",/*(void**)*/( XMLDOMDocument** )&doc);		  		
				free(tmpxml);
			}			
			}

			/* here we can NOT decide entity type 
			we simply link it to data.ptr */
			de->data.type = ENT_DATA_UNKNOWN;
			de->data.ptr = entity;

done:		;
		}
	}	
	
	return 1;

error:
	_err("validate entities in dtd failed\n");
	return 0;
}

static int check_dtd_sub(struct dtd_mngr* dm, void* ptr, int hashi)
{
	XMLDOMNode*	p = (XMLDOMNode*) ptr;
	XMLDOMNode* n;
	XMLDOMNodeList* list;

	struct dtd_node* dn = NULL;
	int i, found = 0;
	int start;

	if(dm == NULL || ptr == NULL) 
		return 0;

	/* if not an element , goto next */
	if(p->nodeType != NODE_ELEMENT)
		goto next;

	/* find in internal DTD defination */
	for(i = 0; i < hashi; i++) {
		for(dn = dm->in_tbl[i]; dn != NULL; dn = dn->hash_nt) {
			if(strcmp(dn->name, p->nodeName)==0) {
				found = 1; break;
			}
		}
		if (found) break;
	}	
	/* find in external DTD defination */
	if(!found) {
		for(i = 0; i < hashi; i++) {
			for(dn = dm->ex_tbl[i]; dn != NULL; dn = dn->hash_nt) {
				if(strcmp(dn->name, p->nodeName)==0) {
					found = 1; break; 
				}
			}
			if (found) break;
		}		
	}	

	if(!found) {
		_log(L_ERR,"element [%s] not found in DTD\n",p->nodeName);
		goto error;
	}

	/* 匹配子接点和ＤＴＤ的声明	*/	
	start = 0;
	
	switch(dn->action) 
	{		
	case ACTION_ONE:
	case ACTION_MULTI:
	case ACTION_PLUS:
	case ACTION_QUESTION:

		/* ===== key code ==== */
		if(!match_child(dn, p, &start))
			goto error;

		if(p->hasChildNodes == TRUE) {
			if(start < p->childNodes->length) {
				list = p->childNodes;
				for(i = start; i < p->childNodes->length; i++) {
					n = list->get_item(p, i);
					if(n->nodeType == NODE_COMMENT ||
						n->nodeType == NODE_ATTRIBUTE)
						continue;
					_log(L_ERR,"extra element [%s] meet in element [%s]\n",
								n->nodeName, p->nodeName);
					goto error;
				}
			}
		}
		/* ===== key code ==== */
		
		break;
	case ACTION_ANY:
		break;
	case ACTION_EMPTY:
		if(p->hasChildNodes == TRUE) {		
			list = p->childNodes;
			list->reset(list);
			while((n = list->nextNode(list)) != (XMLDOMNode*)NULL) {
				switch(n->nodeType)
				{
				case NODE_ELEMENT:
				case NODE_TEXT:
					_log(L_ERR,"element [%s] should be EMPTY\n",dn->name);
					goto error;
				case NODE_COMMENT:
				default:
					break;
				}
			}
		}
		break;
	default:
		_err("unknown action meet");
	}	

next:
	
	if(p->hasChildNodes==TRUE) {
		list = p->childNodes;
		list->reset(list);
		while((n = list->nextNode(list)) != (XMLDOMNode*)NULL) {			
			int result = 0;
			if(!(result = check_dtd_sub(dm, (void*)n, hashi)))			
				return 0;
		}
	}
	
	return 1;

error:
	_log(L_ERR,"DTD validate failed\n");
	return 0;	
}
static int __check_attlist__(XMLDOMNode* p, struct dtd_attnode* atti, struct dtd_attnode* atte)
{
	XMLDOMNodeList* list;
	XMLDOMNode* n;

	if(p == NULL)
		_err("%s[%d] parameter errir\n",__FILE__,__LINE__);
	
	if(p->hasChildNodes != TRUE)
		return 1;
	
	if((list = p->childNodes) == NULL)
		_err("element[%s] has child , but it's childlist is empty\n",
					p->nodeName);

	for(n = list->headNode; n != NULL ; n = n->nextNode) {
		if(n->nodeType == NODE_ATTRIBUTE) {
			int found = 0;
			
			_log(L_DBG,"node[%s] att[%s]\n",n->parentNode->nodeName, n->nodeName);
			
			/* check every attribute iname in each element,
			if any one does NOT appear in DTD's attlist , then
			report error */
			
			/* here we do NOT need to check wether domtree node
			attribute node is duplicate, for expat already do this */
			
			if(atti == NULL && atte == NULL) {
				_log(L_ERR,"[aTT-eRROR] node[%s] have NOT any att define in dtd\n",
						n->parentNode->nodeName);
				goto error;
			}
			
#define __find_in_attnode__(attn, fnd, dn, cmp)\
do {\
struct dtd_attnode *t1,*t2;\
for(t1 = attn; t1 != NULL; t1 = t1->next) {\
	for(t2 = t1; t2 != NULL; t2 = t2->sibling) {\
		if(t2->flag == FLAG_NOUSE)\
			continue;\
		if(strcmp(t2->attr, cmp)==0) {\
			fnd = 1;\
			goto dn;\
		}\
	}\
}\
} while(0)
			__find_in_attnode__(atti, found, done, n->nodeName);
			__find_in_attnode__(atte, found, done, n->nodeName);
done:

			if(!found) {
				_log(L_ERR,"[aTT-eRROR] element[%s].attr[%s].define NOT found in DTD\n",
					n->parentNode->nodeName, n->nodeName);
				goto error;						
			}
		}
	}

	return 1;
	
error:
	return 0;
}


static int __match_attlist__(struct dtd_attnode* attn, XMLDOMNode* p, struct dtd_mngr* dm, int hashi)
{
	struct dtd_attnode *t1, *t2;
	XMLDOMNodeList* list;

/* check wether value in Enumbered in DTD */
#define ___att_enum_check__(attnode, xmlnode) \
do {\
	if(attnode->type == ATT_TYPE_ENUM)\
	if(!match_e_value(attnode->e_value, xmlnode->text)) {\
		_log(L_ERR,"element[%s]'s attr[%s] type[%s] value [%s] not found in DTD Enumbered\n",\
			attnode->name, attnode->attr, \
			attrTypeString(attnode->type),xmlnode->text);\
		goto error;\
	}\
} while(0)

#define ___att_notation_check__(attnode,xmlnode)\
do {\
	if(attnode->type == ATT_TYPE_NOTATION)\
	if(!match_e_value(attnode->e_value, xmlnode->text)) {\
		_log(L_ERR,"element[%s]'s attr[%s] type[%s] value [%s] not found in DTD Enumbered\n",\
			attnode->name, attnode->attr, \
			attrTypeString(attnode->type),xmlnode->text);\
		goto error;\
	}\
} while(0)


/* check wether an validate xml name of ID value 
 */
#define ___att_id_check__(attnode,xmlnode) \
do {\
	if(attnode->type == ATT_TYPE_ID)\
	if(!isvalidatexmlname(xmlnode->text)) {\
		_log(L_ERR,"elmnent[%s]'s ID attribute[%s]'s "\
					"value[%s] NOT an validate xml name\n",\
					attnode->name, attnode->attr, xmlnode->text);\
		goto error;\
	}\
} while(0)

/* check wether an xml name of NMTOKEN value 
 */
#define ___att_nmtoken_check__(attnode,xmlnode)\
do {\
	if(attnode->type == ATT_TYPE_NMTOKEN)\
	if(!isxmlname(xmlnode->text)) {\
		_log(L_ERR,"element[%s]'s NMTOKEN attribute[%s]'s "\
					"value[%s] NOT an xml name\n",\
					attnode->name, attnode->attr, xmlnode->text);\
		goto error;\
	}\
} while(0)

/* wether NMTOKENS attribute values is an xml name 
 */
#define ___att_nmtokens_check__(attnode,xmlnode)\
do {\
	if(attnode->type == ATT_TYPE_NMTOKENS) {\
		char *p, *nmtokens = estrdup(xmlnode->text);\
		if(nmtokens == NULL)\
			_err("not enough memory on nmtokens\n");\
		p = strtok(nmtokens, " ");\
		while(p != NULL) {\
			if(!isxmlname(p)) {\
				_log(L_ERR,"one of element[%s]'s attribute[%s]'s type[%s] value[%s]"\
							" [%s] NOT an validate xml name\n",\
							attnode->name, attnode->attr, \
							attrTypeString(attnode->type), xmlnode->text, p);\
				free(nmtokens);\
				goto error;\
			}\
			p = strtok(NULL, " ");\
		}\
		free(nmtokens);\
	}\
} while(0)

/* add entity check  
check wether entity in default value defined in DTD */
#define ___att_entity_check__(attnode,xmlnode,dm,hashi)\
do {\
	if(attnode->type == ATT_TYPE_ENTITY)\
	if(match_entity(xmlnode->text, dm, hashi) == NULL) {\
		_log(L_ERR,"element[%s]'s attribute[%s]'s type[%s]"\
					" value[%s] NOT an entity\n",\
					attnode->name, attnode->attr, \
					attrTypeString(attnode->type), xmlnode->text);\
		return 0;\
	}\
} while(0)


/* add entity check  
check wether entity in default value defined in DTD */
#define ___att_entities_check__(attnode,xmlnode,dm,hashi)\
do {\
	if(attnode->type == ATT_TYPE_ENTITIES) {\
		char *p, *entities = estrdup(xmlnode->text);\
		if(entities == NULL)\
			_err("not enough memory on entities\n");\
		p = strtok(entities, " ");\
		while(p != NULL) {\
			if(match_entity(p, dm, hashi) == NULL) {\
				_log(L_ERR,"one of element[%s]'s attribute[%s]'s type[%s] value[%s]"\
						" [%s] NOT an entity\n",\
						attnode->name, attnode->attr, \
						attrTypeString(attnode->type), xmlnode->text, p);\
				free(entities);\
				return 0;\
			}\
			p = strtok(NULL, " ");\
		}\
		free(entities);\
	}\
} while(0)


	if(p == NULL)
		_err("%s[%d] parameter errir\n",__FILE__,__LINE__);

	for(t1 = attn; t1 != NULL; t1 = t1->next) {
		for(t2 = t1; t2 != NULL; t2 = t2->sibling) {
			if(t2->flag == FLAG_NOUSE)
				continue;

			/* ===================== */
			/* wehter NOW we support this type */
			/* ===================== */
			if(t2->type != ATT_TYPE_CDATA && 
				t2->type != ATT_TYPE_ENUM &&
				t2->type != ATT_TYPE_ID &&
				t2->type != ATT_TYPE_IDREF &&
				t2->type != ATT_TYPE_IDREFS &&
				t2->type != ATT_TYPE_NMTOKEN &&
				t2->type != ATT_TYPE_NMTOKENS &&
				t2->type != ATT_TYPE_ENTITY &&
				t2->type != ATT_TYPE_ENTITIES &&
				t2->type != ATT_TYPE_NOTATION) {
unknown:
				_log(L_ERR,"unknown attribute type[%d]\n",t2->type);
				_log(L_ERR,"we now only known attribute type:\n"
							"2002-05-18 CDATA\n"
							"2002-05-18 Enumbered\n"
							"2002-05-18 ID\n"
							"2002-05-19 IDREF\n"
							"2002-05-19 IDREFS\n"
							"2002-05-19 NMTOKEN\n"
							"2002-05-19 NMTOKENS\n"
							"2002-05-20 ENTITY\n"
							"2002-05-20 ENTITIES\n"
							"2002-05-23 NOTATION\n");
				goto error;
			}
			
			switch(t2->mode) {
			case ATT_MODE_IMPLIED:
				switch(t2->type) {
				case ATT_TYPE_CDATA:
				case ATT_TYPE_IDREF:
				case ATT_TYPE_IDREFS:
					/* simply do nothing @ here */
					break;
				case ATT_TYPE_ENUM:
				case ATT_TYPE_ID:
				case ATT_TYPE_NMTOKEN:
				case ATT_TYPE_NMTOKENS:
				case ATT_TYPE_ENTITY:
				case ATT_TYPE_ENTITIES:	
				case ATT_TYPE_NOTATION:
					{
					XMLDOMNode* s;
					if(p->hasChildNodes == TRUE) {
						list = p->childNodes;
						for(s = list->headNode; s != NULL ; s = s->nextNode) {
							if (s->nodeType != NODE_ATTRIBUTE)
								continue;
							if(strcmp(s->nodeName, t2->attr)==0) {
								___att_enum_check__(t2,s);
								___att_id_check__(t2,s);
								___att_nmtoken_check__(t2,s);
								___att_nmtokens_check__(t2,s);
								___att_entity_check__(t2,s,dm,hashi);
								___att_entities_check__(t2,s,dm,hashi);
								___att_notation_check__(t2,s);
								break;
							}
						}
					}
					}
					break;
				default:
					goto unknown;
				}
				break;
			case ATT_MODE_REQUIRED:
				switch(t2->type) {
				case ATT_TYPE_CDATA:
				case ATT_TYPE_ENUM:
				case ATT_TYPE_ID:
				case ATT_TYPE_IDREF:
				case ATT_TYPE_IDREFS:
				case ATT_TYPE_NMTOKEN:
				case ATT_TYPE_NMTOKENS:
				case ATT_TYPE_ENTITY:
				case ATT_TYPE_ENTITIES:					
				case ATT_TYPE_NOTATION:
					{
					XMLDOMNode* s;
					int found = 0;
					if(p->hasChildNodes == TRUE) {
						list = p->childNodes;
						for(s = list->headNode; s != NULL ; s = s->nextNode) {
							if (s->nodeType != NODE_ATTRIBUTE)
								continue;
							if(strcmp(s->nodeName, t2->attr)==0) {
								___att_enum_check__(t2,s);
								___att_id_check__(t2,s);
								___att_nmtoken_check__(t2,s);
								___att_nmtokens_check__(t2,s);
								___att_entity_check__(t2,s,dm,hashi);
								___att_entities_check__(t2,s,dm,hashi);
								___att_notation_check__(t2,s);
								found = 1;
								break;
							}
						}
					}
					if(!found) {
						_log(L_ERR,"[aTT-eRROR] element[%s].attr[%s] type[%s] define[#REQUIRED]\n"
								"\tnot found in domtree element[%s]\n",
								t2->name, t2->attr, attrTypeString(t2->type), p->nodeName);
						goto error;
					}
					}
					break;
				default:
					goto unknown;
				}
				break;
			case ATT_MODE_DEFAULT:
				/* 
				expat will automatic add the default value in domtree
				so mainly we don't need to do this at here, 
				except other type check like ATT_TYPE_ENUM */
				switch(t2->type) {
				case ATT_TYPE_CDATA:
				case ATT_TYPE_ENUM: 
				case ATT_TYPE_IDREF:
				case ATT_TYPE_IDREFS:
				case ATT_TYPE_NMTOKEN:
				case ATT_TYPE_NMTOKENS:
				case ATT_TYPE_ENTITY:
				case ATT_TYPE_ENTITIES:					
				case ATT_TYPE_NOTATION:
					{
					XMLDOMNode* s;
					int found = 0;
					if(p->hasChildNodes == TRUE) {
						list = p->childNodes;
						for(s = list->headNode; s != NULL ; s = s->nextNode) {
							if (s->nodeType != NODE_ATTRIBUTE)
								continue;
							if(strcmp(s->nodeName, t2->attr)==0) {
								___att_enum_check__(t2,s);
								___att_id_check__(t2,s);
								___att_nmtoken_check__(t2,s);
								___att_nmtokens_check__(t2,s);
								___att_entity_check__(t2,s,dm,hashi);
								___att_entities_check__(t2,s,dm,hashi);
								___att_notation_check__(t2,s);
								found = 1;
								break;
							}
						}
					}
					if(!found) {
						XMLDOMNode* nn;
						_log(L_NOTE,"[aTT-nOTE] element[%s].attr[%s].define type[%s] default[%s]\n"
							"\tNOT found in domtree element[%s], simply add it\n",
							t2->name, t2->attr, attrTypeString(t2->type), t2->defaut, p->nodeName);
						nn = p->ownerDocument->createNode(NODE_ATTRIBUTE,t2->attr,NULL);
						nn->text = strdup(t2->defaut);
						nn = p->appendChild(p,nn);			
					}
					}	
					break;
				case ATT_TYPE_ID:
					_log(L_ERR,"element[%s]'s ID attribute[%s] can NOT have default value\n",
								t2->name,t2->attr);
					goto error;
				default:
					goto unknown;
				}
				break;
			case ATT_MODE_FIXED:
				switch(t2->type) {
				case ATT_TYPE_CDATA:
				case ATT_TYPE_ENUM:
				case ATT_TYPE_IDREF:
				case ATT_TYPE_IDREFS:					
				case ATT_TYPE_NMTOKEN:
				case ATT_TYPE_NMTOKENS:
				case ATT_TYPE_ENTITY:
				case ATT_TYPE_ENTITIES:
				case ATT_TYPE_NOTATION:
					{
					XMLDOMNode* s;
					int found = 0;
					if(p->hasChildNodes == TRUE) {
						list = p->childNodes;
						for(s = list->headNode; s != NULL ; s = s->nextNode) {
							if (s->nodeType != NODE_ATTRIBUTE)
								continue;
							if(strcmp(s->nodeName, t2->attr)==0) {
								if(strcmp(s->text, t2->defaut)!=0) {
									_log(L_ERR,"[aTT-eRROR] element[%s].attr[%s] [domtree] define[%s]\n"
										"\tNOT match with [DTD] type[%s] #FIXED define[%s]]\n",
										p->nodeName,s->nodeName,s->text, attrTypeString(t2->type), t2->defaut);
									goto error;
								}
								found = 1;
								break;
							}
						}
					}
					if(!found) {
						XMLDOMNode* nn;
						_log(L_NOTE,"[aTT-nOTE] element[%s].attr[%s] type[%s] define[%s]]\n"
							"\tNOT found in domtree element[%s], simply add it\n",
							t2->name, t2->attr, attrTypeString(t2->type), t2->defaut, p->nodeName);
						nn = p->ownerDocument->createNode(NODE_ATTRIBUTE,t2->attr,NULL);
						nn->text = strdup(t2->defaut);
						nn = p->appendChild(p,nn);			
					}
					}				
					break;
				case ATT_TYPE_ID:
					_log(L_ERR,"element[%s]'s ID attribute[%s] can NOT have #FIXED mode\n",
								t2->name,t2->attr);
					goto error;
				default:
					goto unknown;
				}
				break;
			default:
				_err("illegal attnode[%s].attr[%s].mode[%d] meet\n",
							t2->name, t2->attr,t2->mode);
			}
		}
	}	
	
	return 1;
	
error:
	
	return 0;
}


static int __match_idlist__(struct dtd_mngr* dm, const char* value)
{
	struct e_value *t;
	if(dm == NULL || value == NULL)
		return 0;

	for(t = dm->id_list; t != NULL; t = t->next)
		if(strcmp(t->value,value)==0)
			return 1;

	return 0;
}

static int __add_idlist__(struct dtd_mngr* dm, const char* value)
{
	struct e_value *t, *c;
	if(dm == NULL || value == NULL)
		return 0;
	
	t = (struct e_value*) malloc(sizeof(*t));
	if(t == NULL)
		return 0;
	t->value = estrdup(value);		
	t->next = NULL;
	if(t->value == NULL) {
		free(t); return 0;
	}
	
	if(dm->id_list == NULL)
		dm->id_list = t;
	else {
		for(c = dm->id_list; c->next != NULL; c = c->next)
			;
		c->next = t;
	}

	return 1;
}



static int __check_idlist__(struct dtd_mngr* dm, struct dtd_attnode* attn, XMLDOMNode* n)
{
	struct dtd_attnode *t1, *t2;
	
	/* find attribute name in atte and atti , to
	decide wether it a ID attribute, if NOT cont */
	for(t1 = attn; t1 != NULL; t1 = t1->next) {
		for(t2 = t1; t2 != NULL; t2 = t2->sibling) {
			if(t2->flag == FLAG_NOUSE) 
				continue;
			if(strcmp(t2->attr,n->nodeName) !=0)
				continue;
			if(t2->type == ATT_TYPE_ID) {
				if(__match_idlist__(dm, n->text)) {
					_log(L_ERR,"duplicate ID value [%s] found in element [%s]'s ID attribute [%s]\n",
						n->text, 
						(n->parentNode==NULL)?"(null)":n->parentNode->nodeName, 
						n->nodeName);
					return 0;				
				}
				if(!__add_idlist__(dm, n->text))
					_err("memory NOT enough\n");
			}
		}
	}
	
	return 1;
}

static int __check_idreflist__(struct dtd_mngr* dm, struct dtd_attnode* attn, XMLDOMNode* n)
{
	struct dtd_attnode *t1, *t2;
	
	/* find attribute name in atte and atti , to
	decide wether it a IDREF or IDREFS attribute, if NOT cont */
	for(t1 = attn; t1 != NULL; t1 = t1->next) {
		for(t2 = t1; t2 != NULL; t2 = t2->sibling) {
			if(t2->flag == FLAG_NOUSE) 
				continue;
			if(strcmp(t2->attr,n->nodeName) !=0)
				continue;
			if(t2->type == ATT_TYPE_IDREF) {
				/* check wether IDREF value in ID list */
				if(!__match_idlist__(dm, n->text)) {
					_log(L_ERR,"IDREF value [%s] NOT an legal ID value in element [%s]'s IDREF attribute [%s]\n",
						n->text, 
						(n->parentNode==NULL)?"(null)":n->parentNode->nodeName, 
						n->nodeName);
					return 0;				
				}
			}
			else if(t2->type == ATT_TYPE_IDREFS) {
				char *p, *idrefs = estrdup(n->text);
				if(idrefs == NULL)
					_err("not enough memory on ifrefs\n");				
				/* check wether IDREFS value in ID list */
				p = strtok(idrefs, " ");
				while(p != NULL) {
					if(!__match_idlist__(dm, p)) {
						_log(L_ERR,"one of IDREFS[%s] value [%s] NOT an legal ID value in element [%s]'s IDREFS attribute [%s]\n",
							n->text, p, 
							(n->parentNode==NULL)?"(null)":n->parentNode->nodeName, 
							n->nodeName);
						free(idrefs);
						return 0;				
					}
					p = strtok(NULL, " ");
				}				
				free(idrefs);
			}
		}
	}
	
	return 1;
}

static int check_attlist_sub(struct dtd_mngr* dm, void* ptr, int hashi)
{
	XMLDOMNode*	p = (XMLDOMNode*) ptr;
	XMLDOMNode *n;
	struct dtd_attnode *atte, *atti;

	if(dm == NULL || ptr == NULL) 
		return 0;

	/* get external and internal attlist in DTD */
	match_attlist(p->nodeName, dm, hashi, &atte, &atti);

	/* check every attribute in domtree, wether it
	defined in dtd, if not or have duplicate define,
	return error */
	if(!__check_attlist__(p, atti, atte))
		goto error;

	/* recurse every attribute defined in dtd, check with domtree node p
	[1] #IMPLIED		ignore check this in domtree
	[2] #REQUIRED		if not exist in domtree report error, else pass
	[3] #FIXED			if not exist, append this with fixed value, else
						if exist, but value not the defined, report error,
						else (exist and value defined) pass
	[4] default			if not exist, append this with default value, 
						else (exist in domtree) pass
	*/
	if(!__match_attlist__(atti, p, dm, hashi) || !__match_attlist__(atte, p, dm, hashi))
		goto error;

	/* find all ID attribute in node p, decide 
	wether it duplicate in domtree document .
	if so, report error ID attribute use
	 */
	if(p->hasChildNodes == TRUE) {
		XMLDOMNodeList* list = p->childNodes;
		for(n = list->headNode; n != NULL ; n = n->nextNode) {
			if(n->nodeType != NODE_ATTRIBUTE) 
				continue;
			if(!__check_idlist__(dm, atte, n) || !__check_idlist__(dm, atti, n))
				goto error;
		}
	}



	/* check p's child element nodes */
	if(p->hasChildNodes == TRUE) {
		XMLDOMNodeList* list = p->childNodes;
		for(n = list->headNode; n != NULL ; n = n->nextNode) {
			if(n->nodeType == NODE_ELEMENT) {
				if(!check_attlist_sub( dm, (void*)n, hashi))
					goto error;
			}			
		}
	}

	return 1;
	
error:
	return 0;
}


static int check_attlist_all(struct dtd_mngr* dm, void* ptr, int hashi)
{
	XMLDOMNode*	p = (XMLDOMNode*) ptr;
	XMLDOMNode *n;
	struct dtd_attnode *atte, *atti;

	if(dm == NULL || ptr == NULL) 
		return 0;

	/* get external and internal attlist in DTD */
	match_attlist(p->nodeName, dm, hashi, &atte, &atti);

	/* find all IDREF attribute in node p, decide
	wether it's reference is really an legal ID value 
	 */
	if(p->hasChildNodes == TRUE) {
		XMLDOMNodeList* list = p->childNodes;
		for(n = list->headNode; n != NULL ; n = n->nextNode) {
			if(n->nodeType != NODE_ATTRIBUTE) 
				continue;
			if(!__check_idreflist__(dm, atte, n) || 
			   !__check_idreflist__(dm, atti, n))
				goto error;
		}
	}

	/* check p's child element nodes */
	if(p->hasChildNodes == TRUE) {
		XMLDOMNodeList* list = p->childNodes;
		for(n = list->headNode; n != NULL ; n = n->nextNode) {
			if(n->nodeType == NODE_ELEMENT) {
				if(!check_attlist_all( dm, (void*)n, hashi))
					goto error;
			}			
		}
	}

	return 1;
	
error:
	return 0;
}

static void free_hash_tbl(int type, void* v[], int hashi)
{
	if(v == NULL || hashi <= 0)
		return;

	switch(type) {
	case ELEMENT_ITEM:
		{
		struct dtd_node** tbl = (struct dtd_node**) v;
		struct dtd_node *node, *p/* , *t, *n*/;
		int i;

		for(i = 0; i < hashi; i++) {
			if(tbl[i] != NULL) {
				for(node = tbl[i]; node != NULL; ) {
					p = node->hash_nt;

					/* 
					note here we do NOT need to recurse next
					for it's data struct NOT simulator with
					other dtd_xxx struct  
					*/
					release_node(ELEMENT_ITEM, (void**) &node);

					node = p;
				}
			}
		}
		}
		break;
	case ATTLIST_ITEM:
		/* release struct dtd_attnode here */
		{
		struct dtd_attnode** tbl = (struct dtd_attnode**) v;
		struct dtd_attnode *node, *p, *t, *n;
		int i;

		for(i = 0; i < hashi; i++) {
			if(tbl[i] != NULL) {
				for(node = tbl[i]; node != NULL; ) {
					p = node->hash_nt;
					for(n = node; n != NULL; ) {
						t = n->next;
						release_node(ATTLIST_ITEM, (void**) &n);
						n = t;
					}					
					node = p;
				}
			}
		}
		}
		break;
	case ENTITY_ITEM:
		/* release struct dtd_attnode here */
		{
		struct dtd_entity** tbl = (struct dtd_entity**) v;
		struct dtd_entity *node, *p;
		int i;

		for(i = 0; i < hashi; i++) {
			if(tbl[i] != NULL) {
				for(node = tbl[i]; node != NULL; ) {
					p = node->hash_nt;
					release_node(ENTITY_ITEM, (void**) &node);
					node = p;
				}
			}
		}
		}
		break;
	case NOTATION_ITEM:
		/* add notation item release  */
		/* release struct dtd_notation here */
		{
		struct dtd_notation** tbl = (struct dtd_notation**) v;
		struct dtd_notation *node, *p;
		int i;

		for(i = 0; i < hashi; i++) {
			if(tbl[i] != NULL) {
				for(node = tbl[i]; node != NULL; ) {
					p = node->hash_nt;
					release_node(NOTATION_ITEM, (void**) &node);
					node = p;
				}
			}
		}
		}
		break;
	default:
		break;
	}

	return;
}

static int add_entity_sub(struct dtd_mngr* dm, void* ptr, int hashi)
{
	XMLDOMNode*	p = (XMLDOMNode*) ptr;
	XMLDOMNode *n;
	XMLDOMNodeList* list;

	if(dm == NULL || ptr == NULL) 
		return 0;

	/* search this node's children, if entity references found, 
	found entity in DTD, then replace with it */

	if(p->hasChildNodes==TRUE) {
		list = p->childNodes;		
		for(n = list->headNode; n != NULL ; ) {
			switch(n->nodeType) {
			case NODE_ENTITY_REFERENCE:
				{
				struct dtd_entity* de;
				
				if((de = match_raw_entity(n->text, dm, hashi)) == NULL)
					goto error;
	
				/* =========KEY CODE ==========*/
				switch(de->data.type) {
				case ENT_DATA_XML_WHOLE:					
				case ENT_DATA_XML_PIECE:
					{
					XMLDOMNode* en, *dup, *c;					

					/* get entity content's pointer from de->data.ptr,
					according different data type "de->data.type "*/
					en = get_entity(de->data.type, de->data.ptr);
					if(en == NULL) {
						_log(L_ERR,"can NOT get entity domtree object\n");
						goto error;
					}

					/* duplicate every child node, and first add those
					before entity node, then remove entity node */
					_log(L_INFO,"replace entity [%s] <- [%s]\n",n->text, de->entity);
					{
					XMLDOMNode* next;
					for(c = en; c != NULL; c = c->nextNode) {
						/* duplicate en to dup */
						dup = c->duplicateChild(c, NULL);
						if(dup == NULL) {
							_log(L_ERR,"error on duplicate entity domtree\n");
							goto error;
						}
						if(n->insertBefore(n, dup) != TRUE) {
							_log(L_ERR,"error on replace entity\n");
							goto error;
						}
					}

					/* save cur node's next node and delete cur node
					then restore the cur node's next node */
					next = n->nextNode; 
					n->removeChild(n); 
					n = next;
					}
					}
					continue;
					
				default:
					_log(L_ERR,"unknown entity[%s] data type[%d] meet\n",
						de->name, de->data.type);
					goto error;
				}
				/* =========KEY CODE ==========*/
				}
				break;
			case NODE_ELEMENT:
				add_entity_sub(dm, (void*) n, hashi);
				break;
			case NODE_ATTRIBUTE:
				/* YOU NEED TO DO SOMETHING 
				here is the answer: since expat already
				replace common internal entity reference 
				on attribute value, so I do NOT need
				to process attribute's entity reference 
				btw: expat can NOT refernce exteranl 
				common entity in attribute value */
				break;
			default:
				break;
			}

			/* get next node @ here */
			n = n->nextNode;
		}
	}

	return 1;

error:
	_log(L_ERR,"dom add entity failed\n");
	return 0;
}

static int match_node(struct dtd_node* dn, XMLDOMNode* p, int* pstart)
{
	XMLDOMNode* n;
	XMLDOMNodeList* list;
	struct dtd_node* cur;
	int found = 0, i;

	/*
	 * process an #PCDATA element
	 */
	if(dn->child == NULL && strcmp(dn->name,"#PCDATA")==0) {
		/* HERE dn->next must be NULL , else it's error format */
		if(p->hasChildNodes == TRUE) {		
			XMLDOMNode* t;
			int i;
			list = p->childNodes;
			for(i = *pstart; i < list->length; i++) {
				t = list->get_item(p, i);
				if(t->nodeType == NODE_COMMENT ||
					t->nodeType == NODE_ATTRIBUTE)
					continue;
				if(t->nodeType != NODE_TEXT) {
					_log(L_ERR,"only text allowed inside element [%s]\n",
								p->nodeName);
					return 0;
				}
				*pstart = (i + 1);
			}
		}
		return 1;
	}

	/* 
	 * process an child-have element 
	 */
	if(dn->child != NULL) {
		switch(dn->mode) {
		case MODE_ARRAY:
			if(!match_child(dn,p, pstart))
				return 0;
			if(dn->next != NULL)		
				if(!match_node(dn->next, p, pstart))
					return 0;
			break;
		case MODE_OR:
			{
			int pre = *pstart;
			if(!match_child(dn,p, pstart)) {
				*pstart = pre;
				if(dn->next == NULL)
					return 0;
				if(!match_node(dn->next, p, pstart))
					return 0;
			}
			}
			break;
		default:
			_err("invalid mode meet");
		}
		return 1;
	}
	
	/* 
	 * process an normal element 
	 */
	if(dn->child == NULL) {
		switch(dn->mode) {
		case MODE_ARRAY:
			for(cur = dn; cur != NULL; cur = cur->next)  
			{			
				/* group meet */
				if(cur->child != NULL) {
					if(!match_child(cur, p, pstart))
						return 0;
					continue;
				}
				
				switch(cur->action) {
				case ACTION_ONE:	//one
					found = 0;
					if(p->hasChildNodes == TRUE) {		
						list = p->childNodes;	
						for(i = *pstart; i < list->length; i++) {
							n = list->get_item(p, i);
							/* change type "== NODE_COMMENT" to
							"!= NODE_ELEMENT" */
							//if(n->nodeType == NODE_COMMENT)
							if(n->nodeType != NODE_ELEMENT)
								continue;
							if(strcmp(n->nodeName, cur->name)==0)
								found = 1;
							else {
								_log(L_ERR,
	"element [%s] should at the position of element [%s] in element [%s]\n",
									cur->name, n->nodeName,p->nodeName);
								return 0;
							}
							*pstart = i + 1;
							break;
						}
						if( i == list->length )
							*pstart = i;
					}
					break;				
				case ACTION_PLUS:	// 1 or mult						
					found = 0;
					if(p->hasChildNodes == TRUE) {		
						list = p->childNodes;	
						for(i = *pstart; i < list->length; i++) {
							n = list->get_item(p, i);
							/* change type "== NODE_COMMENT" to
							"!= NODE_ELEMENT" */
							//if(n->nodeType == NODE_COMMENT)
							if(n->nodeType != NODE_ELEMENT)
								continue;
							if(strcmp(n->nodeName, cur->name)==0) {
								found = 1;
								continue;
							}
							else {
								if(!found) {
									_log(L_ERR,
	"element [%s] should at the position of element [%s] in element [%s]\n",
										cur->name, n->nodeName,p->nodeName);
									return 0;
								}
								*pstart = i;
								break;
							}
						}
						if( i == list->length )
							*pstart = i;
					}
					break;				
				case ACTION_MULTI:	//0 or multi			
					found = 1;
					if(p->hasChildNodes == TRUE) {		
						list = p->childNodes;	
						for(i = *pstart; i < list->length; i++) {
							n = list->get_item(p, i);
							/* change type "== NODE_COMMENT" to
							"!= NODE_ELEMENT" */
							//if(n->nodeType == NODE_COMMENT)
							if(n->nodeType != NODE_ELEMENT)
								continue;
							if(strcmp(n->nodeName, cur->name)==0)
								continue;
							else {
								_log(L_NOTE,
		"element [%s] not found before element [%s] in element [%s]\n",
									cur->name,n->nodeName,p->nodeName);
								*pstart = i;
								break;
							}
						}
						if( i == list->length )
							*pstart = i;
					}
					break;							
				case ACTION_QUESTION: //0 or 1
					found = 1;
					if(p->hasChildNodes == TRUE) {		
						list = p->childNodes;	
						for(i = *pstart; i < list->length; i++) {
							n = list->get_item(p, i);
							/* change type "== NODE_COMMENT" to
							"!= NODE_ELEMENT" */
							//if(n->nodeType == NODE_COMMENT)
							if(n->nodeType != NODE_ELEMENT)
								continue;
							if(strcmp(n->nodeName, cur->name)==0)
								*pstart = i+1;
							else {
								_log(L_NOTE,"element [%s] not found before element [%s] in element [%s]\n",
									cur->name,n->nodeName,p->nodeName);							
								*pstart = i;
							}
							break;
						}
						if( i == list->length )
							*pstart = i;
					}
					break;
				default:
					_log( L_NOTE, "Invalid branch" );
					break;
				} // switch(cur->action)

				if(!found) {
					//_log(L_ERR,
					_log(L_NOTE, 
	"element [%s] NOT found in parent element [%s]\n",
							cur->name, p->nodeName);
					return 0;
				}
			}
			break;
			
		case MODE_OR:
			{
			int can_empty = 0;
			for(cur = dn; cur != NULL; cur = cur->next)  
			{
				/* group meet */
				if(cur->child != NULL) {

					/* NOTE: if NOT match, but it can be empty
					         we should NOT return 0, then set
							 can_empty = 0, and continue;
					 */

					found = 0;
					if(match_child(cur, p, pstart)) {
						found = 1;
						break;
					}
					continue;
				}
			
				switch(cur->action) {
				case ACTION_ONE:
					found = 0;
					if(p->hasChildNodes == TRUE) {
						list = p->childNodes;
						/* match elements with DTD */
						for(i = *pstart; i < list->length; i++) {
							n = list->get_item(p, i);
							/* change to != NODE_ELEMENT 
							 */
							//if(n->nodeType == NODE_COMMENT)
							if(n->nodeType != NODE_ELEMENT)
								continue;
							if(strcmp(n->nodeName, cur->name)==0) {
								found = 1;
								*pstart = i + 1;
							}
							else {
								_log(L_NOTE,
				"element [%s] not found in element [%s]\n",
									cur->name,p->nodeName);
								*pstart = i;
							}
							break;
						}
						if( i == list->length )
							*pstart = i;
					}
					break;				
				case ACTION_QUESTION:
					can_empty = 1;
					found = 0;
					if(p->hasChildNodes == TRUE) {
						list = p->childNodes;
						/* match elements with DTD */
						for(i = *pstart; i < list->length; i++) {
							n = list->get_item(p, i);
							/* change to != NODE_ELEMENT 
							 */
							//if(n->nodeType == NODE_COMMENT)
							if(n->nodeType != NODE_ELEMENT)
								continue;
							if(strcmp(n->nodeName, cur->name)==0) {
								found = 1;
								*pstart = i + 1;
							}
							else {
								_log(L_NOTE,
			"element [%s] not found in element [%s]\n",
									cur->name,p->nodeName);
								*pstart = i;
							}
							break;
						}
						if( i == list->length )
							*pstart = i;
					}
					break;
				case ACTION_MULTI:	//folloing down...
					can_empty = 1;
					found = 0;
					if(p->hasChildNodes == TRUE) {
						list = p->childNodes;
						/* match elements with DTD */
						for(i = *pstart; i < list->length; i++) {
							n = list->get_item(p, i);
							/* change to != NODE_ELEMENT 
							 */
							//if(n->nodeType == NODE_COMMENT)
							if(n->nodeType != NODE_ELEMENT)
								continue;
							if(strcmp(n->nodeName, cur->name)==0) {
								found = 1;
								continue;
							}
							else {
								_log(L_NOTE,
			"element [%s] not found in element [%s]\n",
									cur->name,p->nodeName);
								*pstart = i;
								break;
							}
						}
						if( i == list->length )
							*pstart = i;
					}
					break;
				case ACTION_PLUS:
					found = 0;
					if(p->hasChildNodes == TRUE) {
						list = p->childNodes;
						/* match elements with DTD */
						for(i = *pstart; i < list->length; i++) {
							n = list->get_item(p, i);
							/* change to != NODE_ELEMENT 
							 */
							if(n->nodeType != NODE_ELEMENT)
							//if(n->nodeType == NODE_COMMENT)
								continue;
							if(strcmp(n->nodeName, cur->name)==0) {
								found = 1;
								continue;
							}
							else {
								_log(L_NOTE,
			"element [%s] not found in element [%s]\n",
									cur->name,p->nodeName);
								*pstart = i;
								break;
							}
						}
						if( i == list->length )
							*pstart = i;
					}
					break;
				default:
					_err("invalid action meet");
				}
				if(found)
					break;
			}
			if(cur == NULL && !can_empty && !found) {
				_log(L_NOTE,"element(s) NOT found in element [%s]\n",
					p->nodeName);
				return 0;
			}
			}
			break;
		default:
			_err("invalid mode meet");
		}
		/* ========= KEY CODE ========= */
		return 1;
	}
		
	return 1;
}

static int match_child(struct dtd_node* dn, XMLDOMNode* p, int* pstart)
{
	int pre;

	switch(dn->action) {
	case ACTION_ONE:				
		if(!match_node(dn->child, p, pstart))
			return 0;
		break;
	case ACTION_QUESTION:
		pre = *pstart;
		if(!match_node(dn->child, p, pstart)) {
			*pstart = pre;
			return 1;
		}
		break;
	case ACTION_PLUS:
		if(!match_node(dn->child, p, pstart))
			return 0;
	case ACTION_MULTI: //following down
		pre = -1; 
		while(!meet_end(*pstart, p) && (*pstart > pre)) {
			pre = *pstart;
			if(!match_node(dn->child, p, pstart)) {
				*pstart = pre;
				return 1;
			}
		}
		break;
	default:
		_err("invalid action meet\n");
	}		
	
	return 1;
}


static struct dtd_entity* match_raw_entity(char* rawent, struct dtd_mngr* dm, int hashi)
{
	char *t, *p = rawent;
	int rawl = strlen(p);
	struct dtd_entity* de;

	/* get entity name */
	if(p == NULL || *p != '&' || p[rawl-1] != ';') {
		_log(L_ERR,"entity node not a entity [%s]\n",(p==NULL)?"null":p);
		return NULL;
	}

	/* trim '&' and ';' , then p is the entity name */
	t = (char*) malloc(rawl-1-1+1);
	if(t == NULL) {
		_log(L_ERR,"memory not enough on match raw entity\n");
		return NULL;
	}
	memset(t, '\0', rawl-1);
	memcpy(t, p+1, rawl-1-1);

	de = match_entity(t, dm, hashi);

	free(t); 

	return de;
}


static struct dtd_entity* match_entity(const char* ent, struct dtd_mngr* dm, int hashi)
{
	int i, found = 0;
	struct dtd_entity* de = NULL;

	/* find entity in internal DTD defination */
	for(i = 0; i < hashi; i++) {
		for(de = dm->ent_in_tbl[i]; de != NULL; de = de->hash_nt) {
			if(de->flag == FLAG_NOUSE) continue;
			if(strcmp(de->name, ent)==0) {
				found = 1; break;
			}
		}
		if (found) break;
	}
	
	/* find entity in external DTD defination */
	if(!found) {
		for(i = 0; i < hashi; i++) {
			for(de = dm->ent_ex_tbl[i]; de != NULL; de = de->hash_nt) {
				if(de->flag == FLAG_NOUSE) continue;
				if(strcmp(de->name, ent)==0) {
					found = 1; break; 
				}
			}
			if (found) break;
		}		
	}	
	
	if(!found) {
		_log(L_ERR,"entity [%s] 's defination not found in dtd\n",ent);
		return NULL;
	}	
	if(de->entity == NULL || de->data.ptr == NULL) {
		_log(L_ERR,"entity [%s] 's defination (or data.ptr) is empty\n",de->name);
		return NULL;
	}

	return de;	
}

static int match_attlist(const char* name, struct dtd_mngr* dm, int hashi,
					struct dtd_attnode** ppe, struct dtd_attnode** ppi)
{
	struct dtd_attnode *de;
	int i;

	*ppe = *ppi = NULL;

	/* find in external DTD attlist */
	for(i = 0; i < hashi; i++) {
		for(de = dm->att_ex_tbl[i]; de != NULL; de = de->hash_nt) {
			if(strcmp(de->name, name)==0) {
				*ppe = de;
				goto cont;
			}
		}
	}		

cont:

	/* find in internal DTD attlist */
	for(i = 0; i < hashi; i++) {
		for(de = dm->att_in_tbl[i]; de != NULL; de = de->hash_nt) {
			if(strcmp(de->name, name)==0) {
				*ppi = de;
				goto done;
			}
		}
	}
	
done:

	if(*ppe == NULL && *ppi == NULL)
		return 0;

	return 1;	
}


static int match_attr(struct dtd_attnode* n, char* attr, struct dtd_attnode* t)
{
	struct dtd_attnode *t1, *t2;
	
	if(n == NULL || attr == NULL)
		return 0;

	for(t1 = n; t1 != NULL; t1 = t1->next) {
		for(t2 = t1; t2 != NULL; t2 = t2->sibling) {
			if(t2->flag == FLAG_NOUSE)
				continue;
			/* search before node t */
			if(t2 == t)
				return 0;
			else if(strcmp(t2->attr, attr) == 0)
				return 1;
		}
	}
	
	return 0;
}

/* wether value v in e_value */
static int match_e_value(struct e_value* pev, char* v)
{
	struct e_value* pv;
	for(pv = pev; pv != NULL; pv = pv->next) {
		if(strcmp(pv->value, v) == 0) {
			return 1;
		}
	}
	return 0;
}

static int match_notation(struct dtd_mngr* dm, 
					struct dtd_attnode* node, struct e_value* ev)
{
	struct e_value* et;	
	int found = 0, HQ;
	
	if(dm == NULL || node == NULL || ev == NULL)
		return 0;

#define __found_in_tbl(dt, value, found)\
do {\
	struct dtd_notation* dn;\
	found = 0;\
	for(dn = dt; dn != NULL; dn = dn->hash_nt) {\
		if(strcmp(dn->name, value)==0) {\
			found = 1; break;\
		}\
	}\
} while(0)

	/* decide wether node->e_value value is an notation */
	for(et = ev; et != NULL; et = et->next) {
		HQ = hash_t(et->value, dm->hashi);
		/* first find in internal then external */
		__found_in_tbl(dm->not_in_tbl[HQ], et->value, found);
		if(!found) {
			_log(L_NOTE,"element[%s]'s attribute[%s] type[%s]"
					" one of notation[%s] NOT found in interanl DTD,"
					" tray found in external DTD\n",
					node->name,node->attr,
					attrTypeString(node->type),et->value);
	
			__found_in_tbl(dm->not_ex_tbl[HQ], et->value, found);
			if(!found) {
				_log(L_ERR,"element[%s]'s attribute[%s] type[%s]"
						" one of notation[%s]\n also NOT found in external DTD error\n",
						node->name,node->attr,
						attrTypeString(node->type),et->value);
				return 0;
			}
		}
	}

	return 1;
}


static int meet_end(int start, XMLDOMNode* p)
{
	XMLDOMNode* n;
	XMLDOMNodeList* list;
	int i;
	
	if(p->hasChildNodes == TRUE) {
		list = p->childNodes;
		for(i = start; i < list->length; i++) {
			n = list->get_item(p, i);
			if(n->nodeType == NODE_COMMENT)
				continue;
			return 0;
		}
	}
	return 1;
}


#define make_tmp_dtd_head(stt ,root) \
do {\
	build_string(1, (char**) &(stt), \
		(char*) "<?xml version=\"1.0\" standalone=\"yes\"?>\n");\
	build_string(0, (char**) &(stt), (char*) "<!DOCTYPE ");\
	build_string(0, (char**) &(stt), (char*) root);\
	build_string(0, (char**) &(stt), (char*) " [\n");\
} while(0)

#define make_tmp_dtd_tail(stt) \
do {\
	build_string(0, (char**) &(stt), (char*) "\n]>");\
} while(0)

static BOOL get_external_dtd(struct dtd_mngr* dm, char** dtd)
{
	int ret;

	if(dm == NULL || *dtd != NULL)
		return FALSE;

	switch(dm->ex_type) {
	case TYPE_PUBLIC:
		if(dm->ex_name == NULL || dm->ex_url == NULL) {
			_log(L_ERR,"DTD public ex_name or ex_url empty");
			return FALSE;
		}
		break;
	case TYPE_SYSTEM:
		if(dm->ex_url == NULL) {
			_log(L_ERR,"DTD system ex_url empty");
			return FALSE;
		}
		break;
	default:
		_err("invalid type meet");
	}

	/* get DTD by url */
	if(strncmp(dm->ex_url,"http://",strlen("http://"))==0) {
		/* get by http protocol */
		_log(L_NOTE,"sorry ! we now can NOT get url resources");
		return FALSE;
	}

	ret = FALSE;

	{
	FILE* fp;
	#define SPOOL_LEN (100)
	char* read_buf[SPOOL_LEN+1];
	int read;
	unsigned char* xml = NULL;
 	XMLDOMDocument* doc;
		
	/* get by local protocol */
	fp = fopen(dm->ex_url, "r");
	if(fp == NULL) {
		_log(L_ERR,"can NOT open local file [%s]\n",dm->ex_url);
		return FALSE;
	}	

	/* -------------------------------------- */
	/* read file's content and make it to xml */
	/* -------------------------------------- */
	make_tmp_dtd_head(xml, dm->root);
	do {
		memset(read_buf, 0, sizeof(read_buf));
		read = fread(read_buf, SPOOL_LEN, 1, fp);
		build_string(0, (char**) &(xml), (char*) read_buf);
	} while(read > 0);
	make_tmp_dtd_tail(xml);

 	fclose(fp);
 		
	/* ----------------------------- */
 	/* let xml check by expat parser */
	/* ----------------------------- */

	if((doc = (XMLDOMDocument*) CreateObject("FOOLFISH.XMLDOM")) == NULL)
		goto abort;

	/* close DTD validate on parse
	I only need struct, no need to validate. 
	 */
	doc->validateOnParse = FALSE;
	if(doc->loadXML(doc, (char *)xml) != TRUE ||		
	  doc->docType == NULL || doc->docType->dtd == NULL) {
		XMLDOMParseError* error = doc->parseError;
	   	_log(L_ERR,"parse external dtd [%s] error\n"
		   			"errorCode=%d\nfilepos=%d\n"
					"line=%d\nlinepos=%d\nreason=%s\n",
	   		dm->ex_url,
		 	error->errorCode, 
			error->filepos,
		 	error->line, 
			error->linepos,
		 	(error->reason == NULL) ? "NULL" : (error->reason));
	  	goto abort;
	}

	/* Not use parse_dtd to get expat parsed external dtd
	 * we simply use dom to get expat parsed external dtd 
	 */

	*dtd = strdup(doc->docType->dtd);		

	ret = TRUE;
	
  		
abort:
	if(xml != NULL)
		free(xml);		
	if(doc != NULL)
	  	ReleaseObject("FOOLFISH.XMLDOM",/*(void**)*/( XMLDOMDocument** )&doc);	

	if(ret == FALSE) {
		_log(L_ERR,"External DTD (1st) parse error\n");
		_log(L_ERR,"Maybe it has error format\n");
	}

	}
	
	return ret;
}


#endif

static void dm_infomation(struct dtd_mngr* dm) 
{
	_log(L_DBG,"\n=== list DTD info [begin] ===\n"
		"root[%s]\nex_type[%d]\n"
		"ex_name[%s]\nex_url[%s]\n"
		"dtd_ex[%s]\ndtd_in[%s]\n"
	"ex_elmn[%d]\tex_attlist[%d[-%d]]\tex_ent[%d[-%d]]\tex_notation[%d[-%d]]\n"
	"in_elmn[%d]\tin_attlist[%d[-%d]]\tin_ent[%d[-%d]]\tin_notation[%d[-%d]]\n"
		"=== list DTD info [end] ===\n",
		(dm->root==NULL)?"NULL":dm->root,
		dm->ex_type,
		(dm->ex_name==NULL)?"NULL":dm->ex_name ,
		(dm->ex_url==NULL)?"NULL":dm->ex_url ,
#if SHOW_DTD_DETAIL
		(dm->dtd_ex==NULL)?"NULL":dm->dtd_ex,
		(dm->dtd_in==NULL)?"NULL":dm->dtd_in,
#else
		"",
		"",
#endif
		dm->eye.ex.elmn,
		dm->eye.ex.attlist,
		dm->eye.ex.attlist_ign,
		dm->eye.ex.entity,
		dm->eye.ex.entity_ign,
		dm->eye.ex.notation,
		dm->eye.ex.notation_ign,
		dm->eye.in.elmn,
		dm->eye.in.attlist,
		dm->eye.in.attlist_ign,
		dm->eye.in.entity,
		dm->eye.in.entity_ign,
		dm->eye.in.notation,
		dm->eye.in.notation_ign
		);
}

#if (0001) //准接口函数

int _XMLDOM_prepare_dtd(struct dtd_mngr** pdm, char* root, const char* dtd)
{
	int ret;
	struct dtd_mngr *dm = NULL;
	char *ptr = NULL;
	//char *t = NULL;
	#define SEP_CHAR " \r\t\n"
	
	ret = 0;
	*pdm = NULL;

	if(root == NULL || dtd == NULL)
		return 0;
	
	{ /* malloc dtd_mngr struct */
	struct dtd_mngr* t = (struct dtd_mngr*) 
							malloc(sizeof(struct dtd_mngr));
	if(t == NULL)
		return 0;
	*pdm = dm = t;
	}
	memset(dm, 0, sizeof(*dm));
			
	ptr = estrdup(dtd);
	if(ptr == NULL)
		goto done;

	/* link hashi in dm struct */
	dm->hashi = HASH;

	{ /* get root element name */
	char* t = strtok(ptr, SEP_CHAR);
	if(strcmp(t, root) != 0)
		goto done;
	dm->root = strdup(t);
	}
	
	/* get external dtd text */
	{
	char* t;
	t = strtok(NULL, SEP_CHAR);
	if(strcmp(t,"SYSTEM")==0) {	//SYSTEM 类型外部DTD
		dm->ex_type = TYPE_SYSTEM;
		dm->ex_name = NULL;		
		dm->ex_url = strdup(strtok(NULL, "\""));		

		t = strtok(NULL, SEP_CHAR);
	}
	else if(strcmp(t,"PUBLIC")==0) {//PUBLIC 类型外部DTD	
		dm->ex_type = TYPE_PUBLIC;
		dm->ex_name = strdup(strtok(NULL, "\""));
		strtok(NULL,"\"");
		dm->ex_url = strdup(strtok(NULL, "\""));

		t = strtok(NULL, SEP_CHAR);
	}
	
	/* get internal dtd text */
	if(t != NULL && strncmp(t,"[",1) ==0) {
		dm->dtd_in = strdup(strtok(NULL, "]"));
	}	
	}

	/* get external DTD contents */
	if(dm->ex_type == TYPE_PUBLIC || dm->ex_type == TYPE_SYSTEM) {
		if( get_external_dtd(dm, &(dm->dtd_ex)) == FALSE) {
			_log(L_ERR,"can NOT get external DTD\n");
			goto done;
		}	
	}

	/* parse DTD contents to struct data */
	dm->eye.ex = prepare_hash_tbl(dm->dtd_ex, 
								dm->ex_tbl, 
								dm->att_ex_tbl, 
								dm->ent_ex_tbl,
								dm->not_ex_tbl,
								HASH);
	dm->eye.in = prepare_hash_tbl(dm->dtd_in, 
								dm->in_tbl, 
								dm->att_in_tbl, 
								dm->ent_in_tbl,
								dm->not_in_tbl,
								HASH);

	_log(L_DBG,"dm infomation after prepare struct\n");
	dm_infomation(dm);

	/* parse element data */
	parse_hash_tbl(dm->ex_tbl, HASH);	
	parse_hash_tbl(dm->in_tbl, HASH);		

	/* parse attlist data */
	/* mark duplicate node with (flag = FLAG_NOUSE) */
	dm->eye.ex.attlist += parse_att_hash_tbl(dm->att_ex_tbl, HASH);
	dm->eye.in.attlist += parse_att_hash_tbl(dm->att_in_tbl, HASH);

	/* parse entity data */
	parse_ent_hash_tbl(dm->ent_ex_tbl, HASH);	
	parse_ent_hash_tbl(dm->ent_in_tbl, HASH);		

	/* parse notation data */
	parse_not_hash_tbl(dm->not_ex_tbl, HASH);	
	parse_not_hash_tbl(dm->not_in_tbl, HASH);		
	
	_log(L_DBG,"dm infomation after parse it\n");
	dm_infomation(dm);
	
	/* check DTD itself 's validatablility */
	/* mark duplicate node with (flag = FLAG_NOUSE) */
	check_hash_tbl(dm, HASH);
	check_ent_hash_tbl(dm, HASH);
	check_not_hash_tbl(dm,HASH);
	check_att_hash_tbl(dm, HASH);

	/* note: at here there maybe have FLAG_NOUSE node
	in dm->ent, dm->att, dm->not. so in later using these
	data, we MUST avoid to use these nodes. or at here
	I erase these nodes ? */

	_log(L_DBG,"dm infomation after check it\n");
	dm_infomation(dm);
	
	_log(L_INFO,"prepare DTD struct *** SUCCESS ***\n");

	ret = 1;

done:
	if(ptr != NULL)
		free(ptr);

	return ret;
}


int _XMLDOM_add_entity(struct dtd_mngr* dm, void* ptr)
{
	/* recurse dom object and if entity references found, 
	find entity defination in DTD struct, then get it's real
	defination and replace entity reference with it */
	
	XMLDOMNode*	p = (XMLDOMNode*) ptr;
	XMLDOMNode* n;
	XMLDOMNodeList* list;

	int hashi;
	//int found = 0;

	if(dm == NULL || ptr == NULL) 
		return 0;
	
	hashi = sizeof(dm->ex_tbl) / sizeof(dm->ex_tbl[0]);
	hashi = sizeof(dm->in_tbl) / sizeof(dm->in_tbl[0]);

	if(p->nodeType == NODE_DOCUMENT  && p->hasChildNodes==TRUE) {
		list = p->childNodes;
		list->reset(list);
		while((n = list->nextNode(list)) != (XMLDOMNode*)NULL) {
			/* found the root element then start to replace 
			fot entity reference MUST be in root element, can
			NOT be a slibing of root element, or replace it */
			if(n->nodeType == NODE_ELEMENT)
				return add_entity_sub(dm, (void*) n, hashi);
		}
	}

	return 0;	
}

int _XMLDOM_match_attlist(struct dtd_mngr* dm, void* ptr)
{
	XMLDOMNode*	p = (XMLDOMNode*) ptr;
	XMLDOMNode* n;
	XMLDOMNodeList* list;

	/*
	ATTLIST check & match algrithm:

	#IMPLIED	we simply ignore this type of attribute
	#REQUIRED	we required this attribute, so first seek wether it exist
				in xml, if NOT exist, report error, else pass it 
	#FIXED		fixed attribute, first we seek wether it exist in xml,
				if NOT exist, append this node, and give this node
				fixed value in DTD. if exist, but it's value doesn't match 
				with appear in DTD, report error, else pass it
	"default"	first check wether it exist in xml, if NOT exist, append
				this node, and give the value appear in DTD, else
				if exist, pass it
	att-in-xml	check every attribute name in each element, if any one
				does NOT appear in DTD's attlist or is duplicate, report
				error
	*/

	int hashi;
	//int found = 0;

	if(dm == NULL || ptr == NULL) 
		return 0;
	
	hashi = sizeof(dm->ex_tbl) / sizeof(dm->ex_tbl[0]);
	hashi = sizeof(dm->in_tbl) / sizeof(dm->in_tbl[0]);

	if(p->nodeType == NODE_DOCUMENT && p->hasChildNodes==TRUE) {
		list = p->childNodes;
		list->reset(list);
		while((n = list->nextNode(list)) != (XMLDOMNode*)NULL) {
			if(n->nodeType == NODE_ELEMENT) {
				int ret;
			
				/* recurse domtree object nodes 1st time, 
				check every element's attributes, wether match
				with attlist defined in DTD */
				ret = check_attlist_sub(dm, (void*) n, hashi);
				if(ret == 0) return 0;
				
				/* recurse domtree object nodes 2nd time
				[a] we have already build ID list during 1st time
				   then we can check IDREF at here
				*/
				ret = check_attlist_all(dm, (void*) n, hashi);
				if(ret == 0) return 0;

				/* find root node , then quit */
				return 1;
			}
		}
	}

	return 0;
}



int _XMLDOM_check_dtd(struct dtd_mngr* dm, void* ptr)
{
	XMLDOMNode*	p = (XMLDOMNode*) ptr;
	XMLDOMNode* n;
	XMLDOMNodeList* list;

	int hashi;
	//int found = 0;

	if(dm == NULL || ptr == NULL) 
		return 0;
	
	hashi = sizeof(dm->ex_tbl) / sizeof(dm->ex_tbl[0]);
	hashi = sizeof(dm->in_tbl) / sizeof(dm->in_tbl[0]);

	/* if root node , check root element declaration */
	if(p->nodeType == NODE_DOCUMENT && p->hasChildNodes == TRUE) {
		list = p->childNodes;
		list->reset(list);
		while((n = list->nextNode(list)) != (XMLDOMNode*)NULL) {
			if(n->nodeType == NODE_ELEMENT) {
				/* check root element */
				if(strcmp(n->nodeName,dm->root) !=0) {
					_log(L_ERR,"root element [%s] not match");
					_log(L_ERR," with DTD root element [%s]\n", 
									n->nodeName,dm->root);
					_log(L_ERR,"DTD validate failed\n");
					return 0;
				}
				/* check other element */
				return check_dtd_sub(dm, (void*) n, hashi);
			}
		}
	}

	return 0;	
}


int _XMLDOM_release_dtd(struct dtd_mngr** pdm)
{
	struct dtd_mngr* dm = *pdm;
	
	if(dm->root != NULL)		free(dm->root);
	if(dm->ex_name != NULL)		free(dm->ex_name);
	if(dm->ex_url != NULL)		free(dm->ex_url);
	if(dm->dtd_ex != NULL)		free(dm->dtd_ex);
	if(dm->dtd_in != NULL)		free(dm->dtd_in);

	free_hash_tbl(ELEMENT_ITEM, (void*) dm->ex_tbl, HASH);
	free_hash_tbl(ELEMENT_ITEM, (void*) dm->in_tbl, HASH);

	free_hash_tbl(ATTLIST_ITEM, (void*) dm->att_ex_tbl, HASH);
	free_hash_tbl(ATTLIST_ITEM, (void*) dm->att_in_tbl, HASH);

	free_hash_tbl(ENTITY_ITEM, (void*) dm->ent_ex_tbl, HASH);
	free_hash_tbl(ENTITY_ITEM, (void*) dm->ent_in_tbl, HASH);

	free_hash_tbl(NOTATION_ITEM, (void*) dm->not_ex_tbl, HASH);
	free_hash_tbl(NOTATION_ITEM, (void*) dm->not_in_tbl, HASH);

	/* free ID list add */
	if(dm->id_list != NULL) {
		struct e_value *p, *t = dm->id_list;
		while(t != NULL) {
			p = t->next; free(t); t = p;
		}
		dm->id_list = NULL;
	}

	free(dm);
	dm = NULL;

	return 1;
}

#endif

#if (0001) //接口函数
int check_DTD(void* v)
{
	XMLDOMDocument* doc = (XMLDOMDocument*) v;
	struct dtd_mngr* dm = NULL;
	int result = 0;
	
	if(doc == NULL || doc->docType == NULL)
		return 0;

	if(setjmp(jmp_env))
		goto done;
	
	/* parse DTD docs, build struct about elements, attlist, entities & etc. */
	result = _XMLDOM_prepare_dtd(&dm, doc->docType->name, 
							(const char*) doc->docType->dtd);
	if(result == 0)
		goto done;

	/* replace entity references in DOMTREE with entity in DTD struct */
	result = _XMLDOM_add_entity(dm, (void*) doc->documentElement);
	_log(L_INFO,"replace entity in domtree *** %s ***\n",
					(result)?"SUCCESS":"FAILURE");
	if(result == 0)
		goto done;

	/* check and match attlist in DOMTREE with attlist in DTD struct */
	result = _XMLDOM_match_attlist(dm, (void*) doc->documentElement);
	_log(L_INFO,"check and match attlist in domtree *** %s ***\n",
					(result)?"SUCCESS":"FAILURE");
	if(result == 0)
		goto done;

	/* check NEW domtree (entity-replaced & att check_matched) 
	match ELEMENT define */
	if(result == 1) {
		result = _XMLDOM_check_dtd(dm, (void*) doc->documentElement);		
		_log(L_INFO,"check DTD *** %s ****\n",(result)?"OK":"NOT match!");
	}

done:
	if(dm != NULL)
		_XMLDOM_release_dtd(&dm);

	if(result == 0) {
		if(doc->parseError == NULL) {
			XMLDOMParseError* e;
			e = (XMLDOMParseError*) malloc(sizeof(*e));
			doc->parseError = e;
		}
		//TODO: rearrange the error code in DTD check
		doc->parseError->errorCode = 0;
		doc->parseError->filepos = 0;
		doc->parseError->line = 0;
		doc->parseError->linepos = 0;
		/* NOTE: at here reason use static string
		 */
		doc->parseError->reason = "dtd check error";
	}

	return result;
}
#endif
