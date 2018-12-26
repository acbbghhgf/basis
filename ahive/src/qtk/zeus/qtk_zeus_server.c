#include "qtk_zeus_server.h"
#include "stdarg.h"
#include "qtk_zeus.h"
#include "wtk/core/wtk_alloc.h"
#include "qtk_zeus_mq.h"
#include "qtk_zeus_timer.h"
#include "qtk_zeus_module.h"
#include "qtk_zeus_handle.h"
#include "qtk/core/qtk_atomic.h"
#include "qtk/core/qtk_deque.h"
#include "qtk_zsocket_server.h"
#include "lzzip.h"
#include "luaconf.h"


typedef struct qtk_zcontext qtk_zcontext_t;
typedef struct qtk_zmesg_drop qtk_zmesg_drop_t;
typedef struct qtk_thread_key_data qtk_thread_key_data_t;
typedef struct qtk_zcommand_func qtk_zcommand_func_t;


struct qtk_zcommand_func {
    const char *name;
    const char *(*func)(qtk_zcontext_t*,const char*);
};


struct qtk_zcontext {
    void *instance;
    qtk_zmodule_t *mod;
    qtk_zmesg_queue_t *queue;
    void *cb_ud;
    qtk_zeus_cb cb;
    uint32_t handle;
    int session_id;
    wtk_lock_t lock;
    char result[32];
    int ref;
    const char *name;           /* just for zhandle to get name */
};


struct qtk_zmesg_drop {
    uint32_t handle;
};


struct qtk_thread_key_data {
    qtk_zserver_t *zserver;
    pthread_cond_t *cond;
    uint32_t handle;
};


static qtk_zserver_t *zserver;


qtk_zserver_t* qtk_zserver_g() {
    return zserver;
}


static void _zcontext_exit(qtk_zcontext_t *ctx, uint32_t handle) {
    if (0 == handle) {
        handle = ctx->handle;
        qtk_zerror(ctx, "KILL self");
    } else {
        qtk_zerror(ctx, "KILL :%08x", handle);
    }
    if (zserver->monitor_exit) {
        qtk_zsend(ctx, handle, zserver->monitor_exit, ZEUS_PTYPE_CLIENT, 0, NULL, 0);
    }
    qtk_zhandle_retire(zserver->handles, handle);
}


static uint32_t _zcontext_tohandle(qtk_zcontext_t *ctx, const char *param) {
    uint32_t handle = 0;
    if (param[0] == '.') {
        handle = qtk_zhandle_findname(zserver->handles, param+1);
    } else if (param[0] == ':') {
        handle = strtol(param+1, NULL, 16);
    } else {
        qtk_zerror(ctx, "Cannot convert %s to handle", param);
    }
    return handle;
}


static const char* _cmd_timeout(qtk_zcontext_t *ctx, const char *param) {
    int time = strtol(param, NULL, 10);
    int session = qtk_zcontext_newsession(ctx);
    qtk_ztimer_timeout(zserver->timer, ctx->handle, time, session);
    sprintf(ctx->result, "%d", session);
    return ctx->result;
}


static const char* _cmd_reg(qtk_zcontext_t *ctx, const char *param) {
    if (param == NULL || param[0] == '\0') {
        sprintf(ctx->result, ":%x", ctx->handle);
        return ctx->result;
    } else if (param[0] == '.') {
        return qtk_zhandle_namehandle(zserver->handles, ctx->handle, param+1);
    } else {
        qtk_zerror(ctx, "Cannot register global name %s in C", param);
        return NULL;
    }
}


static const char* _cmd_query(qtk_zcontext_t *ctx, const char *param) {
    if (param[0] == '.') {
        uint32_t handle = qtk_zhandle_findname(zserver->handles, param+1);
        if (handle) {
            sprintf(ctx->result, ":%x", handle);
            return ctx->result;
        }
    }
    return NULL;
}


