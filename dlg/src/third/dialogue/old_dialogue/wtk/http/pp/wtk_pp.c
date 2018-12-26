#include "wtk_pp.h"
#include "wtk/http/wtk_http.h"
int wtk_pp_feed(wtk_pp_t *pp,wtk_connection_t *c,char *buf,int len);
int wtk_pp_close(wtk_pp_t *pp);
int wtk_pp_delay_link(wtk_pp_t* p,wtk_timer_t *timer);

wtk_pp_t* wtk_pp_new(wtk_pp_cfg_t *cfg,struct wtk_http* http)
{
	wtk_pp_t *pp;

	pp=(wtk_pp_t*)wtk_calloc(1,sizeof(*pp));
	pp->handler=(wtk_parse_handler)wtk_pp_feed;
	pp->close=(wtk_parser_close_handler)wtk_pp_close;
	pp->cfg=cfg;
	pp->http=http;
	pp->response=wtk_http_response_new();
	pp->print_con_info=1;
	return pp;
}

int wtk_pp_delete(wtk_pp_t *p)
{
	if(p->con)
	{
		wtk_connection_clean(p->con);
		//wtk_nk_push_connection(p->http->net,p->con);
		p->con=0;
	}
	wtk_http_response_delete(p->response);
	wtk_free(p);
	return 0;
}

int wtk_pp_reset(wtk_pp_t *p)
{
	wtk_http_response_reset(p->response);
	return 0;
}

int wtk_pp_request(wtk_pp_t *p,wtk_connection_t *c)
{
	wtk_strbuf_t *buf=p->response->body;
	char t[256];
	int n,ret;

	wtk_strbuf_reset(buf);
	wtk_strbuf_push_s(buf,"GET ");
	wtk_strbuf_push(buf,p->cfg->url.data,p->cfg->url.len);
	n=sprintf(t,"?port=%d&cpus=%d&stream_per_cpu=%.1f&frequency=%.0f&ht_enable=%d",p->http->cfg->port,p->cfg->cpus,p->http->cfg->stream_per_cpu,
			p->cfg->cpu_frequency,p->cfg->ht_enable);
	wtk_strbuf_push(buf,t,n);
	wtk_strbuf_push_s(buf," HTTP/1.1\r\nContent-Length: 0\r\n\r\n");
	//print_data(buf->data,buf->pos);
	ret=wtk_connection_write(c,buf->data,buf->pos);
	wtk_strbuf_reset(buf);
	return ret;
}

int wtk_pp_delay_link(wtk_pp_t* p,wtk_timer_t *timer)
{
	return wtk_pp_link(p);
}

void wtk_pp_add_delay_link(wtk_pp_t *p)
{
	wtk_nk_add_timer(p->http->net,p->cfg->time_relink,&(p->timer),(wtk_timer_handler)wtk_pp_delay_link,p);
}

int wtk_pp_link(wtk_pp_t *p)
{
	wtk_connection_t *c;
	wtk_addrinfo_t *addr=p->cfg->addr;
	int ret;
	//char buf[1024];

	wtk_log_log(p->http->net->log,"pp connect %*.*s ...",p->cfg->ip.len,p->cfg->ip.len,p->cfg->ip.data);
	c=wtk_nk_pop_connection(p->http->net);
	wtk_connection_init(c,0,0,CONNECTION_EVENT_READ);
	c->parser=(wtk_parser_t*)p;
	ret=wtk_connection_connect(c,addr->addr,addr->addrlen);
	if(ret!=0){goto end;}
	//wtk_log_log(p->http->net->log,"pp addr: %s",buf);
	ret=wtk_pp_request(p,c);
end:
	if(p->print_con_info)
	{
		wtk_debug("pp connect %*.*s %s.\n",p->cfg->ip.len,p->cfg->ip.len,p->cfg->ip.data,ret==0?"success":"failed");
	}
	if(ret!=0)
	{
		c->parser=0;
		wtk_nk_push_connection(p->http->net,c);
		wtk_pp_add_delay_link(p);
	}else
	{
		p->con=c;
	}
	wtk_log_log(p->http->net->log,"pp link %*.*s %s.",p->cfg->ip.len,p->cfg->ip.len,p->cfg->ip.data,ret==0?"success":"failed");
	return 0;
}

int wtk_pp_flush(wtk_pp_t *p)
{
	int ret;

	if(p->con)
	{
		ret=wtk_pp_request(p,p->con);
	}else
	{
		ret=wtk_pp_link(p);
	}
	return ret;
}

int wtk_pp_raise_response(wtk_pp_t *pp,wtk_http_response_t *response)
{
	int ret;

	//wtk_http_response_print(response);
	ret=(response->status==200)?0:-1;
	return ret;
}

int wtk_pp_feed(wtk_pp_t *pp,wtk_connection_t *c,char *buf,int len)
{
	wtk_http_response_t *response=pp->response;
	int ret=0,left=len;

	//print_data(buf,len);
	while(left>0)
	{
		ret=wtk_http_response_feed(response,buf,left,&left);
		if(ret!=0){goto end;}
		if(response->state==RESPONSE_CONTENT_DONE)
		{
			ret=wtk_pp_raise_response(pp,response);
			wtk_http_response_reset(response);
			if(ret!=0){goto end;}
			pp->print_con_info=0;
			if(pp->cfg->use_touch)
			{
				ret=-1;goto end;
			}
		}
		if(left<=0){break;}
		buf=buf+len-left;
	}
end:
	return ret;
}

int wtk_pp_close(wtk_pp_t *pp)
{
	pp->con=0;
	wtk_log_log(pp->http->net->log,"pp is closed by %*.*s",pp->cfg->ip.len,pp->cfg->ip.len,pp->cfg->ip.data);
	wtk_pp_add_delay_link(pp);
	return 0;
}
