#include "wtk_plink_cfg.h"

int wtk_plink_cfg_init(wtk_plink_cfg_t *cfg)
{
	cfg->nlink=0;
	cfg->links=0;
	cfg->used=0;
	return 0;
}

int wtk_plink_cfg_clean(wtk_plink_cfg_t *cfg)
{
	int i;

	if(cfg->links)
	{
		for(i=0;i<cfg->nlink;++i)
		{
			wtk_plink_host_cfg_clean(&(cfg->links[i]));
		}
		wtk_free(cfg->links);
	}
	return 0;
}

int wtk_plink_cfg_update_local(wtk_plink_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_local_cfg_t *item;
	wtk_array_t *a;
	int ret;
	int i;

	a=wtk_local_cfg_find_array_s(lc,"links");
	if(a)
	{
		wtk_string_t **strs=(wtk_string_t**)a->slot;

		cfg->nlink=a->nslot;
		cfg->links=(wtk_plink_host_cfg_t*)wtk_calloc(a->nslot,sizeof(wtk_plink_host_cfg_t));
		for(i=0;i<a->nslot;++i)
		{
			item=wtk_local_cfg_find_lc(lc,strs[i]->data,strs[i]->len);
			if(!item)
			{
				wtk_debug("[%.*s] not found\n",strs[i]->len,strs[i]->data);
				ret=-1;
				goto end;
			}
			//wtk_debug("[%.*s]\n",strs[i]->len,strs[i]->data);
			wtk_plink_host_cfg_init(&(cfg->links[i]));
			ret=wtk_plink_host_cfg_update_local(&(cfg->links[i]),item);
			if(ret!=0){goto end;}
		}
	}
	ret=0;
end:
	return ret;
}

int wtk_plink_cfg_update(wtk_plink_cfg_t *cfg)
{
	int i;
	int ret;

	if(cfg->links)
	{
		for(i=0;i<cfg->nlink;++i)
		{
			ret=wtk_plink_host_cfg_update(&(cfg->links[i]));
			if(ret!=0){goto end;}
		}
	}
	ret=0;
end:
	return ret;
}
