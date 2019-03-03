#ifndef WTK_CORE_WTK_WAVFILE_H_
#define WTK_CORE_WTK_WAVFILE_H_
#include "wtk/core/wavehdr.h"
#include "wtk/core/wtk_os.h"
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_wavfile wtk_wavfile_t;
/*
 * current used for write wave file.
 */

struct wtk_wavfile
{
	WaveHeader hdr;
	FILE *file;
	int writed;
	int max_pend;
	int pending;
};

wtk_wavfile_t* wtk_wavfile_new(int sample_rate);
void wtk_wavfile_set_channel(wtk_wavfile_t *w,int channel);
void wtk_wavfile_set_channel2(wtk_wavfile_t *f,int c,int bytes_per_sample);
int wtk_wavfile_delete(wtk_wavfile_t *f);
void wtk_wavfile_init(wtk_wavfile_t *f,int sample_rate);
void wtk_wavfile_clean(wtk_wavfile_t *f);
int wtk_wavfile_open(wtk_wavfile_t *f,char *fn);
int wtk_wavfile_open2(wtk_wavfile_t *f,char *prev);
int wtk_wavfile_write(wtk_wavfile_t *f,const char *data,int bytes);
int wtk_wavfile_writef(wtk_wavfile_t *f,float *data,int len);
void wtk_wavfile_write_float(wtk_wavfile_t *f,float **data,int len);
int wtk_wavfile_flush(wtk_wavfile_t *f);
int wtk_wavfile_close(wtk_wavfile_t *f);
void wtk_wavfile_write_mc(wtk_wavfile_t *f,short **mic,int len);
void wtk_wavfile_write_int(wtk_wavfile_t *f,int **mic,int len);
int wtk_wavfile_cancel(wtk_wavfile_t *f,int n);
#ifdef __cplusplus
};
#endif
#endif
