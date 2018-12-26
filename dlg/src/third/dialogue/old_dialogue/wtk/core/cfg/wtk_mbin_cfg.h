#ifndef WTK_CORE_CFG_WTK_MBIN_CFG_H_
#define WTK_CORE_CFG_WTK_MBIN_CFG_H_
#include "wtk/core/cfg/wtk_main_cfg.h"
#include "wtk/core/rbin/wtk_rbin2.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_mbin_cfg wtk_mbin_cfg_t;
#define wtk_mbin_cfg_new_type(type,fn,cfg_fn) wtk_mbin_cfg_new(sizeof(CAT(type,_t)), \
		(wtk_main_cfg_init_f)CAT(type,_init),\
		(wtk_main_cfg_clean_f)CAT(type,_clean),\
		(wtk_main_cfg_update_local_f)CAT(type,_update_local), \
		(wtk_main_cfg_update2_f)CAT(type,_update2),fn,cfg_fn,NULL)
#define wtk_mbin_cfg_new_type3(type,fn,cfg_fn,lc) wtk_mbin_cfg_new(sizeof(CAT(type,_t)), \
		(wtk_main_cfg_init_f)CAT(type,_init),\
		(wtk_main_cfg_clean_f)CAT(type,_clean),\
		(wtk_main_cfg_update_local_f)CAT(type,_update_local), \
		(wtk_main_cfg_update2_f)CAT(type,_update2),fn,cfg_fn,lc)
#define wtk_mbin_cfg_new_type2(seek_pos,type,fn,cfg_fn) wtk_mbin_cfg_new2(seek_pos,sizeof(CAT(type,_t)), \
		(wtk_main_cfg_init_f)CAT(type,_init),\
		(wtk_main_cfg_clean_f)CAT(type,_clean),\
		(wtk_main_cfg_update_local_f)CAT(type,_update_local), \
		(wtk_main_cfg_update2_f)CAT(type,_update2),fn,cfg_fn)

struct wtk_mbin_cfg
{
	void *cfg;
	wtk_cfg_file_t *cfile;
	wtk_rbin2_t *rbin;
	wtk_main_cfg_init_f init;
	wtk_main_cfg_clean_f clean;
	wtk_main_cfg_update_local_f update_lc;
	wtk_main_cfg_update2_f update;
	wtk_source_loader_t sl;
};


wtk_mbin_cfg_t* wtk_mbin_cfg_new(int cfg_bytes,wtk_main_cfg_init_f init,
		wtk_main_cfg_clean_f clean,wtk_main_cfg_update_local_f update_lc,
		wtk_main_cfg_update2_f update,char *bin_fn,char *cfg_fn,wtk_local_cfg_t *custom_lc);
wtk_mbin_cfg_t* wtk_mbin_cfg_new2(int seek_pos,int cfg_bytes,wtk_main_cfg_init_f init,
		wtk_main_cfg_clean_f clean,wtk_main_cfg_update_local_f update_lc,
		wtk_main_cfg_update2_f update,char *bin_fn,char *cfg_fn);
void wtk_mbin_cfg_delete(wtk_mbin_cfg_t *cfg);

wtk_main_cfg_t *wtk_main_cfg_new_bin(int cfg_bytes,wtk_main_cfg_init_f init,
		wtk_main_cfg_clean_f clean,wtk_main_cfg_update_local_f update_lc,
		wtk_main_cfg_update2_f update,char *bin_fn,char *cfg_fn);
#ifdef __cplusplus
};
#endif
#endif
