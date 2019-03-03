#include "wtk_epoll_cfg.h" 

int wtk_epoll_cfg_init(wtk_epoll_cfg_t *cfg)
{
	cfg->size=200;
	cfg->use_et=1;
	return 0;
}

int wtk_epoll_cfg_clean(wtk_epoll_cfg_t *cfg)
{
	return 0;
}

int wtk_epoll_cfg_update_local(wtk_epoll_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_i(lc,cfg,size,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_et,v);
	return 0;
}

int wtk_epoll_cfg_update(wtk_epoll_cfg_t *cfg)
{
	return 0;
}
