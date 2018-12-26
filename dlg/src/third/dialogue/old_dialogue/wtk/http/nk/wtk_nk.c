#include "wtk_nk.h"

void wtk_nk_pipe_init(wtk_nk_pipe_t *p)
{
	wtk_event_t *e;

	wtk_pipequeue_init(&(p->pipe_queue));
	e=&(p->pipe_event);
	memset(e,0,sizeof(*e));
#ifdef WIN32
    e->fd=p->pipe_queue.pipe_fd[0];
#endif
	e->want_read=1;
	e->data=0;
	e->handler=(wtk_event_handler)0;
}

void wtk_nk_send_pipe_event(wtk_nk_t *nk,wtk_queue_node_t *n)
{
	wtk_pipequeue_push(&(nk->pipe->pipe_queue),n);
}

void wtk_nk_set_pipe_handler(wtk_nk_t *nk,void *ths,wtk_event_handler handler)
{
	wtk_event_t *e;

	e=&(nk->pipe->pipe_event);
	e->data=ths;
	e->handler=handler;
}

wtk_nk_t* wtk_nk_new(wtk_nk_cfg_t *cfg,wtk_log_t *log)
{
	wtk_nk_t *nk;

	nk=(wtk_nk_t*)wtk_malloc(sizeof(*nk));
	wtk_nk_init(nk,cfg,log);
	return nk;
}

int wtk_nk_delete(wtk_nk_t *nk)
{
	wtk_nk_clean(nk);
	wtk_free(nk);
	return 0;
}

int wtk_nk_init(wtk_nk_t *n,wtk_nk_cfg_t *cfg,wtk_log_t *log)
{
	int ret;
	int cache_size;

	n->cfg=cfg;
	cache_size=cfg->connection_cache;
#ifndef WIN32
	n->accept_sem=0;
	if(cfg->use_cpu)
	{
		n->cpu=wtk_cpu_new();
	}else
	{
		n->cpu=0;
	}
#endif
	n->nk_test=0;
	n->heap=wtk_heap_new(4096);
	n->log= log;
	n->epoll=(wtk_epoll_t*)wtk_heap_malloc(n->heap,sizeof(wtk_epoll_t));
    n->read_delay=0;
#ifdef	USE_TREE
    n->timer_tree=0;//wtk_rbtree_new();
#else
    wtk_queue_init(&(n->timer_queue));
#endif
	if(cfg->use_pipe)
	{
		n->pipe=(wtk_nk_pipe_t*)wtk_heap_malloc(n->heap,sizeof(wtk_nk_pipe_t));
		wtk_nk_pipe_init(n->pipe);
	}else
	{
		n->pipe=0;
	}
    n->tmp_stk=wtk_stack_new(4096,4096,1);
    n->tmp_buf=wtk_strbuf_new(256,1);
	n->rw_buf=(char*)wtk_memalign(32,cfg->rw_size);
	n->con_hoard=(wtk_hoard_t*)wtk_heap_malloc(n->heap,sizeof(wtk_hoard_t));
	wtk_hoard_init(n->con_hoard,offsetof(wtk_connection_t,net_node_q),cache_size,(wtk_new_handler_t)wtk_nk_new_connection,
			(wtk_delete_handler_t)wtk_connection_delete,n);
	n->stack_hoard=(wtk_hoard_t*)wtk_heap_malloc(n->heap,sizeof(wtk_hoard_t));
	wtk_hoard_init(n->stack_hoard,offsetof(wtk_stack_t,q),cache_size,(wtk_new_handler_t)wtk_nk_new_stack,(wtk_delete_handler_t)wtk_stack_delete,n);
	n->listening=wtk_array_new_h(n->heap,2,sizeof(wtk_listen_t));
	ret=wtk_epoll_init(n->epoll,200);
	return ret;
}

