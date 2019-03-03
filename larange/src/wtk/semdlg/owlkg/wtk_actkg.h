#ifdef USE_CRF
#ifndef WTK_SEMDLG_OWLKG_WTK_ACTKG
#define WTK_SEMDLG_OWLKG_WTK_ACTKG
#include "wtk/core/wtk_type.h" 
#include "wtk/lex/wtk_lex.h"
#include "wtk/lex/fst/wtk_nlgfst.h"
#include "wtk_actkg_cfg.h"
#include "wtk/lua/wtk_lua2.h"
#include "wtk/core/wtk_jsonkv.h"
#include "wtk/core/wtk_fkv.h"
#include "wtk/core/wtk_robin.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_actkg wtk_actkg_t;

typedef struct
{
	wtk_strbuf_t *ask;
	wtk_strbuf_t *ans;
}wtk_actkg_history_t;

struct wtk_actkg
{
	wtk_actkg_cfg_t *cfg;
	wtk_crfact_parser_t* crfact;
	wtk_owlkv_t* owlkv;
	wtk_fkv_t *vkv;
	wtk_lua2_t *lua;
	wtk_lexr_t *lex;
	wtk_nlgfst_t *nlgfst;
	wtk_crfact_t *act;
	wtk_strbuf_t *buf;
	wtk_strbuf_t *tmp;
	wtk_heap_t *heap;
	wtk_strbuf_t *ans_act;
	wtk_owl_item_t *owl_item;
	wtk_owl_item_t *owl_item2;
	wtk_string_t *ask_yes_set;
	wtk_string_t *ask_yes_set2;
	wtk_strbuf_t *ask_p;
	wtk_strbuf_t *ask_p2;
	wtk_strbuf_t *ask_k;
	wtk_strbuf_t *ask_k2;
	wtk_robin_t *history;
	wtk_string_t input;
	void *ext_ths;
	unsigned use_lex:1;
};

wtk_actkg_t* wtk_actkg_new(wtk_actkg_cfg_t *cfg,wtk_lua2_t *lua,wtk_lexr_t *lex);
void wtk_actkg_delete(wtk_actkg_t *kg);
void wtk_actkg_reset(wtk_actkg_t *kg);
void wtk_actkg_set_ext(wtk_actkg_t *kg,void *ths);
wtk_string_t wtk_actkg_get_output(wtk_actkg_t *kg);
wtk_string_t wtk_actkg_process(wtk_actkg_t *kg,char *s,int len);

int wtk_actkg_save_act(wtk_actkg_t *kg,wtk_crfact_t *act);
int wtk_actkg_link_act(wtk_actkg_t *kg,wtk_crfact_t *act,wtk_string_t *p,wtk_string_t *p_cls,wtk_string_t *v,wtk_string_t *t,wtk_string_t *t_cls);
wtk_string_t wtk_actkg_process_nlg(wtk_actkg_t *kg,char *v,int v_bytes);
wtk_string_t wtk_actkg_get_v_expand(wtk_actkg_t *kg,char *v,int v_bytes);
wtk_string_t wtk_actkg_answer_act(wtk_actkg_t *kg,wtk_crfact_t *act);

#ifdef __cplusplus
};
#endif
#endif
#endif
