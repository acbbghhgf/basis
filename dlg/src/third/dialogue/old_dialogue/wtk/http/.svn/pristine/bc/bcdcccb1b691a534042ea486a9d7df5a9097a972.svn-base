#include "wtk_http.h"

int wtk_http_init(wtk_http_t* http,wtk_http_cfg_t *cfg,wtk_log_t *log,void *nke_hook,wtk_http_nke_handler_t nke_handler)
{
	int ret,cache;

	memset(http,0,sizeof(*http));
	cache=cfg->parser_cache;
	http->cfg=cfg;
	http->heap=wtk_heap_new(4096);
	http->net=(wtk_nk_t*)wtk_heap_malloc(http->heap,sizeof(wtk_nk_t));
	ret=wtk_nk_init(http->net,&(cfg->nk),log);
	if(ret!=0){goto end;}
#ifndef WIN32
	http->net->epoll->et=1;
#endif
	http->parser_hoard=(wtk_hoard_t*)wtk_heap_malloc(http->heap,sizeof(wtk_hoard_t));
	wtk_hoard_init(http->parser_hoard,offsetof(wtk_http_parser_t,n),cache,
			(wtk_new_handler_t)wtk_http_new_parser,(wtk_delete_handler_t)wtk_http_parser_delete,http);
	http->request_hoard=(wtk_hoard_t*)wtk_heap_malloc(http->heap,sizeof(wtk_hoard_t));
	wtk_hoard_init(http->request_hoard,offsetof(wtk_request_t,hoard_n),cache,(wtk_new_handler_t)wtk_http_new_request,
			(wtk_delete_handler_t)wtk_request_delete,http);
	http->nke_handler=nke_handler;
	http->nke_hook=nke_hook;
	if(cfg->nk.use_pipe)
	{
		http->net->pipe->pipe_event.data=http;
		http->net->pipe->pipe_event.handler=(wtk_event_handler)wtk_http_read_vm;
	}
	http->loc=wtk_loc_new(&(cfg->loc),http);
	if(cfg->use_pp)
	{
		http->pp=wtk_pp_new(&(cfg->pp),http);
	}else
	{
		http->pp=0;
	}
	if(cfg->plink.used || (cfg->plink.nlink==0))
	{
		http->plink=0;
	}else
	{
		http->plink=wtk_plink_new(&(cfg->plink),http);
		cfg->plink.used=1;
	}
	ret=0;
end:
	return ret;
}

int wtk_http_bytes(wtk_http_t *http)
{
	int b=0;
	int x;

	if(http->parser_hoard)
	{
		b+=x=wtk_hoard_bytes(http->parser_hoard,(wtk_hoard_bytes_f)wtk_http_parser_bytes);
		//wtk_debug("parser: %d\n",x);
	}
	b+=x=wtk_hoard_bytes(http->request_hoard,(wtk_hoard_bytes_f)wtk_request_bytes);
	//wtk_debug("request: %d\n",x);
	b+=wtk_heap_bytes(http->heap);
	//wtk_debug("heap: %d\n",wtk_heap_bytes(http->heap));
	b+=wtk_nk_bytes(http->net);
	/*
	wtk_debug("nk=%d\n",b);
	wtk_debug("request: use=%d,free=%d\n",http->request_hoard->use_length,http->request_hoard->cur_free);
	wtk_debug("parser: use=%d,free=%d\n",http->parser_hoard->use_length,http->parser_hoard->cur_free);
	wtk_debug("b=%d\n",b);
	*/
	return b;
}

void wtk_http_clean_mem(wtk_http_t *http)
{
    if(http->loc)
    {
    	wtk_loc_delete(http->loc);
    }
    if(http->parser_hoard)
    {
	    wtk_hoard_clean(http->parser_hoard);
    }
	wtk_hoard_clean(http->request_hoard);
    if(http->heap)
    {
	    wtk_heap_delete(http->heap);
    }
}

