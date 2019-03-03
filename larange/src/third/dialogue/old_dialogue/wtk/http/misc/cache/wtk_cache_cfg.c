#include "wtk_cache_cfg.h"
#include "wtk/core/wtk_os.h"

int wtk_cache_cfg_init(wtk_cache_cfg_t *cfg)
{
	wtk_httpc_cfg_init(&(cfg->httpc));
	wtk_string_set_s(&(cfg->db_fn),"./cache.db");
	wtk_string_set(&(cfg->http_port),0,0);
	cfg->nslot=1003;
	cfg->use_httpc=0;
	cfg->use_db=1;
	cfg->max_active_slot=1333;
	cfg->httpc_retry=2;
	return 0;
}

int wtk_cache_cfg_clean(wtk_cache_cfg_t *cfg)
{
	return 0;
}

int wtk_cache_cfg_update(wtk_cache_cfg_t *cfg)
{
	if(cfg->use_db)
	{
		if(wtk_file_exist(cfg->db_fn.data))
		{
			//wtk_debug("remove %s.\n",cfg->db_fn.data);
			remove(cfg->db_fn.data);
		}
	}
	if(cfg->use_httpc)
	{
		wtk_httpc_cfg_update(&(cfg->httpc));
	}
	return 0;
}

int wtk_cache_cfg_update_local(wtk_cache_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc=main;
	wtk_string_t *v;
	int ret=0;

	wtk_local_cfg_update_cfg_i(lc,cfg,nslot,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,max_active_slot,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,httpc_retry,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,db_fn,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,http_port,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_db,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_httpc,v);
	if(cfg->use_db)
	{
		ret=cfg->db_fn.len>0?0:-1;
		if(ret!=0){goto end;}
	}
	lc=wtk_local_cfg_find_lc_s(main,"httpc");
	if(lc)
	{
		ret=wtk_httpc_cfg_update_local(&(cfg->httpc),lc);
	}
end:
	return ret;
}
