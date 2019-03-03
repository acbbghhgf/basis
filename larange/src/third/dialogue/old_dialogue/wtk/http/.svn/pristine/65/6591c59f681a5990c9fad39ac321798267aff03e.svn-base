#include "wtk_httpnc_cfg.h"

int wtk_httpnc_cfg_init(wtk_httpnc_cfg_t *cfg)
{
	cfg->ip.len=0;
	cfg->port.len=0;
	cfg->addr=0;
	cfg->time_relink=0;
	cfg->use_1_1=1;
	wtk_cookie_cfg_init(&(cfg->cookie));
	return 0;
}

int wtk_httpnc_cfg_clean(wtk_httpnc_cfg_t *cfg)
{
	wtk_cookie_cfg_clean(&(cfg->cookie));
	if(cfg->addr)
	{
		wtk_addrinfo_delete(cfg->addr);
	}
	return 0;
}

int wtk_httpnc_cfg_update_local(wtk_httpnc_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc;
	wtk_string_t *v;
	int ret;

	lc=main;
	wtk_local_cfg_update_cfg_string_v(lc,cfg,ip,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,port,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,time_relink,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,use_1_1,v);

	lc=wtk_local_cfg_find_lc_s(main,"cookie");
	if(lc)
	{
		ret=wtk_cookie_cfg_update_local(&(cfg->cookie),lc);
		if(ret!=0){goto end;}
	}
	ret=0;
end:
	return ret;
}

int wtk_httpnc_cfg_update(wtk_httpnc_cfg_t *cfg)
{
	int ret;

	cfg->addr=wtk_addrinfo_get2(cfg->ip.data,cfg->port.data);
	if(!cfg->addr){ret=-1;goto end;}
	ret=wtk_cookie_cfg_update(&(cfg->cookie));
end:
	return ret;
}
