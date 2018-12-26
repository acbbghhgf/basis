#include "wtk_nk_cfg.h" 

int wtk_nk_cfg_init(wtk_nk_cfg_t *cfg)
{
	wtk_log_cfg_init(&(cfg->log));
	wtk_poll_cfg_init(&(cfg->poll));
	wtk_listen_cfg_init(&(cfg->listen));
	cfg->rw_size=32*1024;
	cfg->cache_size=1024;
	cfg->debug=0;
	cfg->log_snd=0;
	cfg->log_rcv=0;
	cfg->max_connection=10240;
	cfg->con_buf_size=1024;
	return 0;
}

int wtk_nk_cfg_clean(wtk_nk_cfg_t *cfg)
{
	wtk_poll_cfg_clean(&(cfg->poll));
	wtk_log_cfg_clean(&(cfg->log));
	wtk_listen_cfg_clean(&(cfg->listen));
	return 0;
}

int wtk_nk_cfg_update_local(wtk_nk_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc;
	wtk_string_t *v;
	int ret;

	lc=main;
	wtk_local_cfg_update_cfg_i(lc,cfg,con_buf_size,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,max_connection,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,cache_size,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,rw_size,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,debug,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,log_snd,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,log_rcv,v);
	lc=wtk_local_cfg_find_lc_s(main,"poll");
	if(lc)
	{
		ret=wtk_poll_cfg_update_local(&(cfg->poll),lc);
		if(ret!=0){goto end;}
	}
	lc=wtk_local_cfg_find_lc_s(main,"listen");
	if(lc)
	{
		ret=wtk_listen_cfg_update_local(&(cfg->listen),lc);
		if(ret!=0){goto end;}
	}
	lc=wtk_local_cfg_find_lc_s(main,"log");
	if(lc)
	{
		ret=wtk_log_cfg_update_local(&(cfg->log),lc);
		if(ret!=0){goto end;}
	}
	ret=0;
end:
	return ret;
}

int wtk_nk_cfg_update(wtk_nk_cfg_t *cfg)
{
	int ret;

	ret=wtk_poll_cfg_update(&(cfg->poll));
	if(ret!=0){goto end;}
	ret=wtk_listen_cfg_update(&(cfg->listen));
	if(ret!=0){goto end;}
	ret=wtk_log_cfg_update(&(cfg->log));
	if(ret!=0){goto end;}
end:
	return ret;
}

void wtk_nk_cfg_update_arg(wtk_nk_cfg_t *cfg,wtk_arg_t *arg)
{
	double i;
	int ret;

	ret=wtk_arg_get_number_s(arg,"p",&i);
	if(ret==0)
	{
		cfg->listen.port=(int)i;
	}
}
