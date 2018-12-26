#include <sys/types.h>
#include <sys/wait.h>
#include "wtk_psys.h"
#include "wtk/core/wtk_os.h"
#include <sys/prctl.h>

int wtk_psys_init(wtk_psys_t *s,char *cmd)
{
	int ret;

	s->parent_write_fd[0]=-1;
	s->parent_write_fd[1]=-1;
	s->parent_read_fd[0]=-1;
	s->parent_read_fd[1]=-1;
	ret=pipe(s->parent_read_fd);
	if(ret!=0){goto end;}
	ret=pipe(s->parent_write_fd);
	if(ret!=0){goto end;}
	s->pid=fork();
	if(s->pid<0){ret=-1;goto end;}
	if(s->pid==0)
	{
		//child;
		//wtk_debug("pid=%d:%d\n",getppid(),getpid());
//		prctl(PR_SET_PDEATHSIG,SIGUSR1);
		close(s->parent_write_fd[1]);
		close(s->parent_read_fd[0]);
		dup2(s->parent_write_fd[0],0);
		dup2(s->parent_read_fd[1],1);
		//wtk_debug("run exit %s ...\n",cmd);
//		ret=system(cmd);
		//perror(__FUNCTION__);
		ret=execl(cmd,cmd,NULL);
		if(ret==-1)
		{
			close(s->parent_write_fd[0]);
			close(s->parent_read_fd[1]);
			close(0);
			close(1);
			exit(1);
		}
		close(s->parent_write_fd[0]);
		close(s->parent_read_fd[1]);
		close(0);
		close(1);
		//wait for parent to kill to make valgrind happy.
		while(1)
		{
			wtk_msleep(100000);
			//getchar();
		}
		exit(0);
	}else
	{
		//parent;
		close(s->parent_write_fd[0]);
		s->parent_write_fd[0]=-1;
		close(s->parent_read_fd[1]);
		s->parent_read_fd[1]=-1;
	}
end:
	return ret;
}

void wtk_psys_clean(wtk_psys_t *s)
{
	//wtk_debug("pid=%d:%d:%d\n",getppid(),getpid(),s->pid);
	if(s->parent_write_fd[0]>0)
	{
		close(s->parent_write_fd[0]);
	}
	if(s->parent_write_fd[1]>0)
	{
		close(s->parent_write_fd[1]);
	}
	if(s->parent_read_fd[0]>0)
	{
		close(s->parent_read_fd[0]);
	}
	if(s->parent_read_fd[1]>0)
	{
		close(s->parent_read_fd[1]);
	}
	if(s->pid>0)
	{
		//wtk_debug("kill ...\n");
		if(kill(s->pid,0)==0)
		{
			kill(s->pid,SIGKILL);
		}
		waitpid(s->pid,0,0);
		//wtk_debug("kill ... end\n");
	}
}

int wtk_psys_write(wtk_psys_t *sys,char *data,int bytes)
{
	//print_data(data,bytes);
	return write(sys->parent_write_fd[1],data,bytes);
}

int wtk_psys_read(wtk_psys_t *sys,char *data,int bytes)
{
	return read(sys->parent_read_fd[0],data,bytes);
}
