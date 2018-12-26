#ifndef WTK_HTTP_PLINK_HOST_WTK_PLINK_HOST_CFG_H_
#define WTK_HTTP_PLINK_HOST_WTK_PLINK_HOST_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "wtk/http/misc/httpc/wtk_httpc_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_plink_host_cfg wtk_plink_host_cfg_t;
struct wtk_plink_host_cfg
{
	wtk_httpc_cfg_t httpc;
	int time_relink;
};

int wtk_plink_host_cfg_init(wtk_plink_host_cfg_t *cfg);
int wtk_plink_host_cfg_clean(wtk_plink_host_cfg_t *cfg);
int wtk_plink_host_cfg_update_local(wtk_plink_host_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_plink_host_cfg_update(wtk_plink_host_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