static const char* _cmd_getname(qtk_zcontext_t *ctx, const char *param) {
    uint32_t handle;
    if (1 == sscanf(param, "%x", &handle)) {
        qtk_zcontext_t *c = qtk_zhandle_grab(zserver->handles, handle);
        if (c) {
            return qtk_zcontext_name(c);
        }
    }
    return NULL;
}


static const char* _cmd_name(qtk_zcontext_t *ctx, const char *param) {
    char name[strlen(param)];
    uint32_t handle;
    if (2 == sscanf(param, ".%s :%x", name, &handle)) {
        return qtk_zhandle_namehandle(zserver->handles, handle, name);
    } else {
        qtk_zerror(ctx, "Invalid NAME param: %s", param);
    }
    return NULL;
}


static const char* _cmd_exit(qtk_zcontext_t *ctx, const char *param) {
    _zcontext_exit(ctx, 0);
    return NULL;
}


static const char* _cmd_kill(qtk_zcontext_t *ctx, const char *param) {
    uint32_t handle = _zcontext_tohandle(ctx, param);
    if (handle) {
        _zcontext_exit(ctx, handle);
    }
    return NULL;
}


static const char* _cmd_launch(qtk_zcontext_t *ctx, const char *param) {
    size_t sz = strlen(param);
    char tmp[sz+1];
    strcpy(tmp, param);
    char *args = tmp;
    char *mod = strsep(&args, " \t\r\n");
    args = strsep(&args, "\r\n");
    qtk_zcontext_t *inst = qtk_zcontext_new(zserver, mod, args);
    if (inst) {
        sprintf(ctx->result, ":%x", inst->handle);
        return ctx->result;
    } else {
        return NULL;
    }
}


static const char* _cmd_abort(qtk_zcontext_t *ctx, const char *param) {
    qtk_zhandle_retireall(zserver->handles);
    return NULL;
}


static qtk_zcommand_func_t cmd_funcs[] = {
    {"TIMEOUT", _cmd_timeout},
    {"REG", _cmd_reg},
    {"QUERY", _cmd_query},
    {"NAME", _cmd_name},
    {"GETNAME", _cmd_getname},
    {"EXIT", _cmd_exit},
    {"KILL", _cmd_kill},
    {"LAUNCH", _cmd_launch},
    {"ABORT", _cmd_abort},
    {NULL, NULL}
};


static void _zcontext_inc(qtk_zserver_t *z) {
    QTK_ATOM_INC(&z->nctx);
}


static void _zcontext_dec(qtk_zserver_t *z) {
    QTK_ATOM_DEC(&z->nctx);
}


static void _zcontext_delete(qtk_zserver_t *z, qtk_zcontext_t *ctx) {
    qtk_zmodule_instance_release(ctx->mod, ctx->instance);
    qtk_zmq_mark_release(z->mq, ctx->queue);
    qtk_zcontext_set_name(ctx, NULL);
    wtk_lock_clean(&ctx->lock);
    wtk_free(ctx);
    _zcontext_dec(z);
}


static void _zcontext_dispatch_mesg(qtk_zcontext_t *ctx, qtk_zmesg_t *msg) {
    wtk_lock_lock(&ctx->lock);
    int type = msg->sz >> ZEUS_TYPE_SHIFT;
    int sz = msg->sz & ZEUS_MSG_SIZE_MASK;
    if (!ctx->cb(ctx, ctx->cb_ud, type, msg->session, msg->source, msg->data, sz)) {
        wtk_free(msg->data);
    }
    wtk_lock_unlock(&ctx->lock);
}


static void _drop_mesg(qtk_zmesg_t *msg, void *ud) {
    qtk_zmesg_drop_t *d = (qtk_zmesg_drop_t*)ud;
    wtk_free(msg->data);
    qtk_zsend(NULL, d->handle, msg->source, ZEUS_PTYPE_ERROR, 0, NULL, 0);
}


