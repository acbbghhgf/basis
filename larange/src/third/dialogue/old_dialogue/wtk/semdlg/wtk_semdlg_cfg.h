#ifndef WTK_SEMDLG_WTK_SEMDLG_CFG
#define WTK_SEMDLG_WTK_SEMDLG_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk/lex/wtk_lex.h"
#include "wtk/semdlg/semfld/wtk_semfld.h"
#include "wtk/lua/wtk_lua2.h"
#include "wtk/semdlg/faq/wtk_faq.h"
#include "wtk/core/cfg/wtk_main_cfg.h"
#include "wtk/semdlg/semfst/wtk_semfstc.h"
#include "wtk/semdlg/semfst/wtk_semfstr.h"
#include "wtk/core/cfg/wtk_mbin_cfg.h"
#include "wtk/semdlg/owlkg/wtk_owlkg.h"
#include "wtk/semdlg/owlkg/wtk_actkg.h"
#include "wtk/core/strlike/wtk_chnlike.h"
#include "wtk/semdlg/nlpemot/wtk_nlpemot.h"
#include "wtk/semdlg/faq/wtk_faqc.h"
#include "wtk/semdlg/kgkv/wtk_kgkv.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_semdlg_cfg wtk_semdlg_cfg_t;

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_semfld_cfg_t fld;
}wtk_semfld_item_t;

typedef struct
{
	wtk_string_t name;
	wtk_string_t sex;
	int age;
}wtk_semdlg_assist_cfg_t;

struct wtk_semdlg_cfg
{
	wtk_wrdvec_cfg_t wrdvec;
	wtk_chnlike_cfg_t chnlike;
	wtk_faqc_cfg_t faqc;
	wtk_nlpemot_cfg_t nlpemot;
	wtk_lex_cfg_t lex;
	wtk_lua2_cfg_t lua;
	wtk_lua2_cfg_t robot_lua;
	wtk_semfld_cfg_t domain;
	wtk_faq_cfg_t faq;
	wtk_semfstr_cfg_t semfst;
	wtk_kgkv_cfg_t kgkv;
	char *semfst_net_fn;
	wtk_semfst_net_t *semfst_net;
	wtk_queue_t fld_q;
	wtk_string_t failed_output;
	wtk_string_t empty_output;
	wtk_string_t pre_fld;
	wtk_string_t post_fld;
	wtk_string_t bg_fld;
	wtk_string_t interrupt_fld;
	wtk_semdlg_assist_cfg_t assist;
	//额外领域作为使用
	wtk_array_t *else_field;

	char *lua_set_func;
	char *lua_init_func;
	char *lua_process_init_func;
	char *lua_robot_init_func;
	char *lua_robot_msg_func;
	char *lua_feed_json_func;
	char *lua_rec_choose_func;
	char *lua_usr_get;
	char *lua_feed_rec_hint;
	char *lua_on_empty_output;
	char *lua_reset;
	char *lua_syn_semdlg_state;
	char *lua_int_lua_local_get;	//服务器设置序列化局部变量的函数
	char *lua_int_lua_local_set;
	char *brain_dn;
	char *def_dn;
	char *dat_fn;
	int nhistory;
	float grammar_conf_thresh;
	union
	{
		wtk_main_cfg_t *main_cfg;
		wtk_mbin_cfg_t *bin_cfg;
	}cfg;
	wtk_cfg_file_t *cfile;
	wtk_rbin2_t *rbin;
	unsigned debug:1;
	unsigned use_rbin:1;
	unsigned use_chnlike:1;
	unsigned use_wrdvec:1;
	unsigned use_faq:1;
	unsigned use_nlpemot:1;
	unsigned use_faqc:1;
};

int wtk_semdlg_cfg_init(wtk_semdlg_cfg_t *cfg);
int wtk_semdlg_cfg_clean(wtk_semdlg_cfg_t *cfg);
int wtk_semdlg_cfg_update_local(wtk_semdlg_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_semdlg_cfg_update(wtk_semdlg_cfg_t *cfg);
int wtk_semdlg_cfg_update2(wtk_semdlg_cfg_t *cfg,wtk_source_loader_t *sl);

wtk_semdlg_cfg_t* wtk_semdlg_cfg_new(char *fn);
wtk_semdlg_cfg_t* wtk_semdlg_cfg_new_bin(char *fn);
void wtk_semdlg_cfg_delete(wtk_semdlg_cfg_t *cfg);

wtk_semdlg_cfg_t* wtk_semdlg_cfg_new_bin2(char *bin_fn,int seek_pos,wtk_local_cfg_t *custom);
void wtk_semdlg_cfg_delete_bin2(wtk_semdlg_cfg_t *cfg);

#ifdef __cplusplus
};
#endif
#endif
