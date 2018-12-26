#include <stdio.h>
#include "json/cJSON.h"
#include "wtk/core/param/wtk_module_param.h"
#include "qtk/service/qtk_aisrv_module.h"
#include "wtk/core/cfg/wtk_main_cfg.h"
#include "qtk/audio/qtk_audio_decoder.h"
#include "wtk/edu/eval/engsnt/wtk_engsnt_cfg.h"
#include "wtk/edu/eval/engsnt/wtk_engsnt.h"
#include "qtk/core/qtk_network.h"
#include "qtk/util/qtk_verify.h"


const static int default_dst_sample_rate = 16000;


typedef struct eval_cfg eval_cfg_t;
typedef struct eval_instance eval_instance_t;

struct eval_cfg {
    wtk_engsnt_cfg_t *cfg;
    int dst_sample_rate;
};


struct eval_instance {
    eval_cfg_t *cfg;
    wtk_audio_tag_t audio_tag;
    qtk_audio_decoder_t *decoder;
    wtk_engsnt_t *snt;
    wtk_strbuf_t *resbuf;
    int errno;
    unsigned stop:1;
};


static void eval_reset_audio_tag(wtk_audio_tag_t *audio_tag) {
    audio_tag->type = AUDIO_WAV;
    audio_tag->channel = 1;
    audio_tag->samplerate = default_dst_sample_rate;
    audio_tag->samplesize = 2;
}


static void eval_set_audio_tag(wtk_audio_tag_t *audio_tag, cJSON *jaudio) {
    cJSON *obj;
    for (obj = jaudio->child; obj; obj = obj->next) {
        if (0 == strcmp(obj->string, "audioType")) {
            if (cJSON_String == obj->type && !strcmp(obj->valuestring, "ogg")) {
                audio_tag->type = AUDIO_OGG;
            }
        } else if (0 == strcmp(obj->string, "sampleRate")) {
            if (cJSON_Number == obj->type) {
                audio_tag->samplerate = obj->valueint;
            }
        } else if (0 == strcmp(obj->string, "sampleBytes")) {
            if (cJSON_Number == obj->type) {
                audio_tag->samplesize = obj->valueint;
            }
        } else if (0 == strcmp(obj->string, "channel")) {
            if (cJSON_Number == obj->type) {
                audio_tag->channel = obj->valueint;
            }
        }
    }
}


static void eval_audio_decoder_start(eval_instance_t *asr) {
    if (NULL == asr->decoder) {
        asr->decoder = qtk_audio_decoder_new(asr->cfg->dst_sample_rate);
    }
    qtk_audio_decoder_start(asr->decoder,
                            asr->audio_tag.type,
                            asr->audio_tag.channel,
                            asr->audio_tag.samplerate,
                            asr->audio_tag.samplesize);
}

static int eval_audio_preprocess(eval_instance_t *asr, wtk_string_t *src, wtk_string_t *dst) {
    qtk_audio_decoder_feed(asr->decoder, src->data, src->len, 0);
    wtk_string_set(dst, asr->decoder->resample_audio->data, asr->decoder->resample_audio->pos);
    return 0;
}


static void eval_audio_decoder_stop(eval_instance_t *asr) {
    qtk_audio_decoder_feed(asr->decoder, 0, 0, 1);
}

static void* eval_init(wtk_local_cfg_t *cfg) {
    wtk_string_t *v;
    eval_cfg_t *ecfg = NULL;
    v = wtk_local_cfg_find_string_s(cfg,"cfg");
    if (!v) { goto end; }
    ecfg = wtk_malloc(sizeof(*ecfg));
    ecfg->cfg = wtk_engsnt_cfg_new_bin3(v->data, 0);
    ecfg->dst_sample_rate = default_dst_sample_rate;
    wtk_local_cfg_update_cfg_i(cfg, ecfg, dst_sample_rate, v);
end:
    return ecfg;
}


static void eval_clean(void* inst) {
    eval_cfg_t *ecfg = (eval_cfg_t*)inst;
    if (ecfg->cfg) {
        wtk_engsnt_cfg_delete_bin3(ecfg->cfg);
    }
    wtk_free(inst);
}


static void* eval_create(void* inst) {
    eval_cfg_t *ecfg = (eval_cfg_t*)inst;
    eval_instance_t *eval = wtk_malloc(sizeof(*eval));
    eval->cfg = ecfg;
    eval->snt = wtk_engsnt_new(ecfg->cfg);
    eval->decoder = NULL;
    eval->resbuf = wtk_strbuf_new(1000, 1);
    eval->stop = 0;
    return eval;
}


static int eval_release(void* inst) {
    eval_instance_t *eval = (eval_instance_t*)inst;
    if (eval->snt) {
        wtk_engsnt_delete(eval->snt);
    }
    if (eval->decoder) {
        qtk_audio_decoder_delete(eval->decoder);
    }
    if (eval->resbuf) {
        wtk_strbuf_delete(eval->resbuf);
    }
    wtk_free(eval);
    return 0;
}


