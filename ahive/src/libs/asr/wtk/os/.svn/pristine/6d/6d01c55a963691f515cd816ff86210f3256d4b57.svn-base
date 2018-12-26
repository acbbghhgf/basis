#include "wtk_pipeproc.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/prctl.h>
#include <errno.h>
int wtk_pipeproc_get_read_fd(wtk_pipeproc_t *proc);
int wtk_pipeproc_get_write_fd(wtk_pipeproc_t *proc);

static wtk_pipeproc_t* magic_proc;

//this is now used for clean all opened fd.
static pipeproc_init_handler module_init_handler;
static void* moudle_init_data;

void wtk_pipeproc_set_process_init_handler(pipeproc_init_handler handler,void *data)
{
	module_init_handler=handler;
	moudle_init_data=data;
}

wtk_pipeproc_t* wtk_pipeproc_new(char *name,pipeproc_init_handler init,
		pipeproc_clean_handler clean,pipeproc_handler handler,void *user_data)
{
	return wtk_pipeproc_new2(name,init,clean,handler,user_data,0);
}

wtk_pipeproc_t* wtk_pipeproc_new2(char *name,pipeproc_init_handler init,
		pipeproc_clean_handler clean,pipeproc_handler handler,void *user_data,
		int sync)
{
	wtk_pipeproc_t *p;
	int ret;

	p=(wtk_pipeproc_t*)wtk_malloc(sizeof(*p));
	p->sync=sync;
	ret=wtk_pipeproc_init(p,name,init,clean,handler,user_data);
	if(ret!=0){goto end;}
	ret=wtk_pipeproc_start(p);
end:
	if(ret!=0)
	{
		wtk_pipeproc_delete(p);
		p=0;
	}
	return p;
}

int wtk_pipeproc_delete(wtk_pipeproc_t *p)
{
	wtk_pipeproc_clean(p);
	wtk_pipeproc_kill(p);
	wtk_free(p);
	return 0;
}

int wtk_pipeproc_init(wtk_pipeproc_t *p,char *name,pipeproc_init_handler init,pipeproc_clean_handler clean,pipeproc_handler handler,void *user_data)
{
	int i,ret;

	p->name=name;
	p->init=init;
	p->clean=clean;
	p->handler=handler;
	p->user_data=user_data;
	for(i=0;i<2;++i)
	{
		ret=pipe((p->pipe_fds[i]));
		//wtk_debug("pid=%d,p=%p,fd=%d,%d\n",getpid(),p,p->pipe_fds[i][0],p->pipe_fds[i][1]);
		if(ret!=0){goto end;}
	}
end:
	return ret;
}

int wtk_pipeproc_clean(wtk_pipeproc_t *p)
{
	int i,j;

    for(i=0;i<2;++i)
    {
    	for(j=0;j<2;++j)
    	{
    		if(p->pipe_fds[i][j]>0)
    		{
    			//wtk_debug("pid=%d,p=%p,fd=%d\n",getpid(),p,p->pipe_fds[i][j]);
    			close(p->pipe_fds[i][j]);
    			p->pipe_fds[i][j]=0;
    		}
    	}
    }
    return 0;
}

int wtk_pipeproc_close_nonread(wtk_pipeproc_t* proc)
{
	if(proc->pipe_fds[proc->pipe_wfd_index][0]>0)
	{
		//wtk_debug("pid=%d,p=%p,fd=%d\n",getpid(),proc,proc->pipe_fds[proc->pipe_wfd_index][0]);
		close(proc->pipe_fds[proc->pipe_wfd_index][0]);
		proc->pipe_fds[proc->pipe_wfd_index][0]=0;
	}
	return 0;
}

int wtk_pipeproc_close_nonwrite(wtk_pipeproc_t* proc)
{
	if(proc->pipe_fds[1-proc->pipe_wfd_index][1]>0)
	{
		//wtk_debug("pid=%d,p=%p,fd=%d\n",getpid(),proc,proc->pipe_fds[1-proc->pipe_wfd_index][1]);
		close(proc->pipe_fds[1-proc->pipe_wfd_index][1]);
		proc->pipe_fds[1-proc->pipe_wfd_index][1]=0;
	}
	return 0;
}

void wtk_pipeproc_close_proc_unuse_fd(wtk_pipeproc_t *proc)
{
	int rfd,wfd;
	int i,nfd;

	rfd=wtk_pipeproc_get_read_fd(proc);
	wfd=wtk_pipeproc_get_write_fd(proc);
	//wtk_debug("pid=%d,close fd=[%d,%d]\n",getpid(),rfd,wfd);
	nfd=sysconf(_SC_OPEN_MAX);
	for(i=3;i<nfd;++i)
	{
		if(i==rfd || i==wfd)
		{

		}else
		{
			//wtk_debug("i=%d\n",i);
			close(i);
		}
	}
}

