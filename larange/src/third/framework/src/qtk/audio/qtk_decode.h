#ifndef WTK_AUDIO_WTK_DECODE_H_
#define WTK_AUDIO_WTK_DECODE_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_stack.h"
#include "wtk/core/wavehdr.h"
#include "wtk/os/wtk_lock.h"
#include "wtk/os/wtk_log.h"
#include "wtk/core/wtk_audio_type.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_audio_decode wtk_audio_decode_t;

struct wtk_audio_decode
{
	void *io_ctx;
	int input_size;
	int output_size;
	char *input;
	char *output_alloc;
	short *output;
	wtk_lock_t l;
	wtk_stack_t *stack;
};

wtk_audio_decode_t* wtk_audio_decode_new();
int wtk_audio_decode_delete(wtk_audio_decode_t *d);
int wtk_audio_decode(wtk_audio_decode_t* d,char *buf,int size,char* audio_type,wtk_stack_t *s,wtk_log_t *log);
int wtk_audio_resample(wtk_audio_decode_t *d,char *buf,int len,wtk_stack_t *s);
int wtk_audio_translate(wtk_audio_decode_t *d,char* buf,int len,int audo_type,char** ouput,int *ouput_len,wtk_log_t *log);
#ifdef __cplusplus
};
#endif
#endif
