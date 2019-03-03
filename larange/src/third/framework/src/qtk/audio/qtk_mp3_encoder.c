#include "qtk_mp3_encoder.h"
#include "lame.h"


int qtk_mp3enc_init(qtk_mp3encoder_t *enc, int bufsize) {
    int ret = 0;
    lame_global_flags *gf;
    gf = lame_init();
    if (NULL == gf) {
        qtk_mp3enc_clean(enc);
        ret = -1;
        goto end;
    }
    lame_set_num_channels(gf, 1);
    lame_set_in_samplerate(gf, 16000);
    lame_set_mode(gf, 3);
    lame_set_quality(gf, 2);
    lame_init_params(gf);
    enc->lame_gf = gf;
    enc->out_buf = wtk_malloc(7200 + bufsize);
    if (NULL == enc->out_buf) {
        qtk_mp3enc_clean(enc);
        ret = -1;
        goto end;
    }
    enc->bufsize = bufsize;
    enc->pos = 0;
end:
    return ret;
}


int qtk_mp3enc_feed(qtk_mp3encoder_t *enc, char *pcm, int len) {
    unsigned char *pout = enc->out_buf + enc->pos;
    int bufsize = enc->bufsize;
    int nLeft = bufsize - enc->pos;
    int samples = 0;
    int ret;

    if (0 == len) {
        ret = lame_encode_flush(enc->lame_gf, pout, nLeft+7200);
        if (ret >= 0) {
            enc->pos += ret;
            if (enc->handler) {
                enc->handler(enc->target, (char*)enc->out_buf, enc->pos, 1);
                enc->pos = 0;
            }
        } else {
            wtk_debug("lame_encode_flush error: %d\r\n", ret);
            ret = -1;
        }
        goto end;
    }

    while (len > 0) {
        samples = min(4096, len);
        ret = lame_encode_buffer(enc->lame_gf, (short*)pcm, (short*)pcm,
                                 samples/2, pout, nLeft+7200);
        if (ret >= 0) {
            pcm += samples;
            len -= samples;
            nLeft -= ret;
            pout += ret;
            enc->pos += ret;
        } else {
            wtk_debug("lame_encode_buffer error: %d\r\n", ret);
            ret = -1;
            goto end;
        }
        if (enc->handler && nLeft <= 0) {
            enc->handler(enc->target, (char*)enc->out_buf, enc->pos, 0);
            enc->pos = 0;
            pout = enc->out_buf;
            nLeft = bufsize;
        }
    }
    ret = 0;
end:
    return 0;
}


int qtk_mp3enc_reset(qtk_mp3encoder_t *enc) {
    int ret = 0;
    lame_global_flags *gf;
    lame_close(enc->lame_gf);
    enc->lame_gf = NULL;
    gf = lame_init();
    if (NULL == gf) {
        qtk_mp3enc_clean(enc);
        ret = -1;
        goto end;
    }
    lame_set_num_channels(gf, 1);
    lame_set_in_samplerate(gf, 16000);
    lame_set_mode(gf, 3);
    lame_set_quality(gf, 2);
    lame_init_params(gf);
    enc->lame_gf = gf;
    enc->pos = 0;
end:
    return ret;
}


int qtk_mp3enc_clean(qtk_mp3encoder_t *enc) {
    if (enc->lame_gf) {
        lame_encode_flush(enc->lame_gf, enc->out_buf, 0);
        lame_close(enc->lame_gf);
        enc->lame_gf = NULL;
    }
    if (enc->out_buf) {
        wtk_free(enc->out_buf);
        enc->out_buf = NULL;
    }
    return 0;
}


int qtk_mp3enc_set_notifier(qtk_mp3encoder_t *enc, qtk_mp3enc_ready_f handler, void* udata) {
    enc->handler = handler;
    enc->target = udata;
    return 0;
}
