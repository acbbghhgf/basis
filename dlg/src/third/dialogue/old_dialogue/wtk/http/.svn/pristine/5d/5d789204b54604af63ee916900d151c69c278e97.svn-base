#include "wtk_http_cfg.h"
#include "wtk/os/wtk_proc.h"
#include "wtk/os/wtk_cpu.h"
#include "wtk/os/wtk_socket.h"

char* http_status_map[520];

#define HTTP_STATUS_MAP(no,s) \
	http_status_map[no]=s;

static void wtk_init_map()
{
	HTTP_STATUS_MAP(100,"Continue");
	HTTP_STATUS_MAP(101, "witching Protocols");
	HTTP_STATUS_MAP(200, "OK");
	HTTP_STATUS_MAP(201, "Created");
	HTTP_STATUS_MAP(202, "Accepted");
	HTTP_STATUS_MAP(203, "Non-Authoritative Information");
	HTTP_STATUS_MAP(204, "No Content");
	HTTP_STATUS_MAP(205, "Reset Content");
	HTTP_STATUS_MAP(206, "Partial Content");
	HTTP_STATUS_MAP(300, "Multiple Choices");
	HTTP_STATUS_MAP(301, "Moved Permanently");
	HTTP_STATUS_MAP(302, "Found");
	HTTP_STATUS_MAP(303, "See Other");
	HTTP_STATUS_MAP(304, "Not Modified");
	HTTP_STATUS_MAP(305, "Use Proxy");
	HTTP_STATUS_MAP(307, "Temporary Redirect");
	HTTP_STATUS_MAP(400, "Bad Request");
	HTTP_STATUS_MAP(401, "Unauthorized");
	HTTP_STATUS_MAP(402, "Payment Required");
	HTTP_STATUS_MAP(403, "Forbidden");
	HTTP_STATUS_MAP(404, "Not Found");
	HTTP_STATUS_MAP(405, "Method Not Allowed");
	HTTP_STATUS_MAP(406, "Not Acceptable");
	HTTP_STATUS_MAP(407, "Proxy Authentication Required");
	HTTP_STATUS_MAP(408, "Request Time-out");
	HTTP_STATUS_MAP(409, "Conflict");
	HTTP_STATUS_MAP(410, "Gone");
	HTTP_STATUS_MAP(411, "Length Required");
	HTTP_STATUS_MAP(412, "Precondition Failed");
	HTTP_STATUS_MAP(413, "Request Entity Too Large");
	HTTP_STATUS_MAP(414, "Request-URI Too Large");
	HTTP_STATUS_MAP(415, "Unsupported Media Type");
	HTTP_STATUS_MAP(416, "Requested range not satisfiable");
	HTTP_STATUS_MAP(417, "Expectation Failed");
	HTTP_STATUS_MAP(500, "Internal Server Error");
	HTTP_STATUS_MAP(501, "Not Implemented");
	HTTP_STATUS_MAP(502, "Bad Gateway");
	HTTP_STATUS_MAP(503, "Service Unavailable");
	HTTP_STATUS_MAP(504, "Gateway Time-out");
	HTTP_STATUS_MAP(505, "HTTP Version not supported");
}

int wtk_http_cfg_init(wtk_http_cfg_t *cfg)
{
	wtk_init_map();
	wtk_loc_cfg_init(&(cfg->loc));
	wtk_nk_cfg_init(&(cfg->nk));
	wtk_pp_cfg_init(&(cfg->pp));
	wtk_plink_cfg_init(&(cfg->plink));
	wtk_request_cfg_init(&(cfg->request));
	wtk_response_cfg_init(&(cfg->response));
	wtk_string_set_s(&(cfg->iface),"eth0");
    wtk_string_set_s(&(cfg->ver),STR(VER));
    //wtk_string_set_s(&(cfg->ver),"0.0.2");
    wtk_string_set_s(&(cfg->server),"Qdreamer");
    cfg->http_lfs_url=wtk_strbuf_new(256,1);
	cfg->parser_cache=1024;
	cfg->port=8080;
	cfg->cpus=wtk_get_cpus();
	cfg->stream_per_cpu=10;
	cfg->stream_per_hz=10.0/2.603;
	cfg->streams_use_frequency=1;
	cfg->streams_by_cpu=1;
	cfg->max_streams_per_connection=cfg->max_streams=cfg->cpus*cfg->stream_per_cpu;
	//cfg->use_pipe=1;
	cfg->use_pp=0;
	cfg->use_delay_hint=1;
	cfg->log_req_route=0;
	return 0;
}

int wtk_http_cfg_clean(wtk_http_cfg_t *cfg)
{
	wtk_plink_cfg_clean(&(cfg->plink));
	wtk_strbuf_delete(cfg->http_lfs_url);
	if(cfg->use_pp)
	{
		wtk_pp_cfg_clean(&(cfg->pp));
	}
	wtk_loc_cfg_clean(&(cfg->loc));
	wtk_nk_cfg_clean(&(cfg->nk));
	//wtk_pp_cfg_clean(&(cfg->pp));
	wtk_request_cfg_clean(&(cfg->request));
	wtk_response_cfg_clean(&(cfg->response));
	return 0;
}

void wtk_http_cfg_set_max_stream(wtk_http_cfg_t *cfg,int max_active_stream)
{
	cfg->max_streams_per_connection=cfg->max_streams=max_active_stream;
}

void wtk_http_cfg_update_arg(wtk_http_cfg_t *cfg,wtk_arg_t *arg)
{
	double number;
	int ret;

	ret=wtk_arg_get_number_s(arg,"p",&number);
	if(ret==0)
	{
		cfg->port=number;
	}
}

void wtk_http_cfg_print_usage()
{
	printf("\t-p set http port\n");
}

