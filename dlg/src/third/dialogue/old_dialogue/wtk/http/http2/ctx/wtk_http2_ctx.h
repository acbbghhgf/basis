#ifndef WTK_HTTP2_CTX_WTK_HTTP2_CTX_H_
#define WTK_HTTP2_CTX_WTK_HTTP2_CTX_H_
#include "wtk/core/wtk_type.h"
#include "wtk_http2_ctx_cfg.h"
#include "wtk/os/wtk_lock.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_http2_ctx wtk_http2_ctx_t;
struct wtk_http2;

struct wtk_http2_ctx
{
	wtk_http2_ctx_cfg_t *cfg;
	struct wtk_http2 *http2;
	wtk_lock_t lock;
};

wtk_http2_ctx_t* wtk_http2_ctx_new(wtk_http2_ctx_cfg_t *cfg,struct wtk_http2 *http2);
void wtk_http2_ctx_delete(wtk_http2_ctx_t *ctx);
#ifdef __cplusplus
};
#endif
#endif