int wtk_pipeproc_close_unuse_fd(wtk_pipeproc_t* proc)
{
	wtk_pipeproc_close_nonread(proc);
	wtk_pipeproc_close_nonwrite(proc);
	return 0;
}

int wtk_pipeproc_init_child_env(wtk_pipeproc_t* proc)
{
	if(proc->name)
	{
		//wtk_debug("set name=%s\n",proc->name);
		prctl(PR_SET_NAME,proc->name);
	}
	//prctl(PR_SET_PDEATHSIG,SIGQUIT);
	prctl(PR_SET_PDEATHSIG,SIGUSR1);
	return 0;
}

int wtk_pipeproc_get_read_fd(wtk_pipeproc_t *proc)
{
	return proc->pipe_fds[1-proc->pipe_wfd_index][0];
}

int wtk_pipeproc_get_write_fd(wtk_pipeproc_t *proc)
{
	return proc->pipe_fds[proc->pipe_wfd_index][1];
}

int wtk_pipeproc_read(wtk_pipeproc_t* proc,char* buf,int len)
{
	int fd=proc->pipe_fds[1-proc->pipe_wfd_index][0];
	int readed=0;
	int ret;

	ret=fd_read(fd,buf,len,&readed);
	ret=len==readed ? 0 :-1;
	//perror(__FUNCTION__);
	//if(ret!=0){wtk_debug("ret=%d,len=%d,readed=%d\n",ret,len,readed);}
	return ret;
}

int wtk_pipeproc_write(wtk_pipeproc_t* proc,char* buf,int len)
{
	int fd=proc->pipe_fds[proc->pipe_wfd_index][1];
	int writed;
	int ret;

	fd_write(fd,buf,len,&writed);
	ret=len==writed ? 0 : -1;
	if(ret!=0)
	{
		perror(__FUNCTION__);
		//wtk_debug("pid=%d,len=%d,writed=%d,errno=%d\n",getpid(),len,writed,errno);
	}
	return ret;
}

wtk_string_t* wtk_pipeproc_read_string(wtk_pipeproc_t *p)
{
	int read_length;
	wtk_string_t* s;
	int ret;

	s=0;
	read_length=0;
	ret=wtk_pipeproc_read(p,(char*)&read_length,sizeof(read_length));
	if(ret!=0 || read_length<=0)
	{
		//wtk_debug("pid=%d,readfd=%d\n",getpid(),p->pipe_fds[1-p->pipe_wfd_index][0]);
		perror(__FUNCTION__);
		//wtk_debug("read length failed(proc=%d,errno=%d,ret=%d,read_length=%d).\n",getpid(),errno,ret,read_length);
		goto end;
	}
	s=wtk_string_new(read_length);
	ret=wtk_pipeproc_read(p,s->data,read_length);
	if(ret!=0)
	{
		perror(__FUNCTION__);
		//wtk_debug("read length failed(proc=%d,errno=%d,ret=%d,read_length=%d).\n",getpid(),errno,ret,read_length);
		wtk_string_delete(s);
		s=0;
	}
end:
	return s;
}

int wtk_pipeproc_read_string2(wtk_pipeproc_t *p,wtk_strbuf_t *buf)
{
	int read_length;
	int ret;

	read_length=0;
	wtk_strbuf_reset(buf);
	ret=wtk_pipeproc_read(p,(char*)&read_length,sizeof(read_length));
	if(ret!=0 || read_length<=0)
	{
		perror(__FUNCTION__);
		//wtk_debug("read length failed(proc=%d,errno=%d,ret=%d,read_length=%d).\n",getpid(),errno,ret,read_length);
		ret=-1;
		goto end;
	}
	wtk_strbuf_expand(buf,read_length);
	ret=wtk_pipeproc_read(p,buf->data,read_length);
	if(ret!=0)
	{
		perror(__FUNCTION__);
		//wtk_debug("read length failed(proc=%d,errno=%d,ret=%d,read_length=%d).\n",getpid(),errno,ret,read_length);
		ret=-1;
		goto end;
	}
	buf->pos=read_length;
	ret=0;
end:
	return ret;
}

int wtk_pipeproc_read_stack(wtk_pipeproc_t *p,wtk_stack_t *stack)
{
	int read_length;
	int ret;
	int i,n;
	char buf[4096];

	read_length=0;
	wtk_stack_reset(stack);
	ret=wtk_pipeproc_read(p,(char*)&read_length,sizeof(read_length));
	//wtk_debug("%d,read_length: %d\n",getpid(),read_length);
	if(ret!=0 || read_length<=0)
	{
		perror(__FUNCTION__);
		//wtk_debug("read length failed(proc=%d,errno=%d,ret=%d,read_length=%d).\n",getpid(),errno,ret,read_length);
		ret=-1;
		goto end;
	}
	i=0;
	while(i<read_length)
	{
		n=min(read_length-i,sizeof(buf));
		ret=wtk_pipeproc_read(p,buf,n);
		if(ret!=0){goto end;}
		i+=n;
		wtk_stack_push(stack,buf,n);
		//wtk_debug("pid=%d,n=%d,i=%d,len=%d\n",getpid(),n,i,stack->len);
	}
	ret=0;
end:
	return ret;
}

