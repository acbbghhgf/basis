#include <stdio.h>
#include <stdarg.h>
#include "wtk_ps.h"
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "wtk/os/wtk_fd.h"


wtk_ps_t* wtk_ps_new(wtk_ps_cfg_t *cfg)
{
	wtk_ps_t *ps;

	ps=(wtk_ps_t*)wtk_calloc(1,sizeof(*ps));
	ps->cfg=cfg;
	//ps->buf=wtk_strbuf_new(1024,1);
	ps->tmp=(char*)wtk_malloc(cfg->buf_size);
	return ps;
}

void wtk_ps_delete(wtk_ps_t *p)
{
	wtk_psys_clean(&(p->sys));
	wtk_free(p->tmp);
	//wtk_strbuf_delete(p->buf);
	wtk_free(p);
}

int wtk_ps_process(wtk_ps_t *p,wtk_strbuf_t *buf,char *cmd,...)
{
	wtk_psys_t *s=&(p->sys);
	va_list ap;
	wtk_string_t *v;
	int ret,b=0;
	//wtk_strbuf_t *buf=p->buf;

	ret=wtk_psys_init(s,cmd);
	if(ret!=0)
	{
		wtk_debug("create system[%s] failed.\n",cmd);
		goto end;
	}
	va_start(ap,cmd);
	b=1;
	while(1)
	{
		v=va_arg(ap,wtk_string_t*);
		if(!v){break;}
		//wtk_debug("[%.*s]\n",v->len,v->data);
		ret=wtk_psys_write(s,(char*)&(v->len),4);
		if(ret!=4){ret=-1;goto end;}
		if(v->len>0)
		{
			ret=wtk_psys_write(s,v->data,v->len);
			if(ret!=v->len){ret=-1;goto end;}
		}
	}
	wtk_ps_read_result(p,buf);
end:
	if(b)
	{
		va_end(ap);
	}
	return ret;
}

int wtk_ps_start(wtk_ps_t *p,char *cmd)
{
	int ret;

	ret=wtk_psys_init(&(p->sys),cmd);
	if(ret!=0){goto end;}
end:
	return ret;
}

void wtk_ps_stop(wtk_ps_t *p)
{
	wtk_psys_clean(&(p->sys));
}

int wtk_ps_restart(wtk_ps_t *p)
{
	int ret;

	wtk_ps_stop(p);
	ret=wtk_ps_start(p,p->cfg->cmd_fn);
	return ret;
}

int wtk_ps_write_arg(wtk_ps_t *p,wtk_string_t *arg)
{
	int ret;

	ret=wtk_psys_write(&(p->sys),(char*)&(arg->len),4);
	if(ret!=4){ret=-1;goto end;}
	if(arg->len>0)
	{
//		print_data(arg->data,arg->len);
		ret=wtk_psys_write(&(p->sys),arg->data,arg->len);
		if(ret!=arg->len){ret=-1;goto end;}
	}
	ret=0;
end:
	return ret;
}


int wtk_ps_write_arg_line(wtk_ps_t *p,wtk_string_t *arg)
{
	int ret;
	char c='\n';

	if(arg && arg->len>0)
	{
		ret=wtk_psys_write(&(p->sys),arg->data,arg->len);
		if(ret!=arg->len){ret=-1;goto end;}
	}
	wtk_psys_write(&(p->sys),&c,1);
	ret=0;
end:
	return ret;
}

int wtk_ps_read_result(wtk_ps_t *p,wtk_strbuf_t *buf)
{
	char *tmp=p->tmp;
	int n=p->cfg->buf_size;
	int ret;
	int fd=p->sys.parent_read_fd[0];
	fd_set r_set;
	fd_set e_set;
	struct timeval timeout,*v;
	double t;

	if(p->cfg->select_timeout<0)
	{
		v=0;
	}else
	{
		timeout.tv_sec=p->cfg->select_timeout/1000;
		//100 000 000
		timeout.tv_usec=(p->cfg->select_timeout%1000)*1E3;
		v=&(timeout);
		//wtk_debug("sec=%d,usec=%d\n",(int)timeout.tv_sec,(int)timeout.tv_usec);
	}
	FD_ZERO(&(r_set));
	FD_ZERO(&(e_set));
	wtk_strbuf_reset(buf);
	t=time_get_ms();
	wtk_fd_set_nonblock(fd);
	while(1)
	{
		FD_SET(fd,&(r_set));
		FD_SET(fd,&(e_set));
		//wtk_debug("wait v=%p.....\n",v);
		ret=select(fd+1,&(r_set),0,&(e_set),v);
		if(ret<0)
		{
			break;
		}
		if(p->cfg->timeout>0)
		{
			if((time_get_ms()-t)>p->cfg->timeout)
			{
				//wtk_debug("dt=%f\n",time_get_ms()-t);
				ret=-1;
				goto end;
			}
		}
		if(ret==0)
		{
			continue;
		}
		if(FD_ISSET(fd,&(r_set)))
		{
			ret=wtk_psys_read(&(p->sys),tmp,n);
			//wtk_debug("ret=%d\n",ret);
			//print_data(tmp,ret);
			if(ret<0)
			{
				goto end;
			}else if(ret==0)
			{
				break;
			}
			wtk_strbuf_push(buf,tmp,ret);
		}
		if(FD_ISSET(fd,&(e_set)))
		{
			ret=-1;
			goto end;
		}
	}
	ret=0;
end:
	return ret;
}