int wtk_nk_bytes(wtk_nk_t *nk)
{
	int b;
	int x;

	b=wtk_heap_bytes(nk->heap);
	b+=nk->cfg->rw_size;
#ifdef WIN32
#else
	b+=wtk_epoll_bytes(nk->epoll);
#endif
	b+=x=wtk_hoard_bytes(nk->con_hoard,(wtk_hoard_bytes_f)wtk_connection_bytes);
	//wtk_debug("con: %d\n",x);
	b+=x=wtk_hoard_bytes(nk->stack_hoard,(wtk_hoard_bytes_f)wtk_stack_bytes);
	b+=wtk_stack_bytes(nk->tmp_stk);
	/*
	wtk_debug("stack: %d\n",x);
	wtk_debug("con: use=%d,free=%d\n",nk->con_hoard->use_length,nk->con_hoard->cur_free);
	wtk_debug("stack: use=%d,free=%d\n",nk->stack_hoard->use_length,nk->stack_hoard->cur_free);
	wtk_debug("b=%d\n",b);
	*/
	return b;
}

#ifndef WIN32
void wtk_nk_add_accept_sem(wtk_nk_t *nk)
{
	char buf[32];
	if(!nk->accept_sem)
	{
		sprintf(buf,"%.0f",time_get_ms());
		nk->accept_sem=sem_open(buf,O_CREAT,0666,1);
	}
}
#endif

int wtk_nk_close_fd(wtk_nk_t *n)
{
	wtk_queue_node_t *q;
	wtk_connection_t *c;
	wtk_listen_t *l;
	int i;
	int ret=0;

	if(n->pipe)
	{
		ret=wtk_pipequeue_clean(&(n->pipe->pipe_queue));
	}
	if(n->cfg->passive==0)
	{
		l=(wtk_listen_t*)n->listening->slot;
		for(i=0;i<n->listening->nslot;++i)
		{
			if(l[i].fd>0)
			{
				wtk_socket_close_fd(l[i].fd);
				l[i].fd=-1;
			}
		}
	}
	for(q=n->con_hoard->use;q;q=q->prev)
	{
		c=data_offset(q,wtk_connection_t,net_node_q);
		if(c->fd>0)
		{
			wtk_socket_close_fd(c->fd);
			c->fd=-1;
		}
	}
	return ret;
}

void wtk_nk_clean_mem(wtk_nk_t *n)
{
    if(n->con_hoard)
    {
	   wtk_hoard_clean(n->con_hoard);
    }
    if(n->stack_hoard)
    {
	    wtk_hoard_clean((n->stack_hoard));
    }
    if(n->rw_buf)
    {
	    wtk_free(n->rw_buf);
    }
    if(n->heap)
    {
	    wtk_heap_delete(n->heap);
    }
#ifdef USE_TREE
    if(n->timer_tree)
    {
    	wtk_rbtree_delete(n->timer_tree);
    }
#endif
#ifndef WIN32
    if(n->cpu)
    {
    	wtk_cpu_delete(n->cpu);
    }
#endif
}

int wtk_nk_clean(wtk_nk_t *n)
{
    if(n->con_hoard)
    {
	    wtk_hoard_clean(n->con_hoard);
    }
    if(n->stack_hoard)
    {
	    wtk_hoard_clean((n->stack_hoard));
    }
    if(n->epoll)
    {
	    wtk_epoll_clean(n->epoll);
    }
    if(n->rw_buf)
    {
	    wtk_free(n->rw_buf);
    }
    if(n->heap)
    {
	    wtk_heap_delete(n->heap);
    }
#ifdef	USE_TREE
    if(n->timer_tree)
    {
    	wtk_rbtree_delete(n->timer_tree);
    }
#endif
    wtk_stack_delete(n->tmp_stk);
    wtk_strbuf_delete(n->tmp_buf);
#ifndef WIN32
    if(n->accept_sem)
    {
    	sem_close(n->accept_sem);
    }
    if(n->cpu)
    {
    	wtk_cpu_delete(n->cpu);
    }
#endif
	return 0;
}

