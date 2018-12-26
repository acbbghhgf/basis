#include "wtk_redirect.h"

wtk_redirect_t* wtk_redirect_new(wtk_redirect_cfg_t *cfg,wtk_strbuf_t *buf)
{
	wtk_redirect_t *r;

	r=(wtk_redirect_t*)wtk_malloc(sizeof(*r));
	r->cfg=cfg;
	if(cfg->nhost>0)
	{
		int i;

		r->hosts=(wtk_relay_host_t**)wtk_calloc(cfg->nhost,sizeof(wtk_relay_host_t*));
		for(i=0;i<cfg->nhost;++i)
		{
			r->hosts[i]=wtk_relay_host_new(&(cfg->hosts[i]),buf);
		}
	}else
	{
		r->hosts=0;
	}
	return r;
}

void wtk_redirect_delete(wtk_redirect_t *r)
{
	int i;

	if(r->cfg->nhost>0)
	{
		for(i=0;i<r->cfg->nhost;++i)
		{
			wtk_relay_host_delete(r->hosts[i]);
		}
		wtk_free(r->hosts);
	}
	wtk_free(r);
}
