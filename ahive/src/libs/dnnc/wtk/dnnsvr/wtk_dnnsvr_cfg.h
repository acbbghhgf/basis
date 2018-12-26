#ifndef WTK_DNNSVR_WTK_DNNSVR_CFG
#define WTK_DNNSVR_WTK_DNNSVR_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk/nk/wtk_nk.h"
#include "wtk_dnnupsvr.h"
#include "wtk/core/cfg/wtk_opt.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_dnnsvr_cfg wtk_dnnsvr_cfg_t;
struct wtk_dnnsvr_cfg
{
	wtk_nk_cfg_t nk;
	wtk_dnnupsvr_cfg_t upsvr;
};

int wtk_dnnsvr_cfg_init(wtk_dnnsvr_cfg_t *cfg);
int wtk_dnnsvr_cfg_clean(wtk_dnnsvr_cfg_t *cfg);
int wtk_dnnsvr_cfg_update_local(wtk_dnnsvr_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_dnnsvr_cfg_update(wtk_dnnsvr_cfg_t *cfg);
void wtk_dnnsvr_cfg_update_arg(wtk_dnnsvr_cfg_t *cfg,wtk_arg_t *arg);
void wtk_dnnsvr_cfg_print(wtk_dnnsvr_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