wtk_stack_t* wtk_nk_new_stack(wtk_nk_t *net)
{
	wtk_stack_t *s;

	s=wtk_stack_new(1024,409600,1);
	return s;
}

wtk_connection_t* wtk_nk_new_connection(wtk_nk_t *net)
{
	return wtk_connection_new(net);
}

wtk_connection_t* wtk_nk_pop_connection(wtk_nk_t *net)
{
	//wtk_log_log(net->log,"****get use=%d.",net->con_hoard->use_length);
	return (wtk_connection_t *)wtk_hoard_pop(net->con_hoard);
}

int wtk_nk_push_connection(wtk_nk_t *net,wtk_connection_t *c)
{
	int ret;

	wtk_connection_reset(c);
	//wtk_log_log(net->log,"####save before con=%s,use=%d.",c->name,net->con_hoard->use_length);
	ret=wtk_hoard_push(net->con_hoard,c);
	//wtk_log_log(net->log,"####save after con=%s,use=%d.",c->name,net->con_hoard->use_length);
	return ret;
}

wtk_stack_t* wtk_nk_pop_stack(wtk_nk_t *net)
{
	return (wtk_stack_t*)wtk_hoard_pop((net->stack_hoard));
}

int wtk_nk_push_stack(wtk_nk_t *net,wtk_stack_t* s)
{
	wtk_stack_reset(s);
	return wtk_hoard_push((net->stack_hoard),s);
}

void wtk_nk_add_listen(wtk_nk_t *net,wtk_listen_t *l)
{
	wtk_listen_t *p;

	p=(wtk_listen_t*)wtk_array_push(net->listening);
	*p=*l;
}

int wtk_nk_get_port(wtk_nk_t *n)
{
    wtk_listen_t *l;
    int i;

    l=(wtk_listen_t*)n->listening->slot;
    //wtk_debug("slot: %d\n",n->listening->nslot);
    for(i=0;i<n->listening->nslot;++i)
    {
        l=&(l[i]);
        //wtk_debug("slot: %d\n",n->listening->nslot);
        return l->port;
    }
    return -1;
}

int wtk_nk_open_listen_fd(wtk_nk_t *n)
{
	wtk_nk_cfg_t *cfg=n->cfg;
	wtk_listen_t *l;
	int i,ret;

	ret=-1;
	l=(wtk_listen_t*)n->listening->slot;
	for(i=0;i<n->listening->nslot;++i)
	{
		ret=wtk_listen_listen(&(l[i]),cfg->loop);
		if(ret!=0){goto end;}
		if(n->cfg->prompt)
		{
#ifndef WIN32
			wtk_debug("listen at %d.\n",l[i].port);
#else
			printf("listen at %d.\n",l[i].port);
#endif
		}
	}
	ret=0;
end:
	return ret;
}

int wtk_nk_epoll_add_pipe(wtk_nk_t *nk)
{
	wtk_event_t *e;
	int ret;
	int fd;

	if(nk->pipe && nk->pipe->pipe_event.handler)
	{
		e=&(nk->pipe->pipe_event);
	    fd=nk->pipe->pipe_queue.pipe_fd[0];
		ret=wtk_epoll_add_event(nk->epoll,fd,e);
	}else
	{
		ret=0;
	}
	return ret;
}

int wtk_nk_epoll_add_listen_connection(wtk_nk_t *net)
{
	wtk_connection_t *c;
	wtk_listen_t *l;
	int i,ret;

	if(net->pipe)
	{
		ret=wtk_nk_epoll_add_pipe(net);
		if(ret!=0){goto end;}
	}
	l=(wtk_listen_t*)net->listening->slot;
	for(i=0;i<net->listening->nslot;++i)
	{
		c=wtk_nk_pop_connection(net);
		if(!c){ret=-1;goto end;}
		wtk_connection_init(c,l[i].fd,&l[i],CONNECTION_EVENT_READ|CONNECTION_EVENT_ACCEPT);
		c->addr_text.len=sprintf(c->name,"listen:%d",l->port);
		c->addr_text.data=c->name;
		l->con=c;
		c->ready=1;
		ret=wtk_epoll_add_connection(net->epoll,c);
		if(ret!=0){goto end;}
		wtk_log_log(net->log,"listen at %d.",l[i].port);
	}
	ret=0;
end:
	return ret;
}

