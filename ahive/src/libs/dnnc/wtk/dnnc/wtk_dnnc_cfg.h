#ifndef WTK_FST_WTK_DNNC_CFG
#define WTK_FST_WTK_DNNC_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_dnnc_cfg wtk_dnnc_cfg_t;
struct wtk_dnnc_cfg
{
	char *ip;
	char *port;
	int try_cnt;
	int try_sleep_time;
	int timeout;
	int rw_size;
	unsigned debug:1;
};

int wtk_dnnc_cfg_init(wtk_dnnc_cfg_t *cfg);
int wtk_dnnc_cfg_clean(wtk_dnnc_cfg_t *cfg);
int wtk_dnnc_cfg_update_local(wtk_dnnc_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_dnnc_cfg_update(wtk_dnnc_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
