#include "wtk_http2.h"

wtk_http2_t* wtk_http2_new(wtk_http2_cfg_t *cfg,wtk_log_t *log,
		void *nke_hook,wtk_http_nke_handler_t nke_handler)
{
	wtk_http2_t *h;
	int i,ret;
	int worker;

	worker=cfg->worker;
	h=(wtk_http2_t*)wtk_malloc(sizeof(*h));
	h->cfg=cfg;
	h->log=log;
	h->tmp_buf=wtk_strbuf_new(1024,1);
	h->https=(wtk_http_t*)wtk_calloc(worker,sizeof(wtk_http_t));
	h->threads=(wtk_thread_t*)wtk_calloc(worker,sizeof(wtk_thread_t));
	for(i=0;i<worker;++i)
	{
		ret=wtk_http_init(&(h->https[i]),&(cfg->http),log,nke_hook,nke_handler);
		if(ret!=0){goto end;}
		ret=wtk_thread_init(&(h->threads[i]),(thread_route_handler)wtk_http_run,&(h->https[i]));
		if(ret!=0){goto end;}
	}
	h->ctx=wtk_http2_ctx_new(&(cfg->ctx),h);
	wtk_listen_init(&(h->listen),&(h->cfg->http.nk.listen),h->cfg->http.port,0,0,0);
	ret=wtk_listen_listen(&(h->listen),h->cfg->http.nk.loop);
	if(ret!=0){goto end;}
end:
	if(ret!=0)
	{
		wtk_debug("create http2 failed\n");
		wtk_http2_delete(h);
		h=0;
	}
	return h;
}

void wtk_http2_delete(wtk_http2_t *h)
{
	int i;

	for(i=0;i<h->cfg->worker;++i)
	{
		wtk_thread_clean(&(h->threads[i]));
		wtk_http_clean(&(h->https[i]));
	}
	wtk_http2_ctx_delete(h->ctx);
	wtk_free(h->https);
	wtk_free(h->threads);
	wtk_listen_close_fd(&(h->listen));
	wtk_strbuf_delete(h->tmp_buf);
	wtk_free(h);
}

void wtk_http2_set_raise_able_f(wtk_http2_t *h,void *hook,wtk_http_reuqest_raise_able_f raise)
{
	int i,worker;

	worker=h->cfg->worker;
	for(i=0;i<worker;++i)
	{
		wtk_http_request_set_raise_able(&(h->https[i]),hook,raise);
	}
}

void wtk_http2_set_response_filter(wtk_http2_t *h,void *hook,wtk_http_response_filter_f filter)
{
	int i,worker;

	worker=h->cfg->worker;
	for(i=0;i<worker;++i)
	{
		wtk_http_set_response_filter(&(h->https[i]),hook,filter);
	}
}

int wtk_http2_start(wtk_http2_t *h)
{
	wtk_listen_t *l=&(h->listen);
	int ret=-1,i,worker;

	worker=h->cfg->worker;
	for(i=0;i<worker;++i)
	{
		ret=wtk_http_prepare2(&(h->https[i]),l);
		if(ret!=0){goto end;}
	}
	for(i=0;i<worker;++i)
	{
		ret=wtk_thread_start(&(h->threads[i]));
		if(ret!=0){goto end;}
	}
end:
	return ret;
}

int wtk_http2_stop(wtk_http2_t *h)
{
	int ret=0,i,worker;

	worker=h->cfg->worker;
	for(i=0;i<worker;++i)
	{
		ret=wtk_http_stop(&(h->https[i]));
		if(ret!=0){goto end;}
	}
end:
	return ret;
}

int wtk_http2_join(wtk_http2_t *h)
{
	int ret=0,i,worker;

	worker=h->cfg->worker;
	for(i=0;i<worker;++i)
	{
		ret=wtk_thread_join(&(h->threads[i]));
		if(ret!=0){goto end;}
	}
end:
	return ret;
}

void wtk_http2_get_statics(wtk_http2_t* h,wtk_strbuf_t *buf)
{
	wtk_http_t *http;
	int i;

	wtk_strbuf_push_s(buf,"[");
	for(i=0;i<h->cfg->worker;++i)
	{
		if(i>0)
		{
			wtk_strbuf_push_s(buf,",");
		}
		wtk_strbuf_push_f(buf,"{\"thread\":%d,",(int)h->threads[i].ppid);
		http=&(h->https[i]);
		wtk_strbuf_push_f(buf,"\"req\":%d,\"con\":%d,\"down\":%d}",http->request_hoard->use_length,
				http->net->con_hoard->use_length,http->net->pipe->pipe_queue.length);
	}
	wtk_strbuf_push_s(buf,"]");
}

int wtk_http2_get_conection_count(wtk_http2_t *h)
{
	int c=0;
	int i;

	for(i=0;i<h->cfg->worker;++i)
	{
		c+=h->https[i].net->con_hoard->use_length;
	}
	return c;
}
