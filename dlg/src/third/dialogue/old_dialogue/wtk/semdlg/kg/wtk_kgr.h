#ifndef WTK_SEMDLG_KG_WTK_KGR
#define WTK_SEMDLG_KG_WTK_KGR
#include "wtk/core/wtk_type.h" 
#include "wtk_kgr_cfg.h"
#include "wtk/semdlg/semfld/wtk_act_lua.h"
#include "wtk/core/wtk_jsonkv.h"
#include "wtk/lex/nlg/wtk_nlg2_parser.h"
#include "wtk/lex/fst/wtk_nlgfst2.h"
#include "wtk/lua/wtk_lua2.h"
#include "wtk/core/rbin/wtk_rbin2.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_kgr wtk_kgr_t;
struct wtk_kgr
{
	wtk_kgr_cfg_t *cfg;
	wtk_jsonkv_t *inst_kv;
	wtk_nlg2_t *nlg;
	wtk_nlgfst2_t *fst;
	struct wtk_semfld *fld;
	wtk_kg_item_t *last;
	wtk_kg_item_t *cur;
	wtk_kg_item_t *next;
	wtk_act_t *act;
	wtk_strbuf_t *tmp;
	wtk_strbuf_t *last_p;
	int act_nv;
	unsigned dirty:1;
};

void wtk_kgr_link_lua(wtk_lua2_t *lua2);
wtk_kgr_t* wtk_kgr_new(wtk_kgr_cfg_t *cfg,struct wtk_semfld *fld,wtk_rbin2_t *rbin);
void wtk_kgr_update_env(wtk_kgr_t *kg);
void wtk_kgr_delete(wtk_kgr_t *r);
void wtk_kgr_reset(wtk_kgr_t *r);
int wtk_kgr_can_be_end(wtk_kgr_t *r);
int wtk_kgr_feed(wtk_kgr_t *r,wtk_act_t *act);
wtk_string_t wtk_kgr_process_nlg(wtk_kgr_t *r,char* nlg,int len);
#ifdef __cplusplus
};
#endif
#endif
