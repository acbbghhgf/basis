#include "wtk_connection.h" 
#include "wtk_nk.h"
void wtk_connection_touch_epoll(wtk_connection_t *c);

wtk_connection_t* wtk_connection_new(	wtk_nk_t *nk)
{
	wtk_connection_t *c;

	c=(wtk_connection_t*)wtk_malloc(sizeof(wtk_connection_t));
	c->nk=nk;
	c->fd=-1;
	if(nk->parser_factory.newx)
	{
		c->parser=nk->parser_factory.newx(nk->parser_factory.ths,c);
	}
	c->wbuf=NULL;
	return c;
}

int wtk_connection_delete(wtk_connection_t *c)
{
	if(c->wbuf)
	{
		wtk_buf_delete(c->wbuf);
	}
	if(c->nk->parser_factory.deletex)
	{
		c->nk->parser_factory.deletex(c->parser);
	}
	wtk_free(c);
	return 0;
}


void wtk_connection_init(wtk_connection_t *c,int fd,int evt_type)
{
	socklen_t len;
	int ret;

	wtk_poll_evt_init(&(c->evt),evt_type);
	c->fd=fd;
	if(fd>=0)
	{
		len = sizeof(c->addr);
		if(evt_type & WTK_CONNECT_EVT_ACCEPT)
		{
			ret=getsockname(fd,(struct sockaddr*) &(c->addr), &len);
		}else
		{
			ret=getpeername(fd,(struct sockaddr*) &(c->addr), &len);
		}
		if(ret==0)
		{
			//wtk_sockaddr_print(&(nc->addr));
			inet_ntop(AF_INET,&(c->addr.sin_addr),c->peer_name,sizeof(c->addr));
			c->peer_port=ntohs(c->addr.sin_port);
		}
	}else
	{
		c->peer_name[0]=0;
		c->peer_port=0;
	}
	if(c->nk->parser_factory.init)
	{
		c->nk->parser_factory.init(c->parser);
	}
}

void wtk_connection_clean(wtk_connection_t *c)
{
	if(c->nk->parser_factory.clean)
	{
		c->nk->parser_factory.clean(c->parser);
	}
	if(c->evt.polled)
	{
		wtk_poll_remove_connection(c->nk->poll,c);
	}
	if(c->fd>=0)
	{
		if(c->evt.want_accept==0)
		{
			close(c->fd);
		}
		c->fd=0;
	}
	if(c->wbuf)
	{
		wtk_buf_delete(c->wbuf);
		c->wbuf=NULL;
	}
}

int wtk_connection_set_client_fd_opt(int fd)
{
	int keepalive = 1;
	int reuse = 1;
	int ret;
    int keepalive_time = 3, keepalive_intvl = 3, keepalive_probes = 2;

	ret = wtk_fd_set_nonblock(fd);
	if (ret != 0) {goto end;}
	ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*) &reuse,sizeof(reuse));
	if (ret != 0) {goto end;}
	ret = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char*) (&keepalive),
			(socklen_t) sizeof(keepalive));
	if (ret != 0) {goto end;}

    ret = setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, (void*) (&keepalive_time),
            (socklen_t) sizeof(keepalive_time));
    if (ret != 0) {goto end;}
    ret = setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL,
            (void*) (&keepalive_intvl), (socklen_t) sizeof(keepalive_intvl));
    if (ret != 0) {goto end;}
    ret = setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, (void*) (&keepalive_probes),
            (socklen_t) sizeof(keepalive_probes));
    if (ret != 0) {goto end;}
#ifdef USE_TCP_NODELAY
    {
        int tcp_nodelay = 1;
        ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&tcp_nodelay, sizeof(int));
    }
#endif
end:
	return ret;
}

