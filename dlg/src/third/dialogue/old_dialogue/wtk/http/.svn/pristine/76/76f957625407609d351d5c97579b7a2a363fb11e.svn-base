#include "wtk_response.h"
#include "wtk/http/loc/statics/wtk_statics.h"
#include "wtk/http/wtk_http_cfg.h"
#include "wtk/http/wtk_http.h"
#include <limits.h>

wtk_response_t* wtk_response_new()
{
	wtk_response_t *rep;

	rep=(wtk_response_t*)wtk_calloc(1,sizeof(*rep));
	wtk_response_init(rep);
	return rep;
}

int wtk_response_bytes(wtk_response_t *rep)
{
	return sizeof(*rep);
}

int wtk_response_delete(wtk_response_t *rep)
{
	wtk_response_reset(rep);
	wtk_free(rep);
	return 0;
}

int wtk_response_init(wtk_response_t *rep)
{
	rep->test=0;
	rep->eof=0;
	rep->result=0;
	rep->hint_result=0;
	rep->status=200;
	rep->audio_url=0;
	rep->lua_hdr=0;
    rep->stream_data_received=-1;
    rep->writed=0;
    rep->content_type=0;
    rep->stream_status_id=0;
    rep->custom_hdr_array=0;
    wtk_string_set(&(rep->up),0,0);
    wtk_string_set(&(rep->stream_status),0,0);
    wtk_string_set(&(rep->body),0,0);
    rep->discard=0;
	return 0;
}

int wtk_response_reset(wtk_response_t *rep)
{
	rep->writed=0;
	rep->test=0;
	rep->content_type=0;
	rep->custom_hdr_key=0;
	rep->custom_hdr_value=0;
	rep->custom_hdr_array=0;
	//wtk_stack_reset(rep->stack);
	wtk_string_set(&(rep->body),0,0);
	if(rep->result)
	{
		//param_print(rep->result);
		wtk_param_delete(rep->result);
		rep->result=0;
	}
	if(rep->hint_result)
	{
		wtk_param_delete(rep->hint_result);
		rep->hint_result=0;
	}
	if(rep->lua_hdr)
	{
		wtk_string_delete(rep->lua_hdr);
        rep->lua_hdr=0;
	}
    wtk_response_init(rep);
	return 0;
}

int wtk_response_feedback(wtk_response_t *rep,int status,wtk_connection_t *con,wtk_request_t *req,char *body,int body_len)
{
	rep->status=status;
	wtk_string_set(&(rep->body),body,body_len);
	return wtk_response_write(rep,con,req);
}

void wtk_response_print_hdr_array(wtk_response_t *rep)
{
	wtk_response_hdr_t *hdr,*item;
	int i;

	hdr=(wtk_response_hdr_t*)rep->custom_hdr_array->slot;
	for(i=0;i<rep->custom_hdr_array->nslot;++i)
	{
		item=&(hdr[i]);
		wtk_debug("i=%d,item=%p,k=%p,v=%p\n",i,item,item->k,item->v);
		printf("%.*s: %.*s\n",item->k->len,item->k->data,item->v->len,item->v->data);
	}
}

void wtk_response_attach_hdr_array(wtk_response_t *rep,wtk_stack_t *stk)
{
	wtk_response_hdr_t *hdr,*item;
	int i;

	hdr=(wtk_response_hdr_t*)rep->custom_hdr_array->slot;
	for(i=0;i<rep->custom_hdr_array->nslot;++i)
	{
		item=&(hdr[i]);
		//wtk_debug("item=%p,k=[%.*s],v=[%.*s]\n",item,item->k->len,item->k->data,item->v->len,item->v->data);
		wtk_stack_push(stk,item->k->data,item->k->len);
		wtk_stack_push_s(stk,": ");
		wtk_stack_push(stk,item->v->data,item->v->len);
		wtk_stack_push_s(stk,"\r\n");
	}
}

