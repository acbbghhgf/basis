#ifndef WTK_HTTP2_WTK_HTTP2_CFG_H_
#define WTK_HTTP2_WTK_HTTP2_CFG_H_
#include "wtk/http/wtk_http_cfg.h"
#include "wtk/http/http2/ctx/wtk_http2_ctx_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_http2_cfg wtk_http2_cfg_t;

struct wtk_http2_cfg
{
	wtk_http_cfg_t http;
	wtk_http2_ctx_cfg_t ctx;
	int worker;
};


int wtk_http2_cfg_init(wtk_http2_cfg_t *cfg);
int wtk_http2_cfg_clean(wtk_http2_cfg_t *cfg);
int wtk_http2_cfg_update_local(wtk_http2_cfg_t *cfg,wtk_local_cfg_t *lc);
void wtk_http2_cfg_update_arg(wtk_http2_cfg_t *cfg,wtk_arg_t *arg);
int wtk_http2_cfg_update(wtk_http2_cfg_t *cfg);
void wtk_http2_cfg_print_usage();
#ifdef __cplusplus
};
#endif
#endif
