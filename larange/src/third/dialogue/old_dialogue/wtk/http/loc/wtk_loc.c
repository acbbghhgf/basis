#include "wtk/http/loc/statics/wtk_statics.h"
#include "wtk/http/wtk_http.h"
#include "wtk_loc.h"
int wtk_loc_process_root(wtk_loc_t *l,wtk_request_t *req);

void wtk_loc_add_ctxs(wtk_loc_t *l)
{
	int i;

	if(l->cfg->use_lfs)
	{
		l->lfs=wtk_lfs_new(&(l->cfg->lfs));
		//l->lost_ctx=wtk_loc_add_ctx(l,&(l->lfs->cfg->url),l->lfs,(wtk_loc_ctx_process_f)wtk_lfs_process);
	}
#ifdef USE_STATICS
	if(l->cfg->use_statics)
	{
		wtk_loc_add_ctx(l,&(l->cfg->statics.url_statics),0,wtk_loc_statics_process);
		wtk_loc_add_ctx(l,&(l->cfg->statics.url_cpu),0,wtk_loc_cpu_process);
		wtk_loc_add_ctx(l,&(l->cfg->statics.url_debug),0,wtk_loc_debug_process);
		wtk_loc_add_ctx(l,&(l->cfg->statics.url_speech),0,wtk_loc_speech_process);
	}
#endif
	//wtk_loc_add_ctx(l,&(l->cfg->cd.url),&(l->cfg->cd),(wtk_loc_ctx_process_f)wtk_cd_cfg_process);
	if(l->cfg->use_root)
	{
		//wtk_loc_add_ctx(l,&(l->cfg->url_root),l,(wtk_loc_ctx_process_f)wtk_loc_process_root);
	}
#ifdef WIN32
#else
	for(i=0;i<l->cfg->redirect.nhost;++i)
	{
		//wtk_loc_add_ctx(l,&(l->cfg->redirect.hosts[i].url),(l->redirect->hosts[i]),(wtk_loc_ctx_process_f)wtk_relay_host_process);
	}
	for(i=0;i<l->cfg->redirect.npool;++i)
	{
//		wtk_loc_add_ctx(l,&(l->cfg->redirect.pools[i].url),(l->redirect->cfg->xpools[i]),
//				(wtk_loc_ctx_process_f)wtk_relay_pool_process);
	}
#endif
}


wtk_loc_t* wtk_loc_new(wtk_loc_cfg_t *cfg,wtk_http_t *http)
{
	wtk_loc_t *l;

	l=(wtk_loc_t*)wtk_calloc(1,sizeof(*l));
	l->cfg=cfg;
	l->ctx_hash=wtk_str_hash_new(cfg->hash_nslot);
	l->lost_ctx=0;
	l->http=http;
#ifdef WIN32
#else
	l->redirect=wtk_redirect_new(&(cfg->redirect),http->net->tmp_buf);
#endif
	wtk_queue_init(&(l->ctx_q));
	wtk_loc_add_ctxs(l);
	return l;
}

int wtk_loc_delete(wtk_loc_t *l)
{
	if(l->lfs)
	{
		wtk_lfs_delete(l->lfs);
	}
#ifdef WIN32
#else
	wtk_redirect_delete(l->redirect);
#endif
	wtk_str_hash_delete(l->ctx_hash);
	wtk_free(l);
	return 0;
}

wtk_loc_ctx_t* wtk_loc_add_ctx(wtk_loc_t *l,wtk_string_t *url,void *data,wtk_loc_ctx_process_f process)
{
	wtk_loc_ctx_t *ctx;

	ctx=(wtk_loc_ctx_t*)wtk_heap_malloc(l->ctx_hash->heap,sizeof(*ctx));
	ctx->data=data;
	ctx->process=process;
	wtk_str_hash_add_node(l->ctx_hash,url->data,url->len,ctx,&(ctx->hash_n));
	wtk_queue_push(&(l->ctx_q),&(ctx->q_n));
	return ctx;
}

wtk_loc_ctx_t* wtk_loc_find_ctx(wtk_loc_t *l,char *u,int bytes)
{
	wtk_loc_ctx_t *ctx;

	ctx=(wtk_loc_ctx_t*)wtk_str_hash_find(l->ctx_hash,u,bytes);
	if(!ctx)
	{
		ctx=l->lost_ctx;
	}
	//wtk_debug("%.*s=%p\n",bytes,u,ctx);
	return ctx;
}

int wtk_loc_do(wtk_loc_t *l,wtk_request_t *req)
{
	wtk_loc_ctx_t *ctx;
	int ret=-1;

	//print_data(req->url.data,req->url.len);
	ctx=wtk_loc_find_ctx(l,req->url.data,req->url.len);
	if(!ctx){goto end;}
	ret=ctx->process(ctx->data,req);
end:
	return ret;
}

void wtk_loc_ctx_to_html(wtk_loc_t *l,wtk_strbuf_t *buf)
{
	wtk_queue_node_t *n;
	wtk_loc_ctx_t *ctx;

	wtk_strbuf_push_s(buf,"<ul>");
	for(n=l->ctx_q.pop;n;n=n->next)
	{
		ctx=data_offset(n,wtk_loc_ctx_t,q_n);
		if(ctx==l->lost_ctx)
		{
			continue;
		}
		wtk_strbuf_push_s(buf,"<li><a href=\"");
		wtk_strbuf_push(buf,ctx->hash_n.key.data,ctx->hash_n.key.len);
		wtk_strbuf_push_s(buf,"\">");
		wtk_strbuf_push(buf,ctx->hash_n.key.data,ctx->hash_n.key.len);
		wtk_strbuf_push_s(buf,"</a></li>");
	}
	wtk_strbuf_push_s(buf,"</ul>");
}

int wtk_loc_process_root(wtk_loc_t *l,wtk_request_t *req)
{
	static wtk_string_t html_type=wtk_string("text/html");
	wtk_strbuf_t *buf=req->c->net->tmp_buf;

	wtk_strbuf_reset(buf);
	wtk_strbuf_push_s(buf,"<html><head><title>Qdreamer</title></head><body bgcolor=\
\"white\" text=\"black\"><center><h2>##########################################################################\
</p></h2><h1>");
	wtk_strbuf_push(buf,l->http->cfg->server.data,l->http->cfg->server.len);
	wtk_strbuf_push_s(buf," server</p></h1><h2>##########################################################################\
</p></h2></center>");
	if(l->cfg->show_root)
	{
		wtk_loc_ctx_to_html(l,buf);
	}
	wtk_strbuf_push_s(buf,"</body></html>");
	wtk_response_set_body(req->response,buf->data,buf->pos);
	req->response->content_type=&(html_type);
	return 0;
}
