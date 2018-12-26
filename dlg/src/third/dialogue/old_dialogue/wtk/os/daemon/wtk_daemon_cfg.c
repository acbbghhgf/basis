#include "wtk_daemon_cfg.h"

int wtk_daemon_cfg_init(wtk_daemon_cfg_t *cfg)
{
	cfg->fn=wtk_strbuf_new(1024,1);
	wtk_string_set_s(&(cfg->pid_fn),"http");
	cfg->debug=0;
	cfg->detect_running=1;
	cfg->daemon=1;
	return 0;
}

int wtk_daemon_cfg_clean(wtk_daemon_cfg_t *cfg)
{
	wtk_strbuf_delete(cfg->fn);
	return 0;
}

int wtk_daemon_cfg_update_local(wtk_daemon_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_string_v(lc,cfg,pid_fn,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,debug,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,detect_running,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,daemon,v);
	return 0;
}

void wtk_daemon_cfg_update_arg(wtk_daemon_cfg_t *cfg,wtk_arg_t *arg)
{
	char *v;
	int pid;

	wtk_arg_get_str_s(arg,"s",&v);
	if(v)
	{
		if(strcmp(v,"stop")==0)
		{
			pid=pid_from_file(cfg->fn->data);
			printf("-------------------------------------------------------\n\n");
			if(pid>0)
			{
				if(kill(pid,0)==0)
				{
					printf("send SIQQUIT to master process[pid=%d].\n",pid);
					kill(pid,SIGQUIT);
				}else
				{
					printf("master process not exist.\n");
				}
			}else
			{
				printf("server is alreay shutdown.\n");
			}
			printf("-------------------------------------------------------\n\n");
			if(wtk_file_exist(cfg->fn->data)==0)
			{
				remove(cfg->fn->data);
			}
		}
		exit(0);
	}
	wtk_arg_get_str_s(arg,"d",&v);
	if(v)
	{
		if(strcmp(v,"false")==0)
		{
			cfg->daemon=0;
		}else if(strcmp(v,"true")==0)
		{
			cfg->daemon=1;
		}
	}
	if(wtk_arg_exist_s(arg,"debug"))
	{
		cfg->debug=1;
	}
}

void wtk_daemon_cfg_print_usage()
{
	printf("\t-s stop send stop signal\n");
	printf("\t-d false|true set daemon or not\n");
	printf("\t-debug\n");
}

int wtk_daemon_cfg_update(wtk_daemon_cfg_t *cfg)
{
	char buf[2048];
	int n;

	n=wtk_proc_get_dir(buf,sizeof(buf));
	wtk_strbuf_push(cfg->fn,buf,n);
	wtk_strbuf_push_s(cfg->fn,"/");
	wtk_strbuf_push(cfg->fn,cfg->pid_fn.data,cfg->pid_fn.len);
	wtk_strbuf_push_s(cfg->fn,".");
	gethostname(buf,sizeof(buf));
	wtk_strbuf_push_string(cfg->fn,buf);
	wtk_strbuf_push_s(cfg->fn,".pid");
	wtk_strbuf_push_c(cfg->fn,0);
	--cfg->fn->pos;
	return 0;
}

void wtk_daemon_cfg_init_with_main(wtk_daemon_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc;

	wtk_daemon_cfg_init(cfg);
	lc=wtk_local_cfg_find_lc_s(main,"daemon");
	if(lc)
	{
		wtk_daemon_cfg_update_local(cfg,lc);
	}
}

int wtk_daemon_cfg_is_running(wtk_daemon_cfg_t *cfg)
{
	char *p;
	int pid,n;
	int ret=0;

	if(wtk_file_exist(cfg->fn->data)!=0){goto end;}
	p=file_read_buf(cfg->fn->data,&n);
	pid=wtk_str_atoi(p,n);
	wtk_free(p);
	if(pid<=0){goto end;}
	if(kill(pid,0)==0)
	{
		ret=1;
	}
end:
	return ret;
}

int wtk_daemon_cfg_recheck(wtk_daemon_cfg_t *cfg)
{
	int ret;
	int pos,i;

	if(cfg->detect_running)
	{
		ret=wtk_daemon_cfg_is_running(cfg);
		if(ret==1)
		{
			printf("error: [%s] is already running.\n",cfg->fn->data);
			goto end;
		}
	}
	pos=cfg->fn->pos;
	i=0;
	while(1)
	{
		if(wtk_file_exist(cfg->fn->data)!=0)
		{
			break;
		}else
		{
			printf("warning: pid file %s already exist.\n",cfg->fn->data);
		}
		cfg->fn->pos=pos;
		wtk_strbuf_push_f(cfg->fn,".%d",++i);
		wtk_strbuf_push_c(cfg->fn,0);
	}
	ret=0;
end:
	return ret;
}

