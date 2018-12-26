#ifndef WTK_SEMDLG_SEMFST_WTK_SEMFSTR
#define WTK_SEMDLG_SEMFST_WTK_SEMFSTR
#include "wtk/core/wtk_type.h" 
#include "wtk_semfstr_cfg.h"
#include "wtk/lex/wtk_lex.h"
#include "wtk/core/wtk_queue3.h"
#include "wtk/core/wtk_str_parser.h"
#include "wtk/semdlg/semfld/wtk_act_lua.h"
#include "wtk/core/wtk_jsonkv.h"
#include "wtk_semfst_net.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_semfstr wtk_semfstr_t;


struct wtk_semfstr
{
	wtk_semfstr_cfg_t *cfg;
	wtk_heap_t *heap;
	wtk_strbuf_t *buf;

	wtk_semfst_net_t *net;
	wtk_lexr_t *lexr;
	wtk_lua2_t *lua;

	wtk_strbuf_t *result;
	wtk_strbuf_t *ans;
	wtk_jsonkv_t *jsonkv;
	wtk_semfst_script_t *nxt_script;

	wtk_json_item_t *act_req;
	void *lua_hook;
};

wtk_semfstr_t* wtk_semfstr_new(wtk_semfstr_cfg_t *cfg,wtk_semfst_net_t *net,wtk_lexr_t *lexr,wtk_lua2_t *lua);
void wtk_semfstr_delete(wtk_semfstr_t *r);
void wtk_semfstr_reset(wtk_semfstr_t *r);
wtk_string_t wtk_semfstr_process(wtk_semfstr_t *r,char *data,int bytes);
#ifdef __cplusplus
};
#endif
#endif
