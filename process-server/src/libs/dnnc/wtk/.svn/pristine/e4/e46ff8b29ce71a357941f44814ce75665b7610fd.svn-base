#ifndef WTK_DNNSVR_WTK_CUDNN_ENV
#define WTK_DNNSVR_WTK_CUDNN_ENV
#include "wtk/core/wtk_type.h" 
#include "wtk_cudnn_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_cudnn_env wtk_cudnn_env_t;
struct wtk_cudnn_env
{
	wtk_cudnn_cfg_t *cu_cfg;
	wtk_cudnn_gpu_cfg_t *gpu_cfg;
	wtk_cudnn_matrix_t *input;
	wtk_cudnn_matrix_t **output;
	wtk_cudnn_matrix_t *host_output;
};

wtk_cudnn_env_t* wtk_cudnn_env_new(wtk_cudnn_cfg_t *cfg,wtk_cudnn_gpu_cfg_t *gpu_cfg);
void wtk_cudnn_env_delete(wtk_cudnn_env_t *env);
int wtk_cudnn_env_process(wtk_cudnn_env_t *env,wtk_cudnn_matrix_t *input);
int wtk_cudnn_env_process_cpu(wtk_cudnn_env_t *env,wtk_cudnn_matrix_t *input);
#ifdef __cplusplus
};
#endif
#endif
