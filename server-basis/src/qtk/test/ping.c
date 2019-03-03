#include <assert.h>
#include <ctype.h>
#include "qtk/sframe/qtk_sframe.h"
#include "qtk/core/cJSON.h"
#include "wtk/os/wtk_thread.h"
#include "wtk/core/wtk_os.h"
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "qtk/event/qtk_event_timer.h"
#include "qtk/core/qtk_str_hashtable.h"
#include "qtk/core/qtk_int_hashtable.h"
#include "qtk/http/qtk_request.h"
#include "qtk/os/qtk_rlimit.h"
#include "qtk/http/qtk_request.h"


uint32_t test_get_ms() {
    static uint64_t start_time = 0;
    if (0 == start_time) start_time = time_get_ms();
    return (uint64_t)time_get_ms() - start_time;
}


typedef struct test_bench test_bench_t;
typedef struct test_stream  test_stream_t;
typedef struct test_proc  test_proc_t;
typedef struct test_item test_item_t;

struct test_bench  {
    wtk_thread_t thread;
    qtk_event_timer_t *timer;
    const char *ip;
    int nproc;
    int sleep;
    int port;
    int device_start_id;
    test_proc_t *procs;
    qtk_event_timer_cfg_t timer_cfg;
    qtk_abstruct_hashtable_t *proc_hash;
    qtk_sframe_method_t *method;
    unsigned short run: 1;
};


struct test_proc {
    test_bench_t *bench;
    qtk_hashnode_t sock_id;
    wtk_string_t *device;
    int ping_time;
};


static wtk_string_t* test_string_dup(const char *p) {
    wtk_string_t *s;
    s = wtk_string_new(strlen(p)+1);
    strcpy(s->data, p);
    s->len--;
    return s;
}


static int test_proc_init(test_proc_t *proc, test_bench_t *test, int device_id) {
    char tmp[32];
    proc->bench = test;
    sprintf(tmp, "%x", device_id);
    proc->device = test_string_dup(tmp);
    return 0;
}


static int test_proc_connect(test_proc_t *proc) {
    test_bench_t *test = proc->bench;
    qtk_sframe_method_t *method = test->method;
    qtk_sframe_msg_t *msg;
    qtk_sframe_connect_param_t *con_param;
    int sock_id = proc->sock_id.key._number;

    msg = method->new(method, QTK_SFRAME_MSG_CMD, sock_id);
    con_param = CONNECT_PARAM_NEW(test->ip, test->port, QTK_SFRAME_NOT_RECONNECT);
    method->set_cmd(msg, QTK_SFRAME_CMD_OPEN, con_param);
    method->push(method, msg);

    return 0;
}


static int test_proc_start(test_proc_t *proc) {
    test_bench_t *test = proc->bench;
    qtk_sframe_method_t *method = test->method;
    int sock_id = method->socket(method);

    proc->sock_id.key._number = sock_id;
    qtk_hashtable_add(test->proc_hash, proc);
    qtk_event_timer_add(test->timer, 0,
                        (qtk_event_timer_handler)test_proc_connect, proc);

    return 0;
}


static int test_proc_clean(test_proc_t *proc) {
    wtk_string_delete(proc->device);
    return 0;
}


static int test_update_cfg(test_bench_t *test, wtk_local_cfg_t *lc) {
    wtk_string_t *v;

    wtk_local_cfg_update_cfg_str(lc, test, ip, v);
    wtk_local_cfg_update_cfg_i(lc, test, port, v);
    wtk_local_cfg_update_cfg_i(lc, test, device_start_id, v);
    wtk_local_cfg_update_cfg_i(lc, test, nproc, v);
    wtk_local_cfg_update_cfg_i(lc, test, sleep, v);

    return 0;
}


