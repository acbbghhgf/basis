#ifndef WTK_DNNSVR_WTK_CUDNN
#define WTK_DNNSVR_WTK_CUDNN
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_robin.h"
#include <math.h>
#include "wtk_cudnn_cfg.h"
#include "wtk_cudnn_env.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_cudnn wtk_cudnn_t;

typedef void(*wtk_cudnn_raise_f)(void *ths,wtk_cudnn_t *cudnn,wtk_cudnn_matrix_t *input);

struct wtk_cudnn
{
	wtk_cudnn_cfg_t *cfg;
	wtk_robin_t *robin;
	wtk_cudnn_matrix_t *input;
	wtk_queue_t recv_q;
	int feat_size;
	int input_size;
	int pos;
	int pos2;
	int idx;
	int recv_idx;
	int cnt;
	int calc_cnt;
	float **pv;
	wtk_cudnn_env_t *env;
	wtk_cudnn_raise_f raise;
	void *raise_ths;
	void *hook;
	unsigned is_end:1;
	unsigned want_delete:1;
};

wtk_cudnn_t* wtk_cudnn_new(wtk_cudnn_cfg_t *cfg);
void wtk_cudnn_delete(wtk_cudnn_t *cudnn);
void wtk_cudnn_set_raise(wtk_cudnn_t *dnn,void *ths,wtk_cudnn_raise_f riase);
void wtk_cudnn_reset(wtk_cudnn_t *dnn);
void wtk_cudnn_feed(wtk_cudnn_t *cudnn,float *f,int len,int is_end);
void wtk_cudnn_feed2(wtk_cudnn_t *cudnn,float *f,int len,int is_end);
#ifdef __cplusplus
};
#endif
#endif
