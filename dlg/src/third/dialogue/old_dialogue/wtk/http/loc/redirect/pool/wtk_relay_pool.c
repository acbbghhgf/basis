#include "wtk_relay_pool.h"
#include "wtk/os/wtk_socket.h"
#include "wtk/http/nk/wtk_connection.h"
#include "wtk/http/proto/wtk_response.h"
void wtk_relay_pool_close(wtk_relay_pool_t *p);

wtk_relay_pool_t* wtk_relay_pool_new(wtk_relay_pool_cfg_t *cfg)
{
	wtk_relay_pool_t *p;

	p=(wtk_relay_pool_t*)wtk_malloc(sizeof(*p));
	p->cfg=cfg;
	p->fd=-1;
	p->response=wtk_http_response_new(p->response);
	p->buf=wtk_strbuf_new(1024,1);
	wtk_lock_init(&(p->lock));
	return p;
}

void wtk_relay_pool_delete(wtk_relay_pool_t *p)
{
	wtk_relay_pool_close(p);
	wtk_strbuf_delete(p->buf);
	wtk_http_response_delete(p->response);
	wtk_lock_clean(&(p->lock));
	wtk_free(p);
}

void wtk_relay_pool_close(wtk_relay_pool_t *p)
{
	if(p->fd>0)
	{
		wtk_socket_close_fd(p->fd);
		p->fd=-1;
	}
}

int wtk_relay_pool_process_route(wtk_relay_pool_t *p,wtk_request_t *req)
{
	int ret=0;

	//wtk_request_print(req);
	if(req->params.len>0 && (wtk_string_cmp_s(&(req->params),"link")==0))
	{
		if(p->fd>0)
		{
			wtk_response_set_body_s(req->response,"already uphost linked");
			ret=-1;
			goto end;
		}else
		{
			//wtk_relay_pool_close(p);
			//wtk_debug("link %p,[%.*s].\n",req->c,req->c->addr_text.len,req->c->addr_text.data);
			wtk_log_log(req->c->net->log,"link %p,[%.*s].",req->c,req->c->addr_text.len,req->c->addr_text.data);
			p->fd=dup(req->c->fd);
			wtk_fd_set_block(p->fd);
			//wtk_debug("fd=%d,%d\n",req->c->fd,p->fd);
			wtk_response_set_body_s(req->response,"suc");
			//req->c->want_close=1;
			req->c->event->eof=1;
			req->c->event->read=0;
			ret=0;
			goto end;
		}
	}else
	{
		if(p->fd>0)
		{
			wtk_strbuf_t *buf=p->buf;

			ret=wtk_request_redirect(req,p->fd,buf);
			//wtk_debug("[%.*s]\n",buf->pos,buf->data);
			if(ret!=0)
			{
				wtk_debug("redirect failed ...\n");
				goto end;
			}
			ret=wtk_http_response_feed_fd(p->response,p->fd,buf);
			if(ret!=0)
			{
				wtk_debug("response failed ...\n");
				goto end;
			}
			wtk_request_update_response(req,p->response);
		}else
		{
			ret=-1;
		}
	}
end:
	//wtk_debug("ret=%d\n",ret);
	if(ret!=0)
	{
		wtk_relay_pool_close(p);
		wtk_request_set_disconnect_err(req);
		ret=0;
	}
	return ret;
}

int wtk_relay_pool_process(wtk_relay_pool_t *p,wtk_request_t *req)
{
	int ret;

	ret=wtk_lock_lock(&(p->lock));
	if(ret!=0){goto end;}
	ret=wtk_relay_pool_process_route(p,req);
	wtk_lock_unlock(&(p->lock));
	if(ret!=0){goto end;}
end:
	return ret;
}
