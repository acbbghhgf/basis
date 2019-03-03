#ifndef WTK_SEMDLG_OWLKG_WTK_OWLKV
#define WTK_SEMDLG_OWLKG_WTK_OWLKV
#include "wtk/core/wtk_type.h" 
#include "wtk/lex/owl/wtk_owl.h"
#include "wtk_owlkv_cfg.h"
#include "wtk/core/wtk_jsonkv.h"
#include "wtk/lex/nlg/wtk_nlg2_parser.h"
#include "wtk/core/wtk_str_parser.h"
#include "wtk/lua/wtk_lua2.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_owlkv wtk_owlkv_t;
#define wtk_owlkv_gen_text_ss(kv,p,nlg)  wtk_owlkv_gen_text(kv,p,sizeof(p)-1,nlg,sizeof(nlg)-1,NULL)

typedef struct
{
	wtk_string_t k;
	wtk_string_t v;
}wtk_owlkv_item_t;

struct wtk_owlkv
{
	wtk_owlkv_cfg_t *cfg;
	wtk_nlg2_t *nlg;
	wtk_jsonkv_t *class_kv;
	wtk_jsonkv_t *inst_kv;
	wtk_owl_tree_t *owl;
	wtk_lexr_t *lex;
	wtk_strbuf_t *buf;
	wtk_strbuf_t *tmp;
	wtk_heap_t *heap;
	unsigned save_json:1;
};

wtk_owlkv_t* wtk_owlkv_new(wtk_owlkv_cfg_t *cfg,wtk_lexr_t *lex);
void wtk_owlkv_delete(wtk_owlkv_t *kv);

wtk_owl_class_t* wtk_owlkv_get_class(wtk_owlkv_t *kv,char *hint,int hint_bytes);

wtk_owl_item_t* wtk_owlkv_get_topic2(wtk_owlkv_t *kv,char *hint,int hint_bytes,char *item,int item_bytes);
int wtk_owlkv_is_value_valid(wtk_owlkv_t *kv,char *p,int p_bytes,char *a,int a_bytes,char *v,int v_bytes);
wtk_owl_item_t* wtk_owlkv_set_value(wtk_owlkv_t *kv,char *p,int p_bytes,char *a,int a_bytes,char *v,int v_bytes);
wtk_owl_item_t* wtk_owlkv_set_negative_value(wtk_owlkv_t *kv,char *p,int p_bytes,char *a,int a_bytes,char *v,int v_bytes);

/**
 * p is calss name
 */
wtk_owl_item_t* wtk_owlkv_get_owl_item(wtk_owlkv_t *kv,char *p,int p_len,char *item,int item_len);
wtk_string_t* wtk_owlkv_get_attr(wtk_owlkv_t *kv,char *p,int p_len,char *item,int item_len,char *a,int a_len);
wtk_string_t wtk_owlkv_gen_text(wtk_owlkv_t *kv,char *p,int p_len,char *nlg,int nlg_len,wtk_nlg2_gen_env_t *env);
void wtk_owlkv_touch_inst(wtk_owlkv_t *kv,char *inst,int inst_len,char *cls,int cls_len);
int wtk_owlkv_check_value(wtk_owlkv_t *kv,char *p,int p_len,char *item,int item_len,char *v,int v_len);

/*
 * 获取 人  pvt.喜欢 的值
 */
wtk_owlkv_item_t wtk_owlkv_get_value(wtk_owlkv_t *kv,char *p,int p_len,char *item,int item_len);

wtk_owlkv_item_t wtk_owlkv_get_owl_value(wtk_owlkv_t *kv,char *p,int p_len,wtk_owl_item_t *oi);

/**
 *  获取 sub item 满足的value 的item;
 */
wtk_owl_item_t* wtk_owlkv_get_matched_item(wtk_owlkv_t *kv,wtk_owl_item_t *item,char *a,int a_bytes,wtk_string_t *v);

wtk_owl_item_t* wtk_owlkv_get_matched_owl(wtk_owlkv_t *kv,char *p,int p_bytes,char *a,int a_bytes,char *v,int v_bytes);

int wtk_owlkv_get_matched_item_lex(wtk_owlkv_t *kv,wtk_owl_item_t *item,char *a,int a_bytes,wtk_string_t* v,int use_parent_lex);
wtk_owl_item_t* wtk_owlkv_get_owl_by_p(wtk_owlkv_t *kv,char *p,int p_len,char *item,int item_len);
wtk_string_t wtk_owlkv_ans(wtk_owlkv_t *kv,char *p,int p_len,char *item,int item_len,char *a,int a_bytes,char *v,int v_len);
wtk_string_t wtk_owkv_get_owl_nlg_text(wtk_owlkv_t *kv,wtk_owl_item_t *item,wtk_string_t *nlg_nm,wtk_string_t *p,wtk_nlg2_gen_env_t *env);
wtk_string_t wtk_owkv_get_owl_nlg(wtk_owlkv_t *kv,wtk_owl_item_t *item,wtk_string_t *nlg_nm,wtk_string_t *p);

wtk_string_t* wtk_owlkv_owl_item_get_attr(wtk_owlkv_t *kv,wtk_owl_item_t *item,wtk_string_t *a,wtk_string_t *p);

wtk_owl_item_t* wtk_owlkv_get_unselect_item(wtk_owlkv_t *kv,wtk_owl_item_t *item,wtk_string_t *p);
wtk_owl_item_t* wtk_owlkv_get_select_item(wtk_owlkv_t *kv,wtk_owl_item_t *item,wtk_string_t *p);

wtk_owlkv_item_t wtk_owlkv_get_owl_item_by_rel(wtk_owlkv_t *kv,char *data,int len);

wtk_owl_item_t* wtk_owlkv_get_ask_topic(wtk_owlkv_t *kv,char *hint,int hint_bytes);
wtk_owl_item_t* wtk_owlkv_get_freeask_topic(wtk_owlkv_t *kv,char *hint,int hint_bytes);
wtk_string_t* wtk_owlkv_get_freetalk_nlg(wtk_owlkv_t *kv,char *hint,int hint_bytes);

void wtk_owlkv_set_item_value(wtk_owlkv_t *kv,char *inst,int inst_bytes,wtk_owl_item_t *item,char *v,int v_bytes,int is_neg);
wtk_string_t wtk_owlkv_get_json_value(wtk_owlkv_t *kv,char *p,int p_len,char *item,int item_len);
void wtk_owlkv_link_lua(wtk_lua2_t *lua2);
int wtk_owlkv_item_check_value(wtk_owlkv_t *kv,char *p,int p_len,wtk_owl_item_t *oi,char *v,int v_len);
#ifdef __cplusplus
};
#endif
#endif