static int _thread_timer(void *p, wtk_thread_t *t) {
    qtk_zserver_t *z = (qtk_zserver_t*)p;
    while (!z->quit) {
        qtk_ztimer_update(z->timer);
        if (0 == z->nctx) break;
        usleep(2500);
    }
    pthread_mutex_lock(&z->msg_mutex);
    z->quit = 1;
    pthread_cond_broadcast(&z->msg_cond);
    pthread_mutex_unlock(&z->msg_mutex);
    return 0;
}


static int _thread_socket(void *p, wtk_thread_t *t) {
    qtk_zserver_t *z = (qtk_zserver_t*)p;
    while (!z->quit) {
        qtk_zsocket_server_process(z->socket_server);
    }
    return 0;
}


static int _thread_worker(void *p, wtk_thread_t *t) {
    qtk_zserver_t *z = (qtk_zserver_t*)p;
    qtk_zmesg_queue_t *q = NULL;
    wtk_debug("start...\r\n");
    luaZ_open(LUA_BPATH);
    qtk_zserver_log("worker thread [%u] start", pthread_self());
    while (1) {
        q = qtk_zserver_dispatch(z, q);
        if (NULL == q) {
            if (z->quit) {
                break;
            } else {
                if (!pthread_mutex_lock(&z->msg_mutex)) {
                    ++z->nsleep;
                    pthread_cond_wait(&z->msg_cond, &z->msg_mutex);
                    --z->nsleep;
                    pthread_mutex_unlock(&z->msg_mutex);
                }
            }
        }
    }
    wtk_debug("quit...\r\n");
    qtk_zserver_log("worker thread [%u] quit", pthread_self());
    luaZ_close();
    return 0;
}


int qtk_zsend(qtk_zcontext_t *ctx, uint32_t src, uint32_t dst, int type, int session,
              void *data, size_t sz) {
    int dontcopy = type & ZEUS_PTYPE_TAG_DONTCOPY;
    int allocsession = type & ZEUS_PTYPE_TAG_ALLOCSESSION;
    if (sz > ZEUS_MSG_SIZE_MASK) {
        qtk_zerror(ctx, "The message to %x is too large", dst);
        if (dontcopy) {
            wtk_free((void*)data);
        }
        return -1;
    }
    type &= 0xff;
    if (allocsession) {
        session = qtk_zcontext_newsession(ctx);
    }
    if (!dontcopy && data) {
        char *cp = wtk_malloc(sz+1);
        memcpy(cp, data, sz);
        cp[sz] = '\0';
        data = cp;
    }
    sz |= (size_t)(type & 0xff) << ZEUS_TYPE_SHIFT;
    if (src == 0) {
        src = ctx->handle;
    }
    if (dst) {
        qtk_zmesg_t msg;
        msg.source = src;
        msg.session = session;
        msg.data = data;
        msg.sz = sz;
        if (qtk_zcontext_push(dst, &msg)) {
            wtk_free(data);
            return -1;
        }
    }
    return session;
}


int qtk_zsendname(qtk_zcontext_t *ctx, uint32_t src, const char *addr, int type,
                  int session, void *data, size_t sz) {
    uint32_t dst = _zcontext_tohandle(ctx, addr);
    if (0 == dst) {
        int dontcopy = type & ZEUS_PTYPE_TAG_DONTCOPY;
        if (dontcopy) {
            wtk_free(data);
        }
        return session;
    } else {
        return qtk_zsend(ctx, src, dst, type, session, data, sz);
    }
}


