#ifndef WTK_HTTP2_CTX_WTK_HTTP2_CTX_CFG_H_
#define WTK_HTTP2_CTX_WTK_HTTP2_CTX_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_http2_ctx_cfg wtk_http2_ctx_cfg_t;
struct wtk_http2_ctx_cfg
{
	wtk_string_t url_con;
};

int wtk_http2_ctx_cfg_init(wtk_http2_ctx_cfg_t *cfg);
int wtk_http2_ctx_cfg_clean(wtk_http2_ctx_cfg_t *cfg);
int wtk_http2_ctx_cfg_update_local(wtk_http2_ctx_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_http2_ctx_cfg_update(wtk_http2_ctx_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
