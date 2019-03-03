#ifndef WTK_DNNSVR_WTK_DNNUPSVR_CFG
#define WTK_DNNSVR_WTK_DNNUPSVR_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk_cudnn.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_dnnupsvr_cfg wtk_dnnupsvr_cfg_t;
struct wtk_dnnupsvr_cfg
{
	wtk_cudnn_cfg_t dnn;
	int thread_per_gpu;
	int cache_size;
	unsigned debug:1;
};

int wtk_dnnupsvr_cfg_init(wtk_dnnupsvr_cfg_t *cfg);
int wtk_dnnupsvr_cfg_clean(wtk_dnnupsvr_cfg_t *cfg);
int wtk_dnnupsvr_cfg_update_local(wtk_dnnupsvr_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_dnnupsvr_cfg_update(wtk_dnnupsvr_cfg_t *cfg);
void wtk_dnnupsvr_cfg_print(wtk_dnnupsvr_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
