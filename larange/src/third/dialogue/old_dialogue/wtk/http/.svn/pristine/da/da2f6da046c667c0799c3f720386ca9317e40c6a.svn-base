#include "wtk_http2_ctx.h"
#include "wtk/http/http2/wtk_http2.h"
int wtk_http2_ctx_process_con(wtk_http2_ctx_t *ctx,wtk_request_t *req);

void wtk_http2_ctx_add_loc_ctx(wtk_http2_ctx_t *ctx,wtk_loc_t *loc)
{
	wtk_http2_ctx_cfg_t *cfg;

	cfg=ctx->cfg;
	if(0)
	{
		wtk_loc_add_ctx(loc,&(cfg->url_con),ctx,(wtk_loc_ctx_process_f)wtk_http2_ctx_process_con);
	}
}


void wtk_http2_ctx_add_ctx(wtk_http2_ctx_t *ctx)
{
	wtk_http2_t *http2=ctx->http2;
	wtk_loc_t *loc;
	int i;

	for(i=0;i<http2->cfg->worker;++i)
	{
		loc=http2->https[i].loc;
		wtk_http2_ctx_add_loc_ctx(ctx,loc);
	}
}

wtk_http2_ctx_t* wtk_http2_ctx_new(wtk_http2_ctx_cfg_t *cfg,wtk_http2_t *http2)
{
	wtk_http2_ctx_t *ctx;

	ctx=(wtk_http2_ctx_t*)wtk_malloc(sizeof(*ctx));
	ctx->cfg=cfg;
	ctx->http2=http2;
	wtk_lock_init(&(ctx->lock));
	wtk_http2_ctx_add_ctx(ctx);
	return ctx;
}

void wtk_http2_ctx_delete(wtk_http2_ctx_t *ctx)
{
	wtk_lock_clean(&(ctx->lock));
	wtk_free(ctx);
}

int wtk_http2_ctx_process_con(wtk_http2_ctx_t *ctx,wtk_request_t *req)
{
	wtk_strbuf_t *buf=ctx->http2->tmp_buf;
	wtk_lock_t *lock=&(ctx->lock);
	wtk_http_t *http;
	wtk_queue_node_t *n;
	wtk_connection_t *con;
	int i,j;
	int tot;

	tot=0;
	wtk_lock_lock(lock);
	wtk_strbuf_reset(buf);
	wtk_strbuf_push_s(buf,"{\"http\":[");
	for(i=0;i<ctx->http2->cfg->worker;++i)
	{
		http=&(ctx->http2->https[i]);
		if(i>0)
		{
			wtk_strbuf_push_s(buf,",");
		}
		wtk_strbuf_push_f(buf,"{\"thread\":%d,\"con\":[",ctx->http2->threads[i].ppid);
		tot+=http->net->con_hoard->use_length;
		for(j=0,n=http->net->con_hoard->use;n;n=n->prev,++j)
		{
			con=data_offset(n,wtk_connection_t,net_node_q);
			if(j>0)
			{
				wtk_strbuf_push_s(buf,",\"");
			}else
			{
				wtk_strbuf_push_s(buf,"\"");
			}
			wtk_strbuf_push_string(buf,con->name);
			wtk_strbuf_push_s(buf,"\"");
		}
		wtk_strbuf_push_s(buf,"]}");
	}
	wtk_strbuf_push_f(buf,"],\"tot\":%d}",tot);
	wtk_request_set_response_body(req,buf->data,buf->pos);
	wtk_lock_unlock(lock);
	return 0;
}

