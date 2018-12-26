#include "wtk_dnnc_cfg.h" 

int wtk_dnnc_cfg_init(wtk_dnnc_cfg_t *cfg)
{
	cfg->ip=NULL;
	cfg->port=NULL;
	cfg->try_cnt=1;
	cfg->try_sleep_time=100;
	cfg->timeout=300;
	cfg->rw_size=1024;
	cfg->debug=0;
	return 0;
}

int wtk_dnnc_cfg_clean(wtk_dnnc_cfg_t *cfg)
{
	return 0;
}

int wtk_dnnc_cfg_update_local(wtk_dnnc_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_str(lc,cfg,ip,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,port,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,try_cnt,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,try_sleep_time,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,timeout,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,rw_size,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,debug,v);
	return 0;
}

int wtk_dnnc_cfg_update(wtk_dnnc_cfg_t *cfg)
{
	return 0;
}
