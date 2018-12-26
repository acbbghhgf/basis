#ifndef WTK_SEMDLG_WTK_SEMDLG
#define WTK_SEMDLG_WTK_SEMDLG
#include "wtk_semdlg_cfg.h"
#include "wtk/core/wtk_strbuf.h"
#include "wtk/semdlg/tfidf/wtk_tfidf.h"
#include "wtk/core/json/wtk_json_parse.h"
#include "wtk/core/wtk_jsonkv.h"
#include "wtk/os/wtk_thread2.h"
#include "wtk_semdlg_env.h"
#include "wtk/core/wtk_fkv.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_semdlg wtk_semdlg_t;
#define wtk_semdlg_set_reback_reftxt_s(dlg,s) wtk_semdlg_set_reback_reftxt(dlg,s,sizeof(s)-1)
#define wtk_semdlg_feed_robot_msg_s(dlg,msg) wtk_semdlg_feed_robot_msg(dlg,msg,sizeof(msg)-1)

typedef enum
{
	WTK_SEMDLG_PLAY_FILE,
	WTK_SEMDLG_TTS,
	WTK_SEMDLG_REC_SET_GRAMMAR,
	WTK_SEMDLG_DLG_CMD,
}wtk_semdlg_sys_msg_type_t;

typedef struct
{
	wtk_semdlg_sys_msg_type_t type;
	wtk_string_t v;
	wtk_json_item_t *cmd;
	unsigned syn:1;
}wtk_semdlg_sys_msg_t;

typedef void (*wtk_semdlg_on_msg_f)(void *ths,wtk_semdlg_sys_msg_t *msg);

typedef wtk_string_t (*wtk_semdlg_get_faq_f)(void *ths,char *data,int bytes);
typedef wtk_string_t (*wtk_semdlg_filter_output_f)(void *ths,char *data,int bytes);

struct wtk_semdlg
{
	wtk_semdlg_cfg_t *cfg;
	wtk_semdlg_env_t env;
	wtk_nlpemot_t *nlpemot;
	wtk_chnlike_t *chnlike;
	wtk_jsonkv_t *jsonkv;
	wtk_kgkv_t *kgkv;
	//wtk_fkv_t *fkv;
	wtk_faq_t *faq;
	wtk_faqc_t *faqc;
	wtk_semfstr_t *semfst;
	wtk_wrdvec_t *wrdvec;
	wtk_lexr_t *lexr;
	wtk_lua2_t *lua;
	wtk_sqlite_t *db;
	wtk_json_parser_t *json_parser;
	wtk_json_item_t *ext_item;
	wtk_semfld_t *domain;
	wtk_semfld_t *last_fld;
	wtk_semfld_t *pre_fld;
	wtk_semfld_t *post_fld;
	wtk_semfld_t *bg_fld;
	wtk_semfld_t *interupt_fld;
	wtk_semfld_t *cur_fld;
	wtk_semfld_t *nxt_fld;
	wtk_semfld_t *syn_fld;
	wtk_str_hash_t *fld_hash;
	wtk_strbuf_t *buf;
	wtk_strbuf_t *str_output;
	wtk_heap_t *glb_heap;
	wtk_heap_t *loc_heap;
	wtk_json_t *json;
	wtk_string_t *input;
	wtk_strbuf_t *last_input;
	wtk_strbuf_t *last_output;
	wtk_json_item_t *output;
	wtk_queue_t fld_q;
	wtk_queue_t hist_q;
	float conf;
	int vad_time;
	void *msg_ths;
	wtk_semdlg_on_msg_f msg_func;

	void *faq_get_ths;
	wtk_semdlg_get_faq_f faq_get;
	void *output_filter_ths;
	wtk_semdlg_filter_output_f output_filter;

	wtk_thread2_t *robot_route;
	wtk_lua2_t *robot_lua;
	wtk_strbuf_t *robot_buf;

	wtk_string_t exe_ret;

	wtk_semfld_t *nxt_json_fld;
	wtk_strbuf_t *nxt_json_func;
	//--------------- env section ----------------
	wtk_cfg_file_t *env_parser;

	unsigned int count;
	unsigned playing:1;
	unsigned domained:1;
	unsigned out:1;
	unsigned use_faq:1;
	unsigned keep_fld:1;
	unsigned has_post_audio:1;
};

