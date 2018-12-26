#ifndef WTK_SEMDLG_OWLKG_WTK_OWLKG
#define WTK_SEMDLG_OWLKG_WTK_OWLKG
#include "wtk/core/wtk_type.h" 
#include "wtk_owlkg_cfg.h"
#include "wtk/lex/owl/wtk_owl.h"
#include "wtk/lex/nlg/wtk_nlg.h"
#include "wtk/core/wtk_jsonkv.h"
#include "wtk/lua/wtk_lua2.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_owlkg wtk_owlkg_t;
typedef struct
{
	wtk_queue_node_t q_n;
	wtk_string_t *k;
	wtk_string_t *v;
	wtk_owl_item_t *owl_item;
}wtk_owlkg_act_item_t;

typedef enum
{
	WTK_OWLKG_ACT_ASK,
	WTK_OWLKG_ACT_GET,
	WTK_OWLKG_ACT_ADD,
	WTK_OWLKG_ACT_DEL,
	WTK_OWLKG_ACT_CHECK,
}wtk_owlkg_act_type_t;

typedef struct
{
	wtk_queue_t item_q; //wtk_owlkg_act_item_t
	wtk_owlkg_act_type_t type;
}wtk_owlkg_act_t;

struct wtk_owlkg
{
	wtk_owlkg_cfg_t *cfg;
	wtk_lua2_t *lua;
	wtk_jsonkv_t *kv;
	wtk_owl_tree_t *owl;
	wtk_nlg_t *nlg;
	wtk_strbuf_t *buf;
	wtk_heap_t *heap;
};

wtk_owlkg_t* wtk_owlkg_new(wtk_owlkg_cfg_t *cfg,wtk_lua2_t *lua);
void wtk_owlkg_delete(wtk_owlkg_t *kg);
wtk_string_t wtk_owlkg_process(wtk_owlkg_t *kg,char *s,int s_len);
#ifdef __cplusplus
};
#endif
#endif
