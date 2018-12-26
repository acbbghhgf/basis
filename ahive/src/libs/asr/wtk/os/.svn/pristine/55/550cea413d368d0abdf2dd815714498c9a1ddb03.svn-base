#include <sys/prctl.h>
#include "wtk_daemon.h"

#include <signal.h>

#ifndef USE_R16
#include <execinfo.h>
#endif

void wtk_daemon_setup_signal(wtk_daemon_t *d);
static wtk_daemon_t *static_damon;

wtk_daemon_t* wtk_daemon_new(wtk_daemon_cfg_t *cfg,
		wtk_log_t *log,
		void *usr_data,
		wtk_daemon_start_f start,
		wtk_daemon_stop_f stop,
		wtk_daemon_join_f join)
{
	wtk_daemon_t *d;

	d=(wtk_daemon_t*)wtk_malloc(sizeof(*d));
	d->cfg=cfg;
	d->log=log;
	d->usr_data=usr_data;
	d->start=start;
	d->stop=stop;
	d->join=join;
	d->master_pid=-1;
	wtk_sem_init(&(d->stop_sem),0);
	wtk_daemon_setup_signal(d);
	static_damon=d;
	return d;
}

void wtk_daemon_delete(wtk_daemon_t *d)
{
	wtk_sem_clean(&(d->stop_sem));
	wtk_free(d);
	return;
}

void wtk_daemon_save_pidfile(wtk_daemon_t *d)
{
	pid_save_file(d->master_pid,d->cfg->fn->data);
}

void wtk_daemon_delete_pidfile(wtk_daemon_t *d)
{
	pid_delete_file(d->master_pid,d->cfg->fn->data);
}

void wtk_daemon_exit(wtk_daemon_t *d)
{
	pid_t pid;

	pid=getpid();
	if(pid==d->master_pid)
	{
		wtk_daemon_delete_pidfile(d);
		kill(0,SIGKILL);
	}
	exit(0);
}


