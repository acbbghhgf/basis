#include "wtk/os/wtk_cpu.h"
#include "wtk_http2_cfg.h"

int wtk_http2_cfg_init(wtk_http2_cfg_t *cfg)
{
	wtk_http_cfg_init(&(cfg->http));
	wtk_http2_ctx_cfg_init(&(cfg->ctx));
	cfg->worker=-1;
	return 0;
}

int wtk_http2_cfg_clean(wtk_http2_cfg_t *cfg)
{
	wtk_http_cfg_clean(&(cfg->http));
	wtk_http2_ctx_cfg_clean(&(cfg->ctx));
	return 0;
}

int wtk_http2_cfg_update_local(wtk_http2_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc=main;
	wtk_string_t *v;
	int ret;

	wtk_local_cfg_update_cfg_i(lc,cfg,worker,v);
	lc=wtk_local_cfg_find_lc_s(main,"http");
	if(lc)
	{
		ret=wtk_http_cfg_update_local(&(cfg->http),lc);
		if(ret!=0){goto end;}
	}
	lc=wtk_local_cfg_find_lc_s(main,"ctx");
	if(lc)
	{
		ret=wtk_http2_ctx_cfg_update_local(&(cfg->ctx),lc);
		if(ret!=0){goto end;}
	}
	ret=0;
end:
	return ret;
}

void wtk_http2_cfg_update_arg(wtk_http2_cfg_t *cfg,wtk_arg_t *arg)
{
	double number;
	int ret;

	ret=wtk_arg_get_number_s(arg,"worker",&number);
	if(ret==0)
	{
		cfg->worker=number;
	}
	wtk_http_cfg_update_arg(&(cfg->http),arg);
}

void wtk_http2_cfg_print_usage()
{
	printf("\t-worker set http worker\n");
	wtk_http_cfg_print_usage();
}

int wtk_http2_cfg_update(wtk_http2_cfg_t *cfg)
{
	int ret;

	if(cfg->worker<=0)
	{
		cfg->worker=wtk_get_cpus();
	}
	ret=wtk_http_cfg_update(&(cfg->http));
	cfg->http.nk.passive=1;
	if(ret!=0){goto end;}
	ret=wtk_http2_ctx_cfg_update(&(cfg->ctx));
	if(ret!=0){goto end;}
end:
	return ret;
}