int wtk_response_write(wtk_response_t *rep,wtk_connection_t *c,wtk_request_t *req)
{
	wtk_stack_t *s;
	char buf[1024];
	wtk_param_t *result;
	char *p=0;
	int len=0,found,n;
	//wtk_net_t *net=c->net;
	wtk_time_t *t=c->net->log->t;
	int is_str;
	wtk_http_t *http;
	wtk_response_cfg_t *cfg;

	if(rep->writed || rep->discard)
	{
		return 0;
	}
	http=(wtk_http_t*)((wtk_http_parser_t*)req->c->parser)->layer;
	if(http->response_filter)
	{
		http->response_filter(http->response_filter_hook,req);
	}
	cfg=&(http->cfg->response);
	rep->writed=1;
	s=c->net->tmp_stk;
    if(req->http1_1)
    {
	    n=sprintf(buf,"HTTP/1.1 %d %s\r\n",rep->status,http_status_map[rep->status]);
    }else
    {
        n=sprintf(buf,"HTTP/1.0 %d %s\r\n",rep->status,http_status_map[rep->status]);
    }
    wtk_stack_reset(s);
    wtk_stack_push(s,buf,n);
    if(cfg->use_date)
    {
    	wtk_stack_push_s(s,"Date: ");
        wtk_stack_push(s,t->http_time.data,t->http_time.len);
        wtk_stack_push_s(s,"\r\n");
    }
	//wtk_stack_push_s(s,"\r\nServer: WTK\r\n");
	if(cfg->use_server)
	{
		wtk_stack_push_s(s,"Server: ");
		wtk_stack_push(s,cfg->server.data,cfg->server.len);
		//wtk_stack_push(s,http->cfg->server.data,http->cfg->server.len);
		//wtk_stack_push(s,http->cfg->ver.data,http->cfg->ver.len);
		wtk_stack_push_s(s,"\r\n");
	}
	if(cfg->use_custom_hdr && rep->custom_hdr_array)
	{
		wtk_response_attach_hdr_array(rep,s);
	}
	if(rep->stream_status.len>0)
	{
        wtk_stack_push_s(s,"Status-Info: ");
		wtk_stack_push(s,rep->stream_status.data,rep->stream_status.len);
        wtk_stack_push_s(s,"\r\n");
        n=sprintf(buf,"Status-Id: %d\r\n",rep->stream_status_id);
        wtk_stack_push(s,buf,n);
	}
	//wtk_debug("times: %p,%d,%.*s\n",req->times,req->times->pos,req->times->pos,req->times->data);
	if(cfg->use_times && req->times && req->times->pos>0)
	{
		wtk_stack_push_s(s,"Times: ");
		wtk_stack_push(s,req->times->data,req->times->pos);
		wtk_stack_push_s(s,"\r\n");
		//wtk_debug("%.*s\n",req->times->pos,req->times->data);
	}
    if(cfg->use_stream_length && rep->stream_data_received>=0)
    {
        n=sprintf(buf,"Stream-Length: %d\r\n",rep->stream_data_received);
        wtk_stack_push(s,buf,n);
    }
	if(cfg->use_hook && req->strhook)
	{
		wtk_stack_push_s(s,"Hook: ");
		wtk_stack_push(s,req->strhook->data,req->strhook->len);
		//wtk_debug("hook: %.*s\n",req->strhook->len,req->strhook->data);
		wtk_stack_push_s(s,"\r\n");
	}
	if(req->client)
	{
		wtk_stack_push_s(s,"Client: ");
		wtk_stack_push(s,req->client->data,req->client->len);
		//wtk_debug("hook: %.*s\n",req->strhook->len,req->strhook->data);
		wtk_stack_push_s(s,"\r\n");
	}
	if(cfg->use_log && req->log)
	{
		wtk_stack_push_s(s,"Log: ");
		wtk_stack_push(s,req->log->data,req->log->len);
		//wtk_stack_push_f(s,"time=%.0f;cache=%d;",time_get_ms(),c->wstack?c->wstack->len:0);
		wtk_stack_push_s(s,"\r\n");
	}
	//wtk_debug("id=%d\n",cfg->use_stream_id);
	if(cfg->use_stream_id && req->local_streamid)
	{
		wtk_stack_push_s(s,"Stream-Id: ");
		wtk_stack_push(s,req->local_streamid->data,req->local_streamid->len);
		wtk_stack_push_s(s,"\r\n");
	}
	if(cfg->use_server && rep->up.len>0)
	{
		wtk_stack_push_s(s,"Up-Server: ");
		wtk_stack_push(s,rep->up.data,rep->up.len);
		wtk_stack_push_s(s,"\r\n");
	}
	if(cfg->use_audio_url && rep->audio_url && rep->audio_url->len>0)
	{
		wtk_stack_push_s(s,"Audio-Url: ");
		wtk_stack_push(s,rep->audio_url->data,rep->audio_url->len);
		wtk_stack_push_s(s,"\r\n");
	}
	if(cfg->use_test)
	{
		if(!rep->test)
		{
			rep->test=c->net->nk_test;
		}
		if(rep->test)
		{
			wtk_stack_push_s(s,"Test: ");
			wtk_stack_push(s,rep->test->data,rep->test->len);
			//wtk_stack_push_f(s,"time=%.0f;cache=%d;",time_get_ms(),c->wstack?c->wstack->len:0);
			wtk_stack_push_s(s,"\r\n");
		}
	}
	if(cfg->use_custom_hdr && rep->custom_hdr_key && rep->custom_hdr_value)
	{
		wtk_stack_push(s,rep->custom_hdr_key->data,rep->custom_hdr_key->len);
		wtk_stack_push_s(s,": ");
		wtk_stack_push(s,rep->custom_hdr_value->data,rep->custom_hdr_value->len);
		wtk_stack_push_s(s,"\r\n");
	}
#ifdef WIN32
#else
#ifdef USE_STATICS
    if(c->net->cpu)
    {
		//wtk_stack_push_s(s,"Cpu: ");
		wtk_stack_push_s(s,"Statics: ");
		if(c->net->cfg->use_cfile_cpu)
		{
			wtk_loc_attach_statics2(s,c->net);
		}else
		{
			wtk_loc_attach_statics(s,c->net);
		}
		//wtk_cpu_to_string2(c->net->cpu,s);
		wtk_stack_push_s(s,"\r\n");
    }
#endif
#endif
	/*
	wtk_stack_push_s(s,"Host: ");
	wtk_stack_push(s,req->c->a);
	wtk_stack_push_s(s,"\r\n");
	*/
    if(cfg->use_connection)
    {
		if(req->keep_alive)
		{
            if (!req->http1_1 || cfg->use_connection_keep_alive_http1_1)
            {
                wtk_stack_push_s(s,"Connection: Keep-Alive\r\n");
            }
		}else
		{
			wtk_stack_push_s(s,"Connection: close\r\n");
		}
    }
	if(cfg->use_lua && rep->lua_hdr)
	{
		wtk_stack_push_s(s,"Lua: ");
		wtk_stack_push(s,rep->lua_hdr->data,rep->lua_hdr->len);
		wtk_stack_push_s(s,"\r\n");
	}
	if(cfg->use_stream_message && rep->hint_result)
	{
		result=rep->hint_result;
		wtk_stack_push_s(s,"Stream-Message: ");
		wtk_stack_push(s,result->value.str.data,result->value.str.len);
		wtk_stack_push_s(s,"\r\n");
	}
	result=rep->result;
    if(cfg->use_stream_mode && req->speech)
    {
	    if(rep->eof || result)
	    {
		    wtk_stack_push_s(s,"Stream-Mode: EOF\r\n");
	    }else
	    {
		    wtk_stack_push_s(s,"Stream-Mode: LIVE\r\n");
	    }
    }
	found=0;
	is_str=1;
	if(result)
	{
		if(result->type==WTK_STRING)
		{
			p=result->value.str.data;
			len=result->value.str.len;
			if(len<0){len=0;}
			found=1;
		}
		else if(result->type==WTK_BIN)
		{
			p=(char*)result->value.bin.data;
			len=result->value.bin.len;
			is_str=0;
			found=1;
		}
		else if(result->type==WTK_OCT)
		{
			p=(char*)result->value.bin.data;
			len=result->value.bin.len;
			is_str=2;
			found=1;
		}
		if(!found)
		{
			p=0;len=0;
		}
	}else if(rep->body.len>0)
	{
		p=rep->body.data;
		len=rep->body.len;
	}
	if(cfg->use_content_type && rep->content_type)
	{
        if (!strncmp("text/plain", rep->content_type->data, rep->content_type->len)
                || cfg->use_content_type_text_plain)
        {
            wtk_stack_push_s(s,"Content-Type: ");
            wtk_stack_push(s,rep->content_type->data,rep->content_type->len);
            wtk_stack_push_s(s,"\r\n");
        }
	}else if(cfg->use_content_type)
	{
		if(is_str == 1)
		{
            if (cfg->use_content_type_text_plain)
            {
                wtk_stack_push_s(s,"Content-Type:text/plain\r\n");
            }
		}
		else if(is_str == 2)
		{
			wtk_stack_push_s(s,"Content-Type: application/octet-stream\r\n");
		}
		else if(is_str == 0)
		{
#ifdef WIN32
            wtk_stack_push_s(s,"Content-Type: audio/x-ms-wax\r\n");
#else
			wtk_stack_push_s(s,"Content-Type: audio/mpeg\r\n");
#endif
		}
	}

    n=sprintf(buf,"Content-Length:%d\r\n\r\n",len>0?len:0);
    wtk_stack_push(s,buf,n);
    wtk_stack_push(s,p,len);

	//if(req->global_streamid){wtk_log_log(c->net->log,"[send: %*.*s]",req->global_streamid->pos,req->global_streamid->pos,req->global_streamid->data);}
	return  wtk_connection_write_stack(c,s);
}

