#include "qtk/sframe/qtk_sframe.h"
#include "wtk/core/segmenter/wtk_segmenter.h"
#include "wtk/core/cfg/wtk_main_cfg.h"
#include "wtk/os/wtk_thread.h"
#include <string.h>

typedef struct wordseg wordseg_t;

struct wordseg {
    wtk_segmenter_cfg_t *cfg;
    char *cfn;
    wtk_thread_t thread;
    void *frame;
    int srv_id;
    unsigned run:1;
};


int wordseg_do(wtk_segmenter_t *seg, char *input, int len, wtk_strbuf_t *buf) {
    wtk_strbuf_reset(buf);
	wtk_segmenter_reset(seg);
	wtk_segmenter_parse(seg, input, len, buf);
    wtk_strbuf_push_c(buf, '\0');
    buf->pos--;
    return 0;
}


int wordseg_route(void *data, wtk_thread_t *t) {
    wordseg_t *demo = data;
    void *frame = demo->frame;
    qtk_sframe_method_t *method = frame;
    wtk_segmenter_t *seg = wtk_segmenter_new(demo->cfg, NULL);
    wtk_strbuf_t *buf = wtk_strbuf_new(512, 1);

    while (demo->run) {
        qtk_sframe_msg_t *msg = method->pop(frame, -1);
        if (NULL == msg) continue;
        int type = method->get_type(msg);
        int signal = method->get_signal(msg);
        int id = method->get_id(msg);
        if (type == QTK_SFRAME_MSG_NOTICE){
            switch (signal) {
            case QTK_SFRAME_SIGCONNECT:
                wtk_debug("socket[%d] %s connected\r\n", id, method->get_remote_addr(method, msg)->data);
                break;
            case QTK_SFRAME_SIGDISCONNECT:
            case QTK_SFRAME_SIGECONNECT:
            {
                wtk_debug("socket[%d] %s disconnected\r\n", id, method->get_remote_addr(method, msg)->data);
                qtk_sframe_msg_t *msg = method->new(method, QTK_SFRAME_MSG_CMD, id);
                method->set_cmd(msg, QTK_SFRAME_CMD_CLOSE, NULL);
                method->push(method, msg);
                break;
            }
            default:
                wtk_debug("unexpected socket notice\r\n");
                break;
            }
        } else if (type == QTK_SFRAME_MSG_REQUEST) {
            wtk_strbuf_t *body = method->get_body(msg);
            char *p = strchr(body->data, '=');
            qtk_sframe_msg_t *msg2 = method->new(method, QTK_SFRAME_MSG_RESPONSE, id);
            if (p) {
                p++;
                int len = body->pos - (p - body->data);
                double time = time_get_ms();
                wordseg_do(seg, p, len, buf);
                wtk_debug("time=%lf\r\n", time_get_ms() - time);
                method->set_status(msg2, 200);
                method->add_header(msg2, "content-type", "text/plain");
                method->add_body(msg2, buf->data, buf->pos);
            } else {
                method->set_status(msg2, 400);
            }
            method->push(method, msg2);
        }
        method->delete(method, msg);
    }
    wtk_segmenter_delete(seg);
    wtk_strbuf_delete(buf);
    return 0;
}


wordseg_t *wordseg_new(void *frame) {
    wordseg_t *demo = wtk_malloc(sizeof(*demo));
	wtk_main_cfg_t *main_cfg = NULL;
    wtk_segmenter_cfg_t *cfg = NULL;
    qtk_sframe_method_t *method = frame;
    wtk_local_cfg_t *lc = method->get_cfg(method);
    wtk_string_t *v;

    wtk_local_cfg_update_cfg_str(lc, demo, cfn, v);
    wtk_debug("%s\r\n", demo->cfn);
    main_cfg = wtk_main_cfg_new_type(wtk_segmenter_cfg, demo->cfn);
	cfg = (wtk_segmenter_cfg_t*)main_cfg->cfg;
	cfg->dict_fn = NULL;
	wtk_segmenter_cfg_update(cfg);
    demo->cfg = cfg;
    demo->frame = frame;
    demo->run = 0;
    wtk_thread_init(&demo->thread, wordseg_route, demo);
    return demo;
}


int wordseg_start(wordseg_t *demo) {
    qtk_sframe_method_t *method = demo->frame;
    qtk_sframe_msg_t *msg;
    qtk_sframe_listen_param_t *lis_param;
    int id;

    id = method->socket(demo->frame);
    msg = method->new(demo->frame, QTK_SFRAME_MSG_CMD, id);
    lis_param = LISTEN_PARAM_NEW("0.0.0.0", 8100, 1000);
    method->set_cmd(msg, QTK_SFRAME_CMD_LISTEN, lis_param);
    method->push(demo->frame, msg);

    demo->run = 1;
    wtk_thread_start(&demo->thread);

    return 0;
}


int wordseg_stop(wordseg_t *demo) {
    demo->run = 0;
    // wtk_thread_join(&demo->thread);
    if (demo->srv_id >= 0) {
        qtk_sframe_msg_t *msg;
        qtk_sframe_method_t *method = (qtk_sframe_method_t*)demo->frame;
        msg = method->new(method, QTK_SFRAME_MSG_CMD, demo->srv_id);
        method->set_cmd(msg, QTK_SFRAME_CMD_CLOSE, NULL);
        method->push(method, msg);
    }
    return 0;
}


int wordseg_delete(wordseg_t *demo) {
    // wtk_thread_join(&demo->thread);
    wtk_thread_clean(&demo->thread);
    if (demo->cfg) {
        wtk_segmenter_cfg_clean(demo->cfg);
        demo->cfg = NULL;
    }
    wtk_free(demo);
    return 0;
}
