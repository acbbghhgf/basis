#include "wtk_httpc.h"

wtk_httpc_t* wtk_httpc_new(wtk_httpc_cfg_t *cfg)
{
	wtk_httpc_t* httpc;

	httpc=(wtk_httpc_t*)wtk_malloc(sizeof(*httpc));
	httpc->cfg=cfg;
	httpc->buf=wtk_strbuf_new(1024,1);
	httpc->req=wtk_httpc_request_new(cfg);//cfg->ip.data,cfg->port.data,&(cfg->url));
	if(!cfg->use_1_1)
	{
		httpc->req->response->unknown_content_length=1;
	}
	if(!httpc->req)
	{
		wtk_httpc_delete(httpc);
		httpc=0;
	}
	return httpc;
}

int wtk_httpc_delete(wtk_httpc_t *httpc)
{
	wtk_strbuf_delete(httpc->buf);
	if(httpc->req){wtk_httpc_request_delete(httpc->req);}
	wtk_free(httpc);
	return 0;
}

int wtk_httpc_reset(wtk_httpc_t *httpc)
{
	httpc->req->response->body->pos=0;
	return 0;
}

int wtk_httpc_req(wtk_httpc_t* httpc,char *param,int param_bytes)
{
	return wtk_httpc_request_req(httpc->req,param,param_bytes,0,httpc->cfg->use_1_1);
}

int wtk_httpc_post(wtk_httpc_t* httpc,char *param,int param_bytes)
{
	return wtk_httpc_request_req(httpc->req,param,param_bytes,1,httpc->cfg->use_1_1);
}

wtk_string_t wtk_httpc_get(wtk_httpc_t *httpc)
{
	wtk_string_t v;

	wtk_string_set(&(v),httpc->req->response->body->data,httpc->req->response->body->pos);
	return v;
}

void wtk_httpc_set_hdr_pad(wtk_httpc_t *httpc,void *add_hdr_ths,wtk_httpc_request_add_hdr_f add_hdr)
{
	httpc->req->add_hdr_f=add_hdr;
	httpc->req->add_hdr_ths=add_hdr_ths;
}

void wtk_httpc_test_g()
{
	wtk_httpc_cfg_t cfg,*pcfg=&cfg;
	wtk_httpc_t *hc;
	int i,n;
	double t;
	double tot;

	wtk_httpc_cfg_init(pcfg);
	wtk_string_set_s(&(pcfg->ip),"www.baidu.com");
	wtk_string_set_s(&(pcfg->port),"80");
	wtk_string_set_s(&(pcfg->url),"/");
	hc=wtk_httpc_new(pcfg);
	tot=0;n=3;
	for(i=0;i<n;++i)
	{
		t=time_get_ms();
		wtk_httpc_req_s(hc,"");
		t=time_get_ms()-t;
		tot+=t;
		wtk_http_response_print(hc->req->response);
		//printf("%*.*s\n",hc->req->response->body->pos,hc->req->response->body->pos,hc->req->response->body->data);
		printf("time(%d):\t%.3f ms\n",i,t);
	}
	printf("avg:\t%.3f ms\n",tot/n);
	wtk_httpc_delete(hc);
}

void wtk_httpc_set_body_notify(wtk_httpc_t *httpc,void *ths,wtk_httpc_response_notify_f notify)
{
	wtk_httpc_request_set_notify(httpc->req,ths,notify);
}

int wtk_httpc_wget(char *url,void *ths,wtk_httpc_response_notify_f notify)
{
	return wtk_httpc_wget2(url,ths,notify,2000);
}

int wtk_httpc_wget2(char *url,void *ths,wtk_httpc_response_notify_f notify,int timeout)
{
	return wtk_httpc_wget3(url,ths,notify,timeout,0);
}

int wtk_httpc_wget3(char *url,void *ths,wtk_httpc_response_notify_f notify,int timeout,int use_hack)
{
	int run=1;

	return wtk_httpc_wget4(url,ths,notify,timeout,use_hack,&run);
}

int wtk_httpc_wget4(char *url,void *ths,wtk_httpc_response_notify_f notify,int timeout,int use_hack,int *run)
{
	wtk_http_url_t xu;
	wtk_httpc_cfg_t cfg;
	int ret;
	wtk_httpc_t *httpc=NULL;
	wtk_strbuf_t *buf;
	char ip[256];
	char port[256];

	//wtk_debug("url=%s\n",url);
	ret=wtk_http_url_decode_http(&(xu),url,strlen(url));
	if(ret!=0){goto end;}
	//wtk_http_url_print(&(xu));
	wtk_httpc_cfg_init(&(cfg));
	memcpy(ip,xu.ip.data,xu.ip.len);
	ip[xu.ip.len]=0;
	wtk_string_set(&(cfg.ip),ip,xu.ip.len);
	memcpy(port,xu.port.data,xu.port.len);
	port[xu.port.len]=0;
	wtk_string_set(&(cfg.port),xu.port.data,xu.port.len);
	cfg.timeout=timeout;
	cfg.use_hack=use_hack;
	buf=wtk_strbuf_new(256,1);
	wtk_http_url_encode2(buf,xu.uri.data,xu.uri.len);
	wtk_string_set(&(cfg.url),buf->data,buf->pos);
	//wtk_debug("[%.*s]\n",cfg.url.len,cfg.url.data);
	//exit(0);
	wtk_httpc_cfg_update(&(cfg));
	httpc=wtk_httpc_new(&(cfg));
	//exit(0);
	wtk_httpc_set_body_notify(httpc,ths,notify);
	//wtk_httpc_req(httpc,NULL,0);
	wtk_httpc_request_req2(httpc->req,NULL,0,0,cfg.use_1_1,run);
	wtk_httpc_delete(httpc);
	wtk_strbuf_delete(buf);
end:
	//exit(0);
	return ret;
}
