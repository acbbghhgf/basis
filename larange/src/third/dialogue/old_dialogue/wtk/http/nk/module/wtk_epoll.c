#include "wtk_epoll.h"

int wtk_epoll_init(wtk_epoll_t *epoll,int event_num)
{
	epoll->fd=INVALID_FD;
	epoll->size=event_num;
	epoll->montier_events=0;
	epoll->events=wtk_calloc(epoll->size,sizeof(struct epoll_event));
	epoll->et=0;
	epoll->bw_on_sig=0;
	return epoll->events ? 0 : -1;
}

int wtk_epoll_bytes(wtk_epoll_t *epoll)
{
	return epoll->size*sizeof(struct epoll_event);
}

int wtk_epoll_clean(wtk_epoll_t *epoll)
{
	if(epoll->fd != INVALID_FD)
	{
		close(epoll->fd);
		epoll->fd=INVALID_FD;
	}
	if(epoll->events)
	{
		wtk_free(epoll->events);
		epoll->events=0;
	}
	return 0;
}

int wtk_epoll_create(wtk_epoll_t* epoll)
{
	int ret;

	epoll->fd=epoll_create(epoll->size);
	if(epoll->fd<0)
	{
		perror(__FUNCTION__);
		ret=-1;
	}else
	{
		ret=0;
	}
	return ret;
}

int wtk_epoll_close(wtk_epoll_t *epoll)
{
	if(epoll->fd != INVALID_FD)
	{
		close(epoll->fd);
		epoll->fd=INVALID_FD;
	}
	return 0;
}

int wtk_epoll_add(wtk_epoll_t* epoll, int fd, uint32_t events,void *data)
{
	struct epoll_event ev={events,{data}};
	int ret;

	//ev.events = events;
	//ev.data.ptr = data;
	ret = epoll_ctl(epoll->fd, EPOLL_CTL_ADD, fd, &ev);
	if (ret != 0)
	{
		perror(__FUNCTION__);
	}else
	{
		++epoll->montier_events;
	}
	return ret;
}

int wtk_epoll_mod(wtk_epoll_t* epoll, int fd, uint32_t events, void *data)
{
	struct epoll_event ev={0,};
	int ret;

	ev.events = events;
	ev.data.ptr = data;
	ret = epoll_ctl(epoll->fd, EPOLL_CTL_MOD, fd, &ev);
	if (ret != 0)
	{
		perror(__FUNCTION__);
	}
	return ret;
}

int wtk_epoll_remove(wtk_epoll_t* epoll, int fd)
{
	int ret;

	ret = epoll_ctl(epoll->fd, EPOLL_CTL_DEL, fd, NULL);
	if (ret != 0)
	{
		//wtk_debug("%d,%d,%d\n",getpid(),epoll->fd,fd);
		perror(__FUNCTION__);
	}else
	{
		--epoll->montier_events;
	}
	return ret;
}

uint32_t wtk_epoll_get_event_flags(wtk_epoll_t* epoll,wtk_event_t *event)
{
	uint32_t events;

	events=EPOLLRDHUP|EPOLLHUP|EPOLLERR;
	if(event->want_read || event->want_accept)
	{
		events |= EPOLLIN;
	}
	if(event->want_write)
	{
		events |= EPOLLOUT;
	}
	if(epoll->et && !event->want_accept)
	{
		events |= EPOLLET;
	}
	return events;
}

