#include "wtk_httpc_cfg.h"

wtk_httpc_cfg_t* wtk_httpc_cfg_new()
{
	wtk_httpc_cfg_t *cfg;

	cfg=(wtk_httpc_cfg_t*)wtk_malloc(sizeof(*cfg));
	wtk_httpc_cfg_init(cfg);
	return cfg;
}

int wtk_httpc_cfg_delete(wtk_httpc_cfg_t *cfg)
{
	wtk_free(cfg);
	return 0;
}

int wtk_httpc_cfg_init(wtk_httpc_cfg_t *cfg)
{
	wtk_cookie_cfg_init(&(cfg->cookie));
	wtk_string_set(&(cfg->ip),0,0);
	wtk_string_set_s(&(cfg->url),"/");
	wtk_string_set_s(&(cfg->port),"80");
	cfg->use_1_1=1;
	cfg->timeout=-1;
	cfg->use_hack=0;
	return 0;
}

int wtk_httpc_cfg_clean(wtk_httpc_cfg_t *cfg)
{
	wtk_cookie_cfg_clean(&(cfg->cookie));
	return 0;
}

int wtk_httpc_cfg_update(wtk_httpc_cfg_t *cfg)
{
	int ret;

	ret=wtk_cookie_cfg_update(&(cfg->cookie));
	return ret;
}

int wtk_httpc_cfg_update_local(wtk_httpc_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc;
	wtk_string_t *v;
	int ret;

	lc=main;
	wtk_local_cfg_update_cfg_b(lc,cfg,use_hack,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_1_1,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,timeout,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,ip,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,url,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,port,v);
	if(cfg->ip.len<=0 || cfg->url.len<=0)
	{
		ret=-1;
		goto end;
	}
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

int wtk_httpc_cfg_update_srv(wtk_httpc_cfg_t *cfg,wtk_string_t *host,wtk_string_t *port,int timeout)
{
	cfg->timeout=timeout;
	///wtk_debug("timeout=%d\n",cfg->timeout);
	sprintf(cfg->ip_buf,host->data,host->len);
	wtk_string_set(&(cfg->ip),cfg->ip_buf,strlen(cfg->ip_buf));
	if(port)
	{
		sprintf(cfg->port_buf,port->data,port->len);
		wtk_string_set(&(cfg->port),cfg->port_buf,strlen(cfg->port_buf));
	}else
	{
		wtk_string_set_s(&(cfg->port),"80");
	}
	return 0;
}
