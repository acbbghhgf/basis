#include "wtk_relay_host_cfg.h"


int wtk_relay_host_cfg_init(wtk_relay_host_cfg_t *cfg)
{
	cfg->url.len=0;
	cfg->ip=0;
	cfg->port=0;
	cfg->addr=0;
	return 0;
}

int wtk_relay_host_cfg_clean(wtk_relay_host_cfg_t *cfg)
{
	if(cfg->addr)
	{
		wtk_addrinfo_delete(cfg->addr);
	}
	return 0;
}

int wtk_relay_host_cfg_update_local(wtk_relay_host_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_string_v(lc,cfg,url,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,ip,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,port,v);
	//wtk_local_cfg_update_cfg_b(lc,cfg,use_pool,v);
	return 0;
}

int wtk_relay_host_cfg_update(wtk_relay_host_cfg_t *cfg)
{
	cfg->addr=wtk_addrinfo_get2(cfg->ip,cfg->port);
	return cfg->addr?0:-1;
}
