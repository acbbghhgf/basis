#include "qtk/sframe/qtk_sframe.h"
#include "wtk/os/wtk_thread.h"


typedef struct test_demo test_demo_t;
typedef struct demo_sender demo_sender_t;

struct test_demo {
    wtk_thread_t thread;
    wtk_thread_t req_thread;
    wtk_thread_t iris_thread;
    void *frame;
    const char *lis_addr;
    int pool_id;
    unsigned run:1;
};


struct demo_sender {
    qtk_sframe_method_t *method;
    void *frame;
    qtk_sframe_msg_t *msg;
};


static int cli_id = -1;


static int demo_iris(void *data, wtk_thread_t *t) {
    test_demo_t *demo = (test_demo_t*)data;
    char buf[128];
    char topic[128];
    const char *txt = "abc";
    qtk_sframe_method_t *method = (qtk_sframe_method_t*)demo->frame;
    void *frame = demo->frame;
    int sock_id = method->socket(frame);
    int i;
    qtk_sframe_msg_t *msg;
    qtk_sframe_connect_param_t *con_param = CONNECT_PARAM_NEW("192.168.0.98", 10000, 0);
    wtk_debug("sock_id=%d\r\n", sock_id);
    msg = method->new(frame, QTK_SFRAME_MSG_CMD, sock_id);
    method->set_cmd(msg, QTK_SFRAME_CMD_OPEN, con_param);
    method->push(frame, msg);

    for (i = 0; i < 10000000; ++i) {
        int n = snprintf(topic, sizeof(topic), "/%d/%d", sock_id, i);
        /* watch */
        msg = method->new(frame, QTK_SFRAME_MSG_REQUEST, sock_id);
        method->set_method(msg, HTTP_PUT);
        method->set_url(msg, "/_watch", 7);
        method->add_body(msg, buf, snprintf(buf, sizeof(buf), "{\"%s\":\"\"}", topic));
        method->push(frame, msg);

        /* produce */
        const char *p = txt;
        while (*p) {
            msg = method->new(frame, QTK_SFRAME_MSG_REQUEST, sock_id);
            memset(buf, *p, sizeof(buf));
            method->set_method(msg, HTTP_PUT);
            method->set_url(msg, topic, n);
            method->add_body(msg, buf, sizeof(buf));
            method->push(frame, msg);
            ++p;
        }

        /* unwatch */
        msg = method->new(frame, QTK_SFRAME_MSG_REQUEST, sock_id);
        method->set_method(msg, HTTP_PUT);
        method->set_url(msg, "/_unwatch", 9);
        method->add_body(msg, buf, snprintf(buf, sizeof(buf), "[\"%s\"]", topic));
        method->push(frame, msg);
        wtk_debug("send %d\r", i);
        usleep(10);
    }

    wtk_debug("send close\r\n");
    msg = method->new(frame, QTK_SFRAME_MSG_CMD, sock_id);
    method->set_cmd(msg, QTK_SFRAME_CMD_CLOSE, NULL);
    method->push(frame, msg);

    return 0;
}


static int demo_request(void *data, wtk_thread_t *t) {
    demo_sender_t *sender = data;
    int i;
    qtk_sframe_method_t *method = sender->method;
    void *frame = sender->frame;
    qtk_sframe_msg_t *msg = sender->msg;
    int id = method->get_id(msg);
    wtk_string_t *url = method->get_url(msg);
    wtk_strbuf_t *body = method->get_body(msg);
    int meth = method->get_method(msg);
    for (i = 0; i < 100; ++i) {
        msg = method->new(frame, QTK_SFRAME_MSG_REQUEST, id);
        method->set_url(msg, url->data, url->len);
        method->set_method(msg, meth);
        if (body) {
            method->add_body(msg, body->data, body->pos);
        }
        method->push(frame, msg);
        usleep(10000);
    }
    method->push(frame, sender->msg);
    wtk_free(data);
    return 0;
}


