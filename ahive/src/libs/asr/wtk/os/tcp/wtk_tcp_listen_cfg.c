#include "wtk_tcp_listen_cfg.h" 

int wtk_tcp_listen_cfg_init(wtk_tcp_listen_cfg_t *cfg)
{
	cfg->port=8080;
	cfg->backlog=5;
	cfg->reuse=0;
	return 0;
}

int wtk_tcp_listen_cfg_clean(wtk_tcp_listen_cfg_t *cfg)
{
	return 0;
}

int wtk_tcp_listen_cfg_update_local(wtk_tcp_listen_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_i(lc,cfg,port,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,backlog,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,reuse,v);
	return 0;
}

int wtk_tcp_listen_cfg_update(wtk_tcp_listen_cfg_t *cfg)
{
	return 0;
}