int wtk_response_set_result(wtk_response_t *rep,wtk_param_t* result)
{
	if(rep->result)
	{
		wtk_param_delete(rep->result);
	}
	rep->result=result;
	return 0;
}

int wtk_response_set_hint_result(wtk_response_t *rep,wtk_param_t* result)
{
	if(rep->hint_result)
	{
		wtk_param_delete(rep->hint_result);
	}
	rep->hint_result=result;
	return 0;
}

int wtk_response_set_hint_result2(wtk_response_t *rep,char *s,int len)
{
	wtk_param_t *p;

	p=wtk_param_new_str(s,len);
	return wtk_response_set_hint_result(rep,p);
}

int wtk_response_set_lua_hdr(wtk_response_t *rep,wtk_string_t *lua)
{
	if(rep->lua_hdr)
	{
		wtk_string_delete(rep->lua_hdr);
	}
	rep->lua_hdr=lua;
	return 0;
}

int wtk_response_set_stream_status(wtk_response_t *rep,int id,const char *info,int len)
{
	rep->stream_status_id=id;
	wtk_string_set(&(rep->stream_status),(char*)info,len);
	return 0;
}

void wtk_response_set_body(wtk_response_t *rep,char *data,int len)
{
	rep->body.data=data;
	rep->body.len=len;
}