wtk_semdlg_t* wtk_semdlg_new(wtk_semdlg_cfg_t *cfg);
void wtk_semdlg_delete(wtk_semdlg_t *dlg);
void wtk_semdlg_reset(wtk_semdlg_t *dlg);
void wtk_semdlg_reset_history(wtk_semdlg_t *dlg);
void wtk_semdlg_remove_history(wtk_semdlg_t *dlg,wtk_semfld_t *fld);
void wtk_semdlg_set_env(wtk_semdlg_t *r,char *env,int bytes);
void wtk_semdlg_set_reback_reftxt(wtk_semdlg_t *dlg,char *data,int bytes);
int wtk_semdlg_process(wtk_semdlg_t *dlg,char *data,int bytes);
wtk_string_t wtk_semdlg_process2(wtk_semdlg_t *dlg,char *data,int bytes,int use_json);
void wtk_semdlg_print(wtk_semdlg_t *dlg);

wtk_semfld_t* wtk_semdlg_get_fld(wtk_semdlg_t *dlg,char *data,int bytes);
void wtk_semdlg_set_next_fld(wtk_semdlg_t *dlg,wtk_semfld_t *next_fld);
void wtk_semdlg_set_last_fld(wtk_semdlg_t *dlg,wtk_semfld_t *fld);
void wtk_semdlg_semdlg_quit_fld(wtk_semdlg_t *dlg,wtk_semfld_t *fld);
wtk_string_t wtk_semdlg_get_result(wtk_semdlg_t *dlg);

int wtk_semdlg_process_fld_slot(wtk_semdlg_t *dlg,char *data,int bytes);
void wtk_semdlg_set_ext_json(wtk_semdlg_t *dlg,char *k,int k_len,wtk_json_item_t *item);
void wtk_semdlg_update_lua(wtk_semdlg_t *dlg,char *s,int bytes);

int wtk_semdlg_feed_json(wtk_semdlg_t *dlg,char *data,int bytes);

void wtk_semdlg_set_next_json_handler(wtk_semdlg_t *dlg,wtk_semfld_t *fld,wtk_string_t *f);

void wtk_semdlg_syn(wtk_semdlg_t *dlg,char *v,int v_len,int syn);
void wtk_semdlg_play_file(wtk_semdlg_t *dlg,char *v,int v_len,int syn);
void wtk_semdlg_set_rec_grammar(wtk_semdlg_t *dlg,char *v,int v_len);
void wtk_semdlg_exe(wtk_semdlg_t *dlg,wtk_json_item_t *item);
void wtk_semdlg_skip_session(wtk_semdlg_t *dlg);
void wtk_semdlg_set_bg(wtk_semdlg_t *dlg,int bg);
void wtk_semdlg_set_playing(wtk_semdlg_t *dlg,int talking);
void wtk_semdlg_set_fld(wtk_semdlg_t *dlg,char *fld,int fld_bytes);
void wtk_semdlg_feed_timer(wtk_semdlg_t *dlg,char *func,int func_bytes);
void wtk_semdlg_feed_hint(wtk_semdlg_t *dlg,char *hint,int hint_bytes);
void wtk_semdlg_set_faq_get(wtk_semdlg_t *dlg,void *ths,wtk_semdlg_get_faq_f faq_get);
void wtk_semdlg_set_output_filter(wtk_semdlg_t *dlg,void *ths,wtk_semdlg_filter_output_f output_filter);
int wtk_semdlg_feed_talking_json(wtk_semdlg_t *dlg,char *data,int bytes);

void wtk_semdlg_feed_robot_msg(wtk_semdlg_t *dlg,char *msg,int msg_len);
void wtk_semdlg_feed_robot_msg2(wtk_semdlg_t *dlg,char *msg,int msg_len,char* v,int v_len);
wtk_string_t wtk_semdlg_flush(wtk_semdlg_t *dlg,wtk_string_t *fld,wtk_string_t *v);
void wtk_semdlg_set_random(wtk_semdlg_t *dlg,int random);
wtk_string_t wtk_semdlg_get_usr(wtk_semdlg_t *dlg);

wtk_string_t wtk_semdlg_get_choose_rec(wtk_semdlg_t *dlg,char *json,int len);
int wtk_semdlg_is_bg(wtk_semdlg_t *dlg);
void wtk_semdlg_set_output(wtk_semdlg_t *dlg,char *data,int len);
void wtk_semdlg_set_lua_dat(wtk_semdlg_t *dlg,char *k,char *v);

//sys 得到所有的引擎中的中间状态
int qtk_semdlg_get_inter_state(wtk_semdlg_t *dlg, wtk_strbuf_t* buf);
//sys 设置所有的中间状态
int qtk_semdlg_set_inter_state(wtk_semdlg_t *dlg, char *str, int len);
//sys 得到lua的状态
int qtk_lua_local_get(wtk_semdlg_t *semdlg, wtk_strbuf_t *buf);
//sys 设置lua的领域状态
int qtk_lua_local_set(wtk_semdlg_t *semdlg, const char *str, int len);
#ifdef __cplusplus
};
#endif
#endif
