#include <stdio.h>
#include "qtk/core/cJSON.h"
#include "wtk/core/param/wtk_module_param.h"
#include "qtk/service/qtk_aisrv_module.h"
#include "wtk/core/cfg/wtk_main_cfg.h"
#include "qtk/audio/qtk_audio_decoder.h"
#include "qtk_ifly.h"


const static int default_dst_sample_rate = 16000;


typedef struct ifly_cfg ifly_cfg_t;
typedef struct ifly_instance ifly_instance_t;

struct ifly_cfg {
    wtk_main_cfg_t *cfg;
    int dst_sample_rate;
};


struct ifly_instance {
    ifly_cfg_t *cfg;
    wtk_audio_tag_t audio_tag;
    qtk_audio_decoder_t *decoder;
    qtk_ifly_t *dec;
    unsigned stop:1;
};


static void ifly_reset_audio_tag(wtk_audio_tag_t *audio_tag) {
    audio_tag->type = AUDIO_WAV;
    audio_tag->channel = 1;
    audio_tag->samplerate = default_dst_sample_rate;
    audio_tag->samplesize = 2;
}


static void ifly_set_audio_tag(wtk_audio_tag_t *audio_tag, cJSON *jaudio) {
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


static void ifly_audio_decoder_start(ifly_instance_t *asr) {
    if (NULL == asr->decoder) {
        asr->decoder = qtk_audio_decoder_new(asr->cfg->dst_sample_rate);
    }
    qtk_audio_decoder_start(asr->decoder,
                            asr->audio_tag.type,
                            asr->audio_tag.channel,
                            asr->audio_tag.samplerate,
                            asr->audio_tag.samplesize);
}

static int ifly_audio_preprocess(ifly_instance_t *asr, wtk_string_t *src, wtk_string_t *dst) {
    qtk_audio_decoder_feed(asr->decoder, src->data, src->len, 0);
    wtk_string_set(dst, asr->decoder->resample_audio->data, asr->decoder->resample_audio->pos);
    return 0;
}


static void ifly_audio_decoder_stop(ifly_instance_t *asr) {
    qtk_audio_decoder_feed(asr->decoder, 0, 0, 1);
}

void* ifly_init(wtk_local_cfg_t *cfg) {
    wtk_string_t *v;
    ifly_cfg_t *acfg = NULL;
    v = wtk_local_cfg_find_string_s(cfg,"cfg");
    if (!v) { goto end; }
    acfg = wtk_malloc(sizeof(*acfg));
    acfg->cfg = wtk_main_cfg_new_type(qtk_ifly_cfg, v->data);
    acfg->dst_sample_rate = default_dst_sample_rate;
    wtk_local_cfg_update_cfg_i(cfg, acfg, dst_sample_rate, v);
end:
    return acfg;
}


static void ifly_clean(void* inst) {
    ifly_cfg_t *acfg = (ifly_cfg_t*)inst;
    if (acfg->cfg) {
        wtk_main_cfg_delete(acfg->cfg);
    }
    wtk_free(inst);
}


static void* ifly_create(void* inst) {
    ifly_cfg_t *acfg = (ifly_cfg_t*)inst;
    ifly_instance_t *asr = wtk_malloc(sizeof(*asr));
    asr->stop = 0;
    asr->cfg = acfg;
    asr->dec = qtk_ifly_new(acfg->cfg->cfg);
    asr->decoder = NULL;
    return asr;
}


static int ifly_release(void* inst) {
    ifly_instance_t *asr = (ifly_instance_t*)inst;
    if (asr->dec) {
        qtk_ifly_delete(asr->dec);
    }
    if (asr->decoder) {
        qtk_audio_decoder_delete(asr->decoder);
    }
    wtk_free(asr);
    return 0;
}


static int ifly_start(void* inst, wtk_string_t *param) {
    ifly_instance_t *asr = (ifly_instance_t*)inst;
    wtk_strbuf_t *buf = wtk_strbuf_new(1024, 1);
    wtk_string_t env = {NULL, 0};
    cJSON *jp = NULL, *obj;

    asr->stop = 0;
    ifly_reset_audio_tag(&asr->audio_tag);
    wtk_strbuf_push(buf, param->data, param->len);
    wtk_strbuf_push_c(buf, '\0');
    jp = cJSON_Parse(buf->data);
    if (NULL == jp) goto end;
    obj = cJSON_GetObjectItem(jp, "audio");
    if (obj) {
        ifly_set_audio_tag(&asr->audio_tag, obj);
    }
    obj = cJSON_GetObjectItem(jp, "env");
    if (obj && cJSON_String == obj->type) {
        wtk_string_set(&env, obj->valuestring, strlen(obj->valuestring));
    }
end:
    if (wtk_audio_tag_need_pre(&asr->audio_tag, asr->cfg->dst_sample_rate)) {
        ifly_audio_decoder_start(asr);
    }
    qtk_ifly_start(asr->dec);
    if (jp) {
        cJSON_Delete(jp);
    }
    wtk_strbuf_delete(buf);
    return 0;
}


static int ifly_feed(void* inst, wtk_string_t *data) {
    ifly_instance_t *asr = (ifly_instance_t*)inst;
    wtk_string_t wav;
    if (wtk_audio_tag_need_pre(&asr->audio_tag, asr->cfg->dst_sample_rate)) {
        ifly_audio_preprocess(asr, data, &wav);
    } else {
        wtk_string_set(&wav, data->data, data->len);
    }
    qtk_ifly_process(asr->dec, wav.data, wav.len, 0);
    return 0;
}


static int ifly_stop(void* inst) {
    ifly_instance_t *asr = (ifly_instance_t*)inst;
    asr->stop = 1;
    if (wtk_audio_tag_need_pre(&asr->audio_tag, asr->cfg->dst_sample_rate)) {
        ifly_audio_decoder_stop(asr);
    }
    qtk_ifly_process(asr->dec, NULL, 0, 1);
    return 0;
}


static int ifly_get_result(void* inst, wtk_string_t *result) {
    int ret = QTK_AISRV_SUCCESS_EOF;
    ifly_instance_t *asr = (ifly_instance_t*)inst;
    if (asr->stop) {
        qtk_ifly_get_result(asr->dec, result);
    } else {
        ret = QTK_AISRV_EMPTY_RESULT;
    }
    return ret;
}


int ifly_open(qtk_aisrv_mod_t *mod) {
    mod->init = ifly_init;
    mod->clean = ifly_clean;
    mod->create = ifly_create;
    mod->release = ifly_release;
    mod->start = ifly_start;
    mod->stop = ifly_stop;
    mod->feed = ifly_feed;
    mod->get_result = ifly_get_result;
    return 0;
}
