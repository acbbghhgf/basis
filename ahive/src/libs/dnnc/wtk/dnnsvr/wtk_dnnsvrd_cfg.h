#ifndef WTK_DNNSVR_WTK_DNNSVRD_CFG
#define WTK_DNNSVR_WTK_DNNSVRD_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk/core/cfg/wtk_opt.h"
#include "wtk/os/daemon/wtk_daemon.h"
#include "wtk_dnnsvr.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_dnnsvrd_cfg wtk_dnnsvrd_cfg_t;
struct wtk_dnnsvrd_cfg
{
	wtk_daemon_cfg_t daemon;
	wtk_dnnsvr_cfg_t dnnsvr;
};

int wtk_dnnsvrd_cfg_init(wtk_dnnsvrd_cfg_t *cfg);
int wtk_dnnsvrd_cfg_clean(wtk_dnnsvrd_cfg_t *cfg);
int wtk_dnnsvrd_cfg_update_local(wtk_dnnsvrd_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_dnnsvrd_cfg_update(wtk_dnnsvrd_cfg_t *cfg);
void wtk_dnnsvrd_cfg_update_arg(wtk_dnnsvrd_cfg_t *cfg,wtk_arg_t *arg);
#ifdef __cplusplus
};
#endif
#endif
