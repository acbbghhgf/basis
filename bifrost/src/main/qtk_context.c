#include "qtk_context.h"
#include "qtk_bifrost_main.h"
#include "qtk_module.h"
#include "qtk_handle.h"
#include "qtk_mq.h"
#include "os/qtk_alloc.h"
#include "os/qtk_base.h"

qtk_context_t *qtk_context_new(const char *name, const char *param)
{
    qtk_module_t *mod = qtk_module_query(name);
    if (NULL == mod) { return NULL; }
    void *inst = qtk_module_instance_create(mod);
    if (NULL == inst) { return NULL; }

    qtk_context_t *ctx = qtk_calloc(1, sizeof(qtk_context_t));
    ctx->handle = qtk_handle_register(ctx);
    ctx->instance = inst;
    ctx->mod = mod;
    ctx->ref = 1;
    ctx->session_id = 0;
    qtk_message_queue_t *queue = ctx->queue = qtk_mq_create(ctx->handle);
    qtk_context_inc();

    int ret = qtk_module_instance_init(mod, inst, ctx, param);
    if (0 == ret) {
        qtk_globalmq_push(queue);
        return ctx;
    } else {
        qtk_handle_retire(CTX_GET_HANDLE(ctx));
        qtk_debug("%s instance failed, param is %s\n", name, param);
        return NULL;
    }
}


int qtk_context_newsession(qtk_context_t *ctx)
{
    int session = ++ctx->session_id;
    if (session <= 0) {
        ctx->session_id = 1;
        return 1;
    }

    return session;
}


void qtk_context_callback(qtk_context_t *ctx, qtk_context_cb cb, void *ud)
{
    ctx->cb = cb;
    ctx->ud = ud;
}


int qtk_context_push(uint32_t handle, qtk_message_t *msg)
{
    qtk_context_t *ctx = qtk_handle_grab(handle);
    if (NULL == ctx) { return -1; }
    qtk_mq_push(ctx->queue, msg);
    qtk_context_release(ctx);

    return 0;
}


static void qtk_context_delete(qtk_context_t *ctx)
{
    qtk_mq_mark_release(ctx->queue);
    qtk_module_instance_release(ctx->mod, ctx->instance);
    qtk_free(ctx);
    qtk_context_dec();
}


void qtk_context_release(qtk_context_t *ctx)
{
    if (QTK_ATOM_DEC(&ctx->ref) == 0) {
        qtk_context_delete(ctx);
    }
}
