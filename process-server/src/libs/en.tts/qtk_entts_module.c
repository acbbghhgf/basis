#include <errno.h>
#include <assert.h>
#include "qtk/service/qtk_aisrv_module.h"
#include "wtk/os/wtk_thread.h"
#include "qtk/audio/qtk_mp3_encoder.h"
#include "qtk/core/cJSON.h"
#include "ais_hts_engine.h"
#include "qtk/core/qtk_network.h"
#include "qtk/util/qtk_verify.h"


typedef struct entts_cfg entts_cfg_t;
typedef struct entts_instance entts_instance_t;


struct entts_cfg {
    const char *cfg;
    int coder_buf_size;
};


struct entts_instance {
    entts_cfg_t *cfg;
    ais_voice *vcs;
    qtk_mp3encoder_t mp3enc;
    wtk_strbuf_t *tmpbuf;
    wtk_string_t *result;
    wtk_thread_t thread;
    wtk_sem_t start_sem;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    unsigned running : 1;
    unsigned stream : 1;
};


static void* entts_init(wtk_local_cfg_t *cfg) {
    wtk_string_t *v;
    entts_cfg_t *tcfg = NULL;
    v = wtk_local_cfg_find_string_s(cfg, "cfg");
    if (!v) {
        wtk_debug("load tts config failed\r\n");
        goto end;
    }
    tcfg = wtk_malloc(sizeof(*tcfg));
    tcfg->cfg = v->data;
    tcfg->coder_buf_size = 1024;
    wtk_local_cfg_update_cfg_i(cfg, tcfg, coder_buf_size, v);
end:
    return tcfg;
}


static void entts_clean(void *cfg) {
    wtk_free(cfg);
}


static int qtk_entts_fill_buf(entts_instance_t *tts, char *buf, int len, int is_end) {
    (void)is_end;

    pthread_mutex_lock(&tts->mutex);
    wtk_strbuf_push(tts->tmpbuf, buf, len);
    pthread_mutex_unlock(&tts->mutex);

    if (tts->stream) {
        pthread_cond_signal(&tts->cond);
    }

    return 0;
}


static int entts_thread_route(void *inst, wtk_thread_t *t) {
    entts_instance_t *tts = (entts_instance_t*)inst;
    wtk_strbuf_t *text = wtk_strbuf_new(1024, 1);
    int ret;
    char *outwav;

    while (1) {
        ret = wtk_sem_acquire(&tts->start_sem, -1);
        if (ret) {
            if (errno == EINTR) continue;
            assert(0);
        }
        if (!tts->running) break;
        wtk_strbuf_reset(text);
        wtk_strbuf_push(text, tts->tmpbuf->data, tts->tmpbuf->pos);
        wtk_strbuf_push_c(text, '\0');
        wtk_strbuf_reset(tts->tmpbuf);
        ret = syn_speech_to_buff(tts->vcs, text->data, &outwav, 0, 0);
        if (ret > 44 && outwav) {
            qtk_mp3enc_feed(&tts->mp3enc, outwav+44, ret-44);
            qtk_mp3enc_feed(&tts->mp3enc, NULL, 0);
        }
        tts->running = 0;
        pthread_cond_signal(&tts->cond);
    }
    wtk_strbuf_delete(text);
    return 0;
}


static void* entts_create(void *cfg) {
    entts_cfg_t *tcfg = (entts_cfg_t*)cfg;
    ais_voice *vcs = NULL;
    entts_instance_t *tts = NULL;

    if (NULL == cfg || NULL == tcfg->cfg) goto end;
    vcs = init_engine(tcfg->cfg, NULL);
    if (NULL == vcs) {
        wtk_debug("create en.tts engine failed\r\n");
        goto end;
    }
    tts = wtk_malloc(sizeof(*tts));
    tts->cfg = tcfg;
    tts->vcs = vcs;
    qtk_mp3enc_init(&tts->mp3enc, tcfg->coder_buf_size);
    tts->tmpbuf = wtk_strbuf_new(1024, 1);
    tts->result = NULL;
    wtk_sem_init(&tts->start_sem, 0);
    wtk_thread_init(&tts->thread, entts_thread_route, tts);
    pthread_mutex_init(&tts->mutex, NULL);
    pthread_cond_init(&tts->cond, NULL);
    tts->running = 0;
    qtk_mp3enc_set_notifier(&tts->mp3enc, (qtk_mp3enc_ready_f)qtk_entts_fill_buf, tts);
    wtk_thread_start(&tts->thread);
end:
    return tts;
}


