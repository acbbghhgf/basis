#include "wtk_pp_cfg.h"
#include "wtk/os/wtk_proc.h"
#include "wtk/os/wtk_cpu.h"

int wtk_pp_cfg_init(wtk_pp_cfg_t *cfg)
{
#ifdef WIN32
	cfg->cpu_frequency=2300;
	cfg->ht_enable=0;
#else
	cfg->cpu_frequency=wtk_cpu_get_frequence();
	cfg->ht_enable=wtk_cpu_ht_is_enable();
#endif
	cfg->cpus=wtk_get_cpus();
	cfg->time_relink=1000;
	cfg->use_touch=0;
	wtk_string_set_s(&(cfg->ip),"");
	wtk_string_set_s(&(cfg->port),"");
	wtk_string_set_s(&(cfg->url),"/link");
	cfg->addr=0;
	return 0;
}

int wtk_pp_cfg_clean(wtk_pp_cfg_t *cfg)
{
	if(cfg->addr)
	{
		wtk_addrinfo_delete(cfg->addr);
	}
	return 0;
}

int wtk_pp_cfg_update(wtk_pp_cfg_t *cfg)
{
	if(cfg->addr)
	{
		wtk_addrinfo_delete(cfg->addr);
		cfg->addr=0;
	}
	cfg->addr=wtk_addrinfo_get(cfg->ip.data,cfg->port.data);
	return cfg->addr?0:-1;
}

int wtk_pp_cfg_update_local(wtk_pp_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_b(lc,cfg,use_touch,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,ht_enable,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,time_relink,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,port,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,ip,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,url,v);
	wtk_local_cfg_update_cfg_f(lc,cfg,cpu_frequency,v);
	return 0;
}