int wtk_http_clean(wtk_http_t *http)
{
	if(http->pp){wtk_pp_delete(http->pp);}
    if(http->net)
    {
	    wtk_nk_clean((http->net));
    }
	if(http->plink)
	{
		wtk_plink_delete(http->plink);
	}
    wtk_http_clean_mem(http);
	//if(http->pp){wtk_pp_delete(http->pp);}
	return 0;
}

int wtk_http_close_fd(wtk_http_t *http)
{
	int ret;

	//ret=wtk_pipequeue_clean(http->pipe_queue);
	ret=wtk_nk_close_fd(http->net);
	return ret;
}

int wtk_http_add_links(wtk_http_t *http)
{
	if(http->pp)
	{
		wtk_pp_link(http->pp);
	}
	if(http->plink)
	{
		wtk_plink_link(http->plink);
	}
	return 0;
}

int wtk_http_prepare(wtk_http_t *http)
{
	wtk_listen_t l;
	int ret;

	wtk_listen_init(&l,&(http->cfg->nk.listen),http->cfg->port,
			(wtk_pop_parser_handler)wtk_http_pop_parser,
			(wtk_push_parser_handler)wtk_http_push_parser,
			http);
	wtk_nk_add_listen((http->net),&l);
	ret=wtk_nk_start_listen(http->net);
	if(ret!=0){goto end;}
	ret=wtk_nk_start_epoll(http->net);
	if(ret!=0){goto end;}
	ret=wtk_http_add_links(http);
end:
	return ret;
}

int wtk_http_prepare2(wtk_http_t *http,wtk_listen_t *listened)
{
	wtk_listen_t l;
	int ret;

	l=*listened;
	wtk_listen_set_parser_handler(&(l),http,(wtk_pop_parser_handler)wtk_http_pop_parser,
			(wtk_push_parser_handler)wtk_http_push_parser);
	wtk_nk_add_listen((http->net),&l);
	ret=wtk_nk_start_listen(http->net);
	if(ret!=0){goto end;}
	ret=wtk_nk_start_epoll(http->net);
	if(ret!=0){goto end;}
	ret=wtk_http_add_links(http);
end:
	return ret;
}

int wtk_http_start_listen(wtk_http_t *http,wtk_listen_t *lfd,int accept_sem)
{
	wtk_listen_t l;
	int ret;

	wtk_listen_init(&l,&(http->cfg->nk.listen),http->cfg->port,
			(wtk_pop_parser_handler)wtk_http_pop_parser,
			(wtk_push_parser_handler)wtk_http_push_parser,
			http);
	wtk_listen_cpy(&l,lfd);
	wtk_nk_add_listen(http->net,&l);
#ifndef WIN32
	if(accept_sem){wtk_nk_add_accept_sem(http->net);}
#endif
	ret=wtk_nk_start_epoll(http->net);
	if(ret!=0){goto end;}
	ret=wtk_http_add_links(http);
end:
	return ret;
}

wtk_http_parser_t* wtk_http_new_parser(wtk_http_t *http)
{
	return wtk_http_parser_new(http);
}

wtk_http_parser_t* wtk_http_pop_parser(wtk_http_t *http,wtk_connection_t *c)
{
	wtk_http_parser_t *p;

	p=(wtk_http_parser_t*)wtk_hoard_pop(http->parser_hoard);
	wtk_http_parser_init(p,c);
	return p;
}

int wtk_http_push_parser(wtk_http_t *http,wtk_http_parser_t *p)
{
	wtk_http_parser_reset(p);
	return wtk_hoard_push(http->parser_hoard,p);
}

wtk_request_t* wtk_http_new_request(wtk_http_t *http)
{
	return wtk_request_new(&(http->cfg->request));
}

wtk_request_t* wtk_http_pop_request(wtk_http_t *http,wtk_connection_t *c)
{
	wtk_request_t *r;

	r = (wtk_request_t*)wtk_hoard_pop(http->request_hoard);
	wtk_request_init(r,c);
	return r;
}

