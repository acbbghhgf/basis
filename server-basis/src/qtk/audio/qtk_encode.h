#ifndef WTK_AUDIO_WTK_ENCODE_H_
#define WTK_AUDIO_WTK_ENCODE_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_stack.h"
#ifdef __cplusplus
extern "C" {
#endif

int mp3_encode(char* pcm,int len,char **out,int *out_len);
int mp3_encode2(short* sample_data,int nsample,int sample_rate,char **out,int *out_len,int bitrate);
#ifdef __cplusplus
};
#endif
#endif