int wtk_nk_start_listen(wtk_nk_t* net)
{
	return wtk_nk_open_listen_fd(net);
}

int wtk_nk_start_epoll(wtk_nk_t* net)
{
	int ret;

	ret=wtk_epoll_create(net->epoll);
	if(ret!=0){goto end;}
	ret=wtk_nk_epoll_add_listen_connection(net);
end:
	return ret;
}

int wtk_nk_prepare(wtk_nk_t *nk)
{
	int ret;

	ret=wtk_nk_start_listen(nk);
	if(ret!=0){goto end;}
	ret=wtk_nk_start_epoll(nk);
	if(ret!=0){goto end;}
end:
	return ret;
}


int wtk_nk_epoll_monitor_listen_connection(wtk_nk_t *net,int monitor)
{
	wtk_connection_t *c;
	wtk_listen_t *l;
	int i,ret;

	l=(wtk_listen_t*)net->listening->slot;
	for(i=0;i<net->listening->nslot;++i)
	{
		c=l->con;
		if(monitor)
		{
			wtk_epoll_add_connection(net->epoll,c);
		}else
		{
			wtk_epoll_remove_connection(net->epoll,c);
		}
	}
	ret=0;
	return ret;
}

int wtk_nk_run(wtk_nk_t *net)
{
	wtk_epoll_t *e;
    wtk_log_t *l;
    wtk_time_t *t;
	int ret,has_accept;
	double time_out;
	//int n;
#ifdef WIN32
#else
	double last_time=0;
#endif

	net->run=1;e=net->epoll;
    l=net->log;t=l?l->t:0;
    time_out=net->cfg->poll_timeout;
    has_accept=0;
	while(net->run)
	{
		//wtk_debug("%x,%d\n",(int)pthread_self(),net->con_hoard->use_length);
		if(net->cfg->update_time)
		{
			wtk_time_update(t);
		}
#ifdef WIN32
#else
		if(net->cpu  && t->wtk_cached_time>(last_time))
		{
			wtk_cpu_update(net->cpu);
			last_time=t->wtk_cached_time+net->cfg->cpu_update_timeout;
		}
#endif
#ifdef WIN32
        ret=wtk_epoll_process(e,-1);
#else
		if(net->accept_sem)
		{
			ret=sem_trywait(net->accept_sem);
			has_accept=ret==0?1:0;
			//wtk_debug("%d,%d..\n",getpid(),has_accept);
			if(has_accept)
			{
				wtk_nk_epoll_monitor_listen_connection(net,1);
			}
			ret=wtk_epoll_process(e,has_accept,time_out,net->accept_sem);
			if(has_accept)
			{
				wtk_nk_epoll_monitor_listen_connection(net,0);
			}
		}else
		{
			ret=wtk_epoll_process2(e,time_out,0);
		}
#endif
        if(ret!=0)
        {
#ifdef WIN32
        	wtk_log_log(l,"%s: %d",__FUNCTION__,WSAGetLastError());
#else
        	wtk_log_log(l,"%s: %d",__FUNCTION__,errno);
#endif
            break;
        }
        if(t)
        {
            wtk_timer_t *timer;
#ifdef USE_TREE
            wtk_rbnode_t *tn;
        	//wtk_debug("nk timer:%p,%d\n",net,net->timer_tree?net->timer_tree->len:0);
        	while(net->timer_tree && net->timer_tree->root)
        	{
        		tn=(wtk_rbnode_t*)wtk_treenode_min((wtk_treenode_t*)net->timer_tree);
        		//wtk_debug("%.0f,%.0f,nk=%p,tn=%p,root=%p,left=%p,right=%p\n",tn->key,t->wtk_cached_time,net,tn,net->timer_tree->root,net->timer_tree->root->left,net->timer_tree->root->right);
        		if(!tn){break;}
        		if(tn->key > t->wtk_cached_time){break;}
        		wtk_rbtree_remove(net->timer_tree,tn);
        		timer=data_offset(tn,wtk_timer_t,n);
        		timer->handler(timer->data,timer);
        	}
#else
        	wtk_queue_node_t *n;
        	wtk_queue_t *q=&(net->timer_queue);

        	while(q->length>0)
        	{
        		n=q->pop;
        		timer=data_offset(n,wtk_timer_t,q_n);
        		if(timer->key>t->wtk_cached_time){break;}
        		wtk_queue_pop(q);
        		timer->handler(timer->data,timer);
        	}
#endif
    	}
	}
	wtk_log_log(l,"network thread[%p] exit(run=%d).",net,net->run);
	return 0;
}