static int eval_start(void* inst, wtk_string_t *param) {
    eval_instance_t *eval = (eval_instance_t*)inst;
    wtk_strbuf_t *buf = wtk_strbuf_new(1024, 1);
    wtk_string_t text = {NULL, 0};
    cJSON *jp = NULL, *obj;

    eval->stop = 0;
    eval_reset_audio_tag(&eval->audio_tag);
    wtk_strbuf_push(buf, param->data, param->len);
    wtk_strbuf_push_c(buf, '\0');
    jp = cJSON_Parse(buf->data);
    if (NULL == jp) goto end;
    obj = cJSON_GetObjectItem(jp, "audio");
    if (obj) {
        eval_set_audio_tag(&eval->audio_tag, obj);
    }
    obj = cJSON_GetObjectItem(jp, "text");
    if (obj && cJSON_String == obj->type) {
        wtk_string_set(&text, obj->valuestring, strlen(obj->valuestring));
    }
end:
    if (wtk_audio_tag_need_pre(&eval->audio_tag, eval->cfg->dst_sample_rate)) {
        eval_audio_decoder_start(eval);
    }
    if (text.len) {
        wtk_engsnt_reset(eval->snt);
        if (wtk_engsnt_start(eval->snt, WTK_REFTXT_NORMAL, text.data, text.len)) {
            eval->errno = QTK_AISRV_START_ERROR;
            wtk_strbuf_reset(eval->resbuf);
            wtk_strbuf_push_s(eval->resbuf, "eval start failed");
        } else {
            eval->errno = 0;
        }
    } else {
        eval->errno = QTK_AISRV_INVALID_PARAM;
        wtk_strbuf_reset(eval->resbuf);
        wtk_strbuf_push_s(eval->resbuf, "missing text or text is empty string");
    }
    if (jp) {
        cJSON_Delete(jp);
    }
    wtk_strbuf_delete(buf);
    return 0;
}


static int eval_feed(void* inst, wtk_string_t *data) {
    eval_instance_t *eval = (eval_instance_t*)inst;
    wtk_string_t wav;

    if (eval->errno) return eval->errno;

    if (wtk_audio_tag_need_pre(&eval->audio_tag, eval->cfg->dst_sample_rate)) {
        eval_audio_preprocess(eval, data, &wav);
    } else {
        wtk_string_set(&wav, data->data, data->len);
    }
    if (wtk_engsnt_feed(eval->snt, WTK_PARM_APPEND, wav.data, wav.len)) {
        eval->errno = QTK_AISRV_FEED_ERROR;
        return -1;
    } else {
        return 0;
    }
}


static int eval_stop(void* inst) {
    eval_instance_t *eval = (eval_instance_t*)inst;
    eval->stop = 1;
    if (eval->errno) return eval->errno;
    if (wtk_audio_tag_need_pre(&eval->audio_tag, eval->cfg->dst_sample_rate)) {
        eval_audio_decoder_stop(eval);
    }
    if (wtk_engsnt_feed(eval->snt, WTK_PARM_END, 0, 0)) {
        eval->errno = QTK_AISRV_STOP_ERROR;
        return -1;
    } else {
        return 0;
    }
}


static int eval_get_result(void* inst, wtk_string_t *result) {
    eval_instance_t *eval = (eval_instance_t*)inst;
    char *data = NULL;
    int len;
    int ret = QTK_AISRV_SUCCESS_EOF;
    if (eval->errno) {
        wtk_string_set(result, eval->resbuf->data, eval->resbuf->pos);
        return eval->errno;
    }
    if (eval->stop) {
        wtk_engsnt_get_jsonresult(eval->snt, 100, &data, &len);
        if (data) {
            wtk_strbuf_reset(eval->resbuf);
            wtk_strbuf_push(eval->resbuf, data, len);
            wtk_free(data);
            wtk_string_set(result, eval->resbuf->data, eval->resbuf->pos);
        }
    } else {
        ret = QTK_AISRV_EMPTY_RESULT;
    }
    return ret;
}


int eval_open(qtk_aisrv_mod_t *mod) {
    const char* host = qtk_aisrv_get_auth_host();
    const char* url = qtk_aisrv_get_auth_url();
    const char* srv_type = "eval";
    int port = qtk_aisrv_get_auth_port();
    struct in_addr addr;
    char ip[64];
    if (qtk_get_ip_by_name(host, &addr)) {
        wtk_debug("cannot resolve domain '%s'\r\n", host);
        return -1;
    }
    if (NULL == inet_ntop(AF_INET, &addr, ip, sizeof(ip))) {
        perror("inet_ntop");
        return -1;
    }
    if (qtk_hw_verify(ip, port, host, url, srv_type)) {
        wtk_debug("service '%s' auth failed\r\n", srv_type);
        return -1;
    }
    mod->init = eval_init;
    mod->clean = eval_clean;
    mod->create = eval_create;
    mod->release = eval_release;
    mod->start = eval_start;
    mod->stop = eval_stop;
    mod->feed = eval_feed;
    mod->get_result = eval_get_result;
    return 0;
}
