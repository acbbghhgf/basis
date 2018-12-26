#ifndef WTK_HTTP_LOC_REDIRECT_WTK_REDIRECT_H_
#define WTK_HTTP_LOC_REDIRECT_WTK_REDIRECT_H_
#include "wtk/core/wtk_type.h"
#include "wtk/http/nk/wtk_nk.h"
#include "wtk_redirect_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_redirect wtk_redirect_t;
struct wtk_redirect
{
	wtk_redirect_cfg_t *cfg;
	wtk_relay_host_t **hosts;
};

wtk_redirect_t* wtk_redirect_new(wtk_redirect_cfg_t *cfg,wtk_strbuf_t *buf);
void wtk_redirect_delete(wtk_redirect_t *r);
#ifdef __cplusplus
};
#endif
#endif