int wtk_nk_add_timer(wtk_nk_t *net,double delay,wtk_timer_t *timer,wtk_timer_handler handler,void *data)
{
#ifdef USE_TREE
	if(!net->timer_tree)
	{
		net->timer_tree=wtk_rbtree_new();
	}
	timer->handler=handler;
	timer->data=data;
	timer->n.key=delay+net->log->t->wtk_cached_time;
	wtk_rbtree_insert(net->timer_tree,&(timer->n));
#else
	wtk_queue_node_t *n,*prev;
	wtk_queue_t *q;
	wtk_timer_t *item;

	timer->key=delay+net->log->t->wtk_cached_time;
	timer->handler=handler;
	timer->data=data;
	q=&(net->timer_queue);
	for(prev=0,n=q->pop;n;n=n->next)
	{
		item=data_offset(n,wtk_timer_t,q_n);
		if(item->key<=timer->key)
		{
			prev=n->prev;
			break;
		}
	}
	if(prev)
	{
		wtk_queue_insert_to(q,prev,&(timer->q_n));
	}else
	{
		wtk_queue_push(q,&(timer->q_n));
	}
#endif
	return 0;
}

int wtk_nk_stop(wtk_nk_t *n)
{
#ifdef WIN32
    //closesocket(http->pipe_queue->pipe_fd[0]);
#else
	if(n->pipe)
	{
		wtk_pipequeue_touch_write(&(n->pipe->pipe_queue));
	}
#endif
#ifdef WIN32
	wtk_listen_t *l;
	int i;

	l=(wtk_listen_t*)n->listening->slot;
	for(i=0;i<n->listening->nslot;++i)
    {
        if(l->fd)
        {
            closesocket(l->fd);
            l->fd=0;
        }
	}
#endif
	n->run=0;
	return 0;
}

wtk_connection_t* wtk_nk_add_client_fd(wtk_nk_t *nk,int fd)
{
	wtk_connection_t *c;
	wtk_listen_t *l;
	int ret;

	l=(wtk_listen_t*)nk->listening->slot;
	c=wtk_nk_pop_connection(nk);
	wtk_connection_init(c,fd,&(l[0]),CONNECTION_EVENT_READ);
	ret=wtk_connection_attach_fd(c,fd);
	if(ret!=0)
	{
		wtk_connection_clean(c);
		wtk_nk_push_connection(c->net, c);
		c=0;
	}
	return c;
}

void wtk_nk_print(wtk_nk_t *n)
{
	printf("============== Net Statics ==================\n");
	printf("Connection:\t used=%d,free=%d\n",n->con_hoard->use_length,n->con_hoard->cur_free);
	printf("Statck:\t used=%d,free=%d\n",n->stack_hoard->use_length,n->stack_hoard->cur_free);
}
