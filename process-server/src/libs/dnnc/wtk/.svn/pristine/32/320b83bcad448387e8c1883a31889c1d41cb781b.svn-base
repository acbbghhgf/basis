#include "wtk_listen.h" 

wtk_listen_t* wtk_listen_new(wtk_listen_cfg_t *cfg)
{
	wtk_listen_t *l;

	l=(wtk_listen_t*)wtk_malloc(sizeof(wtk_listen_t));
	l->cfg=cfg;
	l->fd=-1;
	return l;
}

void wtk_listen_delete(wtk_listen_t *l)
{
	if(l->fd>=0)
	{

	}
	wtk_free(l);
}

int wtk_listen_start(wtk_listen_t *l)
{
	wtk_listen_cfg_t *cfg=l->cfg;
	int fd,ret;
	socklen_t len;
	struct sockaddr_in addr={0,};
	int i;

	if(l->fd>=0){ret=0;goto end;};
	ret=-1;
	addr.sin_family=AF_INET;
    addr.sin_addr.s_addr = htonl(cfg->loop_back ? INADDR_LOOPBACK : INADDR_ANY);
	addr.sin_port = htons(cfg->port);
	fd=socket(addr.sin_family,SOCK_STREAM,0);
	if(fd==-1)
	{
        wtk_debug("create socket failed\n");
        perror(__FUNCTION__);
		goto end;
	}
	if(cfg->reuse)
	{
		i=1;
		ret=setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char*)&i,sizeof(i));
		if(ret!=0)
		{
			 perror(__FUNCTION__);
			goto end;
		}
	}
	//this is closed for non-block connect.
	if(cfg->defer_accept_timeout>0)
	{
		i=cfg->defer_accept_timeout;
		ret=setsockopt(fd,IPPROTO_TCP,TCP_DEFER_ACCEPT,&i,sizeof(i));
		if(ret!=0)
		{
			perror(__FUNCTION__);
			goto end;
		}
	}
	ret=bind(fd,(struct sockaddr*)&(addr),sizeof(addr));
	if(ret!=0)
	{
        wtk_debug("%d port already in use.\n",cfg->port);
		//perror(__FUNCTION__);
		goto end;
	}
    ret=wtk_fd_set_nonblock(fd);
    if(ret!=0)
    {
        wtk_debug("set %d port nonblock failed.\n",cfg->port);
    	//perror(__FUNCTION__);
    	goto end;
    }
	ret=listen(fd,cfg->backlog);
end:
	if(ret!=0)
	{
		wtk_listen_stop(l);
	}else
	{
		l->fd=fd;
	}
	return ret;
}

void wtk_listen_stop(wtk_listen_t *l)
{
	if(l->fd>=0)
	{
		close(l->fd);
		l->fd=-1;
	}
}
