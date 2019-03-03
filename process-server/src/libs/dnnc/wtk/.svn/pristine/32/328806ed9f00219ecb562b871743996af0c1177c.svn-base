#include "wtk_nk.h" 

wtk_connection_t* wtk_nk_new_connection(wtk_nk_t *nk)
{
	return wtk_connection_new(nk);
}

wtk_connection_t* wtk_nk_pop_connection(wtk_nk_t *nk)
{
	return (wtk_connection_t*)wtk_hoard_pop(&(nk->con_hoard));
}

void wtk_nk_push_connection(wtk_nk_t *nk,wtk_connection_t *c)
{
	//wtk_debug("%s:%d c=%p\n",c->peer_name,c->peer_port,c);
	wtk_hoard_push(&(nk->con_hoard),c);
//	if(nk->con_hoard.use_length<0)
//	{
//		exit(0);
//	}
}

void wtk_nk_parser_factory_init(wtk_nk_parser_factory_t *f)
{
	memset(f,0,sizeof(wtk_nk_parser_factory_t));
}


wtk_nk_t* wtk_nk_new(wtk_nk_cfg_t *cfg)
{
	wtk_nk_t *nk;

	nk=(wtk_nk_t*)wtk_malloc(sizeof(wtk_nk_t));
	nk->cfg=cfg;
	nk->log=wtk_log_cfg_new_log(&(cfg->log));
	nk->poll=wtk_poll_new(&(cfg->poll));
	nk->listen=wtk_listen_new(&(cfg->listen));
	nk->rw_buf=(char*)wtk_memalign(32,cfg->rw_size);
	wtk_hoard_init(&(nk->con_hoard),offsetof(wtk_connection_t,hoard_n),cfg->cache_size,
			(wtk_new_handler_t)wtk_nk_new_connection,(wtk_delete_handler_t)wtk_connection_delete,nk);
	wtk_nk_parser_factory_init(&(nk->parser_factory));
	wtk_pipequeue_init(&(nk->pipeq));
	nk->run=0;
	nk->process_pipe_ths=NULL;
	nk->process_pipe=NULL;
	return nk;
}

void wtk_nk_delete(wtk_nk_t *nk)
{
	wtk_pipequeue_clean(&(nk->pipeq));
	wtk_free(nk->rw_buf);
	wtk_hoard_clean(&(nk->con_hoard));
	wtk_listen_delete(nk->listen);
	wtk_poll_delete(nk->poll);
	wtk_log_delete(nk->log);
	wtk_free(nk);
}

wtk_connection_t* wtk_nk_add_connection(wtk_nk_t *nk,int fd,int type)
{
	wtk_connection_t *c;

	c=wtk_nk_pop_connection(nk);
	wtk_connection_init(c,fd,type);
	wtk_poll_add_connection(nk->poll,c);
	return c;
}

void wtk_nk_set_process_pipe(wtk_nk_t *nk,void *ths,wtk_nk_process_pipe_f p)
{
	nk->process_pipe_ths=ths;
	nk->process_pipe=p;
}

void wtk_nk_process_pipe(wtk_nk_t *nk,wtk_poll_evt_t *evt)
{
	//wtk_debug("want read pipe\n");
	if(nk->process_pipe)
	{
		nk->process_pipe(nk->process_pipe_ths);
	}
}

int wtk_nk_start(wtk_nk_t *nk)
{
	wtk_connection_t *c;
	int ret;

	ret=wtk_poll_start(nk->poll);
	if(ret!=0)
	{
		wtk_log_log0(nk->log,"create epoll failed.");
		goto end;
	}
	ret=wtk_listen_start(nk->listen);
	wtk_log_log(nk->log,"listen at %d %s.",nk->listen->cfg->port,ret==0?"success":"fail");
	if(ret!=0){goto end;}
	c=wtk_nk_add_connection(nk,nk->listen->fd,WTK_CONNECT_EVT_READ|WTK_CONNECT_EVT_ACCEPT);
	wtk_connection_update_addr(c);

	wtk_poll_evt_init(&(nk->pipe_evt.evt),WTK_CONNECT_EVT_READ);
	nk->pipe_evt.ths=nk;
	nk->pipe_evt.process=(wtk_poll_process_f)wtk_nk_process_pipe;
	wtk_poll_add_process_evt(nk->poll,nk->pipeq.pipe_fd[0],&(nk->pipe_evt));
	nk->run=1;
	ret=0;
end:
	return ret;
}

int wtk_nk_run(wtk_nk_t *nk)
{
	int ret;

	while(nk->run)
	{
		ret=wtk_poll_process(nk->poll);
		//wtk_debug("connection: use=%d free=%d\n",nk->con_hoard.use_length,nk->con_hoard.cur_free);
        if(ret!=0)
        {
        	wtk_log_log(nk->log,"errno=%d",errno);
            break;
        }
	}
	//wtk_debug("run end\n");
	return 0;
}

int wtk_nk_stop(wtk_nk_t *nk)
{
	nk->run=0;
	return 0;
}
