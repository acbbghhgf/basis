#include "wtk_listen.h"
#include "wtk/http/nk/wtk_connection.h"
#ifdef WIN32
#include <ws2tcpip.h>
#define close closesocket
#endif

int wtk_listen_init(wtk_listen_t* l,wtk_listen_cfg_t *cfg,short port,
		wtk_pop_parser_handler pop,wtk_push_parser_handler push,void *user_data)
{
	memset(l,0,sizeof(*l));
	l->cfg=cfg;
	l->fd=INVALID_FD;
	l->type=SOCK_STREAM;
	l->addr.sin_family=AF_INET;
	l->port=port;
	l->pop_parser=pop;
	l->push_parser=push;
	l->user_data=user_data;
	return 0;
}

void wtk_listen_set_parser_handler(wtk_listen_t *l,void *data,wtk_pop_parser_handler pop,wtk_push_parser_handler push)
{
	l->user_data=data;
	l->pop_parser=pop;
	l->push_parser=push;
}

void wtk_listen_cpy(wtk_listen_t *dst,wtk_listen_t *src)
{
	dst->fd=src->fd;
	dst->addr=src->addr;
	dst->port=src->port;
}

int wtk_listen_listen(wtk_listen_t* l,int loop)
{
	int fd,ret;
	socklen_t len;

	if(l->fd!=INVALID_FD){return 0;};
	ret=-1;
    l->addr.sin_addr.s_addr = htonl(loop ? INADDR_LOOPBACK : INADDR_ANY);
	l->addr.sin_port = htons(l->port);
	fd=socket(l->addr.sin_family,l->type,0);
	if(fd==-1)
	{
        wtk_debug("create socket failed\n");
		//perror(__FUNCTION__);
		goto end;
	}
	if(l->cfg->reuse)
	{
		int reuse=1;

		ret=setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char*)&reuse,sizeof(reuse));
		if(ret!=0)
		{
			 perror(__FUNCTION__);
			goto end;
		}
	}
#ifndef WIN32
	//this is closed for non-block connect.
	if(l->cfg->defer_accept)
	{
		int timeout=l->cfg->defer_accept_timeout;

		ret=setsockopt(fd,IPPROTO_TCP,TCP_DEFER_ACCEPT,&timeout,sizeof(timeout));
		if(ret!=0)
		{
			perror(__FUNCTION__);
			goto end;
		}
	}
#endif
    ret=wtk_fd_set_nonblock(fd);
    if(ret!=0)
    {
        wtk_debug("set %d port nonblock failed.\n",l->port);
    	//perror(__FUNCTION__);
    	goto end;
    }
	ret=bind(fd,(struct sockaddr*)&(l->addr),sizeof(l->addr));
	if(ret!=0)
	{
        wtk_debug("%d port already in use.\n",l->port);
		//perror(__FUNCTION__);
		goto end;
	}
    len=sizeof(l->addr);
    ret=getsockname(fd,(struct sockaddr*)&(l->addr),&len);
    if(ret!=0)
    {
        wtk_debug("get sockname failed.\n");
        //perror(__FUNCTION__);
        goto end;
    }
    l->port=ntohs(l->addr.sin_port);
	ret=listen(fd,l->backlog);
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

void wtk_listen_close_fd(wtk_listen_t *l)
{
	if(l->fd != INVALID_FD)
	{
		wtk_socket_close_fd(l->fd);
		l->fd=INVALID_FD;
	}
}

