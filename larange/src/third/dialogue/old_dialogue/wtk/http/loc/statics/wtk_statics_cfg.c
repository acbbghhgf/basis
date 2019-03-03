#include "wtk_statics_cfg.h"

int wtk_statics_cfg_init(wtk_statics_cfg_t *cfg)
{
	wtk_string_set_s(&(cfg->url_statics),"/statics");
	wtk_string_set_s(&(cfg->url_debug),"/debug");
	wtk_string_set_s(&(cfg->url_speech),"/speech");
	wtk_string_set_s(&(cfg->url_cpu),"/cpu");
	return 0;
}

int wtk_statics_cfg_clean(wtk_statics_cfg_t *cfg)
{
	return 0;
}

int wtk_statics_cfg_update_local(wtk_statics_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_string_v(lc,cfg,url_statics,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,url_debug,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,url_speech,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,url_cpu,v);
	return 0;
}

int wtk_statics_cfg_update(wtk_statics_cfg_t *cfg)
{
	return 0;
}
