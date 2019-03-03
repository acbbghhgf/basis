#include "wtk_httpnc.h"
#include "wtk/http/nbc/wtk_nbc.h"
int wtk_httpnc_feed(wtk_httpnc_t *h,wtk_connection_t *con,char *data,int len);
int wtk_httpnc_close(wtk_httpnc_t *h);
void wtk_httpnc_add_delay_link(wtk_httpnc_t *p);

void wtk_httpnc_req_to_string(wtk_httpnc_t *h,wtk_httpnc_req_t *req,int use_1_1,wtk_strbuf_t *buf)
{
	char tmp[64];
	int n;

	wtk_strbuf_reset(buf);
	if(!req->use_get)
	{
		wtk_strbuf_push_s(buf,"POST ");
		wtk_strbuf_push(buf,req->url.data,req->url.len);
		if(use_1_1)
		{
			wtk_strbuf_push_s(buf," HTTP/1.1\r\n");
		}else
		{
			wtk_strbuf_push_s(buf," HTTP/1.0\r\n");
		}
		wtk_strbuf_push_s(buf,"Content-Length: ");
		n=sprintf(tmp,"%d",req->param.len);
		wtk_strbuf_push(buf,tmp,n);
		wtk_strbuf_push_s(buf,"\r\n");
		if(req->add_hdr)
		{
			req->add_hdr(req->ths,buf);
		}else
		{
			wtk_strbuf_push_s(buf,"Content-Type: application/x-www-form-urlencoded\r\n");
		}
	}else
	{
		wtk_strbuf_push_s(buf,"GET ");
		wtk_strbuf_push(buf,req->url.data,req->url.len);
		if(req->param.len>0)
		{
			wtk_strbuf_push_s(buf,"?");
			wtk_strbuf_push(buf,req->param.data,req->param.len);
		}
		if(use_1_1)
		{
			wtk_strbuf_push_s(buf," HTTP/1.1\r\nContent-Length: 0\r\n");
		}else
		{
			wtk_strbuf_push_s(buf," HTTP/1.0\r\nContent-Length: 0\r\n");
		}
		if(req->add_hdr)
		{
			req->add_hdr(req->ths,buf);
		}
	}
	if(h->cfg->cookie.cookie && h->cfg->cookie.cookie->pos>0)
	{
		wtk_strbuf_push_s(buf,"Cookie: ");
		wtk_strbuf_push(buf,h->cfg->cookie.cookie->data,h->cfg->cookie.cookie->pos);
		wtk_strbuf_push_s(buf,"\r\n");
	}
	wtk_strbuf_push_s(buf,"\r\n");
	if(!req->use_get && req->param.len>0)
	{
		wtk_strbuf_push(buf,req->param.data,req->param.len);
	}
}

wtk_httpnc_t* wtk_httpnc_new(wtk_httpnc_cfg_t *cfg,wtk_nbc_t *nbc)
{
	wtk_httpnc_t *nc;

	nc=(wtk_httpnc_t*)wtk_calloc(1,sizeof(*nc));
	nc->handler=(wtk_parse_handler)wtk_httpnc_feed;
	nc->close=(wtk_parser_close_handler)wtk_httpnc_close;
	nc->cfg=cfg;
	nc->nbc=nbc;
	nc->response=wtk_http_response_new();
	nc->con=0;
	nc->req=0;
	wtk_nbc_add_httpnc(nbc,nc);
	return nc;
}

void wtk_httpnc_delete(wtk_httpnc_t *h)
{
	if(h->req)
	{
		h->req->notify(h->req->ths,WTK_HTTPNC_DISCONNECT,0);
		h->req=0;
	}
	if(h->con)
	{
		wtk_connection_clean(h->con);
		//wtk_nk_push_connection(p->http->net,p->con);
		h->con=0;
	}
	wtk_http_response_delete(h->response);
	wtk_free(h);
}

int wtk_httpnc_close(wtk_httpnc_t *h)
{
	if(h->req)
	{
		h->req->notify(h->req->ths,WTK_HTTPNC_DISCONNECT,0);
		h->req=0;
	}
	h->con=0;
	wtk_httpnc_add_delay_link(h);
	return 0;
}

void wtk_httpnc_process_request(wtk_httpnc_t *h,wtk_httpnc_req_t *req)
{
	wtk_strbuf_t *buf=h->nbc->nk->tmp_buf;

	wtk_httpnc_req_to_string(h,req,h->cfg->use_1_1,buf);
	//print_data(buf->data,buf->pos);
	//wtk_debug("[%.*s]\n",buf->pos,buf->data);
	wtk_connection_write(h->con,buf->data,buf->pos);
}


void wtk_httpnc_request(wtk_httpnc_t *h,wtk_httpnc_req_t *req)
{
	h->req=req;
	if(h->con)
	{
		wtk_httpnc_process_request(h,req);
	}
}

int wtk_httpnc_delay_link(wtk_httpnc_t* p,wtk_timer_t *timer)
{
	return wtk_httpnc_connect(p);
}

void wtk_httpnc_add_delay_link(wtk_httpnc_t *p)
{
	wtk_nk_add_timer(p->nbc->nk,p->cfg->time_relink,&(p->connect_timer),(wtk_timer_handler)wtk_httpnc_delay_link,p);
}

int wtk_httpnc_connect(wtk_httpnc_t *h)
{
	wtk_nk_t *nk=h->nbc->nk;
	wtk_connection_t *c;
	int ret;

	c=wtk_nk_pop_connection(nk);
	wtk_connection_init(c,0,0,CONNECTION_EVENT_READ);
	c->parser=(wtk_parser_t*)h;
	ret=wtk_connection_connect(c,h->cfg->addr->addr,h->cfg->addr->addrlen);
	if(ret!=0){goto end;}
	h->con=c;
	if(h->req)
	{
		wtk_httpnc_process_request(h,h->req);
	}
	//h->count=0;
end:
	//wtk_debug("%p connect ret=%d,c=%p,%p\n",h,ret,c,h->con);
	if(ret!=0)
	{
		h->con=0;
		c->parser=0;
		wtk_nk_push_connection(nk,c);
		wtk_httpnc_add_delay_link(h);
	}
	return 0;
}

int wtk_httpnc_raise_response(wtk_httpnc_t *h,wtk_http_response_t *response)
{
	//wtk_http_response_print(response);
	//++h->count;
	//wtk_debug("count=%d\n",h->count);
	if(h->req)
	{
		h->req->notify(h->req->ths,WTK_HTTPNC_RESPONSE,response);
		h->req=0;
	}
	return 0;
}

int wtk_httpnc_feed(wtk_httpnc_t *p,wtk_connection_t *con,char *buf,int len)
{
	wtk_http_response_t *response=p->response;
	int ret=0,left=len;
	wtk_string_t *v;

	//print_data(buf,len);
	while(left>0)
	{
		ret=wtk_http_response_feed(response,buf,left,&left);
		if(ret!=0){goto end;}
		if(response->state==RESPONSE_CONTENT_DONE)
		{
			ret=wtk_httpnc_raise_response(p,response);
			wtk_http_response_reset(response);
			if(ret!=0){goto end;}
			v=wtk_str_hash_find_s(response->hash,"set-cookie");
			if(v)
			{
				wtk_cookie_cfg_update_cookie(&(p->cfg->cookie),v->data,v->len);
			}
			v=wtk_str_hash_find_s(response->hash,"connection");
			if(v && (wtk_string_cmp_s(v,"close")==0))
			{
				ret=-1;
				goto end;
			}
		}
		if(left<=0){break;}
		buf=buf+len-left;
	}
end:
	return ret;
}