static int test_proc_handle(test_proc_t *proc) {
    test_bench_t *test = proc->bench;
    qtk_sframe_method_t *method = test->method;

    qtk_sframe_msg_t *msg = method->new(method, QTK_SFRAME_MSG_REQUEST, proc->sock_id.key._number);
    method->set_method(msg, HTTP_POST);
    method->set_url(msg, "/ping", 5);
    method->add_body(msg, "hello", 5);
    method->push(method, msg);
    proc->ping_time = time_get_ms();
    wtk_debug("ping\r\n");

    return 0;
}


static int test_proc_response(test_proc_t *proc, wtk_string_t *ping, wtk_strbuf_t *buf) {
    assert(ping);
    return 0;
}


static int test_process_notice(test_proc_t *proc, qtk_sframe_msg_t *msg) {
    qtk_sframe_method_t *method = proc->bench->method;
    int signal = method->get_signal(msg);

    switch (signal) {
    case QTK_SFRAME_SIGCONNECT:
        wtk_debug("connected\r\n");
        test_proc_handle(proc);
        break;
    case QTK_SFRAME_SIGDISCONNECT:
        wtk_debug("disconnect\r\n");
        break;
    case QTK_SFRAME_SIGECONNECT:
        wtk_debug("connect failed\r\n");
        break;
    default:
        break;
    }
    return 0;
}


static int test_route(void *data, wtk_thread_t *thread) {
    test_bench_t *test = data;
    qtk_sframe_method_t *method = test->method;
    qtk_sframe_msg_t *msg;
    test_proc_t *proc;
    while (test->run) {
        msg = method->pop(method, 10);
        if (msg) {
            int msg_type = method->get_type(msg);
            int sock_id = method->get_id(msg);
            proc = qtk_hashtable_find(test->proc_hash, (qtk_hashable_t*)&sock_id);
            assert(proc);
            if (msg_type == QTK_SFRAME_MSG_NOTICE) {
                test_process_notice(proc, msg);
                method->delete(method, msg);
                continue;
            }
            int status = method->get_status(msg);
            wtk_strbuf_t* body = method->get_body(msg);
            wtk_string_t *ping = method->get_header(msg, "ping");
            (void)status;
            test_proc_response(proc, ping, body);
            qtk_event_timer_add(proc->bench->timer, test->sleep,
                                (qtk_event_timer_handler)test_proc_handle, proc);
            method->delete(method, msg);
        }
        qtk_event_timer_process(test->timer);
    }
    return 0;
}


void* ping_new(void *frame) {
    test_bench_t *test = wtk_malloc(sizeof(*test));
    test->nproc = 1;
    test->method = frame;
    test->run = 0;
    wtk_thread_init(&test->thread, test_route, test);
    wtk_local_cfg_t *lc = test->method->get_cfg(test->method);
    test_update_cfg(test, lc);
    test->procs = wtk_calloc(test->nproc, sizeof(test->procs[0]));
    test->proc_hash = qtk_int_hashtable_new(test->nproc * 1.2, offsetof(test_proc_t, sock_id));
    qtk_event_timer_cfg_init(&test->timer_cfg);
    test->timer = qtk_event_timer_new(&test->timer_cfg);
    qtk_event_timer_init(test->timer, &test->timer_cfg);
    qtk_rlimit_set_nofile(1024*1024);
    return test;
}


int ping_start(test_bench_t *test) {
    int i;
    for (i = 0; i < test->nproc; i++) {
        test_proc_init(test->procs + i, test, test->device_start_id + i);
        test_proc_start(test->procs + i);
    }
    test->run = 1;
    wtk_thread_start(&test->thread);
    return 0;
}


int ping_stop(test_bench_t *test) {
    int i;
    test->run = 0;
    wtk_thread_join(&test->thread);
    for (i = 0; i < test->nproc; i++) {
        test_proc_clean(test->procs + i);
    }
    return 0;
}


int ping_delete(test_bench_t *test) {
    qtk_event_timer_delete(test->timer);
    qtk_hashtable_delete(test->proc_hash);
    wtk_free(test->procs);
    wtk_free(test);
    return 0;
}
