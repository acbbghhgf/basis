#include "wtk_socketproc.h"
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/prctl.h>

static wtk_socketproc_t* magic_proc;

wtk_socketproc_t* wtk_socketproc_new(char* name,pipeproc_init_handler init,
		pipeproc_clean_handler clean,pipeproc_handler handler,void *data)
{
	wtk_socketproc_t* proc;
	int ret;

	proc=(wtk_socketproc_t*)wtk_calloc(1,sizeof(*proc));
	proc->name=name;
	proc->init=init;
	proc->clean=clean;
	proc->handler=handler;
	proc->user_data=data;
	ret=socketpair(AF_LOCAL,SOCK_STREAM,0,proc->v);
	if(ret!=0){goto end;}
	ret=wtk_socketproc_start(proc);
end:
	if(ret!=0)
	{
		perror(__FUNCTION__);
		wtk_free(proc);
		proc=0;
	}
	return proc;
}

int wtk_socketproc_delete(wtk_socketproc_t *p)
{
	close(wtk_socketproc_fd(p));
	pid_kill(p->pid);
	wtk_free(p);
	return 0;
}

int wtk_socketproc_loop(wtk_socketproc_t *proc)
{
	wtk_string_t *req,*rep;
	int ret;
	int fd;

	fd=0;
	if(proc->init)
	{
		ret=proc->init(proc->user_data,0);
		if(ret !=0){goto end;};
	}
	fd=wtk_socketproc_fd(proc);
	while(1)
	{
		req=wtk_fd_read_string(fd);
		if(!req){break;}
		rep=proc->handler(proc->user_data,req);
		wtk_string_delete(req);
		if(!rep){break;}
		ret=wtk_fd_write_string(fd,rep->data,rep->len);
		wtk_string_delete(rep);
		if(ret!=0){break;}
	}
	if(proc->clean)
	{
		ret=proc->clean(proc->user_data);
	}
end:
	if(fd>0){close(fd);}
	return ret;
}


int wtk_socketproc_start(wtk_socketproc_t* proc)
{
	int ret;
	pid_t pid;

	pid=fork();
	if(pid==0)
	{
		magic_proc=proc;
		pid=getpid();
		//pid_setup_signal(pid);
		if(proc->name)
		{
			prctl(PR_SET_NAME,proc->name);
		}
		proc->vi=1;
		close(proc->v[0]);
		ret=wtk_socketproc_loop(proc);
		wtk_debug("proc %d exit.\n",pid);
		exit(ret);
	}else if(pid>0)
	{
		proc->vi=0;
		close(proc->v[1]);
		wtk_debug("create proc %s (%d) .\n",proc->name?proc->name:"anon",pid);
		proc->pid=pid;
		ret=0;
	}else
	{
		ret=-1;
	}
	return ret;
}

