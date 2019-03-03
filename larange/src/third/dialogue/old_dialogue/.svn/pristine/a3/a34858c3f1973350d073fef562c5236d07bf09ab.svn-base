#ifndef WTK_SEMDLG_SEMFLD_WTK_SEMFLD
#define WTK_SEMDLG_SEMFLD_WTK_SEMFLD
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_larray.h"
#include "wtk_semfld_cfg.h"
#include "wtk/lex/fst/wtk_nlgfst.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_semfld wtk_semfld_t;
struct wtk_semdlg;

struct wtk_semfld
{
	wtk_queue_node_t q_n;
	wtk_queue_node_t hist_n;
	wtk_semfld_cfg_t *cfg;
	wtk_semfi_t *fi;
	wtk_nlgfst_t *fst;
	wtk_nlgfst2_t *fst2;
	wtk_kgr_t *kg;
	wtk_act_t *input_act;
	struct wtk_semdlg *dlg;
	wtk_string_t *output;
	wtk_semslot_t *slot;
	wtk_strbuf_t *ask_slot;
	wtk_strbuf_t *ask_slot_value;
	wtk_strbuf_t *ask_deny_lua_func;
	wtk_strbuf_t *ask_deny_lua_str;
	int ntry;
	unsigned is_finish:1;
	unsigned remember:1;
	unsigned want_add_hist:1;
};

wtk_semfld_t* wtk_semfld_new(wtk_semfld_cfg_t *cfg,struct wtk_semdlg *dlg);
void wtk_semfld_delete(wtk_semfld_t *fld);
void wtk_semfld_reset(wtk_semfld_t *fld);
void wtk_semfld_update_env(wtk_semfld_t *fld);

/**
 * -1, failed
 * 0, success
 * 1, need redomain and process
 */
int wtk_semfld_process(wtk_semfld_t *fld,char *data,int bytes);

wtk_act_t* wtk_semfld_get_input_act(wtk_semfld_t *fld);
void wtk_semfld_set_output(wtk_semfld_t *fld,char *data,int bytes);

int wtk_semfld_set_str(wtk_semfld_t *fld,char *k,int klen,char *v,int vlen,int use_history);
int wtk_semfld_del(wtk_semfld_t *fld,char *k,int klen);
wtk_string_t* wtk_semfld_get(wtk_semfld_t *fld,char *k,int klen);
wtk_string_t wtk_semfld_get_ask_slot(wtk_semfld_t *fld);
wtk_string_t wtk_semfld_get_ask_slot_value(wtk_semfld_t *fld);
void wtk_semfld_set_ask_slot(wtk_semfld_t *fld,char *data,int bytes,char *v,int v_bytes);
void wtk_semfld_set_ask_deny_func(wtk_semfld_t *fld,char *deny_func,int deny_func_bytes,char *deny_str,int deny_str_bytes);
void wtk_semfld_set_end(wtk_semfld_t *fld);
int wtk_semfld_can_be_end(wtk_semfld_t *fld);
void wtk_semfld_touch_end(wtk_semfld_t *fld);
/*
	使用系统slot解析nlg  sys_answer()
	*/
void wtk_semfld_process_nlg(wtk_semfld_t *fld,char *data,int bytes);
/*
	使用参数解析nlg  sys_answer(a="b")
*/
int wtk_semfld_process_nlg2(wtk_semfld_t *fld,char *data,int bytes);
int wtk_semfld_valid_input(wtk_semfld_t *fld,char *data,int bytes);
void wtk_semfld_set_fst_state(wtk_semfld_t *fld,char *state,int state_bytes);
wtk_string_t* wtk_semfld_process_owl(wtk_semfld_t *fld,wtk_act_t *act);

int wtk_semfld_goto_state(wtk_semfld_t *fld,char *nm,int nm_bytes);

typedef struct
{
	union{
		wtk_string_t *str;
		wtk_json_item_t *json;
	}v;
	unsigned is_str:1;
}wtk_semfld_ext_data_t;
int wtk_semfld_feed_lua(wtk_semfld_t *fld,wtk_act_t *act,char *func,wtk_semfld_ext_data_t *ext,wtk_strbuf_t *buf);

int wtk_semfld_process3(wtk_semfld_t *fld,char *data,int bytes);
#ifdef __cplusplus
};
#endif
#endif
