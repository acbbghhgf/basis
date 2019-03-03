#ifndef QTK_AUDIO_QTK_AUDIO_RESAMPLER_H_
#define QTK_AUDIO_QTK_AUDIO_RESAMPLER_H_
#include "qtk/audio/qtk_decode.h"
#include "wtk/core/wtk_strbuf.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct qtk_audio_resampler qtk_audio_resampler_t;
typedef int (*qtk_audio_resampler_write_handler_t)(void *hook,char *buf,int buf_size);

struct qtk_audio_resampler
{
	void *ctx;
	int output_size;
	char *output_alloc;
	short *output;
	void *hook;
	qtk_audio_resampler_write_handler_t write;
	int dst_channel;
	int dst_samplerate;
	int dst_samplesize;

	int src_channel;
	int src_samplerate;
	int src_samplesize;

	int input_size;  //input buffer hint
	unsigned need_resample:1;
};

qtk_audio_resampler_t* qtk_audio_resampler_new(int dst_samplerate,void *hook,qtk_audio_resampler_write_handler_t write);
int qtk_audio_resampler_delete(qtk_audio_resampler_t *r);
int qtk_audio_resampler_start(qtk_audio_resampler_t *r,int channel,int samplerate,int samplesize);
int qtk_audio_resampler_stop(qtk_audio_resampler_t *r);
int qtk_audio_resampler_feed(qtk_audio_resampler_t *r,char *buf,int bytes);
int qtk_audio_resampler_run2(qtk_audio_resampler_t *r,int is_last,short *data,int len,int *pconsumed,wtk_strbuf_t *buf);
int qtk_audio_resampler_run3(qtk_audio_resampler_t *r,int is_last,short *data,int len,int *pconsumed,wtk_strbuf_t *buf);
#ifdef __cplusplus
};
#endif
#endif
