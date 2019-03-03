#include "wtk_relay_host.h"
void wtk_relay_host_close(wtk_relay_host_t *host);
int wtk_relay_host_connect(wtk_relay_host_t *host,int *pfd);

wtk_relay_host_t* wtk_relay_host_new(wtk_relay_host_cfg_t *cfg,wtk_strbuf_t *tmp_buf)
{
	wtk_relay_host_t *host;

	host=(wtk_relay_host_t*)wtk_malloc(sizeof(*host));
	host->cfg=cfg;
	host->tmp_buf=tmp_buf;
	host->response=wtk_http_response_new();
	host->fd=INVALID_FD;
	wtk_relay_host_connect(host,&(host->fd));
	return host;
}

void wtk_relay_host_delete(wtk_relay_host_t *host)
{
	wtk_relay_host_close(host);
	wtk_http_response_delete(host->response);
	wtk_free(host);
}

void wtk_relay_host_close(wtk_relay_host_t *host)
{
	if(host->fd>0)
	{
		wtk_socket_close_fd(host->fd);
		host->fd=-1;
	}
}

int wtk_relay_host_connect(wtk_relay_host_t *host,int *pfd)
{
	int fd=INVALID_FD;
	int ret=-1;

	fd=socket(AF_INET,SOCK_STREAM,0);
	if(fd<0){goto end;}
	ret=connect(fd,host->cfg->addr->addr,host->cfg->addr->addrlen);
	if(ret!=0){goto end;}
	*pfd=fd;
end:
	if((ret!=0) && (fd>0))
	{
		wtk_socket_close_fd(fd);
	}
	return ret;
}

int wtk_relay_host_reconnect(wtk_relay_host_t *host)
{
	wtk_relay_host_close(host);
	return wtk_relay_host_connect(host,&(host->fd));
}

int wtk_relay_host_prepare(wtk_relay_host_t *host)
{
	int ret;

	if(host->fd<0)
	{
		ret=wtk_relay_host_connect(host,&(host->fd));
	}else
	{
		ret=0;
	}
	return ret;
}

int wtk_relay_host_process(wtk_relay_host_t *host,wtk_request_t *req)
{
	wtk_strbuf_t *buf=host->tmp_buf;
	wtk_http_response_t *rep=host->response;
	int writed;
	int ret;

	//wtk_request_print(req);
	ret=wtk_relay_host_prepare(host);
	if(ret!=0){goto end;}
	wtk_strbuf_reset(buf);
	wtk_request_tobin2(req,buf,0,0);
	ret=wtk_fd_send(host->fd,buf->data,buf->pos,&writed);
	if(ret!=0)
	{
		//if fd has error,reconnect;
		ret=wtk_relay_host_reconnect(host);
		if(ret!=0){goto end;}
		ret=wtk_fd_send(host->fd,buf->data,buf->pos,&writed);
	}
	if(ret!=0 && writed!=buf->pos)
	{
		ret=-1;goto end;
	}
	ret=wtk_http_response_feed_fd(rep,host->fd,buf);
end:
	if(ret!=0)
	{
		wtk_request_set_disconnect_err(req);
		wtk_relay_host_close(host);
		ret=0;
	}else
	{
		wtk_request_update_response(req,rep);
	}
	//wtk_http_response_print(rep);
	//wtk_debug("ret=%d,status=%d,%d\n",ret,req->response->status,rep->status);
	return ret;
}
