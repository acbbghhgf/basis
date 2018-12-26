#include "wtk_redirect_cfg.h"

int wtk_redirect_cfg_init(wtk_redirect_cfg_t *cfg)
{
	cfg->hosts=0;
	cfg->nhost=0;
	cfg->pools=0;
	cfg->npool=0;
	cfg->xpools=0;
	return 0;
}

int wtk_redirect_cfg_clean(wtk_redirect_cfg_t *cfg)
{
	int i;

	if(cfg->hosts)
	{
		for(i=0;i<cfg->nhost;++i)
		{
			wtk_relay_host_cfg_clean(&(cfg->hosts[i]));
		}
		wtk_free(cfg->hosts);
	}
	if(cfg->xpools)
	{
		for(i=0;i<cfg->npool;++i)
		{
			wtk_relay_pool_delete(cfg->xpools[i]);
		}
		wtk_free(cfg->xpools);
	}
	if(cfg->pools)
	{
		for(i=0;i<cfg->npool;++i)
		{
			wtk_relay_pool_cfg_clean(&(cfg->pools[i]));
		}
		wtk_free(cfg->pools);
	}
	return 0;
}

int wtk_redirect_cfg_update_local(wtk_redirect_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_array_t *a;
	wtk_local_cfg_t *item;
	int i;
	int ret;

	a=wtk_local_cfg_find_array_s(lc,"hosts");
	if(a)
	{
		wtk_string_t **strs=(wtk_string_t**)a->slot;

		cfg->nhost=a->nslot;
		cfg->hosts=(wtk_relay_host_cfg_t*)wtk_calloc(a->nslot,sizeof(wtk_relay_host_cfg_t));
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
			wtk_relay_host_cfg_init(&(cfg->hosts[i]));
			wtk_relay_host_cfg_update_local(&(cfg->hosts[i]),item);
		}
	}
	a=wtk_local_cfg_find_array_s(lc,"pools");
	if(a)
	{
		wtk_string_t **strs=(wtk_string_t**)a->slot;

		cfg->npool=a->nslot;
		cfg->pools=(wtk_relay_pool_cfg_t*)wtk_calloc(a->nslot,sizeof(wtk_relay_pool_cfg_t));
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
			wtk_relay_pool_cfg_init(&(cfg->pools[i]));
			wtk_relay_pool_cfg_update_local(&(cfg->pools[i]),item);
		}
	}
	ret=0;
end:
	return ret;
}

int wtk_redirect_cfg_update(wtk_redirect_cfg_t *cfg)
{
	int i;
	int ret;

	for(i=0;i<cfg->nhost;++i)
	{
		ret=wtk_relay_host_cfg_update(&(cfg->hosts[i]));
		if(ret!=0){goto end;}
	}
	if(cfg->npool>0)
	{
		cfg->xpools=(wtk_relay_pool_t**)wtk_calloc(cfg->npool,sizeof(wtk_relay_pool_t*));
		for(i=0;i<cfg->npool;++i)
		{
			cfg->xpools[i]=wtk_relay_pool_new(&(cfg->pools[i]));
		}
	}
	ret=0;
end:
	return 0;
}