int wtk_epoll_add_event(wtk_epoll_t *epoll,int fd,wtk_event_t* event)
{
	uint32_t events;
	int ret;

	//wtk_event_print(event);
	events=wtk_epoll_get_event_flags(epoll,event);
	//wtk_debug("event=%#x\n",events);
	if(event->in_queue)
	{
		ret=wtk_epoll_mod(epoll,fd,events,event);
	}else
	{
		ret=wtk_epoll_add(epoll,fd,events,event);
	}
	//wtk_debug("ret=%d,%d,%x,%x,%x\n",ret,event->writeepolled,events & EPOLLOUT,events,EPOLLOUT);
	if(ret==0)
	{
		event->readepolled=(events & EPOLLIN)?1:0;
		event->errepolled=(events & EPOLLERR)?1:0;
		event->writeepolled=(events & EPOLLOUT)?1:0;
		event->in_queue=1;
	}
	//wtk_debug("ret=%d,%d\n",ret,event->writeepolled);
	//wtk_event_print(event);
	return ret;
}

int wtk_epoll_add_connection(wtk_epoll_t *epoll,wtk_connection_t *c)
{
	//wtk_debug("add connection: %s\n",c->name);
	return wtk_epoll_add_event(epoll,c->fd,(c->event));
}

int wtk_epoll_remove_connection(wtk_epoll_t *epoll,wtk_connection_t *c)
{
	wtk_event_t *event;
	int ret;

	event=c->event;
	if(wtk_event_epolled(event))
	{
		//wtk_debug("remove event: %p\n",c->event);
		ret=wtk_epoll_remove(epoll,c->fd);
		event->readepolled=event->writeepolled=event->errepolled=event->in_queue=0;
	}else
	{
		ret=0;
	}
	return ret;
}


int wtk_epoll_event_remove_read(wtk_epoll_t *epoll,int fd,wtk_event_t *event)
{
	int ret;

	if(event->readepolled)
	{
		event->want_read=0;
		ret=wtk_epoll_add_event(epoll,fd,event);
	}else
	{
		ret=0;
	}
	return ret;
}

int wtk_epoll_event_add_read(wtk_epoll_t *epoll,int fd,wtk_event_t *event)
{
	int ret;

	if(event->readepolled==0)
	{
		event->want_read=1;
		ret=wtk_epoll_add_event(epoll,fd,event);
	}else
	{
		ret=0;
	}
	return ret;
}

int wtk_epoll_event_add_write(wtk_epoll_t *epoll,int fd,wtk_event_t *event)
{
	event->want_write=1;
	return wtk_epoll_add_event(epoll,fd,event);
}

int wtk_epoll_event_remove_write(wtk_epoll_t *epoll,int fd,wtk_event_t *event)
{
	event->want_write=0;
	return wtk_epoll_add_event(epoll,fd,event);
}

int wtk_epoll_wait(wtk_epoll_t* epoll,int timeout)
{
	int ret;

	while (1)
	{
		ret = epoll_wait(epoll->fd, epoll->events,
				epoll->size, timeout);
		//wtk_debug("%d,timeout=%d\n",ret,timeout);
		if (ret == -1 && errno == EINTR)
		{
			if(epoll->bw_on_sig)
			{
				return 0;
			}else
			{
				continue;
			}
		}
		else
		{
			break;
		}
	}
	return ret;
}

int wtk_epoll_handle_event(wtk_epoll_t *epoll,wtk_event_t *event)
{
	//wtk_debug("on event\n");
	event->handler(event->data,event);
	return 0;
}