void qtk_zerror(qtk_zcontext_t *ctx, const char *msg, ...) {
    static uint32_t logger = 0;
    if (0 == logger) {
        logger = qtk_zhandle_findname(zserver->handles, "logger");
    }
    if (0 == logger) {
        wtk_debug("no logger found\r\n");
        qtk_zserver_log0("no logger module found");
        return;
    }

    char tmp[ZEUS_LOG_MESG_SIZE];
    va_list ap;
    va_start(ap, msg);
    int len = vsnprintf(tmp, ZEUS_LOG_MESG_SIZE, msg, ap);
    va_end(ap);
    if (len < 0) {
        wtk_debug("vsnprintf failed\r\n");
        qtk_zserver_log0("vsnprintf failed");
        return;
    }
    if (len >= ZEUS_LOG_MESG_SIZE) {
        tmp[ZEUS_LOG_MESG_SIZE-1] = '\0';
        len = ZEUS_LOG_MESG_SIZE - 1;
    }
    char *data;
    data = wtk_malloc(len+1);
    memcpy(data, tmp, len+1);
    qtk_zmesg_t zmsg;
    if (ctx) {
        zmsg.source = qtk_zcontext_handle(ctx);
    } else {
        zmsg.source = 0;
    }
    zmsg.session = 0;
    zmsg.data = data;
    zmsg.sz = len | (ZEUS_PTYPE_TEXT << ZEUS_TYPE_SHIFT);
    qtk_zcontext_push(logger, &zmsg);
}


uint64_t qtk_zeus_now() {
    return qtk_ztimer_now(zserver->timer);
}


uint32_t qtk_zcontext_handle(qtk_zcontext_t *ctx) {
    return ctx->handle;
}


uint32_t* qtk_zcontext_handlep(qtk_zcontext_t* ctx) {
    return &ctx->handle;
}


const char* qtk_zcontext_name(qtk_zcontext_t *ctx) {
    return ctx->name;
}


void qtk_zcontext_set_name(qtk_zcontext_t *ctx, const char *name) {
    if (ctx->name) {
        wtk_free((void*)(ctx->name));
    }
    if (name) {
        size_t name_sz = strlen(name);
        char *name_new = wtk_malloc(name_sz+1);
        strcpy(name_new, name);
        ctx->name = name_new;
    } else {
        ctx->name = NULL;
    }
}


void qtk_zcontext_grab(struct qtk_zcontext *ctx) {
    QTK_ATOM_INC(&ctx->ref);
}


qtk_zcontext_t* qtk_zcontext_release(qtk_zcontext_t *ctx) {
    if (QTK_ATOM_DEC(&ctx->ref) == 0) {
        _zcontext_delete(zserver, ctx);
        return NULL;
    }
    return ctx;
}


qtk_zcontext_t *qtk_zcontext_new(qtk_zserver_t *z, const char *name, const char *param) {
    qtk_zmodule_t *mod = qtk_zmodule_query(z->modules, name);
    if (mod == NULL) return NULL;

    void *inst = qtk_zmodule_instance_create(mod);
    if (inst == NULL) return NULL;
    qtk_zcontext_t *ctx = wtk_malloc(sizeof(*ctx));
    wtk_lock_init(&ctx->lock);
    ctx->mod = mod;
    ctx->instance = inst;
    ctx->cb_ud = NULL;
    ctx->cb = NULL;
    ctx->session_id = 0;
    ctx->ref = 2;
    ctx->name = NULL;
    ctx->handle = 0;
    ctx->handle = qtk_zhandle_register(z->handles, ctx);
    qtk_zmesg_queue_t *queue = ctx->queue = qtk_zmq_newq(ctx->handle);
    _zcontext_inc(z);
    wtk_lock_lock(&ctx->lock);
    int r = qtk_zmodule_instance_init(ctx->mod, inst, ctx, param);
    wtk_lock_unlock(&ctx->lock);
    if (r) {
        wtk_debug("FAILED launch %s %s\r\n", name, param ? param : "");
        qtk_zerror(NULL, "FAILED launch %s %s", name, param ? param : "");
        uint32_t handle = ctx->handle;
        qtk_zcontext_release(ctx);
        qtk_zhandle_retire(z->handles, handle);
        qtk_zmesg_drop_t drop = {handle};
        qtk_zmq_release(z->mq, queue, _drop_mesg, &drop);
        ctx = NULL;
    } else {
        ctx = qtk_zcontext_release(ctx);
        qtk_zmq_pushq(z->mq, queue);
        if (ctx) {
            wtk_debug("LAUNCH %s %s\r\n", name, param ? param : "");
            qtk_zerror(NULL, "LAUNCH %s %s", name, param ? param : "");
        }
    }
    return ctx;
}


