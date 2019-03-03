#include "wtk_plink_host.h"
#include "wtk/http/wtk_http.h"
#include "wtk/http/proto/wtk_http_parser.h"
void wtk_plink_host_add_delay_link(wtk_plink_host_t *p);

wtk_plink_host_t* wtk_plink_host_new(wtk_plink_host_cfg_t *cfg,wtk_http_t *http)
{
	wtk_plink_host_t *host;

	host=(wtk_plink_host_t*)wtk_malloc(sizeof(*host));
	host->cfg=cfg;
	host->httpc=wtk_httpc_new(&(cfg->httpc));
	host->http=http;
	host->listener.ths=host;
	host->listener.close=(wtk_connection_close_notify_f)wtk_plink_host_add_delay_link;
	return host;
}

void wtk_plink_host_delete(wtk_plink_host_t *host)
{
	wtk_httpc_delete(host->httpc);
	wtk_free(host);
}

int wtk_plink_host_delay_link(wtk_plink_host_t* p,wtk_timer_t *timer)
{
	if(p->httpc->req->fd>0)
	{
		wtk_httpc_request_close(p->httpc->req);
	}
	return wtk_plink_host_link(p);
}


void wtk_plink_host_add_delay_link(wtk_plink_host_t *p)
{
	//wtk_debug("delay link %d...\n",p->cfg->time_relink);
	wtk_nk_add_timer(p->http->net,p->cfg->time_relink,&(p->timer),(wtk_timer_handler)wtk_plink_host_delay_link,p);
}

int wtk_plink_host_link(wtk_plink_host_t *host)
{
	wtk_connection_t *c;
	int ret;
	int fd;

	ret=wtk_httpc_req_s(host->httpc,"link");
	if(ret!=0){goto end;}
	fd=host->httpc->req->fd;
	if(fd<0){ret=-1;goto end;}
	//wtk_http_response_print(host->httpc->req->response);
	c=wtk_nk_add_client_fd(host->http->net,fd);
	if(!c){ret=-1;goto end;}
	c->evt_listener=&(host->listener);
	ret=0;
end:
	wtk_log_log(host->http->net->log,"link ret=%d,errno=%d.",ret,errno);
	if(ret!=0)
	{
		wtk_plink_host_add_delay_link(host);
	}
	return ret;
}


