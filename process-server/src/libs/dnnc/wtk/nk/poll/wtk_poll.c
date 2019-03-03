#include "wtk_poll.h" 

wtk_poll_t* wtk_poll_new(wtk_poll_cfg_t *cfg)
{
	wtk_poll_t *poll;

	poll=(wtk_poll_t*)wtk_malloc(sizeof(wtk_poll_t));
	poll->cfg=cfg;
	if(cfg->use_epoll)
	{
		poll->v.epoll=wtk_epoll_new(&(cfg->epoll));
	}
	return poll;
}

void wtk_poll_delete(wtk_poll_t *poll)
{
	if(poll->cfg->use_epoll)
	{
		wtk_epoll_delete(poll->v.epoll);
	}
	wtk_free(poll);
}

int wtk_poll_start(wtk_poll_t *poll)
{
	int ret;

	if(poll->cfg->use_epoll)
	{
		ret=wtk_epoll_start(poll->v.epoll);
	}else
	{
		ret=-1;
	}
	return ret;
}

void wtk_poll_stop(wtk_poll_t *poll)
{
	if(poll->cfg->use_epoll)
	{
		wtk_epoll_stop(poll->v.epoll);
	}else
	{
	}
}


int wtk_poll_add_connection(wtk_poll_t *poll,wtk_connection_t *c)
{
	int ret;

	if(poll->cfg->use_epoll)
	{
		ret=wtk_epoll_add_evt(poll->v.epoll,c->fd,&(c->evt));
	}else
	{
		ret=-1;
	}
	return ret;
}

void wtk_poll_remove_connection(wtk_poll_t *poll,wtk_connection_t *c)
{
	if(poll->cfg->use_epoll)
	{
		wtk_epoll_remove_evt(poll->v.epoll,c->fd,&(c->evt));
	}
}

void wtk_poll_connection_add_write(wtk_poll_t *poll,wtk_connection_t *c)
{
	if(poll->cfg->use_epoll)
	{
		c->evt.want_write=1;
		wtk_epoll_add_evt(poll->v.epoll,c->fd,&(c->evt));
	}
}

void wtk_poll_connection_remove_write(wtk_poll_t *poll,wtk_connection_t *c)
{
	if(poll->cfg->use_epoll)
	{
		c->evt.want_write=0;
		wtk_epoll_add_evt(poll->v.epoll,c->fd,&(c->evt));
	}
}

void wtk_poll_add_process_evt(wtk_poll_t *poll,int fd,wtk_poll_process_evt_t *evt)
{
	if(poll->cfg->use_epoll)
	{
		evt->evt.use_ext_handler=1;
		wtk_epoll_add_evt(poll->v.epoll,fd,&(evt->evt));
	}
}

void wtk_poll_process_connection(wtk_poll_evt_t *evt)
{
	wtk_connection_t *c;
	wtk_poll_process_evt_t *xe;

	//wtk_debug("================= evt=%p ext=%d =================\n",evt,evt->use_ext_handler);
	if(evt->use_ext_handler)
	{
		xe=data_offset2(evt,wtk_poll_process_evt_t,evt);
		xe->process(xe->ths,evt);
	}else
	{
		c=data_offset(evt,wtk_connection_t,evt);
		//wtk_connection_print(c);
		wtk_connection_process(c);
		//exit(0);
	}
}

int wtk_poll_process(wtk_poll_t *poll)
{
	int ret;

	if(poll->cfg->use_epoll)
	{
		ret=wtk_epoll_process(poll->v.epoll,poll->cfg->timeout,wtk_poll_process_connection);
	}else
	{
		ret=-1;
	}
	return ret;
}
