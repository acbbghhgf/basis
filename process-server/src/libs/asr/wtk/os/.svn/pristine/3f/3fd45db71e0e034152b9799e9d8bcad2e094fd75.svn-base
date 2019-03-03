#ifndef WTK_OS_DTP_WTK_DTP_CFG_H_
#define WTK_OS_DTP_WTK_DTP_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "wtk/os/wtk_cpu.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_dtp_cfg wtk_dtp_cfg_t;

struct wtk_dtp_cfg
{
	int	min;
	int max;
	int timeout;
};

int wtk_dtp_cfg_init(wtk_dtp_cfg_t *cfg);
int wtk_dtp_cfg_clean(wtk_dtp_cfg_t *cfg);
int wtk_dtp_cfg_update_local(wtk_dtp_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_dtp_cfg_update(wtk_dtp_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
