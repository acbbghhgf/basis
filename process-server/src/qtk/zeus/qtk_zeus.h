#ifndef QTK_ZEUS_QTK_ZEUS_H
#define QTK_ZEUS_QTK_ZEUS_H
#include "stdio.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ZEUS_PTYPE_TEXT 0
#define ZEUS_PTYPE_RESPONSE 1
#define ZEUS_PTYPE_SYSTEM 2
#define ZEUS_PTYPE_CLIENT 3
#define ZEUS_PTYPE_SOCKET 6
#define ZEUS_PTYPE_ERROR 7
#define ZEUS_PTYPE_LUA 10
#define ZEUS_PTYPE_SRVX 11

#define ZEUS_PTYPE_TAG_DONTCOPY 0x10000
#define ZEUS_PTYPE_TAG_ALLOCSESSION 0x20000

#define ZEUS_TYPE_SHIFT 24
#define ZEUS_MSG_SIZE_MASK ((1<<ZEUS_TYPE_SHIFT)-1)
#define ZEUS_LOG_MESG_SIZE 1024


struct qtk_zcontext;

typedef int (*qtk_zeus_cb)(struct qtk_zcontext *ctx, void *ud, int type, int session,
                           uint32_t src, const char *msg, size_t sz);

int qtk_zsend(struct qtk_zcontext *ctx, uint32_t src, uint32_t dst, int type,
              int session, void *data, size_t sz);
int qtk_zsendname(struct qtk_zcontext *ctx, uint32_t src, const char *addr, int type,
                  int session, void *data, size_t sz);
void qtk_zerror(struct qtk_zcontext *ctx, const char *msg, ...);
void qtk_zcallback(struct qtk_zcontext *ctx, void *ud, qtk_zeus_cb cb);
const char* qtk_zcommand(struct qtk_zcontext *ctx, const char *cmd, const char *param);
uint64_t qtk_zeus_now();

#ifdef __cplusplus
}
#endif

#endif
