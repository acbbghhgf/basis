#ifndef WTK_API_SPEECHC_WTK_OGGENC
#define WTK_API_SPEECHC_WTK_OGGENC

#include "speex.h"
#include "wtk/core/wtk_type.h" 
#include "wtk_oggenc_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wtk_oggenc wtk_oggenc_t;

typedef void (*wtk_oggenc_write_f)(void *ths,char *data,int len);

struct wtk_oggenc
{
	wtk_oggenc_cfg_t *cfg;
    const SpeexMode  *spx_mode;
    void       *spx_state;
    SpeexBits   spx_bits;
    SpeexHeader spx_header;
    spx_int32_t spx_frame_size;
    spx_int32_t spx_lookahead;

    spx_int32_t spx_quality;
    spx_int32_t spx_rate;
    spx_int32_t spx_channels;
    spx_int32_t spx_bits_per_sample;
    spx_int32_t spx_frames_per_packet;
    spx_int32_t spx_vbr;
    spx_int32_t spx_complexity;

    int         spx_frame_id;
    int         spx_frame_id_pageout;   /* 4 pageout per second */

    char       *spx_version;
    char        spx_vendor[64];
    char       *spx_comments;
    char        spx_bits_buf[2048];

    spx_int16_t spx_frame_buf[1024];
    int         spx_frame_buf_pos;


    ogg_stream_state og_stream_state;
    ogg_page         og_page;
    ogg_packet       og_packet;

    float            og_pages_per_second;

    void *write_ths;
    wtk_oggenc_write_f write;
};

wtk_oggenc_t* wtk_oggenc_new(wtk_oggenc_cfg_t *cfg);
void wtk_oggenc_delete(wtk_oggenc_t *enc);
void wtk_oggenc_reset(wtk_oggenc_t *enc);
void wtk_oggenc_set_write(wtk_oggenc_t *enc,void *ths,wtk_oggenc_write_f write);
int wtk_oggenc_start(wtk_oggenc_t *enc, int rate, int channels, int bits);
int wtk_oggenc_encode(wtk_oggenc_t *enc, const void *data, int size,int eof);
#ifdef __cplusplus
};
#endif
#endif
