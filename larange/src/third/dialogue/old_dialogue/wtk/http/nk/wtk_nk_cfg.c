#include "wtk_nk_cfg.h"

int wtk_nk_cfg_init(wtk_nk_cfg_t *cfg)
{
	cfg->connection_cache=1024;
	cfg->loop=0;
	cfg->max_connections=5000;
	cfg->rw_size=32*1024;
	cfg->max_read_count=10;
	cfg->passive=0;
	cfg->debug=0;
	cfg->cpu_update_timeout=500;
	cfg->poll_hint=200;
	cfg->update_time=1;
	cfg->poll_timeout=500;
	cfg->use_cpu=1;
	cfg->attach_test=0;
	cfg->use_cfile_cpu=0;
	cfg->log_rcv=0;
	cfg->log_snd=0;
	cfg->log_event=0;
	cfg->log_connection=1;
	cfg->prompt=1;
	cfg->use_pipe=1;
	wtk_listen_cfg_init(&(cfg->listen));
	return 0;
}

int wtk_nk_cfg_clean(wtk_nk_cfg_t *cfg)
{
	wtk_listen_cfg_clean(&(cfg->listen));
	return 0;
}

int wtk_nk_cfg_update(wtk_nk_cfg_t *cfg)
{
	int ret;

	ret=wtk_listen_cfg_update(&(cfg->listen));
	return ret;
}

int wtk_nk_cfg_update_local(wtk_nk_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc=main;
	wtk_string_t *v;
	int ret;

	wtk_local_cfg_update_cfg_i(lc,cfg,rw_size,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,connection_cache,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,max_connections,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,max_read_count,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,cpu_update_timeout,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,poll_hint,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,poll_timeout,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,loop,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,passive,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,debug,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,update_time,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_cpu,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,log_snd,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,log_rcv,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,attach_test,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_cfile_cpu,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,log_event,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,log_connection,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,prompt,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_pipe,v);
	lc=wtk_local_cfg_find_lc_s(lc,"listen");
	if(lc)
	{
		ret=wtk_listen_cfg_update_local(&(cfg->listen),lc);
		if(ret!=0){goto end;}
	}
	ret=0;
end:
	return ret;
}

void wtk_nk_cfg_print(wtk_nk_cfg_t *cfg)
{
	printf("----------  NK ----------\n");
	print_cfg_i(cfg,connection_cache);
	print_cfg_i(cfg,loop);
	print_cfg_i(cfg,max_connections);
	print_cfg_i(cfg,rw_size);
}