int wtk_epoll_process(wtk_epoll_t *epoll,int has_accept,int timeout,sem_t *sem)
{
	struct epoll_event* event;
	wtk_event_t *e;
	uint32_t events;
	int ret,i;
	wtk_queue_t *a;
	wtk_queue_t *c;

	if(has_accept)
	{
		a=&(epoll->accept_queue);
		c=&(epoll->client_queue);
		wtk_queue_init(a);
		wtk_queue_init(c);
	}else
	{
		a=c=0;
	}
	ret=wtk_epoll_wait(epoll,timeout);
	//wtk_debug("%d epoll...\n",getpid());
	if(ret<=0)
	{
		if(has_accept && sem)
		{
			sem_post(sem);
		}
		goto end;
	}
	for(i=0;i<ret;++i)
	{
		event=&(epoll->events[i]);
		events=event->events;
		//wtk_debug("%x\n",events);
		e=(wtk_event_t*)(event->data.ptr);
		e->read=events & EPOLLIN ? 1 : 0;
		e->write=events & EPOLLOUT ? 1 : 0;
		e->error=events & EPOLLERR ? 1 : 0;
		if(!e->eof)
		{
			e->eof=events & EPOLLHUP ? 1 : 0;
		}
		if(!e->reof)
		{
			e->reof=events & EPOLLRDHUP ? 1 : 0;
		}
		if(e->reof)
		{
			e->read=1;
		}
		if(has_accept)
		{
			wtk_queue_push(e->want_accept ? a: c,&(e->epoll_n));
		}else
		{
			e->handler(e->data,e);
		}
	}
	if(has_accept)
	{
		wtk_queue_node_t *n,*p;

		if(sem)
		{
			sem_post(sem);
		}
		for(n=a->pop;n;n=p)
		{
			p=n->next;
			e=data_offset(n,wtk_event_t,epoll_n);
			e->handler(e->data,e);
		}
		for(n=c->pop;n;n=p)
		{
			p=n->next;
			e=data_offset(n,wtk_event_t,epoll_n);
			e->handler(e->data,e);
		}
	}
	ret=0;
end:
	return ret;
}

int wtk_epoll_process2(wtk_epoll_t *epoll,int timeout,int *pret)
{
	struct epoll_event* event;
	wtk_event_t *e;
	uint32_t events;
	int ret,i;

	ret=wtk_epoll_wait(epoll,timeout);
	//wtk_debug("%d epoll %d...\n",getpid(),ret);
	if(pret)
	{
		*pret=ret;
	}
	if(ret<=0){goto end;}
	for(i=0;i<ret;++i)
	{
		event=&(epoll->events[i]);
		events=event->events;
		e=(wtk_event_t*)(event->data.ptr);
		e->events=events;
		e->read=events & EPOLLIN ? 1 : 0;
		e->write=events & EPOLLOUT ? 1 : 0;
		e->error=events & EPOLLERR ? 1 : 0;
		if(!e->eof)
		{
			e->eof=events & EPOLLHUP ? 1 : 0;
		}
		if(!e->reof)
		{
			e->reof=events & EPOLLRDHUP ? 1 : 0;
		}
		if(e->reof)
		{
			e->read=1;
		}
		//wtk_event_print(e);
		e->handler(e->data,e);
	}
	ret=0;
end:
	return ret;
}

/*
int wtk_epoll_process2(wtk_epoll_t *epoll,int timeout,int *pret)
{
	struct epoll_event* event;
	wtk_event_t *e;
	uint32_t events;
	int ret,i;

	ret=wtk_epoll_wait(epoll,timeout);
	//wtk_debug("%d epoll %d...\n",getpid(),ret);
	if(pret)
	{
		*pret=ret;
	}
	if(ret<=0){goto end;}
	for(i=0;i<ret;++i)
	{
		event=&(epoll->events[i]);
		events=event->events;
		e=(wtk_event_t*)(event->data.ptr);
		e->events=events;
		e->read=events & EPOLLIN ? 1 : 0;
		e->write=events & EPOLLOUT ? 1 : 0;
		e->error=events & EPOLLERR ? 1 : 0;
		if(!e->eof)
		{
			e->eof=events & EPOLLHUP ? 1 : 0;
		}
		if(!e->reof)
		{
			e->reof=events & EPOLLRDHUP ? 1 : 0;
		}
		if(e->reof)
		{
			e->read=1;
		}
		e->processed=0;
		//wtk_event_print(e);
		//e->handler(e->data,e);
	}
	for(i=0;i<ret;++i)
	{
		e=(wtk_event_t*)(event->data.ptr);
		if(e->processed==0)
		{
			e->handler(e->data,e);
			e->processed=1;
		}
	}
	ret=0;
end:
	return ret;
}
*/
