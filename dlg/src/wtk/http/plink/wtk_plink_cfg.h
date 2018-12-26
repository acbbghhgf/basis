#ifndef WTK_HTTP_PLINK_WTK_PLINK_CFG_H_
#define WTK_HTTP_PLINK_WTK_PLINK_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "wtk/http/plink/host/wtk_plink_host.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_plink_cfg wtk_plink_cfg_t;
struct wtk_plink_cfg
{
	wtk_plink_host_cfg_t *links;
	int nlink;
	unsigned used:1;		//plink keep single instance;
};

int wtk_plink_cfg_init(wtk_plink_cfg_t *cfg);
int wtk_plink_cfg_clean(wtk_plink_cfg_t *cfg);
int wtk_plink_cfg_update_local(wtk_plink_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_plink_cfg_update(wtk_plink_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