int wtk_ps_read_result2(wtk_ps_t *p,wtk_strbuf_t *buf)
{
	char *tmp=p->tmp;
	int n=p->cfg->buf_size;
	int ret;

	wtk_strbuf_reset(buf);
	while(1)
	{
		ret=wtk_psys_read(&(p->sys),tmp,n);
		//wtk_debug("ret=%d\n",ret);
		//print_data(tmp,ret);
		if(ret<0)
		{
			goto end;
		}else if(ret==0)
		{
			break;
		}
		wtk_strbuf_push(buf,tmp,ret);
	}
	ret=0;
end:
	return ret;
}


int wtk_ps_read_result3(wtk_ps_t *p,wtk_strbuf_t *buf)
{
	char *tmp=p->tmp;
	int n=p->cfg->buf_size;
	int ret;
	int bytes;
	int len;

	wtk_strbuf_reset(buf);
	ret=wtk_psys_read(&(p->sys),(char*)&bytes,4);
	if(ret!=4)
	{
		ret=-1;
		goto end;
	}
	//wtk_debug("ret=%d bytes=%d\n",ret,bytes);
	while(bytes>0)
	{
		len=min(n,bytes);
		ret=wtk_psys_read(&(p->sys),tmp,len);
		if(ret!=len)
		{
			ret=-1;
			goto end;
		}
		wtk_strbuf_push(buf,tmp,len);
		bytes-=n;
	}
	ret=0;
end:
	//wtk_debug("ret=%d\n",ret);
	return ret;
}


int wtk_ps_read_result4(wtk_ps_t *p,wtk_strbuf_t *buf,int seek)
{
	char *tmp=p->tmp;
	int n=p->cfg->buf_size;
	int ret;
	int fd=p->sys.parent_read_fd[0];
	fd_set r_set;
	fd_set e_set;
	struct timeval timeout,*v;
	double t;
	wtk_strmsg_t msg;

	wtk_strmsg_init(&(msg),buf,seek);
	if(p->cfg->select_timeout<0)
	{
		v=0;
	}else
	{
		timeout.tv_sec=p->cfg->select_timeout/1000;
		//100 000 000
		timeout.tv_usec=(p->cfg->select_timeout%1000)*1E3;
		v=&(timeout);
		//wtk_debug("sec=%d,usec=%d\n",(int)timeout.tv_sec,(int)timeout.tv_usec);
	}
	FD_ZERO(&(r_set));
	FD_ZERO(&(e_set));
	t=time_get_ms();
	wtk_fd_set_nonblock(fd);
	while(1)
	{
		FD_SET(fd,&(r_set));
		FD_SET(fd,&(e_set));
		//wtk_debug("wait v=%p.....\n",v);
		ret=select(fd+1,&(r_set),0,&(e_set),v);
		//wtk_debug("ret=%d\n",ret);
		if(ret<0)
		{
			break;
		}
		if(p->cfg->timeout>0)
		{
			if((time_get_ms()-t)>p->cfg->timeout)
			{
				//wtk_debug("dt=%f\n",time_get_ms()-t);
				ret=-1;
				goto end;
			}
		}
		if(ret==0)
		{
			//perror(__FUNCTION__);
			continue;
		}
		if(FD_ISSET(fd,&(r_set)))
		{
			ret=wtk_psys_read(&(p->sys),tmp,n);
			//wtk_debug("ret=%d\n",ret);
			//print_data(tmp,ret);
			if(ret<0)
			{
				goto end;
			}else if(ret==0)
			{
				break;
			}
			ret=wtk_strmsg_feed(&(msg),tmp,ret,NULL);
			if(ret!=0){goto end;}
			if(wtk_strmsg_is_filled(&(msg)))
			{
				ret=0;
				goto end;
			}
		}
		if(FD_ISSET(fd,&(e_set)))
		{
			ret=-1;
			goto end;
		}
	}
	ret=0;
end:
	wtk_fd_set_block(fd);
	return ret;
}




int wtk_ps_process2(wtk_ps_t *p,wtk_strbuf_t *buf,char *cmd,...)
{
	wtk_psys_t *s=&(p->sys);
	char *tmp=p->tmp;
	int n=p->cfg->buf_size;
	va_list ap;
	wtk_string_t *v;
	int ret,b=0;
	//wtk_strbuf_t *buf=p->buf;

	ret=wtk_psys_init(s,cmd);
	if(ret!=0)
	{
		wtk_debug("create system[%s] failed.\n",cmd);
		goto end;
	}
	va_start(ap,cmd);
	b=1;
	while(1)
	{
		v=va_arg(ap,wtk_string_t*);
		if(!v){break;}
		//wtk_debug("[%.*s]\n",v->len,v->data);
		ret=wtk_psys_write(s,(char*)&(v->len),4);
		if(ret!=4){ret=-1;goto end;}
		if(v->len>0)
		{
			ret=wtk_psys_write(s,v->data,v->len);
			if(ret!=v->len){ret=-1;goto end;}
		}
	}
	wtk_strbuf_reset(buf);
	while(1)
	{
		ret=wtk_psys_read(s,tmp,n);
		if(ret<0)
		{
			goto end;
		}else if(ret==0)
		{
			break;
		}
		wtk_strbuf_push(buf,tmp,ret);
	}
	//print_data(buf->data,buf->pos);
end:
	if(b)
	{
		va_end(ap);
	}
	return ret;
}

