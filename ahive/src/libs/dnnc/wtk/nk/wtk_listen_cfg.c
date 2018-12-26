#include "wtk_listen_cfg.h" 

int wtk_listen_cfg_init(wtk_listen_cfg_t *cfg)
{
	cfg->port=80;
	cfg->backlog=511;
	cfg->defer_accept_timeout=100;
	cfg->loop_back=0;
	cfg->reuse=1;
	return 0;
}

int wtk_listen_cfg_clean(wtk_listen_cfg_t *cfg)
{
	return 0;
}

int wtk_listen_cfg_update_local(wtk_listen_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_i(lc,cfg,port,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,backlog,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,defer_accept_timeout,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,loop_back,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,reuse,v);
	return 0;
}

int wtk_listen_cfg_update(wtk_listen_cfg_t *cfg)
{
	return 0;
}
