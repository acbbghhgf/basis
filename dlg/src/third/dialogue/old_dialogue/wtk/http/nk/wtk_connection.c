#include "wtk_connection.h"
#include "wtk_nk.h"
#include "wtk/http/nk/listen/wtk_listen.h"
#include <errno.h>
#ifdef WIN32
#include <Winsock2.h>
#include <Ws2tcpip.h>
#include "module/wtk_select.h"
#define wtk_close closesocket
#else
#define wtk_close close
#endif
//#define DEBUG_CON
int wtk_connection_touch_epoll(wtk_connection_t *c);

wtk_connection_t* wtk_connection_new(wtk_nk_t *n)
{
	wtk_connection_t *c;

	c = (wtk_connection_t*) wtk_calloc(1,sizeof(wtk_connection_t));
	c->net = n;
	c->event = (wtk_event_t*) wtk_calloc(1,sizeof(wtk_event_t));
	c->addr_text.data=c->name;
	//wtk_debug("new con %p.\n",c);
	return c;
}

int wtk_connection_bytes(wtk_connection_t *c)
{
	return sizeof(*c)+sizeof(wtk_event_t);
}

int wtk_connection_delete(wtk_connection_t *c)
{
	//wtk_debug("del con %p.\n",c);
	//wtk_debug("delete %p\n",c);
	wtk_connection_clean(c);
	wtk_free(c->event);
	wtk_free(c);
	return 0;
}

int wtk_connection_init(wtk_connection_t *c, int fd, wtk_listen_t *l, int t)
{
	wtk_event_t *e;

	c->app_data=0;
	c->fd = fd;
	c->listen = l;
	c->evt_listener=0;
	e = c->event;
	memset(e, 0, sizeof(wtk_event_t));
	e->data = c;
#ifdef WIN32
	e->fd=fd;
#endif
	e->handler = (wtk_event_handler) wtk_connection_process;
	e->want_read = t & CONNECTION_EVENT_READ ? 1 : 0;
	e->want_write = t & CONNECTION_EVENT_WRITE ? 1 : 0;
	e->want_accept = t & CONNECTION_EVENT_ACCEPT ? 1 : 0;
	if (!e->want_accept && l)
	{
		c->parser = (wtk_parser_t*) c->listen->pop_parser(c->listen->user_data,c);
	} else
	{
		c->parser = 0;
	}
	c->wstack = 0;
	c->want_close = 0;
	c->busy = 0;
	c->recv_delay = 0;
	c->active_count = 0;
	c->keep_alive=0;
	//c->ready=0;
	c->valid=1;
	return 0;
}

int wtk_connection_reset(wtk_connection_t *c)
{
	wtk_event_t *e;

	e=c->event;
	wtk_event_reset_sig(e);
	if(c->wstack)
	{
		wtk_stack_reset(c->wstack);
	}
	c->want_close = 0;
	c->busy = 0;
	c->recv_delay = 0;
	c->active_count = 0;
	c->keep_alive=0;
	c->ready=0;
	c->evt_listener=0;
	return 0;
}


int wtk_connection_clean(wtk_connection_t *c)
{
	int ret;

	wtk_log_log(c->net->log,"close %s.",c->name);
	if(c->evt_listener)
	{
		c->evt_listener->close(c->evt_listener->ths);
	}
	if (c->fd > 0)
	{
		if (wtk_event_epolled(c->event))
		{
			ret = wtk_epoll_remove_connection((c->net->epoll), c);
			if (ret != 0)
			{
				//never should be here.
				wtk_log_log(c->net->log,"epoll remove %d failed.",c->fd);
			}
		}
		if(c->net->cfg->log_connection)
		{
			wtk_log_log(c->net->log,"close %s.",c->name);
		}
        if((c->event->want_accept==0) || (c->net->cfg->passive==0))
        {
#ifdef WIN32
        	closesocket(c->fd);
#else
        	//shutdown(c->fd,SHUT_RDWR);
        	close(c->fd);
#endif
        }
		c->fd = 0;
	}
	if (c->wstack)
	{
		wtk_nk_push_stack(c->net, c->wstack);
		c->wstack = 0;
	}
	if (c->parser)
	{
		if(c->parser->close)
		{
			c->parser->close(c->parser);
		}
		if(c->listen)
		{
			c->listen->push_parser(c->listen->user_data, c->parser);
		}
		c->parser = 0;
		c->listen = 0;
	}
	return 0;
}

