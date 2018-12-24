#include "qtk_bifrost_main.h"
#include "qtk_context.h"
#include "qtk_timer.h"
#include "qtk_handle.h"
#include "qtk_mq.h"
#include "qtk_bifrost.h"
#include "os/qtk_base.h"
#include "os/qtk_alloc.h"

#include <stdlib.h>
#include <string.h>

struct qtk_command_func {
    const char *name;
    const char *(*func)(qtk_context_t *ctx, const char *param);
};


static uint32_t param_tohandle(qtk_context_t *ctx, const char *param)
{
    uint32_t handle = 0;
    if (param[0] == ':') {
        handle = strtoul(param+1, NULL, 16);
    } else if (param[0] == '.') {
        handle = qtk_handle_findname(param + 1);
    } else {
        qtk_debug("Can't convert %s to handle\n", param);
    }
    return handle;
}


static const char *cmd_timeout(qtk_context_t *ctx, const char *param)
{
    int time = atoi(param);
    int session = qtk_context_newsession(ctx);
    qtk_timeout(CTX_GET_HANDLE(ctx), time, session);
    snprintf(ctx->result, sizeof(ctx->result), "%d", session);

    return ctx->result;
}


static const char *cmd_reg(qtk_context_t *ctx, const char *param)
{
    if (NULL == param || param[0] == '\0') {
        snprintf(ctx->result, sizeof(ctx->result), ":%x", CTX_GET_HANDLE(ctx));
        return ctx->result;
    } else if (param[0] == '.') {
        return qtk_handle_namehandle(CTX_GET_HANDLE(ctx), param + 1);
    } else {
        qtk_debug("Can't register global name %s\n", param);
        return NULL;
    }
}


static const char *cmd_launch(qtk_context_t *ctx, const char *param)
{
    int len = strlen(param);
    char tmp[len+1];
    memcpy(tmp, param, len+1);
    char *arg = tmp;
    char *mod = strsep(&arg, " \t");
    qtk_context_t *new = qtk_context_new(mod, arg);
    if (NULL == new) {
        qtk_debug("error LAUNCH %s\n", param);
        return NULL;
    } else {
        snprintf(ctx->result, sizeof(ctx->result), ":%x", CTX_GET_HANDLE(new));
        return ctx->result;
    }
}


static const char *cmd_exit(qtk_context_t *ctx, const char *param)
{
    qtk_handle_retire(CTX_GET_HANDLE(ctx));
    return NULL;
}


static const char *cmd_kill(qtk_context_t *ctx, const char *param)
{
    uint64_t handle = param_tohandle(ctx, param);
    qtk_handle_retire(handle);

    return NULL;
}


static const char *cmd_name(qtk_context_t *ctx, const char *param)
{
    int size = strlen(param);
    char name[size + 1];
    char handle[size + 1];
    sscanf(param, "%s %s", name, handle);

    if (name[0] != '.' || handle[0] != ':') {
        qtk_debug("Can't not name %s\n", param);
        return NULL;
    }

    uint32_t handle_id = strtoul(handle+1, NULL, 16);

    return qtk_handle_namehandle(handle_id, name + 1);
}


static const char *cmd_query(qtk_context_t *ctx, const char *param)
{
    if (param[0] == '.') {
        uint32_t handle = qtk_handle_findname(param+1);
        if (handle != 0) {
            snprintf(ctx->result, sizeof(ctx->result), ":%x", handle);
            return ctx->result;
        }
    }
    return NULL;
}


static struct qtk_command_func cmd_funcs[] = {
    { "TIMEOUT", cmd_timeout },
    { "LAUNCH",  cmd_launch },
    { "EXIT",    cmd_exit },
    { "KILL",    cmd_kill},
    { "REG",     cmd_reg },
    { "NAME",    cmd_name },
    { "QUERY",   cmd_query },
    { NULL,      NULL},
};


const char *qtk_command(qtk_context_t *ctx, const char *cmd, const char *param) //根据cmd找出对应的处理函数
{
    struct qtk_command_func *method = NULL;

    for (method = &cmd_funcs[0]; method->name; method ++) {
        if (0 == strcmp(method->name, cmd)) { return method->func(ctx, param); }
    }
    qtk_debug("cmd [%s] not found\n", cmd);

    return NULL;
}


/* processing QTK_TAG_* */
static void _filter_args(qtk_context_t *ctx, int type, int *session, void **data, size_t *sz)
{
    int allc_session = type & QTK_TAG_ALLOCSESSION;
    int need_copy = !(type & QTK_TAG_DONOTCOPY);
    if (allc_session) {
        *session = qtk_context_newsession(ctx);
    }

    if (need_copy) {
        char *msg = qtk_malloc(*sz + 1);
        memcpy(msg, *data, *sz);
        msg[*sz] = '\0';
        *data = msg;
    }

    type &= 0xFF;
    *sz |= (size_t)type << MESSAGE_TYPE_SHIFT;
}


int qtk_bifrost_send(qtk_context_t *ctx, uint32_t source, uint32_t dest,
                     int type, int session, void *data, size_t sz)
{
    if ((sz & MESSAGE_TYPE_MASK) != sz) {       //报错
        qtk_debug("message too large\n");
        if (type & QTK_TAG_DONOTCOPY) { qtk_free(data); }
        return -1;
    }

    _filter_args(ctx, type, &session, (void **)&data, &sz);

    if (0 == dest) { return session; }
    source = source > 0 ? source : CTX_GET_HANDLE(ctx);

    qtk_message_t msg;
    msg.source = source;
    msg.session = session;
    msg.data = data;
    msg.sz = sz;

    if (qtk_context_push(dest, &msg)) {
        qtk_debug("error context send, dest is %d\n", (int)dest);
        qtk_free(data);
        return -1;
    }

    return session;
}


int qtk_bifrost_sendname(qtk_context_t *ctx, uint32_t source, const char *dest,
                         int type, int session, void *data, size_t sz)
{
    uint32_t dest_handle = 0;

    if (dest[0] == ':') {
        dest_handle = strtoul(dest + 1, NULL, 16);
    } else if (dest[0] == '.') {
        dest_handle = qtk_handle_findname(dest + 1);
    }

    if (0 == dest_handle) {
        qtk_debug("can't find dest [%s], source [%d]\n", dest, CTX_GET_HANDLE(ctx));
        if (type & QTK_TAG_DONOTCOPY) { qtk_free(data); }
        return -1;
    }

    return qtk_bifrost_send(ctx, source, dest_handle, type, session, data, sz);
}

int qtk_bifrost_log(qtk_context_t *ctx, const char *level, const char *msg)
{
    char tmp[4096];
    char time[32];
    time_t now = qtk_timer_now() / 100 + qtk_timer_starttime();
    struct tm tm;
    strftime(time, sizeof(time), "{%a %b %d %T %G}--", localtime_r(&now, &tm));
    int len = snprintf(tmp, sizeof(tmp), "%s [%s] %s", time, level, msg);
    return qtk_bifrost_sendname(ctx, ctx->handle, ".logger", QTK_TAG_ALLOCSESSION, 0, tmp, len);
}
