#ifndef WTK_SEMDLG_KG_WTK_KG
#define WTK_SEMDLG_KG_WTK_KG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk/lex/wtk_lex.h"
#include "wtk/lex/fst/wtk_nlgfst.h"
#include "wtk/lex/nlg/wtk_nlg2_parser.h"
#include "wtk/core/wtk_array.h"
#include "wtk/core/cfg/wtk_cfg_file.h"
#include "wtk/core/wtk_queue3.h"
#include "wtk/core/wtk_if.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_kg wtk_kg_t;

typedef struct wtk_kg_class wtk_kg_class_t;
typedef struct wtk_kg_item wtk_kg_item_t;

typedef enum
{
	WTK_KG_ITEM_STR,
	WTK_KG_ITEM_ARRAY,
	WTK_KG_ITEM_INT,
}wtk_kg_item_value_t;

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_string_t *k;
	wtk_string_t *v;
}wtk_kg_item_attr_kv_t;

typedef struct
{
	wtk_string_t *ask;
	wtk_string_t *set;
	wtk_string_t *answer;
	wtk_string_t *ask2;
	wtk_string_t *confirm;
	wtk_string_t *check;
	wtk_string_t *args;
	char *post_v;
	wtk_queue3_t *attr_q;
}wtk_kg_item_attr_t;

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_if_t *_if;
	union
	{
		wtk_kg_item_t *item;
		char *lua;
	}v;
	unsigned is_item:1;
}wtk_kg_item_next_item_t;

typedef struct
{
	wtk_queue3_t next_q;	//wtk_kg_item_next_item_t
}wtk_kg_item_next_t;

struct wtk_kg_item
{
	wtk_queue_node_t q_n;
	wtk_string_t *name;
	wtk_nlgnet_t *nlg_net;
	wtk_kg_item_attr_t *attr;
	wtk_kg_item_next_t *next;
	wtk_kg_item_value_t vtype;
	unsigned _virtual:1;
	unsigned use_last_best:1;
};


struct wtk_kg_class
{
	wtk_string_t *name;
	wtk_queue3_t item_q;	//wtk_kg_item_t
	wtk_kg_item_t *freeask;
	wtk_kg_item_t *freetalk;
};

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_kg_item_t *item;
	union {
		wtk_string_t *str;
		struct
		{
			wtk_string_t **strs;
			int n;
		}a;
		int i;
	}v;
}wtk_kg_inst_value_t;

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_string_t *name;
	wtk_queue3_t item_q; //wtk_kg_inst_value_t
}wtk_kg_inst_t;

struct wtk_kg
{
	wtk_heap_t *heap;
	wtk_kg_class_t *_class;
	wtk_queue3_t inst_q;
};

wtk_kg_t* wtk_kg_new();
void wtk_kg_delete(wtk_kg_t *kg);

wtk_kg_class_t* wtk_kg_find_class(wtk_kg_t *kg,char *nm,int nm_len,int insert);
wtk_kg_item_t* wtk_kg_class_get_item(wtk_kg_t *kg,wtk_kg_class_t *cls,char *nm,int nm_len,int insert);
int wtk_kg_item_set_attr(wtk_kg_t *kg,wtk_kg_item_t *ki,char *data,int len);
int wtk_kg_item_set_next(wtk_kg_t *kg,wtk_kg_item_t *ki,char *data,int len);
void wtk_kg_set_inst_value(wtk_kg_t *kg,wtk_kg_inst_t *inst,char *data,int len);

#define wtk_kg_get_inst_s(kg,name) wtk_kg_get_inst(kg,name,sizeof(name)-1,0)

wtk_kg_inst_t* wtk_kg_get_inst(wtk_kg_t *kg,char *nm,int nm_len,int insert);
void wtk_kg_update(wtk_kg_t *kg);
void wtk_kg_print(wtk_kg_t *kg);

wtk_string_t* wtk_kg_item_get_attr(wtk_kg_item_t *ki,char *p,int len);

#define wtk_kg_set_inst_value_item_str_s(inst,k,v) wtk_kg_set_inst_value_item_str(inst,k,sizeof(k)-1,v)
#define wtk_kg_set_inst_value_item_number_s(inst,k,v) wtk_kg_set_inst_value_item_number(inst,k,sizeof(k)-1,v)

void wtk_kg_set_inst_value_item_str(wtk_kg_inst_t *inst,char *k,int k_len,wtk_string_t *v);
void wtk_kg_set_inst_value_item_number(wtk_kg_inst_t *inst,char *k,int k_len,int i);
#ifdef __cplusplus
};
#endif
#endif
