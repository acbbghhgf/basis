#ifndef WTK_AUDIO_WTK_RESAMPLE2_H_
#define WTK_AUDIO_WTK_RESAMPLE2_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_strbuf.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_resample2 wtk_resample2_t;

struct wtk_resample2
{
	float *input;
	float *output;
	int input_size;
	int output_size;
	void *handle;
	double factor;
};

//cache like 4096
wtk_resample2_t* wtk_resample2_new(int cache);
int wtk_resample2_start(wtk_resample2_t *r,int src_rate,int dst_rate);
int wtk_resample2_close(wtk_resample2_t *r);
int wtk_resample2_delete(wtk_resample2_t *r);
int wtk_resample2_process(wtk_resample2_t *r,int is_last,short *data,int len,int *consumed,wtk_strbuf_t *output_buf);
#ifdef __cplusplus
};
#endif
#endif