int wtk_connection_set_client_fd_opt(wtk_connection_t *c, int fd)
{
	int keepalive = 1;
	int reuse = 1;
	int ret;

	ret = wtk_fd_set_nonblock(fd);
	if (ret != 0) {
		goto end;
	}
	ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*) &reuse,
			sizeof(reuse));
	if (ret != 0) {
		goto end;
	}
	ret = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char*) (&keepalive),
			(socklen_t) sizeof(keepalive));
#ifndef WIN32
	if (ret != 0) {
		goto end;
	}

#ifndef USE_TCP_KEEPALIVE_CFG
#define USE_TCP_KEEPALIVE_CFG 1
#endif

#if USE_TCP_KEEPALIVE_CFG
    {
        int keepalive_time = 3, keepalive_intvl = 3, keepalive_probes = 2;
        ret = setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, (void*) (&keepalive_time),
                (socklen_t) sizeof(keepalive_time));
        if (ret != 0) {
            goto end;
        }
        ret = setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL,
                (void*) (&keepalive_intvl), (socklen_t) sizeof(keepalive_intvl));
        if (ret != 0) {
            goto end;
        }
        ret = setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, (void*) (&keepalive_probes),
                (socklen_t) sizeof(keepalive_probes));
    }
#endif

#endif

#ifdef USE_TCP_NODELAY
    {
        int tcp_nodelay = 1;
        ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&tcp_nodelay, sizeof(int));
    }
#endif

	end: return ret;
}

int wtk_connection_accept(wtk_connection_t *c)
{
	wtk_connection_t *n;
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	int ret, fd,bytes;
	char *p;

	ret = -1;
	fd = accept(c->fd, (struct sockaddr*) &addr, &len);
	if (fd < 0)
	{
		goto end;
	}
	if (c->net->con_hoard->use_length > c->net->cfg->max_connections)
	{
		//if there is too many connections, close the connection.
		wtk_log_log(c->net->log,"max connection[use=%d,max=%d],close it.",c->net->con_hoard->use_length,c->net->cfg->max_connections);
		wtk_close(fd);
		return 0;
	}
	ret = wtk_connection_set_client_fd_opt(c, fd);
	if (ret != 0)
	{
		goto end;
	}
	n = wtk_nk_pop_connection(c->net);
	if (!n)
	{
		ret = -1;
		goto end;
	}
	wtk_connection_init(n, fd, c->listen, CONNECTION_EVENT_READ);
	n->ready=1;
    ret=wtk_epoll_add_connection(n->net->epoll,n);
    if(ret!=0){goto end;}
	//getpeername can't be failed.
	ret=getpeername(c->fd,(struct sockaddr*) &addr, &len);
#ifdef WIN32
	p=inet_ntoa(addr.sin_addr);
#else
	p=(char*)inet_ntop(AF_INET,&(addr.sin_addr),n->name,sizeof(addr));
#endif
	c->remote_port=ntohs(addr.sin_port);
	bytes=strlen(p);
	wtk_string_set(&(n->remote_ip),p,bytes);
	bytes+=sprintf(p+bytes,":%d",c->remote_port);
	wtk_string_set(&(n->addr_text),n->name,bytes);
#ifdef SAVE_SOCK_PORT
	ret=getsockname(c->fd,(struct sockaddr*) &addr, &len);
	bytes=sprintf(n->sock_port, "%d", ntohs(addr.sin_port));
	wtk_string_set(&(n->sock_port_text),n->sock_port,bytes);
#endif
	//wtk_debug("accpet %s.\n",n->name);
	wtk_log_log(n->net->log,"accept %s.",n->name);
	n->event->read = n->event->write = 1;
	n->addr = addr;
	ret=wtk_connection_process(n, n->event);
	//ret = 0;
end:
	if (ret != 0)
	{
		//perror(__FUNCTION__);
		if (fd > 0){close(fd);}
		ret = 0;
	}
	return ret;
}

