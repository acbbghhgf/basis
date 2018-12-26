#include "wtk_log_cfg.h"
#include "wtk_log.h"

int wtk_log_cfg_init(wtk_log_cfg_t *cfg)
{
	wtk_string_set_s(&(cfg->fn),"httpd");

	cfg->level=wtk_log_notice;
	cfg->fn_pad_timestamp=0;
	cfg->use_console=0;
	cfg->log_ts=1;
    cfg->daily=0;
    cfg->log_touch=0;
	return 0;
}

int wtk_log_cfg_clean(wtk_log_cfg_t *cfg)
{
	return 0;
}

int wtk_log_cfg_update_local(wtk_log_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_string_v(lc,cfg,fn,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,level,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,fn_pad_timestamp,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_console,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,log_ts,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,log_touch,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,daily,v);
	return 0;
}

void wtk_log_cfg_update_arg(wtk_log_cfg_t *cfg,wtk_arg_t *arg)
{
	double n;
	int ret;
	char *fn;

	ret=wtk_arg_get_number_s(arg,"level",&n);
	if(ret==0)
	{
		cfg->level=n;
	}
	ret=wtk_arg_exist_s(arg,"console");
	if(ret==1)
	{
		cfg->use_console=1;
	}
	ret=wtk_arg_get_str_s(arg,"logfn",&fn);
	if(ret==0)
	{
		wtk_string_set(&(cfg->fn),fn,strlen(fn));
		cfg->fn_pad_timestamp=0;
	}
}

void wtk_log_cfg_print_usage()
{
	printf("\t-level set log level\n");
	printf("\t-console set log to console or not\n");
	printf("\t-logfn set log file name\n");
}

int wtk_log_cfg_update(wtk_log_cfg_t *cfg)
{
	char buf[64];

	sprintf(cfg->log_fn,"%.*s",cfg->fn.len,cfg->fn.data);
	if(cfg->fn_pad_timestamp)
	{
		wtk_log_g_print_time(buf);
		strcat(cfg->log_fn,".");
		strcat(cfg->log_fn,buf);
	}

	strcat(cfg->log_fn,".log");
	return 0;
}

wtk_log_t* wtk_log_cfg_new_log(wtk_log_cfg_t *cfg)
{
	wtk_log_t *log;

	if(cfg->use_console)
	{
		log=wtk_log_new2(0,cfg->level);
	}else
	{
		log=wtk_log_new3(cfg->log_fn,cfg->level,cfg->daily);
	}
	log->log_ts=cfg->log_ts;
	log->log_touch=cfg->log_touch;
	return log;
}
