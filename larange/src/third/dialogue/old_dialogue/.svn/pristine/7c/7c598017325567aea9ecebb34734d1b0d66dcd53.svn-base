#ifndef WTK_SEMDLG_SEMFST_WTK_SEMFST_NET
#define WTK_SEMDLG_SEMFST_WTK_SEMFST_NET
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_queue.h"
#include "wtk/core/wtk_queue3.h"
#include "wtk/lex/wtk_lex.h"
#include "wtk/core/wtk_str_parser.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_semfst_net wtk_semfst_net_t;


typedef struct
{
	wtk_queue_node_t q_n;
	wtk_string_t *k;
	wtk_string_t *v;
}wtk_semfst_slot_t;

typedef enum
{
	WTK_SEMFSTSTR_OUTPUT_STR,
	WTK_SEMFSTSTR_OUTPUT_LUA,
}wtk_semfst_output_type_t;

typedef struct
{
	wtk_queue_node_t q_n;
	union {
		wtk_string_t *str;
		wtk_string_t *var;
	}v;
	unsigned is_var;
}wtk_semfst_output_str_item_t;

typedef struct
{
	wtk_semfst_output_type_t type;
	union
	{
		char *lua;
		wtk_queue3_t *strq;
	}v;
}wtk_semfst_output_item_t;

typedef struct wtk_semfst_script wtk_semfst_script_t;

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_queue3_t slot_q; //wtk_semfst_slot_t
	wtk_semfst_script_t *next;
	wtk_semfst_output_item_t **output_items;
	int n_output_item;
	unsigned get_answer:1;
	unsigned reset:1;
	unsigned skip:1;
}wtk_semfst_output_t;

struct wtk_semfst_script
{
	wtk_queue_node_t q_n;
	wtk_string_t *name;
	wtk_queue3_t output_q;
	wtk_semfst_output_t *other;
	//wtk_semfst_output_t *fail;
	wtk_lex_script_t *lexscript;
	wtk_lex_net_t *lexnet;
};

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_queue3_t script_q;
	wtk_semfst_script_t *init;
	wtk_string_t *name;
}wtk_semfst_sceen_t;

struct wtk_semfst_net
{
	wtk_heap_t *heap;
	wtk_strbuf_t *buf;
	wtk_lexc_t *lexc;
	wtk_lex_script_t *lexlib;
	wtk_queue_t sceen_q;
};

wtk_semfst_net_t* wtk_semfst_net_new(wtk_lexc_t *lexc);
void wtk_semfst_net_delete(wtk_semfst_net_t *r);

int wtk_semfst_net_set_lexlib(wtk_semfst_net_t *r,char *fn,int fn_bytes);

wtk_semfst_sceen_t* wtk_semfst_net_pop_sceen(wtk_semfst_net_t *r,char *nm,int nm_bytes);
wtk_semfst_script_t* wtk_semfst_net_find_script(wtk_semfst_net_t *r,wtk_semfst_sceen_t *sceen,char *nm,int nm_bytes,int insert);

int wtk_semfst_net_set_script_lex(wtk_semfst_net_t *r,wtk_semfst_script_t *script,char *lex,int lex_len);
wtk_semfst_output_t* wtk_semfst_net_pop_script_output(wtk_semfst_net_t *r,wtk_semfst_script_t *script);
void wtk_semfst_net_set_script_output_slot(wtk_semfst_net_t *r,wtk_semfst_output_t *output,wtk_string_t *k,wtk_string_t *v);
wtk_semfst_output_item_t* wtk_semfst_net_output_new_output_item(wtk_semfst_net_t *r,char *data,int len);
void wtk_semfst_net_set_output_items(wtk_semfst_net_t *r,wtk_semfst_output_t *output,wtk_semfst_output_item_t **items,int n);

wtk_semfst_output_item_t* wtk_semfst_output_get(wtk_semfst_output_t *output);
void wtk_semfst_net_print(wtk_semfst_net_t *r);

#ifdef __cplusplus
};
#endif
#endif