int wtk_connection_attach_fd(wtk_connection_t *c,int fd)
{
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	int ret,n;

	getpeername(fd,(struct sockaddr*) &addr, &len);
	n=sprintf(c->name, "%s:%d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	wtk_string_set(&(c->addr_text),c->name,n);
	ret=wtk_connection_set_client_fd_opt(c,fd);
	if(ret!=0){goto end;}
	c->fd=fd;
	ret=wtk_epoll_add_connection(c->net->epoll,c);
	//wtk_debug("epoll: %d\n",ret);
end:
	return ret;
}

int wtk_connection_connect(wtk_connection_t *c,struct sockaddr *ai_addr,socklen_t ai_addrlen)
{
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	int fd,ret,n;

	fd=socket(AF_INET,SOCK_STREAM,0);
	if(fd<0){ret=-1;goto end;}
#ifdef NON_BLOCK_CONNECT
	ret=wtk_connection_set_client_fd_opt(c,fd);
	if(ret!=0){goto end;}
	ret=connect(fd,(struct sockaddr*)addr,sizeof(*addr));
	if(errno != EINPROGRESS)
	{
		ret=-1;goto end;
	}
#else
	ret=connect(fd,ai_addr,ai_addrlen);
	if(ret!=0){goto end;}
	getpeername(fd,(struct sockaddr*) &addr, &len);
	n=sprintf(c->name, "%s:%d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	//wtk_debug("%s\n",c->name);
	wtk_string_set(&(c->addr_text),c->name,n);
#ifdef SAVE_SOCK_PORT
	ret=getsockname(c->fd,(struct sockaddr*) &addr, &len);
	n=sprintf(c->sock_port, "%d", ntohs(addr.sin_port));
	wtk_string_set(&(c->sock_port_text),c->sock_port,n);
#endif
	ret=wtk_connection_set_client_fd_opt(c,fd);
	if(ret!=0){goto end;}
#endif
	c->fd=fd;
	ret=wtk_epoll_add_connection(c->net->epoll,c);
	//wtk_debug("epoll: %d\n",ret);
end:
	if(ret!=0 && fd>0)
	{
		close(fd);
		fd=0;
	}
	return ret;
}

int wtk_connection_connect2(wtk_connection_t *c,struct sockaddr *ai_addr,socklen_t ai_addrlen,int timeout)
{
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	int fd,ret,n;

	ret=wtk_socket_connect3(ai_addr,ai_addrlen,&fd,timeout);
	if(ret!=0){goto end;}
	ret = wtk_fd_set_nonblock(fd);
	if (ret != 0) {goto end;}
	getpeername(fd,(struct sockaddr*) &addr, &len);
	n=sprintf(c->name, "%s:%d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	wtk_string_set(&(c->addr_text),c->name,n);
	c->fd=fd;
	ret=wtk_epoll_add_connection(c->net->epoll,c);
end:
	if(ret!=0 && fd>0)
	{
		close(fd);
		fd=0;
	}
	return ret;
}

int wtk_connection_write(wtk_connection_t *c, char* buf, int len)
{
	wtk_fd_state_t s;
	int writed, left;
	wtk_event_t *event;
	int ret;

	//wtk_debug("snd: %d\n",c->net->cfg->log_snd);
	//wtk_debug("%.*s\n",len,buf);
	if(c->net->cfg->log_snd)
	{
		wtk_log_log(c->net->log,"send=[\n%.*s\n]",len,buf);
	}
	event=c->event;
	if(event->want_write==0){event->want_write=1;}
	if (c->wstack && c->wstack->len > 0)
	{
		//if there data chached, so push data to cache.
		s = WTK_OK;
		wtk_stack_push(c->wstack, buf, len);
		ret=wtk_connection_flush(c);
		if(ret!=0){goto end;}
	} else
	{
		writed = 0;
		s = wtk_fd_send(c->fd, buf, len, &writed);
		left = len - writed;
		if (left > 0)
		{
			if (!c->wstack)
			{
				c->wstack = wtk_nk_pop_stack(c->net);
			}
			wtk_stack_push(c->wstack, buf + writed, left);
		}
		ret=(s == WTK_OK || s == WTK_AGAIN) ? 0 : -1;
	}
	event->writepending=(c->wstack && c->wstack->len>0);
	wtk_connection_touch_epoll(c);
end:
	return ret;
}

int wtk_connection_write_stack(wtk_connection_t *c, wtk_stack_t *s)
{
	stack_block_t *b;
	int ret,len;

	ret = 0;
	for (b = s->pop; b; b = b->next)
	{
        len=b->push - b->pop;
        if(len<=0){continue;}
		ret = wtk_connection_write(c, (char*) b->pop, len);
		if (ret != 0)
		{
			goto end;
		}
	}
end:
	return ret;
}

wtk_fd_state_t wtk_connection_read_available(wtk_connection_t* c,int *p_readed)
{
	wtk_fd_state_t s;
	int ret, readed;
	char *buf;
	int len;
	//int lac,max_active_count;
    int tot_readed;

	len = c->net->cfg->rw_size;
	buf = c->net->rw_buf;
	ret = 0;
	s = WTK_OK;
	tot_readed=0;
	//lac=c->active_count;max_active_count = lac+c->net->cfg->max_read_count;
	while(c->event->read)
	{
		s = wtk_fd_recv(c->fd, buf, len, &readed);
		if (s == WTK_EOF||(readed==0 && tot_readed==0 && s==WTK_OK))
		{
			//if read available, but read 0 bytes, for the remote connection is closed.
			if(c->net->cfg->log_connection)
			{
				wtk_log_log(c->net->log,"read EOF in %s(errno=%d) .",c->name,errno);
			}
            //if read available, but read 0 bytes, for the remote connection is closed. this is for windows.
            s=WTK_EOF;
			break;
		}
		tot_readed+=readed;
		if (readed > 0)
		{
			//print_data(buf,readed);
			if(c->net->cfg->log_rcv)
			{
				wtk_log_log(c->net->log,"recv=[\n%.*s\n]",readed,buf);
			}
			ret = c->parser->handler(c->parser, c, buf, readed);
			if (ret != 0)
			{
				wtk_log_log(c->net->log,"%s parse failed(%d),readed=%d,data=(%*.*s).",c->name,ret,readed,readed,readed,buf);
				s = WTK_ERR;
				break;
			}
			/*
			if(c->active_count>=max_active_count)
			{
				//wtk_debug("%s: break,lac=%d,ac=%d,mc=%d\n",c->name,lac,c->active_count,max_active_count);
				break;
			}
			*/
		}else
		{
			//wtk_log_log(c->net->log,"%s state=%d,errno=%d,readed=%d.",c->name,s,errno,readed);
			break;
		}
	}
	//wtk_log_log(c->net->log,"con_use=%d,readed=%d,errno=%d.",c->net->con_hoard->use_length,tot_readed,errno);
	if(p_readed)
	{
		*p_readed=tot_readed;
	}
	return s;
}


int wtk_connection_read(wtk_connection_t *c)
{
	wtk_event_t *e;
	wtk_fd_state_t s;
	int ret;

	e = (c->event);
	if (e->want_accept)
	{
		return wtk_connection_accept(c);
	}
#ifdef WIN32
	if (c->net->read_delay == 0 && c->recv_delay == 0)
#else
	if (e->reof||e->error||e->eof||c->net->epoll->et||(c->net->read_delay == 0 && c->recv_delay == 0))
#endif
	{
		int readed;

		s = wtk_connection_read_available(c,&readed);
		ret = (s == WTK_OK || s == WTK_AGAIN) ? 0 : -1;
		/*
		if(ret==-1)
		{
			wtk_log_log(c->net->log,"readed=%d,state=%d",readed,s);
		}
		*/
		//wtk_debug("s=%d,ret=%d\n",s,ret);
	} else
	{
		ret = 0;
	}
	if ((ret == 0) && (wtk_event_epolled(c->event) == 0))
	{
		wtk_epoll_add_connection(c->net->epoll, c);
	}
	return ret;
}

int wtk_connection_flush(wtk_connection_t *c)
{
	wtk_stack_t *s;
	wtk_event_t *e;
	wtk_fd_state_t state;
	int errno;
	socklen_t len;
	int ret;

	if(!c->ready)
	{
		//this is for nonblocking connect socket.
		len=sizeof(errno);
		ret=getsockopt(c->fd,SOL_SOCKET,SO_ERROR,&errno,&len);
		if(ret!=0){return ret;}
		if(errno!=0){return -1;}
	}
	e = c->event;
	s = c->wstack;
	if(s)
	{
		state=wtk_fd_flush_send_stack(c->fd, s);
		if (s->len == 0)
		{
			c->event->writepending = 0;
		}
		//if(state==WTK_AGAIN){wtk_log_log(c->net->log,"%s write again left=%d.",c->name,s->len);}
	}else
	{
		state=e->reof?WTK_EOF:WTK_OK;
	}
	ret=(state==WTK_OK||state==WTK_AGAIN)?0:-1;
	return ret;
}

int wtk_connection_process(wtk_connection_t *c, wtk_event_t* event)
{
	int ret,x;

    if (!c || !event) {
        return -1;
    }

	//wtk_event_print(event);
#ifdef WIN32
#else
	if(c->net->cfg->log_event)
	{
		wtk_log_log(c->net->log,"%.*s: events=%#x.",c->addr_text.len,c->addr_text.data,event->events);
	}
#endif
	ret = 0;
	if (event->read)
	{
		ret = wtk_connection_read(c);
	}
	if (event->write)
	{
		ret = wtk_connection_flush(c);
		if (ret != 0)
		{
			wtk_log_log(c->net->log,"%s","flush failed.");
			goto end;
		}
	}
	if(event->reof)
	{
		c->want_close=1;
	}
	if (c->want_close && c->active_count <= 0 && (!c->wstack || c->wstack->len== 0))
	{
		//wtk_log_log(c->net->log,"%s","want to close.");
		//close the connection.
		ret = -1;
	}
end:
	if(c->parser && c->parser->shot)
	{
		c->parser->shot(c->parser);
	}
	if (ret != 0 || event->eof || event->error)
	{
		if(c->parser && c->parser->close_notify)
		{
			x=c->parser->close_notify(c->parser);
		}else
		{
			x=0;
		}
		wtk_log_log(c->net->log,"want close %s.",c->name);
#ifdef DEBUG_SOCKET
		wtk_debug("con=%s,ret=%d,eof=%d,err=%d,active=%d,x=%d\n",c->name,ret,event->eof,event->error,c->active_count,x);
#endif
#ifdef WIN32
       wtk_log_log(c->net->log,"ret=%d,eof=%d,err=%d,active=%d,notify=%d,errno=%d.",ret,event->eof,event->error,c->active_count,x,GetLastError());
#else
       if(c->net->cfg->log_connection)
       {
    	   wtk_log_log(c->net->log,"ret=%d,eof=%d,err=%d,active=%d,notify=%d,errno=%d,event=%#x.",ret,event->eof,event->error,c->active_count,x,errno,event->events);
       }
#endif
		c->valid=0;
		c->want_close=1;
		if (wtk_event_epolled(c->event))
		{
			//wtk_debug("remove connection %s,event=%p,x=%d\n",c->name,c->event,x);
			wtk_epoll_remove_connection((c->net->epoll), c);
		}
		//if there is no request is the high layer, close connection
		//for the high layer maybe using the resource of connection.
		//keep connection available.
		if (c->active_count <= 0 && x==0)
		{
			//if the connection have valid speech request, never come to here.
			wtk_connection_clean(c);
			wtk_nk_push_connection(c->net, c);
		}
		ret = 0;
	}else
	{
		//update epoll write events.
		ret=wtk_connection_touch_epoll(c);
	}
	return ret;
}

int wtk_connection_touch_epoll(wtk_connection_t *c)
{
	wtk_event_t *event=c->event;
	int ret=0;

	//wtk_debug("touch %d,%d,%d...\n",event->writepending,event->want_write,event->writeepolled);
	if(event->writepending==1)
	{
		if(event->writeepolled==0)
		{
			ret = wtk_epoll_event_add_write(c->net->epoll, c->fd, event);
		}
	}else
	{
		if(event->writeepolled==1)
		{
			ret = wtk_epoll_event_remove_write(c->net->epoll, c->fd, event);
		}
	}
	return ret;
}

void wtk_connection_print(wtk_connection_t *c)
{
	printf("########## %s ###########\n", __FUNCTION__);
	printf("want close:\t%d\n", c->want_close);
	printf("active:\t %d\n", c->active_count);
	wtk_stack_print(c->wstack);
	printf("read event:\t%d\n", c->event->read);
	printf("write event:\t%d\n", c->event->write);
}

void wtk_connection_shutdown(wtk_connection_t *c)
{
	if(c->fd>0)
	{
#ifdef WIN32
			shutdown(c->fd,SD_BOTH);
#else
			shutdown(c->fd, SHUT_RDWR);
#endif
	}
}

int wtk_connection_try_close(wtk_connection_t *c)
{
	if (c->want_close && c->active_count <= 0 && ((c->valid==0) || (!c->wstack || c->wstack->len == 0)))
	{
		if(c->valid==0)
		{
			//if connection is closed, just clean resource.
			wtk_connection_clean(c);
			wtk_nk_push_connection(c->net,c);
		}else
		{
			//if connection is on, tell epoll to close connection.
			wtk_connection_shutdown(c);
		}
		return 1;
	} else
	{
		return 0;
	}
}

void wtk_connection_set_delay(wtk_connection_t* c,int delay)
{
	c->recv_delay=delay;
	if(delay)
    {
        if(c->event->readepolled)
        {
        	//wtk_debug("set delay %s.\n",c->name);
        	wtk_log_log(c->net->log,"set delay %s.",c->name);
            wtk_epoll_event_remove_read(c->net->epoll,c->fd,c->event);
        }
    }else
    {
        if(!c->event->readepolled)
        {
        	//wtk_debug("wake delay %s.\n",c->name);
        	wtk_log_log(c->net->log,"wake delay %s.",c->name);
            wtk_epoll_event_add_read(c->net->epoll,c->fd,c->event);
        }
    }
}

int wtk_connection_get_sock_peer_name(wtk_connection_t *c,char *buf)
{
	struct sockaddr_in peer_addr;
	socklen_t peer_len = sizeof(peer_addr);
	struct sockaddr_in sock_addr;
	socklen_t sock_len = sizeof(sock_addr);
	int ret;

	ret=getpeername(c->fd,(struct sockaddr*) &peer_addr, &peer_len);
	if(ret!=0){goto end;}
	ret=getsockname(c->fd,(struct sockaddr*) &sock_addr, &sock_len);
	if(ret!=0){goto end;}
	sprintf(buf,"peer=%s:%d,sock=%s:%d", inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port),
			inet_ntoa(sock_addr.sin_addr), ntohs(sock_addr.sin_port));
end:
	return ret;
}

void wtk_connection_log_read_failed(wtk_connection_t *c)
{
	char buf[1024];
	int ret;

	ret=wtk_connection_get_sock_peer_name(c,buf);
	if(ret==0)
	{
		wtk_log_log(c->net->log,"read failed %s.",buf);
	}else
	{
		wtk_log_log(c->net->log,"ret=%d.",ret);
	}
}
