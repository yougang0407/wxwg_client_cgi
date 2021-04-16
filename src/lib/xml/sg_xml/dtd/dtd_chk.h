/* change enum define from enum {} var; to enum var {}; */

enum __dtd_item__ {
	INVALID_ITEM = 0,
	ELEMENT_ITEM = 1,
	ATTLIST_ITEM = 2,
	NOTATION_ITEM = 3,
	ENTITY_ITEM = 4,
};

enum __flag__ {
	FLAG_INVALID = 0,			//解析错误
	FLAG_NORMAL = 1,
	FLAG_NOUSE = 2,				//__flag__ 功能目前没用
}; 

enum __action__ {	//use in struct dtd_node
	ACTION_INVALID = 0,
	ACTION_ONE = 1,				//
	ACTION_MULTI = 2,			//*
	ACTION_PLUS = 3,			//+
	ACTION_QUESTION = 4,		//?

	ACTION_ANY = 101,
	ACTION_EMPTY = 102,
};

enum __mode__ {	//use in struct dtd_node
	MODE_INVALID = 0,
	MODE_ARRAY = 1,
	MODE_OR = 2,
};

struct dtd_node {
	char* name;					//tag名称		#ANY, #EMPTY, normal
	char* define;				//tag定义字符串		
	enum __flag__ flag;			//tag node 状态
	enum __action__ action;		//tag动作 		* | + | ? | one
	enum __mode__ mode;			//tag模式 		and , or
	struct dtd_node* child; 	//子tag列表		tag child node list
	struct dtd_node* prev;		//tag的前一个兄弟
	struct dtd_node* next;		//tag的下一个兄弟
	struct dtd_node* hash_nt;	//hash表上的下一个接点
};


enum __att_type__ {	//use in struct dtd_attnode
	ATT_TYPE_INVALID = 0,
	ATT_TYPE_CDATA,
	ATT_TYPE_ENUM,
	ATT_TYPE_ID,
	ATT_TYPE_IDREF,
	ATT_TYPE_IDREFS,
	ATT_TYPE_ENTITY,
	ATT_TYPE_ENTITIES,
	ATT_TYPE_NMTOKEN,
	ATT_TYPE_NMTOKENS,
	ATT_TYPE_NOTATION,
};

#define attrTypeString(t)\
	((t==ATT_TYPE_CDATA)?"CDDATA":\
	((t==ATT_TYPE_ENUM)?"ENUM":\
	((t==ATT_TYPE_ID)?"ID":\
	((t==ATT_TYPE_IDREF)?"IDREF":\
	((t==ATT_TYPE_IDREFS)?"IDREFS":\
	((t==ATT_TYPE_ENTITY)?"ENTITY":\
	((t==ATT_TYPE_ENTITIES)?"ENTITIES":\
	((t==ATT_TYPE_NMTOKEN)?"NMTOKEN":\
	((t==ATT_TYPE_NMTOKENS)?"NMTOKENS":\
	((t==ATT_TYPE_NOTATION)?"NOTATION":\
	"unknown attr type"))))))))))

enum __att_mode__ {
	ATT_MODE_DEFAULT = 0,				//no mode set
	ATT_MODE_REQUIRED = 1,				//must exist
	ATT_MODE_IMPLIED = 2,				//exist or not
	ATT_MODE_FIXED = 3,					//fixed value
};

struct e_value {	//use in struct dtd_attnode
	char* value;
	struct e_value* next;
};

struct dtd_attnode {
	char* name;
	char* define;
	char* attr;
	enum __flag__ flag;
	enum __att_type__ type;
	enum __att_mode__ mode;
	struct e_value* e_value;
	char* defaut;
	struct dtd_attnode* sibling;
	struct dtd_attnode* next;
	struct dtd_attnode* hash_nt;
};


enum __entity_type__ {	//use in struct dtd_entity
	ENT_TYPE_INVALID = 0,
	ENT_TYPE_COMMON_INTERNAL = 1,
	ENT_TYPE_COMMON_EXTERNAL,
	ENT_TYPE_PARAM_INTERNAL,			//NOT support
	ENT_TYPE_PARAM_EXTERNAL,			//NOT support
};


enum __ent_data_type__ {
	ENT_DATA_INVALID = 0,
	ENT_DATA_XML_WHOLE = 1,		//struct XMLDOMDocument*
	ENT_DATA_XML_PIECE = 2,		//struct XMLDOMDocument* root == foolfish
	ENT_DATA_UNKNOWN =   99,	//struct { int len; char* raw_data;};
};

struct __entity_data__ {
	enum __ent_data_type__ type;	
	void* ptr;
};

struct dtd_entity {
	char* name;
	char* define;
	enum __flag__ flag;
	enum __entity_type__ type;
	char* entity;
	struct __entity_data__ data;

	/* TODO here should have next otem */
	
	/* this struct have NO next element node.  
	  for, if found same entity in ent_ex_tbl or
	  ent_in_tbl simply ignore the second one , 
	  do this as xmlspy and ie action 
	  */
	  
	struct dtd_entity* hash_nt;
};


#define HASH 1 //37

enum __dtd_extype__ {	
	TYPE_SYSTEM = 1,
	TYPE_PUBLIC = 2,
};


struct dtd_notation {
	char* name;
	char* define;
	enum __flag__ flag;
	enum __dtd_extype__ type;
	char* publicID;
	char* url;	
	struct dtd_notation* hash_nt;
};

struct dtd_count {
	int elmn;
	int attlist;
	int attlist_ign;
	int notation;
	int notation_ign;
	int entity;
	int entity_ign;
};

struct dtd_eye {
	struct dtd_count ex;
	struct dtd_count in;
};

struct dtd_mngr {
	char* root;						//根元素名称	
	int ex_type;
	char* ex_name;
	char* ex_url;	
	char* dtd_ex;					//外部DTD（可能不用）	
	char* dtd_in;					//内部DTD（可能不用）	
	int hashi;
	struct dtd_node* ex_tbl[HASH];	//外部dtd node接点散列表
	struct dtd_node* in_tbl[HASH];	//内部dtd node接点散列表
	
	struct dtd_attnode* att_ex_tbl[HASH];
	struct dtd_attnode* att_in_tbl[HASH];

	/* add entity struct */
	struct dtd_entity* ent_ex_tbl[HASH];
	struct dtd_entity* ent_in_tbl[HASH];

	/* add notation struct */
	struct dtd_notation* not_ex_tbl[HASH];
	struct dtd_notation* not_in_tbl[HASH];

	/* add every ID value in domtree */
	struct e_value* id_list;
	
	struct dtd_eye eye;				//统计数据
};

int check_DTD(void* v);