static int entts_release(void *inst) {
    entts_instance_t *tts = (entts_instance_t*)inst;
    if (tts->vcs) {
        release_engine(tts->vcs);
    }
    if (tts->tmpbuf) {
        wtk_strbuf_delete(tts->tmpbuf);
    }
    if (tts->result) {
        wtk_string_delete(tts->result);
    }
    tts->running = 0;
    wtk_sem_inc(&tts->start_sem);
    wtk_thread_clean(&tts->thread);
    pthread_mutex_destroy(&tts->mutex);
    pthread_cond_destroy(&tts->cond);
    wtk_sem_clean(&tts->start_sem);
    qtk_mp3enc_clean(&tts->mp3enc);
    wtk_free(tts);
    return 0;
}


static int entts_start(void *inst, wtk_string_t *param) {
    entts_instance_t *tts = (entts_instance_t*)inst;
    cJSON *jp = NULL, *obj;
    char *buf = NULL;
    int ret = -1;

    if (tts->running) goto end;

    /* reset buf and engine */
    wtk_strbuf_reset(tts->tmpbuf);
    qtk_mp3enc_reset(&tts->mp3enc);
    buf = wtk_malloc(param->len+1);
    memcpy(buf, param->data, param->len);
    buf[param->len] = '\0';
    tts->stream = 1;
    jp = cJSON_Parse(buf);
    if (!jp) goto end;
    for (obj = jp->child; obj; obj = obj->next) {
        if (!strcmp(obj->string, "volume")) {
            set_volume2(tts->vcs, obj->valuedouble);
        } else if (!strcmp(obj->string, "speed")) {
            set_speed2(tts->vcs, obj->valuedouble);
        } else if (!strcmp(obj->string, "pitch")) {
            set_pitch(tts->vcs, obj->valuedouble);
        } else if (!strcmp(obj->string, "text")) {
            wtk_strbuf_push(tts->tmpbuf, obj->valuestring, strlen(obj->valuestring));
        } else if (!strcmp(obj->string, "useStream")) {
            tts->stream = (obj->type == cJSON_False) ? 0 : 1;
        }
    }
    tts->running = 1;
    wtk_sem_inc(&tts->start_sem);
    ret = 0;
end:
    if (buf) {
        wtk_free(buf);
    }
    if (jp) {
        cJSON_Delete(jp);
    }
    return ret;
}


static int entts_get_result(void *inst, wtk_string_t *result) {
    entts_instance_t *tts = (entts_instance_t*)inst;
    wtk_strbuf_t *buf = tts->tmpbuf;

    if (tts->result) {
        wtk_string_delete(tts->result);
        tts->result = NULL;
    }
    pthread_mutex_lock(&tts->mutex);
    if (tts->running) {
        while (pthread_cond_wait(&tts->cond, &tts->mutex)) {
            if (!tts->running || buf->pos) break;
        }
    }
    if (0 == buf->pos) {
        wtk_strbuf_push_c(buf, 0);
    }
    tts->result = wtk_string_dup_data(buf->data, buf->pos);
    wtk_strbuf_reset(buf);
    pthread_mutex_unlock(&tts->mutex);
    *result = *tts->result;
    return tts->running ? 0 : 1;
}


int en_tts_open(qtk_aisrv_mod_t *mod) {
    const char* host = qtk_aisrv_get_auth_host();
    const char* url = qtk_aisrv_get_auth_url();
    const char* srv_type = "en.tts";
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
    mod->init = entts_init;
    mod->clean = entts_clean;
    mod->create = entts_create;
    mod->release = entts_release;
    mod->start = entts_start;
    mod->feed = NULL;
    mod->stop = NULL;
    mod->get_result = entts_get_result;
    return 0;
}