static void
_backtrace(const char *sigh)
{
#ifndef USE_R16
    void* array[100];
    char** strings;
    int i, n, rv;
    char exe[256] = {0};

    time_t t;
    struct tm tm;

    FILE *file;

    time(&t);
    localtime_r(&t, &tm);

    rv = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
    if (rv != -1) {
        exe[rv] = 0;
    }

    file = fopen("/data/log/crash.log", "a");
    if (file) {
        fprintf(file, "%d-%02d-%02d %02d:%02d:%02d %s %s\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, sigh, exe);
    }

    fprintf(stderr, "%d-%02d-%02d %02d:%02d:%02d %s %s\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, sigh, exe);

    n = backtrace(array, sizeof(array)/sizeof(void*));
    strings = backtrace_symbols(array, n);
    for (i = 0; i < n; ++i) {
        if (file) {
            fprintf(file, "    %s\n", strings[i]);
        }
        fprintf(stderr, "    %s\n", strings[i]);
    }

    if (file) {
        fclose(file);
    }
    free(strings);
#endif
}


static void wtk_daemon_on_signal(int sig)
{
	wtk_daemon_t *d;
    const char *sigh = NULL;

	d=static_damon;
	if(d->log)
	{
		wtk_log_log(d->log,"pid[%d] recv signal[%d].",(int)getpid(),sig);
	}
	switch(sig)
	{
    case SIGABRT:
        sigh = sigh ? sigh : "SIGABRT";
    case SIGBUS:
        sigh = sigh ? sigh : "SIGBUS";
    case SIGFPE:
        sigh = sigh ? sigh : "SIGFPE";
	case SIGSEGV:
        sigh = sigh ? sigh : "SIGSEGV";
        _backtrace(sigh);
        wtk_daemon_exit(d);
		break;
	case SIGINT:
		if(d->master_pid>0)
		{
			kill(0,SIGKILL);
		}
		wtk_daemon_exit(d);
		break;
	case SIGQUIT:
		printf("[pid=%d]: recv QUIT msg.\n",(int)getpid());
	case SIGUSR1:
		if(d->master_pid == getpid())
		{
			kill(d->worker_pid,SIGQUIT);
			wtk_daemon_exit(d);
		}else
		{
			d->run=0;
			wtk_sem_release(&(d->stop_sem),1);
		}
		break;
	case SIGPIPE:
		//dump_stack();
		break;
	default:
		wtk_debug("%d: receive %d signal.\n",getpid(),sig);
		break;
	}
}

void wtk_daemon_setup_signal(wtk_daemon_t *d)
{
	int signals[]={SIGABRT,SIGBUS,SIGFPE,SIGSEGV,SIGPIPE,SIGQUIT,SIGINT,SIGUSR1};
	int i,count;

	count=sizeof(signals)/sizeof(int);
	for(i=0;i<count;++i)
	{
		signal(signals[i],wtk_daemon_on_signal);
	}
}

int wtk_daemon_run_proc(wtk_daemon_t *d)
{
	int ret;

	if(d->start)
	{
		ret=d->start(d->usr_data);
		if(ret!=0){goto end;}
	}
	if(d->cfg->debug)
	{
		getchar();
	}else
	{
		d->run=1;
		while(d->run)
		{
			wtk_sem_acquire(&(d->stop_sem),-1);
		}
	}
	if(d->stop)
	{
		ret=d->stop(d->usr_data);
		if(ret!=0){goto end;}
	}
	if(d->join)
	{
		ret=d->join(d->usr_data);
		if(ret!=0){goto end;}
	}
	ret=0;
end:
	return ret;
}

int wtk_daemon_run_daemon(wtk_daemon_t *d)
{
	pid_t pid;
	int status,count;

	pid_daemonize();
	setpgid(0,0);
	d->master_pid=getpid();
	wtk_daemon_save_pidfile(d);
	count=0;
	while(1)
	{
		pid=fork();
		if(pid==0)
		{
			prctl(PR_SET_PDEATHSIG,SIGUSR1);
			if(d->log)
			{
				wtk_log_log(d->log,"run proc[pid=%d,count=%d].",(int)getpid(),count);
			}
			wtk_daemon_run_proc(d);
			exit(0);
		}else
		{
			d->worker_pid=pid;
			pid=waitpid(pid,&status,0);
			if(pid==-1)
			{
				perror(__FUNCTION__);
			}
			++count;
		}
	}
	wtk_daemon_delete_pidfile(d);
	return 0;
}

int wtk_daemon_run(wtk_daemon_t *d)
{
	int ret;

	if(d->cfg->daemon)
	{
		ret=wtk_daemon_run_daemon(d);
	}else
	{
		ret=wtk_daemon_run_proc(d);
	}
	return ret;
}


//====================== Test Section ==================
#include "wtk/core/cfg/wtk_main_cfg.h"

int wtk_daemon_test(int argc,char **argv)
{
	wtk_main_cfg_t *cfg;
	wtk_daemon_t *d=0;
	char *fn;
	int ret;
	wtk_log_t *log;

	log=wtk_log_new(0);
	fn="../ext/daemon.cfg";
	cfg=wtk_main_cfg_new3(sizeof(wtk_daemon_cfg_t),
			(wtk_main_cfg_init_f)wtk_daemon_cfg_init,
			(wtk_main_cfg_clean_f)wtk_daemon_cfg_clean,
			(wtk_main_cfg_update_local_f)wtk_daemon_cfg_update_local,
			(wtk_main_cfg_update_f)wtk_daemon_cfg_update,fn,argc,argv);
	if(!cfg){goto end;}
	ret=wtk_daemon_cfg_recheck((wtk_daemon_cfg_t*)cfg->cfg);
	if(ret!=0)
	{
		wtk_debug("recheck failed.\n");
		goto end;
	}
	d=wtk_daemon_new((wtk_daemon_cfg_t*)cfg->cfg,log,0,0,0,0);
	wtk_daemon_run(d);
end:
	if(d)
	{
		wtk_daemon_delete(d);
	}
	if(cfg)
	{
		wtk_main_cfg_delete(cfg);
	}
	wtk_log_delete(log);
	return 0;
}
