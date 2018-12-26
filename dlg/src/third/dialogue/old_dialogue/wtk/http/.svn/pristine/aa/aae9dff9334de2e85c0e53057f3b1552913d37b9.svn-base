#include "wtk_nbc.h"
int wtk_nbc_read_request(wtk_nbc_t* nbc,wtk_event_t *e);
int wtk_nbc_run(wtk_nbc_t *nbc,wtk_thread_t *thread);


wtk_nbc_t* wtk_nbc_new(wtk_nbc_cfg_t *cfg,wtk_log_t *log)
{
	wtk_nbc_t *nbc;

	nbc=(wtk_nbc_t*)wtk_malloc(sizeof(*nbc));
	nbc->cfg=cfg;
	nbc->nk=wtk_nk_new(&(cfg->nk),log);
	wtk_nk_set_pipe_handler(nbc->nk,nbc,(wtk_event_handler)wtk_nbc_read_request);
	wtk_queue_init(&(nbc->httpnc_q));
	wtk_thread_init(&(nbc->thread),(thread_route_handler)wtk_nbc_run,nbc);
	return nbc;
}

void wtk_nbc_delete(wtk_nbc_t *nbc)
{
	wtk_thread_clean(&(nbc->thread));
	wtk_nk_delete(nbc->nk);
	wtk_free(nbc);
}

void wtk_nbc_add_httpnc(wtk_nbc_t *nbc,wtk_httpnc_t *httpnc)
{
	wtk_queue_push(&(nbc->httpnc_q),&(httpnc->pend_n));
}

int wtk_nbc_prepare(wtk_nbc_t *nbc)
{
	wtk_queue_node_t *n;
	wtk_httpnc_t *h;
	int ret;

	ret=wtk_nk_prepare(nbc->nk);
	if(ret!=0){goto end;}
	for(n=nbc->httpnc_q.pop;n;n=n->next)
	{
		h=data_offset(n,wtk_httpnc_t,pend_n);
		wtk_debug("connect %s:%s\n",h->cfg->ip.data,h->cfg->port.data);
		wtk_httpnc_connect(h);
	}
end:
	return ret;
}

int wtk_nbc_read_request(wtk_nbc_t* nbc,wtk_event_t *e)
{
	wtk_pipequeue_t *p=&(nbc->nk->pipe->pipe_queue);
	wtk_queue_node_t *n;
	wtk_httpnc_req_t *req;

	while(p->length>0)
	{
		n=wtk_pipequeue_pop(p);
		if(!n){goto end;}
		req=data_offset(n,wtk_httpnc_req_t,pipe_n);
		wtk_httpnc_request(req->httpnc,req);
	}
end:
	return 0;
}

void wtk_nbc_send_request(wtk_nbc_t *nbc,wtk_httpnc_req_t *req)
{
	wtk_nk_send_pipe_event(nbc->nk,&(req->pipe_n));
}

int wtk_nbc_start(wtk_nbc_t *nbc)
{
	int ret;

	ret=wtk_nbc_prepare(nbc);
	if(ret!=0){goto end;}
	ret=wtk_thread_start(&(nbc->thread));
end:
	return ret;
}

int wtk_nbc_stop(wtk_nbc_t *nbc)
{
	int ret;

	ret=wtk_nk_stop(nbc->nk);
	if(ret!=0){goto end;}
	ret=wtk_thread_join(&(nbc->thread));
end:
	return ret;
}

int wtk_nbc_run(wtk_nbc_t *nbc,wtk_thread_t *thread)
{
	int ret;

	ret=wtk_nk_run(nbc->nk);
	return ret;
}

//------------------------ test section ------------------------------
int wtk_httpnc_test_req(wtk_httpnc_t *h,wtk_timer_t *timer);
void wtk_httpnc_test_notify(wtk_httpnc_t *h,wtk_httpnc_notify_type_t type,wtk_http_response_t *response)
{
	wtk_http_response_print(response);
	//wtk_nk_add_timer(h->nk,10,&(h->req_timer),(wtk_timer_handler)wtk_httpnc_test_req,h);
}

int wtk_httpnc_test_req(wtk_httpnc_t *h,wtk_timer_t *timer)
{
	static wtk_httpnc_req_t req;

	req.add_hdr=0;
	req.notify=(wtk_httpnc_notify_f)wtk_httpnc_test_notify;
	req.param.len=0;
	req.ths=h;
	req.use_get=1;
	wtk_string_set_s(&(req.url),"/");
	wtk_httpnc_request(h,&req);
	return 0;
}
