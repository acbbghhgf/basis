#ifndef QTK_AUDIO_QTK_AUDIO_DECODER_H_
#define QTK_AUDIO_QTK_AUDIO_DECODER_H_
#include "qtk/audio/qtk_decode.h"
#include "wtk/core/wtk_strbuf.h"
#include "wtk/os/wtk_sem.h"
#include "wtk/os/wtk_blockqueue.h"
#include "qtk_audio_parser.h"
#include "wtk/core/wtk_hoard.h"
#include "qtk_adevent.h"
#include "qtk_resample2.h"
#include "wtk/os/wtk_thread.h"
#include "qtk_audio_resampler.h"
#include "wtk/core/wtk_audio_type.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct qtk_audio_decoder qtk_audio_decoder_t;

//#define USE_LIBRESAMPLE
//#define DEBUG_RESAMPLE_WAV

struct qtk_audio_decoder
{
	wtk_hoard_t input_hoard;
	wtk_lock_t input_hoard_lock;
	wtk_blockqueue_t input_queue;
	wtk_sem_t start_sem;
	wtk_sem_t output_sem;
	wtk_thread_t decoder_thread;
	qtk_audio_parser_t *parser;
#ifdef USE_LIBRESAMPLE
	wtk_resample2_t *resampler;
#else
	qtk_audio_resampler_t *resampler;
#endif
#ifdef DEBUG_RESAMPLE_WAV
	wtk_strbuf_t *output_wav_buf;
#endif
	wtk_strbuf_t *input_audio;
	wtk_strbuf_t *decode_audio;
	wtk_strbuf_t *resample_audio;
	wtk_strbuf_t *temp_buf; //need opt,used cache data for resampler.

	int channel;
	int sampelrate;
	int samplebytes;

	unsigned is_end:1;
	unsigned need_decode:1;
	unsigned need_resample:1;
	unsigned run:1;
	unsigned is_decode:1;
};

qtk_audio_decoder_t* qtk_audio_decoder_new(int dst_samplerate);
int qtk_audio_decoder_delete(qtk_audio_decoder_t *d);
int qtk_audio_decoder_start(qtk_audio_decoder_t *d,int type,int channel,int samplerate,int samplesize);
int qtk_audio_decoder_feed(qtk_audio_decoder_t *d,char *c,int bytes,int is_end);
wtk_strbuf_t* qtk_audio_decoder_get_audio(qtk_audio_decoder_t *d);
int qtk_audio_decoder_push_adevent(qtk_audio_decoder_t *d,qtk_adevent_t *e);
#ifdef __cplusplus
};
#endif
#endif
