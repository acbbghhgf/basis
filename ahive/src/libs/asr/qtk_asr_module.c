#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qtk/core/cJSON.h"
#include "wtk/core/param/wtk_module_param.h"
#include "qtk/service/qtk_aisrv_module.h"
#include "wtk/asr/wfst/kaldifst/qtk_decoder_wrapper.h"
#include "wtk/core/cfg/wtk_main_cfg.h"
#include "qtk/audio/qtk_audio_decoder.h"
#include "qtk/core/qtk_network.h"
#include "qtk/util/qtk_verify.h"

#include <assert.h>

const static int default_dst_sample_rate = 16000;
const static char *default_ebnf_root_dir = "/data/qtk/local/ebnf";


typedef struct asr_cfg asr_cfg_t;
typedef struct asr_instance asr_instance_t;

struct asr_cfg {
    union {
        wtk_main_cfg_t *mcfg;
        qtk_decoder_wrapper_cfg_t *wfst;
    } cfg;
    char *default_usr_bin;
    const char *ebnf_root_dir;
    const char *hot_words_file;
    int dst_sample_rate;
    short use_bin:1;            /* for cfg */
};


struct asr_instance {
    asr_cfg_t *cfg;
    wtk_audio_tag_t audio_tag;
    qtk_audio_decoder_t *decoder;
    qtk_decoder_wrapper_t *dec;
    wtk_strbuf_t *resbuf;
    wtk_strbuf_t *hot_buf;
    wtk_strbuf_t *ebnf_fn;
    void *ebnf_net;
    unsigned stop:1;
    unsigned stripSpace:1;
};


static void asr_reset_audio_tag(wtk_audio_tag_t *audio_tag) {
    audio_tag->type = AUDIO_WAV;
    audio_tag->channel = 1;
    audio_tag->samplerate = default_dst_sample_rate;
    audio_tag->samplesize = 2;
}


