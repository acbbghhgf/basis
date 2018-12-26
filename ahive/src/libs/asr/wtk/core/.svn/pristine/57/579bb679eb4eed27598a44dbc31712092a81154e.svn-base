#ifndef WTK_CORE_CFG_WTK_OPT_H_
#define WTK_CORE_CFG_WTK_OPT_H_
#include "wtk/core/cfg/wtk_main_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_opt wtk_opt_t;
#define wtk_opt_new_type(argc,argv,type) wtk_opt_new(argc,argv,sizeof(CAT(type,_t)), \
		(wtk_main_cfg_init_f)CAT(type,_init),\
		(wtk_main_cfg_clean_f)CAT(type,_clean),\
		(wtk_main_cfg_update_local_f)CAT(type,_update_local), \
		(wtk_main_cfg_update_f)CAT(type,_update))

struct wtk_opt
{
	wtk_arg_t *arg;
	wtk_main_cfg_t *main_cfg;
	char *cfg_fn;
	char *input_fn;
	char *input_s;
	char *output_fn;
	FILE *output;
	FILE *log;
};


wtk_opt_t* wtk_opt_new(int argc,char **argv,int cfg_bytes,
		wtk_main_cfg_init_f init,
		wtk_main_cfg_clean_f clean,
		wtk_main_cfg_update_local_f update_lc,
		wtk_main_cfg_update_f update
		);
void wtk_opt_delete(wtk_opt_t *opt);
void wtk_opt_print_usage();
#ifdef __cplusplus
};
#endif
#endif
