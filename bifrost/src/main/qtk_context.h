#ifndef MAIN_QTK_CONTEXT_H_
#define MAIN_QTK_CONTEXT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "os/qtk_atomic.h"

#include <glob.h>
#include <stdint.h>

#define CTX_GET_HANDLE(ctx)          (ctx->handle)
#define qtk_context_grab(ctx)        QTK_ATOM_INC(&ctx->ref)

struct qtk_message;
typedef struct qtk_context qtk_context_t;
typedef int (*qtk_context_cb)(qtk_context_t *ctx, void *ud, int type, int session,
             uint32_t source, const void *msg, size_t sz);

struct qtk_context {
    uint32_t handle;
    void *instance;
    struct qtk_module *mod;
    struct qtk_message_queue *queue;
    qtk_context_cb cb;
    /* for generating msg session, if -1 == session, sender need no response */
    uint32_t session_id;
    char result[32];
    int ref;
    void *ud;
};

qtk_context_t *qtk_context_new(const char *name, const char *param);
void qtk_context_callback(qtk_context_t *ctx, qtk_context_cb cb, void *ud);
int qtk_context_push(uint32_t handle, struct qtk_message *msg);
void qtk_context_release(qtk_context_t *ctx);
int qtk_context_newsession(qtk_context_t *ctx);

#ifdef __cplusplus
};
#endif
#endif