int wtk_http_cfg_update_local(wtk_http_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc=main;
	wtk_string_t* v;
	int ret=0;

	wtk_local_cfg_update_cfg_i(lc,cfg,parser_cache,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,port,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,cpus,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,max_streams,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,max_streams_per_connection,v);
	wtk_local_cfg_update_cfg_f(lc,cfg,stream_per_cpu,v);
	wtk_local_cfg_update_cfg_f(lc,cfg,stream_per_hz,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,streams_by_cpu,v);
	//wtk_local_cfg_update_cfg_b(lc,cfg,use_pipe,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_pp,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_delay_hint,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,log_req_route,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,streams_use_frequency,v);
    wtk_local_cfg_update_cfg_string_v(lc,cfg,ver,v);
    wtk_local_cfg_update_cfg_string_v(lc,cfg,iface,v);
    wtk_local_cfg_update_cfg_string_v(lc,cfg,server,v);
	lc=wtk_local_cfg_find_lc_s(main,"loc");
	if(lc)
	{
		ret=wtk_loc_cfg_update_local(&(cfg->loc),lc);
		if(ret!=0){goto end;}
	}
	lc=wtk_local_cfg_find_lc_s(main,"nk");
	if(lc)
	{
		ret=wtk_nk_cfg_update_local(&(cfg->nk),lc);
		if(ret!=0){goto end;}
	}
	lc=wtk_local_cfg_find_lc_s(main,"pp");
	if(lc)
	{
		ret=wtk_pp_cfg_update_local(&(cfg->pp),lc);
		if(ret!=0){goto end;}
	}
	lc=wtk_local_cfg_find_lc_s(main,"plink");
	if(lc)
	{
		ret=wtk_plink_cfg_update_local(&(cfg->plink),lc);
		if(ret!=0){goto end;}
	}
	lc=wtk_local_cfg_find_lc_s(main,"request");
	if(lc)
	{
		ret=wtk_request_cfg_update_local(&(cfg->request),lc);
		if(ret!=0){goto end;}
	}
	lc=wtk_local_cfg_find_lc_s(main,"response");
	if(lc)
	{
		ret=wtk_response_cfg_update_local(&(cfg->response),lc);
		if(ret!=0){goto end;}
	}
	if(cfg->server.data && 0)
	{
		//wtk_strbuf_t *buf;
		int n;
		char *buf=cfg->ver_buf;
		wtk_strbuf_t *b;

	    //n=sprintf(buf,"%.*s.",cfg->ver.len,cfg->ver.data);
		n=0;
	    n+=wtk_get_build_timestamp(buf+n);
	    wtk_string_set(&(cfg->ver),buf,n);
	    b=wtk_strbuf_new(64,1);
	    wtk_strbuf_push(b,cfg->server.data,cfg->server.len);
	    wtk_strbuf_push_c(b,'.');
	    wtk_strbuf_push(b,cfg->ver.data,cfg->ver.len);
	    wtk_heap_fill_string(main->heap,&(cfg->server),b->data,b->pos);
	    wtk_strbuf_delete(b);
	}
end:
	return ret;
}

int wtk_http_cfg_update_url(wtk_http_cfg_t *cfg)
{
#ifdef WIN32
#else
    char *ip;
#endif
    int ret;

#ifdef WIN32
    wtk_strbuf_reset(cfg->http_lfs_url);
    wtk_strbuf_push_f(cfg->http_lfs_url,"http://127.0.0.1:%d",cfg->port);
#else
    ip=wtk_get_host_ip(cfg->iface.data);
    if(ip)
    {
        wtk_strbuf_reset(cfg->http_lfs_url);
        wtk_strbuf_push_f(cfg->http_lfs_url,"http://%s:%d",ip,cfg->port);
        wtk_free(ip);
    }else
    {
        ret=-1;goto end;
    }
#endif
    ret=0;
end:
        return ret;
}

int wtk_http_cfg_update(wtk_http_cfg_t *cfg)
{
	int ret;
	/*
    char *buf=cfg->ver_buf;
    int n;

    n=sprintf(buf,"%.*s.",cfg->ver.len,cfg->ver.data);
    n+=wtk_get_build_timestamp(buf+n);
    wtk_string_set(&(cfg->ver),buf,n);
    */
    ret=wtk_http_cfg_update_url(cfg);
#ifdef WIN32
#else
    if(cfg->streams_use_frequency)
    {
    	cfg->stream_per_cpu=cfg->stream_per_hz*wtk_cpu_get_frequence();
    }
#endif
	if(cfg->streams_by_cpu)
	{
		wtk_http_cfg_set_max_stream(cfg,cfg->cpus*cfg->stream_per_cpu);
	}
	if(cfg->use_pp)
	{
		ret=wtk_pp_cfg_update(&(cfg->pp));
		if(ret!=0){goto end;}
	}
	ret=wtk_plink_cfg_update(&(cfg->plink));
	if(ret!=0){goto end;}
	ret=wtk_loc_cfg_update(&(cfg->loc));
	if(ret!=0){goto end;}
	ret=wtk_nk_cfg_update(&(cfg->nk));
	if(ret!=0){goto end;}
	ret=wtk_request_cfg_update(&(cfg->request));
	if(ret!=0){goto end;}
	ret=wtk_response_cfg_update(&(cfg->response));
	if(ret!=0){goto end;}
end:
	return ret;
}

void wtk_http_cfg_print(wtk_http_cfg_t *cfg)
{
	printf("------ HTTP ---------\n");
	print_cfg_i(cfg,port);
	wtk_nk_cfg_print(&(cfg->nk));
}

void wtk_http_cfg_set_port(wtk_http_cfg_t *cfg,short port)
{
    cfg->port=port;
    wtk_http_cfg_update_url(cfg);
}
