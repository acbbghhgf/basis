#include "wtk_http2_ctx_cfg.h"

int wtk_http2_ctx_cfg_init(wtk_http2_ctx_cfg_t *cfg)
{
	wtk_string_set_s(&(cfg->url_con),"/con");
	return 0;
}

int wtk_http2_ctx_cfg_clean(wtk_http2_ctx_cfg_t *cfg)
{
	return 0;
}

int wtk_http2_ctx_cfg_update_local(wtk_http2_ctx_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_string_v(lc,cfg,url_con,v);
	return 0;
}

int wtk_http2_ctx_cfg_update(wtk_http2_ctx_cfg_t *cfg)
{
	return 0;
}
