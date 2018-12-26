#include "qtk_usc.h"


qtk_usc_t* qtk_usc_new(qtk_usc_cfg_t *cfg) {
    qtk_usc_t *usc = wtk_calloc(1, sizeof(qtk_usc_t));
    USC_HANDLE handle;
    int ret = usc_create_service(&handle);
    if (USC_ASR_OK != ret) {
        wtk_debug("usc_create_service_ext error %d\r\n", ret);
        goto end;
    }
    ret = usc_set_option(handle, USC_OPT_ASR_APP_KEY, cfg->app_key);
    ret = usc_set_option(handle, USC_OPT_USER_SECRET, cfg->secret_key);

    usc->handle = handle;
    usc->buf = wtk_strbuf_new(1024, 1);
    usc->bufx = wtk_strbuf_new(1024, 1);
    usc->ready = 0;
end:
    if (ret && usc) {
        wtk_free(usc);
        usc = NULL;
    }
    return usc;
}


void qtk_usc_delete(qtk_usc_t *usc) {
    wtk_strbuf_delete(usc->buf);
    wtk_strbuf_delete(usc->bufx);
    usc_release_service(usc->handle);
    wtk_free(usc);
}


int qtk_usc_start(qtk_usc_t *usc) {
    int ret = usc_login_service(usc->handle);
    if (USC_ASR_OK != ret) {
        wtk_debug("usc_login_service error %d\r\n", ret);
        goto end;
    }
    ret = usc_set_option(usc->handle, USC_OPT_INPUT_AUDIO_FORMAT, AUDIO_FORMAT_PCM_16K);
    ret = usc_set_option(usc->handle, USC_OPT_RECOGNITION_FIELD, RECOGNITION_FIELD_GENERAL);
    ret = usc_start_recognizer(usc->handle);
    if (USC_ASR_OK != ret) {
        wtk_debug("usc_start_recognizer error %d\r\n", ret);
        goto end;
    }
    wtk_strbuf_reset(usc->buf);
    usc->ready = 1;
end:
    return ret;
}


int qtk_usc_process(qtk_usc_t *usc,char *data,int len,int is_end) {
    int ret;
    const char *res;

    if (!usc->ready) {
        ret = -1;
        goto end;
    }

    ret = usc_feed_buffer(usc->handle, data, len);

    if (ret == USC_RECOGNIZER_PARTIAL_RESULT ||
        ret == USC_RECOGNIZER_SPEAK_END) {
        res = usc_get_result(usc->handle);
        wtk_strbuf_push(usc->buf, (char*)res, strlen(res));
    }

    ret = 0;
    if (is_end) {
        ret = usc_stop_recognizer(usc->handle);
        if (0 == ret) {
            res = usc_get_result(usc->handle);
            wtk_strbuf_push(usc->buf, (char*)res, strlen(res));
        } else {
            // 网络出现错误退出
            wtk_debug("usc_stop_recognizer error %d\r\n", ret);
        }
    }
end:
    return ret;
}


void qtk_usc_get_result(qtk_usc_t *usc,wtk_string_t *v) {
    wtk_strbuf_reset(usc->bufx);
    wtk_strbuf_push_s(usc->bufx, "{\"rec\":\"");
    wtk_strbuf_push(usc->bufx, usc->buf->data, usc->buf->pos);
    wtk_strbuf_push_s(usc->bufx, "\",\"conf\":3.7,\"tag\":\"usc\"}");
    wtk_string_set(v, usc->bufx->data, usc->bufx->pos);
}