int wtk_connection_accept(wtk_connection_t *c)
{
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	int fd;
	int ret;
	wtk_connection_t *nc=NULL;
	wtk_nk_t *nk=c->nk;

	fd = accept(c->fd, (struct sockaddr*) &addr, &len);
	if (fd < 0)
	{
		ret=-1;
		goto end;
	}
	if(nk->con_hoard.use_length>nk->cfg->max_connection)
	{
		wtk_log_log(c->nk->log,"max connection[use=%d,max=%d],close it.",nk->con_hoard.use_length,nk->cfg->max_connection);
		close(fd);
		ret=0;
		goto end;
	}
	ret=wtk_connection_set_client_fd_opt(fd);
	if(ret!=0){goto end;}
	nc=wtk_nk_pop_connection(nk);
	wtk_connection_init(nc,fd,WTK_CONNECT_EVT_READ);
	ret=wtk_poll_add_connection(nk->poll,nc);
	if(ret!=0){goto end;}
	wtk_log_log(nk->log,"accept %s:%d.",nc->peer_name,nc->peer_port);
	nc->evt.write=1;
	nc->evt.read=1;
	ret=wtk_connection_process(nc);
	nc=NULL;
end:
	if(ret!=0)
	{
		if(fd>=0)
		{
			close(fd);
		}
		if(nc)
		{
			wtk_connection_clean(nc);
			wtk_nk_push_connection(c->nk, nc);
		}
	}
	return 0;
}

wtk_fd_state_t wtk_connection_read_available(wtk_connection_t* c,int *p_readed)
{
	wtk_poll_evt_t *evt=&(c->evt);
	wtk_nk_t *nk=c->nk;
	wtk_fd_state_t s;
	int ret, readed;
	char *buf;
	int len;
	//int lac,max_active_count;
    int tot_readed;
    wtk_nk_parser_feed_f readf=nk->parser_factory.feed;

	len = nk->cfg->rw_size;
	buf = nk->rw_buf;
	ret = 0;
	s = WTK_OK;
	tot_readed=0;
	while(evt->read)
	{
		s = wtk_fd_recv(c->fd, buf, len, &readed);
		if (s == WTK_EOF||(readed==0 && tot_readed==0 && s==WTK_OK))
		{
            //if read available, but read 0 bytes, for the remote connection is closed. this is for windows.
            s=WTK_EOF;
			break;
		}
		tot_readed+=readed;
		if (readed > 0)
		{
			//print_data(buf,readed);
			if(nk->cfg->log_rcv)
			{
				wtk_log_log(nk->log,"recv=[\n%.*s\n]",readed,buf);
			}
			if(readf)
			{
				ret=readf(c->parser,buf,readed);
			}else
			{
				print_data(buf,readed);
				ret=0;
			}
			if (ret != 0)
			{
				wtk_log_log(nk->log,"%s parse failed(%d),readed=%d",c->peer_name,ret,readed);
				print_hex(buf,min(readed,32));
				s = WTK_ERR;
				break;
			}
		}else
		{
			break;
		}
	}
	if(p_readed)
	{
		*p_readed=tot_readed;
	}
//	{
//		static int ki=0;
//
//		ki+=tot_readed;
//		wtk_debug("=========== readed %d\n",ki);
//	}
	return s;
}


int wtk_connection_read(wtk_connection_t *c)
{
	wtk_poll_evt_t *evt=&(c->evt);
	int ret;
	wtk_fd_state_t s;
	int readed;

	if(evt->want_accept)
	{
		return wtk_connection_accept(c);
	}
	s=wtk_connection_read_available(c,&readed);
	ret = (s == WTK_OK || s == WTK_AGAIN) ? 0 : -1;
	return ret;
}


int wtk_connection_flush(wtk_connection_t *c)
{
	wtk_poll_evt_t *evt=&(c->evt);
	wtk_buf_t *buf=c->wbuf;
	int ret;
	wtk_buf_item_t *item;
	wtk_fd_state_t s;
	int writed;

	if(!buf)
	{
		ret=0;
		evt->writepending=0;
		goto end;
	}
	//wtk_debug("================ flush ---------------------------------------------\n");
	item=buf->front;
	if(item->len>0)
	{
		s=WTK_OK;
		while(item->len>0)
		{
			s=wtk_fd_send(c->fd,item->data+item->pos,item->len,&writed);
			if(writed>0)
			{
				if(c->nk->cfg->log_snd)
				{
					wtk_log_log(c->nk->log,"snd=[\n%.*s\n]",writed,item->data+item->pos);
				}
				item->pos+=writed;
				item->len-=writed;
				if(item->len<=0)
				{
					if(item->next)
					{
						buf->front=item->next;
						wtk_buf_item_delete(item);
						item=buf->front;
					}else
					{
						item->pos=0;
						item->len=0;
						break;
					}
				}
			}
			if(s!=WTK_OK){break;}
		}
		evt->writepending=item->len>0?1:0;
		ret=(s==WTK_OK||s==WTK_AGAIN)?0:-1;
	}else
	{
		evt->writepending=0;
		ret=0;
	}
end:
	return ret;
}