void qtk_zcontext_dispatchall(qtk_zcontext_t *ctx) {
    qtk_zmesg_queue_t *queue = ctx->queue;
    qtk_zmesg_t msg;
    while (0 == qtk_zmq_pop(queue, &msg)) {
        _zcontext_dispatch_mesg(ctx, &msg);
    }
}


int qtk_zcontext_push(uint32_t handle, qtk_zmesg_t *msg) {
    qtk_zcontext_t *ctx = qtk_zhandle_grab(zserver->handles, handle);
    if (NULL == ctx) {
        return -1;
    }
    qtk_zmq_push(ctx->queue, msg, zserver);
    qtk_zcontext_release(ctx);
    return 0;
}


int qtk_zcontext_newsession(struct qtk_zcontext *ctx) {
    int session = ++ctx->session_id;
    if (session <= 0) {
        ctx->session_id = 1;
        return 1;
    }
    return session;
}


qtk_zserver_t* qtk_zserver_new(qtk_zeus_cfg_t *cfg, qtk_log_t *log) {
    int i;
    qtk_zserver_t *z = (qtk_zserver_t*)wtk_malloc(sizeof(qtk_zserver_t));
    z->cfg = cfg;
    z->log = log;
    z->nctx = 0;
    z->handles = qtk_zhandle_new();
    z->timer = qtk_ztimer_new();
    z->mq = qtk_zmq_new();
    z->modules = qtk_zmodule_new("./");
    z->socket_server = qtk_zsocket_server_new(1000);
    pthread_mutex_init(&z->msg_mutex, NULL);
    pthread_cond_init(&z->msg_cond, NULL);
    z->nthread = 8;
    z->threads = wtk_calloc(z->nthread, sizeof(z->threads[0]));
    for (i = 0; i < z->nthread; ++i) {
        wtk_thread_init(z->threads+i, _thread_worker, z);
    }
    wtk_thread_init(&z->timer_thread, _thread_timer, z);
    wtk_thread_init(&z->socket_thread, _thread_socket, z);
    z->nsleep = 0;
    z->quit = 0;
    z->monitor_exit = 0;
    zserver = z;
    return z;
}


qtk_log_t* qtk_zserver_get_log() {
    return zserver->log;
}


int qtk_zserver_start(qtk_zserver_t *z) {
    qtk_zcontext_t *ctx = qtk_zcontext_new(z, "logger", z->cfg->log_fn);
    if (NULL == ctx) {
        wtk_debug("cannot launch logger service\r\n");
        qtk_zserver_log("cannot launch logger[fn=%s] service", z->cfg->log_fn);
        exit(1);
    }
    luaZ_global_init();
    ctx = qtk_zcontext_new(z, "zlua", "bootstrap");
    if (NULL == ctx) {
        wtk_debug("Bootstrip error\r\n");
        qtk_zerror(NULL, "Bootstrap error\r\n");
        qtk_zcontext_dispatchall(ctx);
        exit(1);
    }

    int i;
    wtk_thread_start(&z->timer_thread);
    wtk_thread_start(&z->socket_thread);
    for (i = 0; i < z->nthread; ++i) {
        wtk_thread_start(z->threads + i);
    }
    return 0;
}


int qtk_zserver_join(qtk_zserver_t *z) {
    int i;
    for (i = 0; i < z->nthread; ++i) {
        wtk_thread_join(z->threads+i);
    }
    wtk_thread_join(&z->timer_thread);
    wtk_thread_join(&z->socket_thread);
    return 0;
}


