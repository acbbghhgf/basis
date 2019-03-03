#ifndef WTK_AUDIO_SPEEX_WTK_OGG_DEC_H_
#define WTK_AUDIO_SPEEX_WTK_OGG_DEC_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_strbuf.h"
#include <speex/speex.h>
#include <ogg/ogg.h>
#include <speex/speex_header.h>
#include <speex/speex_stereo.h>
#include <speex/speex_callbacks.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_ogg_dec wtk_ogg_dec_t;
/**
 * simple wrapper speex decoder;
 *
 *	wtk_ogg_dec_t* dec;
 *
 *	dec=wtk_ogg_dec_new();
 *	wtk_ogg_dec_start(dec, ...)
 *	wtk_ogg_dec_feed(dec,...)
 *	wtk_ogg_dec_stop(dec,...)
 *	wtk_ogg_dec_delete(dec);
 */

/**
 *	@brief write data, returned value is not check current;
 */
typedef int (*wtk_ogg_dec_write_f)(void *hook,char *buf,int size);

struct wtk_ogg_dec
{
	ogg_sync_state oy;
	ogg_page og;
	ogg_packet op;
	ogg_stream_state os;
	SpeexBits bits;
	void *st;
	ogg_int64_t page_granule;
	ogg_int64_t last_granule;
	int frame_size;
	int granule_frame_size;
	int nframes;
	int packet_count;
	int rate;
	int channels;
	int extra_headers;
	int lookahead;
	int speex_serialno;
	SpeexStereoState stereo;
	unsigned eos:1;
	unsigned stream_init:1;
	//configure section
	int forceMode;
	float loss_percent;
	unsigned enh_enabled:1;
	wtk_ogg_dec_write_f write_f;
	void* write_hook;
};

/**
 *	@brief create new decoder;
 */
wtk_ogg_dec_t* wtk_ogg_dec_new();

/**
 * @brief delete decoder;
 */
int wtk_ogg_dec_delete(wtk_ogg_dec_t *d);

/**
 * @brief initialize environment for feed, and data is packed to write;
 */
void wtk_ogg_dec_start(wtk_ogg_dec_t *d,wtk_ogg_dec_write_f write,void *hook);

/**
 * @brief feed audio and decoded wav is saved to write hook;
 * @return 0 on success;
 */
int wtk_ogg_dec_feed(wtk_ogg_dec_t *d,char *data,int len);

/**
 * @brief clean decoder;
 */
void wtk_ogg_dec_stop(wtk_ogg_dec_t *d);
#ifdef __cplusplus
};
#endif
#endif