int wtk_connection_write(wtk_connection_t *c, char* buf, int len)
{
	wtk_poll_evt_t *evt=&(c->evt);
	wtk_fd_state_t s;
	int writed, left;
	int ret;

//	if(c->nk->cfg->log_snd)
//	{
//		wtk_log_log(c->nk->log,"snd=[\n%.*s\n]",len,buf);
//	}
	if(c->wbuf && c->wbuf->front->len>0)
	{
		wtk_buf_push(c->wbuf,buf,len);
		ret=wtk_connection_flush(c);
		if(ret!=0){goto end;}
	}else
	{
		writed = 0;
		//wtk_debug("len=%d\n",len);
		s=wtk_fd_send(c->fd, buf, len, &writed);
		left=len-writed;
		if(c->nk->cfg->log_snd && writed>0)
		{
			wtk_log_log(c->nk->log,"snd=[\n%.*s\n]",writed,buf);
		}
		if(left>0)
		{
			if(!c->wbuf)
			{
				c->wbuf=wtk_buf_new(c->nk->cfg->con_buf_size);
			}
			wtk_buf_push(c->wbuf,buf+writed,left);
		}
		ret=(s == WTK_OK || s == WTK_AGAIN) ? 0 : -1;
	}
	evt->writepending=c->wbuf&&c->wbuf->front->len>0;
	wtk_connection_touch_epoll(c);
end:
	return ret;
}


void wtk_connection_touch_epoll(wtk_connection_t *c)
{
	wtk_poll_evt_t *evt=&(c->evt);
	int ret=0;

	//wtk_debug("touch %d,%d,%d...\n",event->writepending,event->want_write,event->writeepolled);
	if(evt->writepending)
	{
		if(evt->writepolled==0)
		{
			//wtk_poll_add
			wtk_poll_connection_add_write(c->nk->poll,c);
		}
	}else
	{
		if(evt->writepolled==1)
		{
			wtk_poll_connection_remove_write(c->nk->poll,c);
		}
	}
}


int wtk_connection_process(wtk_connection_t *c)
{
	wtk_poll_evt_t *evt=&(c->evt);
	int ret;

	//wtk_debug("c=%p\n",c);
	//wtk_poll_evt_print(evt);
	if(evt->read)
	{
		ret=wtk_connection_read(c);
		if(ret!=0){goto end;}
	}
	if(evt->write)
	{
		ret=wtk_connection_flush(c);
		if(ret!=0){goto end;}
	}
	if(evt->reof)
	{
		evt->want_close=1;
	}
end:
	if(ret!=0 || evt->eof || evt->error)
	{
		evt->want_close=1;
		if(c->nk->parser_factory.want_close)
		{
			c->nk->parser_factory.want_close(c->parser);
		}
		if(evt->polled)
		{
			wtk_poll_remove_connection(c->nk->poll,c);
		}
		if(evt->ref==0)
		{
			wtk_connection_clean(c);
			wtk_nk_push_connection(c->nk, c);
		}
	}else
	{
		//update epoll write events.
		wtk_connection_touch_epoll(c);
		ret=0;
	}
	return ret;
}

void wtk_connection_update_addr(wtk_connection_t *c)
{
	socklen_t len;

    len=sizeof(c->addr);
    getsockname(c->fd,(struct sockaddr*)&(c->addr),&len);
}

void wtk_sockaddr_print(struct sockaddr_in *addr)
{
	char buf[256];
	const char *p;
	int port;

	p=inet_ntop(AF_INET,&(addr->sin_addr),buf,sizeof(struct sockaddr_in));
	port=ntohs(addr->sin_port);
	printf("%s:%d\n",p,port);
}

void wtk_connection_print(wtk_connection_t *c)
{
	socklen_t len;
	struct sockaddr_in addr;
	int ret;

    len=sizeof(addr);
	ret=getpeername(c->fd,(struct sockaddr*) &addr, &len);
	if(ret==0)
	{
		printf("peer:\n\t");
		wtk_sockaddr_print(&(addr));
	}
    len=sizeof(addr);
	ret=getsockname(c->fd,(struct sockaddr*) &addr, &len);
	if(ret==0)
	{
		printf("local:\n\t");
		wtk_sockaddr_print(&(addr));
	}
	wtk_poll_evt_print(&(c->evt));
}