int wtk_pipeproc_write_string(wtk_pipeproc_t* proc,char* buf,int len)
{
	int length;
	int ret;

	length=len;
	ret=wtk_pipeproc_write(proc,(char*)&length,sizeof(length));
	if(ret!=0){goto end;}
	ret=wtk_pipeproc_write(proc,buf,len);
end:
	return ret;
}

int wtk_pipeproc_write_stack(wtk_pipeproc_t* proc,wtk_stack_t *s)
{
	stack_block_t *b;
	int ret,count,t;
	int32_t length;

	length=s->len;
	ret=wtk_pipeproc_write(proc,(char*)&length,sizeof(length));
	//wtk_debug("ret=%d\n",ret);
	if(ret!=0){goto end;}
	count=0;
	for(b=s->pop;b;b=b->next)
	{
		t=b->push-b->pop;
		//wtk_debug("%p\n",b->pop);
		ret=wtk_pipeproc_write(proc,b->pop,t);
		//wtk_debug("ret=%d,t=%d\n",ret,t);
		if(ret!=0){goto end;}
		count+=t;
	}
	ret=count==length?0:-1;
end:
	if(ret!=0)
	{

	}
	return ret;
}

int wtk_pipeproc_loop(wtk_pipeproc_t *proc)
{
	wtk_string_t *req,*rep;
	int ret;

	if(proc->init)
	{
		ret=proc->init(proc->user_data,proc);
		if(ret !=0)
		{
			//wtk_debug("init failed.\n");
			goto end;
		}
	}
	while(proc->run)
	{
		req=wtk_pipeproc_read_string(proc);
		if(!req){break;}
		rep=proc->handler(proc->user_data,req);
		wtk_string_delete(req);
		if(!rep)
		{
			if(!proc->sync)
			{
				continue;
			}else
			{
				break;
			}
		}
		ret=wtk_pipeproc_write_string(proc,rep->data,rep->len);
		wtk_string_delete(rep);
		if(ret!=0){break;}
	}
	if(proc->clean)
	{
		ret=proc->clean(proc->user_data);
	}
end:
	//wtk_debug("%p exit.\n",proc);
	return ret;
}

//#include <mcheck.h>

int wtk_pipeproc_start(wtk_pipeproc_t* proc)
{
	int ret;
	pid_t pid;

	proc->run=1;
	pid=fork();
	if(pid==0)
	{
		magic_proc=proc;
		pid=getpid();
		pid_setup_signal(pid);
		wtk_pipeproc_init_child_env(proc);
		proc->pipe_wfd_index=1;
		wtk_pipeproc_close_unuse_fd(proc);
		wtk_pipeproc_close_proc_unuse_fd(proc);
		if(module_init_handler)
		{
			ret=module_init_handler(moudle_init_data,proc);
			if(ret!=0)
			{
				wtk_debug("init pipe process module failed.\n");
				exit(0);
			}
		}
		ret=wtk_pipeproc_loop(proc);
		//wtk_debug("proc %d exit.\n",pid);
		exit(ret);
	}else if(pid>0)
	{
		proc->pipe_wfd_index=0;
		wtk_pipeproc_close_unuse_fd(proc);
		//wtk_debug("create proc %s (%d) .\n",proc->name?proc->name:"anon",pid);
		proc->pid=pid;
		ret=0;
	}else
	{
		ret=-1;
	}
	return ret;
}

int wtk_pipeproc_join(wtk_pipeproc_t *proc)
{
	pid_t pid;
	int status;

	pid=waitpid(proc->pid,&status,0);
	return pid==proc->pid?0:-1;
}

int wtk_pipeproc_wait(wtk_pipeproc_t *proc, int timeout) {
    pid_t pid;
    int status;
    int sleep_time;

    if (-1 == timeout) {
        return wtk_pipeproc_join(proc);
    }

    while (1) {
        pid = waitpid(proc->pid, &status, WNOHANG);
        if (pid) break;
        sleep_time = min(timeout, 10);
        if (sleep_time) {
            usleep(sleep_time * 1000);
        }
        timeout -= sleep_time;
        if (0 == timeout) break;
    }

    return pid == proc->pid ? 0 : -1;
}

int wtk_pipeproc_kill(wtk_pipeproc_t* proc)
{
	int ret;

	if(proc->pid<=0){return 0;}
	ret=kill(proc->pid,SIGKILL);
	if(ret==0)
	{
		waitpid(proc->pid,0,0);
		proc->pid=0;
	}
	return ret;
}
