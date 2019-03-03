#include "qtk_bifrost_main.h"
#include "os/qtk_base.h"
#include "os/qtk_alloc.h"
#include "os/qtk_limit.h"
#include "qtk_mq.h"
#include "qtk_timer.h"
#include "qtk_context.h"
#include "qtk_bifrost_core.h"
#include "qtk_module.h"
#include "qtk_handle.h"
#include "qtk_env.h"
#include "qtk/sframe/qtk_sframe.h"
#include "qtk/http/qtk_request.h"

#include "third/lua5.3.4/src/lua.h"
#include "third/lua5.3.4/src/lualib.h"
#include "third/lua5.3.4/src/lauxlib.h"


#include <unistd.h>

struct bifrost_node {
    int total;
    qtk_bifrost_t *self;
};

static struct bifrost_node G_NODE;


int qtk_context_total()
{
    return G_NODE.total;
}


void qtk_context_inc()
{
    QTK_ATOM_INC(&G_NODE.total);
}


void qtk_context_dec()
{
    QTK_ATOM_DEC(&G_NODE.total);
}

qtk_bifrost_t *qtk_bifrost_self()
{
    return G_NODE.self;
}

static void qtk_global_init(qtk_bifrost_t *b)
{
    G_NODE.total = 0;
    G_NODE.self = b;
}

static void qtk_bifrost_dispatch_message(qtk_context_t *ctx, qtk_message_t *msg)
{
    int reserve_msg = 1;
    int type = msg->sz >> MESSAGE_TYPE_SHIFT;
    size_t sz = msg->sz & MESSAGE_TYPE_MASK;

    if (ctx->cb) {
        reserve_msg = ctx->cb(ctx, ctx->ud, type, msg->session, msg->source, msg->data, sz);
        if (!reserve_msg) { qtk_free(msg->data); }
    } else {
        qtk_free(msg->data);
    }
}


static void *qtk_bifrost_worker_route(void *arg)    //two big queue
{
    qtk_message_queue_t *q = NULL;
    qtk_message_t msg;
    uint32_t handle = 0;
    qtk_context_t *ctx = NULL;

    while (1) {
        q = qtk_globalmq_pop();
        if (NULL == q) {
            qtk_debug("thread exit\n");
            break;
        }

        handle = MQ_GET_HANDLE(q);
        ctx = qtk_handle_grab(handle);
        if (NULL == ctx) {
            qtk_mq_release(q, NULL, NULL);
            continue;
        }

        if (qtk_mq_pop(q, &msg)) {
            qtk_context_release(ctx);
            continue;
        }

        qtk_bifrost_dispatch_message(ctx, &msg);
        qtk_globalmq_push(q);
        qtk_context_release(ctx);
    }

    return NULL;
}


static void *qtk_bifrost_timer_route(void *arg)
{
    qtk_bifrost_t *b = arg;

    for ( ;b->run; ) {
        qtk_updatetime();
        if (0 == qtk_context_total()) {
            qtk_debug("timer break\n");
            qtk_debug("upload thread exit\n");
            break;
        }
        usleep(2500);
    }

    b->run = 0;
    int i, nworker;
    nworker = qtk_optnumber("nworker", 1);
    for (i=0; i<nworker; i++) {
        qtk_globalmq_touch();
    }

    return NULL;
}

static int qtk_bifrost_init(qtk_bifrost_t *b)
{
    qtk_globalmq_init();
    qtk_timer_init();
    qtk_module_init("./cservice/?.so");
    qtk_handle_init();
    qtk_global_init(b);

    int i, nworker;
    nworker = qtk_optnumber("nworker", 1);
    for (i=0; i<nworker; i++) {
        qtk_thread_init(&b->workers[i], qtk_bifrost_worker_route, b);
    }
    qtk_thread_init(&b->timer_thread, qtk_bifrost_timer_route, b);

    return 0;
}


qtk_bifrost_t *qtk_bifrost_new(void *gate)
{
    qtk_bifrost_t *b = qtk_calloc(1, sizeof(qtk_bifrost_t));
    if (NULL == b) { return NULL; }
    b->gate = gate;
    qtk_env_init();
    char *cfn = "../ext/bifrost.cfg";
    if (qtk_env_update(cfn, strlen(cfn))) { return NULL; }
    int nworker = qtk_optnumber("nworker", 1);
    b->workers = qtk_calloc(1, nworker * sizeof(qtk_thread_t));
    b->up = qtk_calloc(1, sizeof(qtk_upload_mode_t));
    qtk_bifrost_init(b);

    return b;
}


int qtk_bifrost_start(qtk_bifrost_t *b)
{
    int i, nworker;

    int max_files = qtk_optnumber("maxFd", 65536);
    qtk_limit_set_maxfiles(max_files);

    b->run = 1;
    nworker = qtk_optnumber("nworker", 1);
    for (i=0; i<nworker; i++) {
        qtk_thread_start(&b->workers[i]);
    }

    const char *lpath = qtk_optstring("logPath", "bifrost.log");//lpath = "/home/ww/bifrost/ext/bifrost.log"
    qtk_context_t *logger = qtk_context_new("logger", lpath);
    if (NULL == logger) {
        qtk_debug("error new logger\n");
    }
    qtk_command(logger, "REG", ".logger");
    qtk_thread_start(&b->timer_thread);

    qtk_context_t *bctx = qtk_context_new("blua", "bootstrap");
    if (bctx == NULL) {
        qtk_debug("error new bootstrap\n");
    }
    qtk_context_t *upctx = qtk_context_new("upload", NULL);
    if (upctx == NULL) {
        qtk_debug("error new upload\n");
    }
    return 0;
}


int qtk_bifrost_delete(qtk_bifrost_t *b)
{
    qtk_handle_clean();
    qtk_timer_clean();
    qtk_globalmq_clean();
    qtk_module_clean();
    qtk_env_clean();
    
    qtk_free(b->workers);
    qtk_free(b->up);
    qtk_free(b);

    return 0;
}


int qtk_bifrost_stop(qtk_bifrost_t *b)
{
    qtk_handle_retireall();

    int i, nworker;
    nworker = qtk_optnumber("nworker", 1);
    for (i=0; i<nworker; i++) {
        qtk_thread_join(&b->workers[i]);
    }
    qtk_thread_join(&b->timer_thread);

    return 0;
}
