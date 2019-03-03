#include "wtk_cd_cfg.h"

int wtk_cd_cfg_init(wtk_cd_cfg_t *cfg)
{
	wtk_string_set_s(&(cfg->url),"/crossdomain.xml");
	return 0;
}

int wtk_cd_cfg_clean(wtk_cd_cfg_t *cfg)
{
	return 0;
}

int wtk_cd_cfg_update_local(wtk_cd_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_string_v(lc,cfg,url,v);
	return 0;
}

int wtk_cd_cfg_update(wtk_cd_cfg_t *cfg)
{
	return 0;
}

int wtk_cd_cfg_process(wtk_cd_cfg_t *cfg,wtk_request_t *req)
{
	wtk_response_set_body_s((req->response),"<?xml version=\"1.0\"?><cross-domain-policy> <site-control permitted-cross-domain-policies=\"all\"/><allow-http-request-headers-from domain=\"*\" headers=\"*\"/> <allow-access-from domain=\"*\" /> </cross-domain-policy>");
	return 0;
}