static void asr_set_audio_tag(wtk_audio_tag_t *audio_tag, cJSON *jaudio) {
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


static void asr_audio_decoder_start(asr_instance_t *asr) {
    if (NULL == asr->decoder) {
        asr->decoder = qtk_audio_decoder_new(asr->cfg->dst_sample_rate);
    }
    qtk_audio_decoder_start(asr->decoder,
                            asr->audio_tag.type,
                            asr->audio_tag.channel,
                            asr->audio_tag.samplerate,
                            asr->audio_tag.samplesize);
}

static int asr_audio_preprocess(asr_instance_t *asr, wtk_string_t *src, wtk_string_t *dst) {
    qtk_audio_decoder_feed(asr->decoder, src->data, src->len, 0);
    wtk_string_set(dst, asr->decoder->resample_audio->data, asr->decoder->resample_audio->pos);
    return 0;
}


static void asr_audio_decoder_stop(asr_instance_t *asr) {
    qtk_audio_decoder_feed(asr->decoder, 0, 0, 1);
}


static int asr_set_ebnf_net(asr_instance_t *inst, const char *ebnf_fn) {
    wtk_wfstdec_t *dec = inst->dec->ebnfdec2->dec;
    if (dec->cfg->usrec_is_active || dec->cfg->use_ebnf) {
        // wtk_debug("DONT support set ebnf net\r\n");
        return -1;
    }
    if (ebnf_fn) {
        if (strcmp(ebnf_fn, inst->ebnf_fn->data)) {
            wtk_fst_net_t *old_net = dec->cfg->usr_net;
            char ebnf_path[256];
            wtk_strbuf_reset(inst->ebnf_fn);
            wtk_strbuf_push(inst->ebnf_fn, ebnf_fn, strlen(ebnf_fn)+1);
            snprintf(ebnf_path, sizeof(ebnf_path), "%s/%s", inst->cfg->ebnf_root_dir, ebnf_fn);
            wtk_fst_sym_delete(dec->res_net->cfg->sym_out);
            dec->res_net->cfg->sym_out = NULL;
            if (0 == wtk_wfstdec_cfg_set_ebnf_net(dec->cfg, ebnf_path)) {
                dec->res_net = dec->cfg->usr_net;
                if (old_net) {
                    wtk_fst_net_delete(old_net);
                }
            }
	//    wtk_debug("xxxifcfg->sym_out==%p\n", dec->res_net->cfg->sym_out);
        }
    } else {
        if (inst->ebnf_fn->pos) {
            wtk_fst_net_t *old_net = dec->cfg->usr_net;
            wtk_fst_sym_delete(dec->res_net->cfg->sym_out);
            dec->res_net->cfg->sym_out = NULL;
            wtk_wfstdec_cfg_set_ebnf_net(dec->cfg, inst->cfg->default_usr_bin);
            dec->res_net = dec->cfg->usr_net;
            wtk_strbuf_reset(inst->ebnf_fn);
            wtk_strbuf_push_c(inst->ebnf_fn, '\0');
	  //  wtk_debug("xxxelsecfg->sym_out==%p\n", dec->res_net->cfg->sym_out);
            inst->ebnf_fn->pos--;
            if (old_net) {
                wtk_fst_net_delete(old_net);
            }
        }
    }
    return 0;
}


void* asr_init(wtk_local_cfg_t *cfg) {
    wtk_string_t *v;
    asr_cfg_t *acfg = NULL;
    acfg = wtk_malloc(sizeof(*acfg));
    acfg->dst_sample_rate = default_dst_sample_rate;
    acfg->use_bin = 0;
    acfg->ebnf_root_dir = default_ebnf_root_dir;
    wtk_local_cfg_update_cfg_b(cfg, acfg, use_bin, v);
    wtk_local_cfg_update_cfg_i(cfg, acfg, dst_sample_rate, v);
    wtk_local_cfg_update_cfg_str(cfg, acfg, ebnf_root_dir, v);
    wtk_local_cfg_update_cfg_str(cfg, acfg, hot_words_file, v);
    v = wtk_local_cfg_find_string_s(cfg,"cfg");
    if (!v) { goto end; }
    assert(acfg->use_bin == 0);
    if (acfg->use_bin) {
        //acfg->cfg.wfst = qtk_decoder_wrapper_cfg_new_bin(v->data);
        //acfg->default_usr_bin = acfg->cfg.wfst->ebnfdec2.usr_bin;
    } else {
        acfg->cfg.mcfg = qtk_decoder_wrapper_cfg_new(v->data);
        acfg->default_usr_bin = ((qtk_decoder_wrapper_cfg_t*)(acfg->cfg.mcfg->cfg))->ebnfdec2.usr_bin;
    }
end:
    return acfg;
}


static void asr_clean(void* inst) {
    asr_cfg_t *acfg = (asr_cfg_t*)inst;
    assert(acfg->use_bin == 0);
    if (acfg->use_bin) {
       // if (acfg->cfg.wfst) {
       //     qtk_decoder_wrapper_cfg_delete_bin(acfg->cfg.wfst);
       // }
    } else {
        if (acfg->cfg.mcfg) {
            qtk_decoder_wrapper_cfg_delete(acfg->cfg.mcfg);
        }
    }
    wtk_free(inst);
}


static void* asr_create(void* inst) {
    asr_cfg_t *acfg = (asr_cfg_t*)inst;
    asr_instance_t *asr = wtk_malloc(sizeof(*asr));
    qtk_decoder_wrapper_cfg_t *cfg = acfg->use_bin ? acfg->cfg.wfst : acfg->cfg.mcfg->cfg;
    asr->cfg = acfg;
    asr->dec = qtk_decoder_wrapper_new(cfg);
    asr->ebnf_fn = wtk_strbuf_new(64, 1);
    asr->hot_buf = wtk_strbuf_new(128, 1);

    asr->decoder = NULL;
    asr->resbuf = NULL;
    asr->stop = 0;
    return asr;
}


static int asr_release(void* inst) {
    asr_instance_t *asr = (asr_instance_t*)inst;
    if (asr->dec) {
        qtk_decoder_wrapper_delete(asr->dec);
    }
    if (asr->decoder) {
        qtk_audio_decoder_delete(asr->decoder);
    }
    if (asr->resbuf) {
        wtk_strbuf_delete(asr->resbuf);
    }
    if (asr->ebnf_fn) {
        wtk_strbuf_delete(asr->ebnf_fn);
    }
    if (asr->hot_buf) {
        wtk_strbuf_delete(asr->hot_buf);
    }
    wtk_free(asr);
    return 0;
}


static int asr_start(void* inst, wtk_string_t *param) {
    asr_instance_t *asr = (asr_instance_t*)inst;
    wtk_strbuf_t *buf = wtk_strbuf_new(1024, 1);
    wtk_string_t env = {NULL, 0};
    char *ebnf_fn = NULL;
    cJSON *jp = NULL, *obj;

    asr->stripSpace = 0;
    asr->stop = 0;
    asr_reset_audio_tag(&asr->audio_tag);
    wtk_strbuf_push(buf, param->data, param->len);
    wtk_strbuf_push_c(buf, '\0');
 
 //   wtk_debug("xxxxxacfg->hot_file == %s\n", asr->cfg->hot_words_file);
    int file = open(asr->cfg->hot_words_file, O_RDONLY, S_IRWXU);
    if(file == -1){
        perror("open fail\n");
        wtk_debug("open file fail\n");
    }
    else if(file != -1){
        char buf[128];
        int buf_count = 0;
        buf_count = read(file, buf, 128-1);
        while(buf_count)
        {
            wtk_strbuf_push(asr->hot_buf, buf, buf_count);
            memset(buf, 0, sizeof(buf));
            buf_count = read(file, buf, 128-1);
        }
	*(asr->hot_buf->data + asr->hot_buf->pos) = '\0';
   //     wtk_debug("xxxxasr->hot_buf->data = %s\n", asr->hot_buf->data);
    }

    if(asr->hot_buf->pos != 0){
        qtk_kwfstdec_set_hot_words(asr->dec->asr[0]->dec, asr->hot_buf->data);
	wtk_strbuf_delete(asr->hot_buf);
	asr->hot_buf = wtk_strbuf_new(128,1);
	memset(asr->hot_buf->data, 0, 128);
    }
    jp = cJSON_Parse(buf->data);
    if (NULL == jp) goto end;
    obj = cJSON_GetObjectItem(jp, "stripSpace");
    if (obj && cJSON_Number == obj->type && obj->valueint) {
        asr->stripSpace = 1;
    }
    obj = cJSON_GetObjectItem(jp, "audio");
    if (obj) {
        asr_set_audio_tag(&asr->audio_tag, obj);
    }
    obj = cJSON_GetObjectItem(jp, "env");
    if (obj && cJSON_String == obj->type) {
        wtk_string_set(&env, obj->valuestring, strlen(obj->valuestring));
    }
    obj = cJSON_GetObjectItem(jp, "ebnf");
    if (obj && cJSON_String == obj->type) {
        ebnf_fn = obj->valuestring;
    }
end:
    if (wtk_audio_tag_need_pre(&asr->audio_tag, asr->cfg->dst_sample_rate)) {
        asr_audio_decoder_start(asr);
    }
    qtk_decoder_wrapper_reset(asr->dec);
    asr_set_ebnf_net(asr, ebnf_fn);
    qtk_decoder_wrapper_start2(asr->dec, env.data, env.len);
 //   if(asr->hot_buf->pos != 0){
  //      qtk_kwfstdec_set_hot_words(asr->dec->asr[0]->dec, asr->hot_buf->data);
  //  }
    if (jp) {
        cJSON_Delete(jp);
    }
    wtk_strbuf_delete(buf);
    return 0;
}


static int asr_feed(void* inst, wtk_string_t *data) {
    asr_instance_t *asr = (asr_instance_t*)inst;
    wtk_string_t wav;
    if (wtk_audio_tag_need_pre(&asr->audio_tag, asr->cfg->dst_sample_rate)) {
        asr_audio_preprocess(asr, data, &wav);
    } else {
        wtk_string_set(&wav, data->data, data->len);
    }
    return qtk_decoder_wrapper_feed(asr->dec, wav.data, wav.len, 0);
}


static int asr_stop(void* inst) {
    asr_instance_t *asr = (asr_instance_t*)inst;
    asr->stop = 1;
    if (wtk_audio_tag_need_pre(&asr->audio_tag, asr->cfg->dst_sample_rate)) {
        asr_audio_decoder_stop(asr);
    }
    return qtk_decoder_wrapper_feed(asr->dec, NULL, 0, 1);
}


static int asr_get_result(void* inst, wtk_string_t *result) {
    asr_instance_t *asr = (asr_instance_t*)inst;
    int ret = QTK_AISRV_SUCCESS_EOF;
    if (asr->stop) {
        qtk_decoder_wrapper_get_result(asr->dec, result);
    } else {
        //qtk_decoder_wrapper_get_hint_result(asr->dec, result);
	result->len = 0;
        ret = result->len ? QTK_AISRV_SUCCESS : QTK_AISRV_EMPTY_RESULT;
    }
    if (asr->stripSpace) {
        if (NULL == asr->resbuf) {
            asr->resbuf = wtk_strbuf_new(result->len, 1);
        } else {
            wtk_strbuf_reset(asr->resbuf);
            wtk_strbuf_expand(asr->resbuf, result->len);
        }
        wtk_strbuf_push_skip_ws(asr->resbuf, result->data, result->len);
        wtk_string_set(result, asr->resbuf->data, asr->resbuf->pos);
    }
    return ret;
}


int asr_open(qtk_aisrv_mod_t *mod) {
    const char* host = qtk_aisrv_get_auth_host();
    const char* url = qtk_aisrv_get_auth_url();
    const char* srv_type = "asr";
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
    mod->init = asr_init;
    mod->clean = asr_clean;
    mod->create = asr_create;
    mod->release = asr_release;
    mod->start = asr_start;
    mod->stop = asr_stop;
    mod->feed = asr_feed;
    mod->get_result = asr_get_result;
    return 0;
}