int qtk_zserver_stop(qtk_zserver_t *z) {
    /* retire all context */
    qtk_zcommand(NULL, "ABORT", NULL);
    z->quit = 1;
    pthread_cond_broadcast(&z->msg_cond);
    qtk_zserver_join(z);
    luaZ_global_clean();
    return 0;
}


int qtk_zserver_delete(qtk_zserver_t *z) {
    int i;
    for (i = 0; i < z->nthread; ++i) {
        wtk_thread_clean(z->threads+i);
    }
    wtk_thread_clean(&z->timer_thread);
    wtk_thread_clean(&z->socket_thread);
    if (z->socket_server) {
        qtk_zsocket_server_delete(z->socket_server);
        z->socket_server = NULL;
    }
    if (z->modules) {
        qtk_zmodule_delete(z->modules);
        z->modules = NULL;
    }
    if (z->mq) {
        qtk_zmq_delete(z->mq);
        z->mq = NULL;
    }
    if (z->timer) {
        qtk_ztimer_delete(z->timer);
        z->timer = NULL;
    }
    if (z->handles) {
        qtk_zhandle_delete(z->handles);
    }
    if (z->threads) {
        wtk_free(z->threads);
        z->threads = NULL;
    }
    pthread_mutex_destroy(&z->msg_mutex);
    pthread_cond_destroy(&z->msg_cond);
    wtk_free(z);
    return 0;
}


qtk_zmesg_queue_t* qtk_zserver_dispatch(qtk_zserver_t *z, qtk_zmesg_queue_t *q) {
    if (NULL == q) {
        q = qtk_zmq_popq(z->mq);
        if (NULL == q) {
            return NULL;
        }
    }

    uint32_t handle = qtk_zmq_handle(q);
    qtk_zcontext_t *ctx = qtk_zhandle_grab(z->handles, handle);
    if (NULL == ctx) {
        qtk_zmesg_drop_t drop = {handle};
        qtk_zmq_release(z->mq, q, _drop_mesg, &drop);
        return qtk_zmq_popq(z->mq);
    }

    qtk_zmesg_t msg;
    int r;
    while (!(r = qtk_zmq_pop(q, &msg))) {
        if (ctx->cb) {
            _zcontext_dispatch_mesg(ctx, &msg);
        } else {
            wtk_free(msg.data);
        }
    }

    qtk_zmesg_queue_t *nq = qtk_zmq_popq(z->mq);
    if (r) {
        q = NULL;
    }
    if (nq) {
        if (q) {
            qtk_zmq_pushq(z->mq, q);
        }
        q = nq;
    }

    qtk_zcontext_release(ctx);
    return q;
}


void qtk_zcallback(qtk_zcontext_t *ctx, void *ud, qtk_zeus_cb cb) {
    ctx->cb = cb;
    ctx->cb_ud = ud;
}


const char* qtk_zcommand(qtk_zcontext_t *ctx, const char *cmd, const char *param) {
    qtk_zcommand_func_t *method = &cmd_funcs[0];
    while (method->name) {
        if (!strcmp(method->name, cmd)) {
            return method->func(ctx, param);
        }
        ++method;
    }
    return NULL;
}


int qtk_zsocket_open(uint32_t src, const char *addr, size_t sz, int mode) {
    return qtk_ss_cmd_open(zserver->socket_server, src, addr, sz, mode);
}


void qtk_zsocket_close(uint32_t src, int id) {
    qtk_ss_cmd_close(zserver->socket_server, src, id);
}


void qtk_zsocket_send(uint32_t src, int id, const char *msg, size_t sz) {
    qtk_ss_cmd_send(zserver->socket_server, src, id, msg, sz);
}


void qtk_zsocket_send_buffer(uint32_t src, int id, qtk_deque_t *dq) {
    qtk_ss_cmd_send_buffer(zserver->socket_server, src, id, dq);
}


int qtk_zsocket_listen(uint32_t src, const char *addr, size_t sz, int backlog) {
    return qtk_ss_cmd_listen(zserver->socket_server, src, addr, sz, backlog);
}
