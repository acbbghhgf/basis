#include "wtk_epoll.h" 

wtk_epoll_t* wtk_epoll_new(wtk_epoll_cfg_t *cfg)
{
	wtk_epoll_t *poll;

	poll=(wtk_epoll_t*)wtk_malloc(sizeof(wtk_epoll_t));
	poll->cfg=cfg;
	poll->events=wtk_calloc(cfg->size,sizeof(struct epoll_event));
	poll->fd=-1;
	return poll;
}

void wtk_epoll_delete(wtk_epoll_t *poll)
{
	if(poll->fd>=0)
	{
		wtk_epoll_stop(poll);
	}
	wtk_free(poll->events);
	wtk_free(poll);
}

int wtk_epoll_start(wtk_epoll_t *poll)
{
	int ret;

	poll->fd=epoll_create(poll->cfg->size);
	if(poll->fd<0)
	{
		perror(__FUNCTION__);
		ret=-1;
		goto end;
	}
	ret=0;
end:
	return ret;
}

void wtk_epoll_stop(wtk_epoll_t *poll)
{
	if(poll->fd != -1)
	{
		close(poll->fd);
		poll->fd=-1;
	}
}

uint32_t wtk_epoll_get_event_flags(wtk_epoll_t* epoll,wtk_poll_evt_t *event)
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
	if(epoll->cfg->use_et && !event->want_accept)
	{
		events |= EPOLLET;
	}
	return events;
}

int wtk_epoll_add(wtk_epoll_t* epoll, int fd, uint32_t events,void *data,int polled)
{
	struct epoll_event ev={events,{data}};
	int ret;

	ret = epoll_ctl(epoll->fd, polled?EPOLL_CTL_MOD:EPOLL_CTL_ADD, fd, &ev);
	if (ret != 0)
	{
		perror(__FUNCTION__);
	}
	return ret;
}

int wtk_epoll_remove_evt(wtk_epoll_t *epoll,int fd,wtk_poll_evt_t *evt)
{
	int ret;

	ret = epoll_ctl(epoll->fd, EPOLL_CTL_DEL, fd, NULL);
	if (ret != 0)
	{
		//wtk_debug("%d,%d,%d\n",getpid(),epoll->fd,fd);
		perror(__FUNCTION__);
	}else
	{
		evt->polled=0;
	}
	return ret;
}


int wtk_epoll_add_evt(wtk_epoll_t *epoll,int fd,wtk_poll_evt_t *evt)
{
	uint32_t events;
	int ret;

	events=wtk_epoll_get_event_flags(epoll,evt);
	//wtk_debug("add fd=%d rw=%d/%d\n",fd,evt->want_read,evt->want_write);
	ret=wtk_epoll_add(epoll,fd,events,evt,evt->polled);
	if(ret==0)
	{
		evt->polled=1;
		evt->readpolled=(events & EPOLLIN)?1:0;
		evt->writepolled=(events & EPOLLOUT)?1:0;
	}
	return ret;
}

int wtk_epoll_wait(wtk_epoll_t* epoll,int timeout)
{
	int ret;

	while (1)
	{
		ret = epoll_wait(epoll->fd, epoll->events,
				epoll->cfg->size, timeout);
		//wtk_debug("%d,timeout=%d\n",ret,timeout);
		if (ret == -1 && errno == EINTR)
		{
			continue;
		}
		else
		{
			break;
		}
	}
	return ret;
}


int wtk_epoll_process(wtk_epoll_t *epoll,int timeout,wtk_epoll_process_evt_f process_evt)
{
	struct epoll_event* event;
	wtk_poll_evt_t *evt;
	uint32_t events;
	int i,ret;

	ret=wtk_epoll_wait(epoll,timeout);
	if(ret<=0){goto end;}
	for(i=0;i<ret;++i)
	{
		event=&(epoll->events[i]);
		events=event->events;
		evt=(wtk_poll_evt_t*)(event->data.ptr);
		evt->read=events & EPOLLIN ? 1 : 0;
		evt->write=events & EPOLLOUT ? 1 : 0;
		evt->error=events & EPOLLERR ? 1 : 0;
		if(!evt->eof)
		{
			evt->eof=events & EPOLLHUP ? 1 : 0;
		}
		if(!evt->reof)
		{
			evt->reof=events & EPOLLRDHUP ? 1 : 0;
		}
		if(evt->reof)
		{
			evt->read=1;
		}
		//wtk_event_print(e);
		process_evt(evt);
	}
	ret=0;
end:
	return ret;
}