int wtk_http_push_request(wtk_http_t *http,wtk_request_t* r)
{
	wtk_request_reset(r);
	return wtk_hoard_push(http->request_hoard,r);
}

int wtk_http_raise_event(wtk_http_t *http,wtk_nk_event_t *event)
{
	wtk_pipequeue_t *p;

	if(!http->nke_handler){return -1;}
	p=&(http->net->pipe->pipe_queue);
	if(p && p->length>0)
	{
		//when long block parse,maybe there is message back from vm,check vm.
		wtk_http_read_vm(http,0);
	}
	event->pipe_queue=p;
	return http->nke_handler(http->nke_hook,event);
}

int wtk_http_lockqueue_nke_handler(wtk_lockqueue_t* lk,wtk_nk_event_t *event)
{
	return wtk_lockqueue_push(lk,&(event->pipe_n));
}

int wtk_nk_event_done(wtk_nk_event_t *nke)
{
	wtk_request_t *req;
	wtk_http_parser_t* parser;
	int ret=0;

	switch(nke->type)
	{
	case WTK_NK_REQUEST:
		req=(wtk_request_t*)nke->data;
        //wtk_request_print(req);
		ret=wtk_http_parser_done_request((wtk_http_parser_t*)req->c->parser,req);
		break;
	case WTK_NK_CON_CLOSE:
		parser=(wtk_http_parser_t*)nke->data;
		ret=wtk_http_parser_done_unlink(parser);
		break;
	}
	return ret;
}

int wtk_http_read_vm(wtk_http_t* http,wtk_event_t *e)
{
	wtk_pipequeue_t *p=&(http->net->pipe->pipe_queue);
	wtk_queue_node_t *n;
	wtk_nk_event_t *nke;
	int ret=0;

	while(p->length>0)
	{
		n=wtk_pipequeue_pop(p);
		if(!n){goto end;}
		nke=data_offset(n,wtk_nk_event_t,pipe_n);
		ret=wtk_nk_event_done(nke);
	}
end:
	return ret;
}


void wtk_http_attach_test_hdr(wtk_http_t *h,wtk_thread_t *t)
{
	char buf[256];
	int n;

	if(h->net->cfg->attach_test && t)
	{
		n=sprintf(buf,"thread=%d;",t->ppid);
		h->net->nk_test=wtk_heap_dup_string(h->net->heap,buf,n);
	}
}

int wtk_http_run(wtk_http_t *http,wtk_thread_t *t)
{
	int ret;

	if(t)
	{
		wtk_log_log(http->net->log,"http[%d] is ready.",t->ppid);
	}
	wtk_http_attach_test_hdr(http,t);
	ret=wtk_nk_run(http->net);
	wtk_log_log(http->net->log,"http[%p] is exit.",http);
	return ret;
}

int wtk_http_stop(wtk_http_t *http)
{
    if(http->net)
    {
    	wtk_nk_stop(http->net);
    }
	return 0;
}

int wtk_http_do_location(wtk_http_t *http,wtk_request_t *req)
{
	//wtk_request_print(req);
	return wtk_loc_do(http->loc,req);
}

void wtk_http_request_set_raise_able(wtk_http_t *http,void *hook,wtk_http_reuqest_raise_able_f raise)
{
	http->request_raise_able_hook=hook;
	http->request_raise_able=raise;
}


int wtk_http_request_is_raise_able(wtk_http_t *http,wtk_request_t *request)
{
	if(http->request_raise_able)
	{
		return http->request_raise_able(http->request_raise_able_hook,request);
	}else
	{
		return wtk_request_is_speech(request);
		/*
		if((request->url.len==1) && (request->speech||request->params.len>0))
		{
			return 1;
		}else
		{
			return 0;
		}
		*/
	}
}

void wtk_http_set_response_filter(wtk_http_t *http,void *hook,wtk_http_response_filter_f filter)
{
	http->response_filter_hook=hook;
	http->response_filter=filter;
}