static int demo_route(void *data, wtk_thread_t *t) {
    test_demo_t *demo = data;
    void *frame = demo->frame;
    qtk_sframe_method_t *method = frame;

    while (demo->run) {
        qtk_sframe_msg_t *msg = method->pop(frame, -1);
        if (NULL == msg) continue;
        int type = method->get_type(msg);
        int signal = method->get_signal(msg);
        int id = method->get_id(msg);
        if (type == QTK_SFRAME_MSG_NOTICE) {
            switch (signal) {
            case QTK_SFRAME_SIGCONNECT:
                if (id == -demo->pool_id) {
                    wtk_thread_start(&demo->req_thread);
                    wtk_debug("pool %d connected\r\n", -id);
                } else {
                    wtk_debug("socket %d connected\r\n", id);
                    qtk_sframe_msg_t *m = method->new(frame, QTK_SFRAME_MSG_REQUEST, id);
                    method->set_method(m, HTTP_POST);
                    const char *p = "/register";
                    method->set_url(m, "/register", strlen(p));
                    p = "{\"tag\":\"test\"}";
                    method->add_body(m, p, strlen(p));
                    method->push(frame, m);
                }
                break;
            case QTK_SFRAME_SIGDISCONNECT:
                wtk_debug("disconnected\r\n");
                break;
            }
        } else if (type == QTK_SFRAME_MSG_REQUEST) {
            if (id >= 0) {
                if (0 == strcmp(demo->lis_addr, method->get_local_addr(frame, msg)->data)) {
                    wtk_string_t *ctype = method->get_header(msg, "content-type");
                    wtk_strbuf_t *body = method->get_body(msg);
                    qtk_sframe_msg_t *msg2 = method->new(frame, QTK_SFRAME_MSG_RESPONSE, id);
                    method->set_status(msg2, 200);
                    method->set_signal(msg2, 0);
                    if (ctype) {
                        ctype = wtk_string_dup_data_pad0(ctype->data, ctype->len);
                        method->add_header(msg2, "content-type", ctype->data);
                        wtk_string_delete(ctype);
                    }
                    if (body) {
                        method->add_body(msg2, body->data, body->pos);
                    }
                    method->push(frame, msg2);
                    msg2 = method->new(frame, QTK_SFRAME_MSG_CMD, id);
                    method->set_signal(msg2, QTK_SFRAME_CMD_CLOSE);
                    method->push(frame, msg2);
                } else {
                    /* int meth = method->get_method(msg); */
                    /* wtk_string_t *url = method->get_url(msg); */
                    /* wtk_string_t *addr = method->get_remote_addr(frame, msg); */
                    /* wtk_debug("request[%d, %.*s] from %s\r\n", */
                    /*           meth, url?url->len:0, url?url->data:NULL, addr->data); */
                }
            } else {
                //wtk_debug("pool: send request failed\r\n");
            }
        } else if (type == QTK_SFRAME_MSG_RESPONSE) {
            /* int status = method->get_status(msg); */
            /* wtk_string_t *addr = method->get_remote_addr(frame, msg); */
            /* wtk_debug("response %d from %s\r\n", status, addr->data); */
            /* qtk_response_print(msg->info.response); */
        }
        method->delete(frame, msg);
   }

    return 0;
}


test_demo_t* demo_new(void * frame) {
    test_demo_t *demo = wtk_malloc(sizeof(*demo));
    demo->frame = frame;
    demo->run = 0;
    demo->pool_id = 0;
    wtk_thread_init(&demo->thread, demo_route, demo);
    return demo;
}


int demo_start(test_demo_t *demo) {
    qtk_sframe_method_t *method = demo->frame;
    qtk_sframe_msg_t *msg;
    qtk_sframe_listen_param_t *lis_param;
    qtk_sframe_connect_param_t *con_param;
    int id;

    demo->run = 1;
    wtk_thread_start(&demo->thread);

    /*
      open listen port
     */
    id = method->socket(demo->frame);
    msg = method->new(demo->frame, QTK_SFRAME_MSG_CMD, id);
    demo->lis_addr = "0.0.0.0:8888";
    lis_param = LISTEN_PARAM_NEW("0.0.0.0", 8888, 100);
    method->set_cmd(msg, QTK_SFRAME_CMD_LISTEN, lis_param);
    method->push(demo->frame, msg);

    /*
      connect to test.port
     */
    id = method->socket(demo->frame);
    cli_id = id;
    msg = method->new(demo->frame, QTK_SFRAME_MSG_CMD, id);
    con_param = CONNECT_PARAM_NEW("192.168.0.98", 10000, QTK_SFRAME_RECONNECT_LAZY);
    method->set_cmd(msg, QTK_SFRAME_CMD_OPEN, con_param);
    method->push(demo->frame, msg);

    id = method->request_pool(demo->frame);
    msg = method->new(demo->frame, QTK_SFRAME_MSG_CMD, -id);
    con_param = CONNECT_PARAM_NEW("192.168.0.98", 8000, 5);
    method->set_cmd(msg, QTK_SFRAME_CMD_OPEN_POOL, con_param);
    demo->pool_id = id;
    method->push(demo->frame, msg);

    msg = method->new(demo->frame, QTK_SFRAME_MSG_REQUEST, -id);
    method->set_method(msg, HTTP_GET);
    method->set_url(msg, "/xxx", 4);
    demo_sender_t *sender = wtk_malloc(sizeof(*sender));
    sender->method = method;
    sender->msg = msg;
    sender->frame = demo->frame;
    wtk_thread_init(&demo->req_thread, demo_request, sender);
    wtk_thread_init(&demo->iris_thread, demo_iris, demo);
    wtk_thread_start(&demo->iris_thread);

    return 0;
}


int demo_stop(test_demo_t *demo) {
    if (demo->pool_id > 0) {
        qtk_sframe_msg_t *msg;
        qtk_sframe_method_t *method = (qtk_sframe_method_t*)demo->frame;
        void *frame = demo->frame;
        msg = method->new(frame, QTK_SFRAME_MSG_CMD, demo->pool_id);
        method->set_cmd(msg, QTK_SFRAME_CMD_CLOSE, NULL);
        method->push(frame, msg);
    }

    wtk_thread_join(&demo->req_thread);
    demo->run = 0;
    return 0;
}


int demo_delete(test_demo_t *demo) {
    wtk_thread_clean(&demo->req_thread);
    wtk_thread_clean(&demo->iris_thread);
    wtk_thread_join(&demo->thread);
    wtk_thread_clean(&demo->thread);
    wtk_free(demo);
    return 0;
}
