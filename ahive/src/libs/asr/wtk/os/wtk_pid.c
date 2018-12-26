#include "wtk_pid.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/prctl.h>
#include "wtk_proc.h"
#include "wtk/core/wtk_str.h"

int pid_kill(pid_t pid)
{
	int ret;

	ret=kill(pid,SIGKILL);
	if(ret==0)
	{
		waitpid(pid,0,0);
	}
	return ret;
}



static void proc_process_sig(int sig)
{
	wtk_debug("%d: receive %d signal.\n",getpid(),sig);//magic_proc->name?magic_proc->name:"anon",sig);
	switch(sig)
	{
	case SIGSEGV:
		wtk_proc_dump_stack();
		exit(0);
		break;
	case SIGFPE:
		wtk_proc_dump_stack();
		exit(0);
		break;
	}
}

int pid_setup_signal()
{
	int signals[]={SIGPIPE,SIGSEGV,SIGFPE};
	int i,count;

	count=sizeof(signals)/sizeof(int);
	for(i=0;i<count;++i)
	{
		signal(signals[i],proc_process_sig);
	}
	return 0;
}

void pid_daemonize()
{
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	if (0 != fork()) exit(0);
	if (-1 == setsid()) exit(0);
	signal(SIGHUP, SIG_IGN);
	if (0 != fork()) exit(0);
	//if (0 != chdir("/")) exit(0);
}

void pid_save_file(pid_t pid,char* pf)
{
	char buf[32];
	int n;

	n=sprintf(buf,"%d",pid);
	file_write_buf(pf,buf,n);
}

void pid_delete_file(pid_t pid,char* pf)
{
	//char buf[32];

	//sprintf(buf,"%d",pid);
	if(wtk_file_exist(pf)==0)
	{
		remove(pf);
	}
}

int pid_from_file(char *fn)
{
	int id=-1,n;
	char *p;

	if(wtk_file_exist(fn)!=0){goto end;}
	p=file_read_buf(fn,&n);
	id=wtk_str_atoi(p,n);
	wtk_free(p);
end:
	return id;
}

