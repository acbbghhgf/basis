#include "wtk_plink.h"


wtk_plink_t* wtk_plink_new(wtk_plink_cfg_t *cfg,struct wtk_http *http)
{
	wtk_plink_t *p;
	int i;

	p=(wtk_plink_t*)wtk_malloc(sizeof(*p));
	p->cfg=cfg;
	p->hosts=(wtk_plink_host_t**)wtk_calloc(cfg->nlink,sizeof(wtk_plink_host_t*));
	for(i=0;i<cfg->nlink;++i)
	{
		p->hosts[i]=wtk_plink_host_new(&(cfg->links[i]),http);
	}
	return p;
}

void wtk_plink_delete(wtk_plink_t *p)
{
	int i;

	for(i=0;i<p->cfg->nlink;++i)
	{
		wtk_plink_host_delete(p->hosts[i]);
	}
	wtk_free(p->hosts);
	wtk_free(p);
}

int wtk_plink_link(wtk_plink_t *p)
{
	int i;

	//wtk_debug("link ...\n");
	for(i=0;i<p->cfg->nlink;++i)
	{
		wtk_plink_host_link(p->hosts[i]);
	}
	return 0;
}
