#ifndef WTK_OS_PS_WTK_PS_CFG_H_
#define WTK_OS_PS_WTK_PS_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_ps_cfg wtk_ps_cfg_t;

struct wtk_ps_cfg
{
	char *cmd_fn;
	int buf_size;
	int timeout;		//ms;
	int select_timeout;
};

int wtk_ps_cfg_init(wtk_ps_cfg_t *cfg);
int wtk_ps_cfg_clean(wtk_ps_cfg_t *cfg);
int wtk_ps_cfg_update_local(wtk_ps_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_ps_cfg_update(wtk_ps_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
