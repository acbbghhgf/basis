#include "wtk_poll_cfg.h" 

int wtk_poll_cfg_init(wtk_poll_cfg_t *cfg)
{
	wtk_epoll_cfg_init(&(cfg->epoll));
	cfg->timeout=100;
	cfg->use_epoll=1;
	return 0;
}

int wtk_poll_cfg_clean(wtk_poll_cfg_t *cfg)
{
	wtk_epoll_cfg_clean(&(cfg->epoll));
	return 0;
}

int wtk_poll_cfg_update_local(wtk_poll_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc;
	wtk_string_t *v;
	int ret;

	lc=main;
	wtk_local_cfg_update_cfg_i(lc,cfg,timeout,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_epoll,v);
	lc=wtk_local_cfg_find_lc_s(main,"epoll");
	if(lc)
	{
		ret=wtk_epoll_cfg_update_local(&(cfg->epoll),lc);
		if(ret!=0){goto end;}
	}
	ret=0;
end:
	return ret;
}

int wtk_poll_cfg_update(wtk_poll_cfg_t *cfg)
{
	int ret;

	if(cfg->use_epoll)
	{
		ret=wtk_epoll_cfg_update(&(cfg->epoll));
		if(ret!=0){goto end;}
	}
	ret=0;
end:
	return ret;
}
