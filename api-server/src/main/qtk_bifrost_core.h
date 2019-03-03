#ifndef MAIN_QTK_BIFROST_CORE_H__
#define MAIN_QTK_BIFROST_CORE_H__

#include "qtk_context.h"

const char *qtk_command(struct qtk_context *ctx, const char *cmd, const char *param);
int qtk_bifrost_send(qtk_context_t *ctx, uint32_t source, uint32_t dest,
                     int type, int session, void *data, size_t sz);
int qtk_bifrost_sendname(qtk_context_t *ctx, uint32_t source, const char *dest,
                         int type, int session, void *data, size_t sz);
int qtk_bifrost_log(qtk_context_t *ctx, const char *level, const char *msg);

#endif
