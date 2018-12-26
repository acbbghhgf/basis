#ifndef WTK_DNNSVR_WTK_CUDNN_CFG
#define WTK_DNNSVR_WTK_CUDNN_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk/core/cfg/wtk_source.h"
#include "wtk/core/wtk_larray.h"
#include "wtk/core/math/wtk_math.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_cudnn_cfg wtk_cudnn_cfg_t;

/**
1. trans file:  (x+bias)*window    |1*1320|*|1320*1320|
{{{
<bias> 1320 1320
v 1320
...
<window> 1320 1320
v 1320
...
}}}

2. net file:  wx+b
{{{
<biasedlinearity> 256 880
m 256 880
--
v 256
<sigmoid> 256 256
<biasedlinearity> 256 256
m 256 256
--
v 256
<sigmoid> 256 256
<biasedlinearity> 2 256
m 2 256
v 2
<softmax> 2 2
}}}

11*80=880
|1*880|
   |   (x+bias)*window
|1*880|*|1*880|+bias*window  //对应的行相乘
   |
|1*880|
   |  wx+b
|1*880|*|880*256|+|1*256|
   |
|1*256|
   | ..

 */
typedef struct
{
	int len;
	float *v;
}wtk_cudnn_vector_t;

typedef struct
{
	int row;
	int col;
	float *v;
}wtk_cudnn_matrix_t;

typedef struct
{
	wtk_cudnn_vector_t *b;
	wtk_cudnn_vector_t *w;
}wtk_cudnn_trans_t;

typedef enum
{
	wtk_dnn_sigmoid=0,
	wtk_dnn_softmax,
	wtk_dnn_linear,
}wtk_dnn_post_type_t;


typedef struct
{
	wtk_dnn_post_type_t type;
	wtk_cudnn_matrix_t *w;   //wx+b;
	wtk_cudnn_vector_t *b;
}wtk_cudnn_layer_t;


typedef struct
{
	int idx;
	int max_thread;
	int nlayer;
	wtk_cudnn_trans_t *trans;
	wtk_cudnn_layer_t **layer;
}wtk_cudnn_gpu_cfg_t;


struct wtk_cudnn_cfg
{
	wtk_cudnn_gpu_cfg_t **gpu;
	int ngpu;
	int cache_size;
	int win;
	char *net_fn;
	char *trans_fn;
	int blk_size;
	int nlayer;
	int skip_frame;
	wtk_cudnn_trans_t *trans;
	wtk_cudnn_layer_t **layer;
	unsigned use_cuda:1;
	unsigned use_single:1;
	unsigned use_linear_output:1;
	unsigned debug:1;
};

int wtk_cudnn_cfg_init(wtk_cudnn_cfg_t *cfg);
int wtk_cudnn_cfg_clean(wtk_cudnn_cfg_t *cfg);
int wtk_cudnn_cfg_update_local(wtk_cudnn_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_cudnn_cfg_update(wtk_cudnn_cfg_t *cfg);
int wtk_cudnn_cfg_update_cuda(wtk_cudnn_cfg_t *cfg);

wtk_cudnn_matrix_t* wtk_cudnn_matrix_new(int row,int col);
void wtk_cudnn_matrix_delete(wtk_cudnn_matrix_t *m);

void wtk_cudnn_layer_delete_cuda(wtk_cudnn_layer_t *layer);
void wtk_cudnn_trans_delete_cuda(wtk_cudnn_trans_t *v);
void wtk_cudnn_vector_delete_cuda(wtk_cudnn_vector_t *v);
void wtk_cudnn_matrix_delete_cuda(wtk_cudnn_matrix_t *m);

wtk_cudnn_matrix_t* wtk_cudnn_matrix_new_cuda(int row,int col);

void wtk_cudnn_test_cuda();
wtk_cudnn_gpu_cfg_t* wtk_cudnn_gpu_cfg_new(int idx);
void wtk_cudnn_gpu_cfg_delete(wtk_cudnn_gpu_cfg_t *cfg);
void wtk_cudnn_trans_delete(wtk_cudnn_trans_t *v);
void wtk_cudnn_layer_delete(wtk_cudnn_layer_t *layer);

void wtk_cudnn_cfg_print(wtk_cudnn_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
