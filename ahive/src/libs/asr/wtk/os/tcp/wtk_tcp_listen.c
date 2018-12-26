#include "wtk_tcp_listen.h"

wtk_tcp_listen_t* wtk_tcp_listen_new(wtk_tcp_listen_cfg_t *cfg)
{
	wtk_tcp_listen_t *t;

	t=(wtk_tcp_listen_t*)wtk_malloc(sizeof(wtk_tcp_listen_t));
	t->cfg=cfg;
	t->fd=-1;
	memset(&(t->addr),0,sizeof(t->addr));
	t->notify_ths=NULL;
	t->notify=NULL;
	t->run=0;
	return t;
}

void wtk_tcp_listen_delete(wtk_tcp_listen_t *l)
{
	if(l->fd>0)
	{
		close(l->fd);
	}
	wtk_free(l);
}

void wtk_tcp_listen_set_notify(wtk_tcp_listen_t *l,void *ths,wtk_tcp_listen_notify_f notify)
{
	l->notify_ths=ths;
	l->notify=notify;
}


int wtk_tcp_listen_listen(wtk_tcp_listen_t *l)
{
	int reuse;
	int fd;
	int ret=-1;

	l->addr.sin_family=AF_INET;
	l->addr.sin_addr.s_addr=htonl(INADDR_ANY);
	l->addr.sin_port=htons(l->cfg->port);
	fd=socket(l->addr.sin_family,SOCK_STREAM,0);
	if(fd<0)
	{
		wtk_debug("create socket failed\n");
		goto end;
	}
	if(l->cfg->reuse)
	{
		reuse=1;
		ret=setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char*)&reuse,sizeof(reuse));
		if(ret!=0)
		{
			 perror(__FUNCTION__);
			goto end;
		}
	}
	ret=bind(fd,(struct sockaddr*)&(l->addr),sizeof(l->addr));
	if(ret!=0)
	{
		perror(__FUNCTION__);
		goto end;
	}
	ret=listen(fd,l->cfg->backlog);
	if(ret!=0)
	{
		perror(__FUNCTION__);
		goto end;
	}
	ret=0;
end:
	if(ret!=0)
	{
		if(fd>0)
		{
			close(fd);
		}
	}else
	{
		l->fd=fd;
	}
	return ret;
}

int wtk_tcp_listen_run(wtk_tcp_listen_t *l)
{
	struct sockaddr_in addr;
	socklen_t len=sizeof(addr);
	int fd;
	int ret;

	l->run=1;
	ret=wtk_tcp_listen_listen(l);
	if(ret!=0){goto end;}
	wtk_debug("listen %d\n",l->cfg->port);
	while(l->run)
	{
		wtk_debug("wait accpet\n");
		fd=accept(l->fd,(struct sockaddr*)&(addr),&len);
		if(fd<0)
		{
			wtk_debug("accept failed.\n");
			goto end;
		}
		{
			char *p;
			char buf[64];
			int port;

			getpeername(fd,(struct sockaddr*)&(addr),&len);
			p=(char*)inet_ntop(AF_INET,&(addr.sin_addr),buf,sizeof(buf));
			port=ntohs(addr.sin_port);
			wtk_debug("accept %s:%d\n",p,port);
		}
		if(l->notify)
		{
			wtk_debug("notify start\n");
			l->notify(l->notify_ths,fd);
			wtk_debug("notify end\n");
		}
	}
	ret=0;
end:
	return ret;
}
