#include "wtk_http_parser.h"
#include "wtk/http/wtk_http.h"
#define wtk_http_parser_feedback_s(p,req,status,info) wtk_http_parser_feedback(p,req,status,info,sizeof(info)-1)

wtk_http_parser_t* wtk_http_parser_new(struct wtk_http *http)
{
	wtk_http_parser_t* p;

	p=(wtk_http_parser_t*)wtk_calloc(1,sizeof(*p));
	p->layer=http;
	p->close=0;
	p->shot=0;
	p->close_notify=(wtk_parser_close_notify_handler)wtk_http_parser_notify_close;
	p->nk_to_vm_event.data=p;
	p->nk_to_vm_event.type=WTK_NK_CON_CLOSE;
	p->nk_to_vm_event.used=0;
	return p;
}

int wtk_http_parser_delete(wtk_http_parser_t *p)
{
	wtk_http_parser_reset(p);
	wtk_free(p);
	return 0;
}

int wtk_http_parser_init(wtk_http_parser_t *p,wtk_connection_t* c)
{
	p->handler=(wtk_parse_handler)wtk_http_parser_feed_hdr;
	p->con=c;
	p->request=0;
	p->nk_to_vm_event.used=0;
	wtk_queue_init(&(p->stream_queue));
	return 0;
}

int wtk_http_parser_bytes(wtk_http_parser_t *p)
{
	return sizeof(*p);
}

int wtk_http_parser_reset(wtk_http_parser_t *p)
{
	wtk_http_t *http;

	http=(wtk_http_t*)p->layer;
	if(p->request)
	{
		wtk_http_push_request(http,p->request);
		p->request=0;
	}
	return 0;
}


void wtk_http_parser_update_connection_delay(wtk_http_parser_t* p,wtk_connection_t* c)
{
	wtk_http_t *http;
	int delay;

	http=(wtk_http_t*)p->layer;
	if(!http->cfg->use_delay_hint){return;}
	if(p->nk_to_vm_event.used==0 && c->valid)
	{
		//wtk_debug("ac=%d,max=%d\n",c->active_count,http->cfg->max_streams_per_connection)
		delay=p->stream_queue.length>=http->cfg->max_streams_per_connection && (c->active_count>=http->cfg->max_streams_per_connection);
	    //wtk_connection_set_delay(c,c->active_count>=http->cfg->max_streams_per_connection);
	    wtk_connection_set_delay(c,delay);
	}
}

int wtk_http_parser_feedback(wtk_http_parser_t *p,wtk_request_t *req,int status,const char *info,int info_len)
{
    //send feedback direct to client, for plain error.
	wtk_response_t* rep;
	wtk_param_t q;

	q.type=WTK_STRING;
	q.is_ref=1;
	q.value.str.data=(char*)info;
	q.value.str.len=info_len;
	q.value.str.is_ref=1;
	rep=req->response;
	rep->status=status;
	wtk_response_set_result(rep,&q);
	//wtk_response_set_stream_status(rep,info,info_len);
	wtk_response_write(rep,req->c,req);
	rep->result=0;
	rep->hint_result=0;
	return 0;
}

int wtk_http_parser_feed_content(wtk_http_parser_t *p,wtk_connection_t *c,char *buf,int len)
{
	int ret,left;

	ret=wtk_request_process_body(p->request,buf,len,&left);
	if(ret==0)
	{
		wtk_http_parser_try_raise(p,c);
        if(left>0)
        {
            ret=p->handler(p,c,buf+len-left,left);
        }
	}
	if(ret!=0)
	{
		wtk_log_log(c->net->log,"%s content bad request.",c->name);
	}
	return ret;
}

int wtk_http_parser_feed_hdr(wtk_http_parser_t *p,wtk_connection_t *c,char *buf,int len)
{
	int ret,left;
	wtk_http_t *http;

	//print_data(buf,len);
	http=(wtk_http_t*)p->layer;
	if(!p->request)
	{
		p->request=wtk_http_pop_request(http,c);
	}
	left=0;
	ret=wtk_request_process_hdr(p->request,buf,len,&left);
	if(ret==0)
	{
		if(p->request->state==REQUEST_HDR_DONE)
		{
			p->handler=(wtk_parse_handler)wtk_http_parser_feed_content;
		}else if(p->request->state==REQUEST_CONTENT_DONE)
		{
			wtk_http_parser_try_raise(p,c);
		}
        if(left>0)
        {
             ret=p->handler(p,c,buf+len-left,left);
        }
	}else
	{
		#define POLICY "<policy-file-request/>"
		if((len==sizeof(POLICY))&& strncmp(buf,POLICY,sizeof(POLICY))==0)
		{
			wtk_connection_write_s(c,"<?xml version=\"1.0\"?><cross-domain-policy><site-control permitted-cross-domain-policies=\"all\"/><allow-access-from domain=\"*\" to-ports=\"*\"/></cross-domain-policy>\0");
			return -1;
		}
	}
	if(ret!=0)
	{
		wtk_log_log(http->net->log,"%s bad hdr request(client=%s,data=%d).",c->name,c->name,len);
	}
	if(ret!=0 && p->request)
	{
		wtk_string_t *err;

		err=&(p->request->err);
		wtk_http_parser_feedback(p,p->request,HTTP_BAD_REQUEST,err->data,err->len);
		wtk_request_reset(p->request);
	}
	return ret;
}

void wtk_http_parser_try_raise(wtk_http_parser_t *p,wtk_connection_t *c)
{
	wtk_request_t *request;
	int ret;
	wtk_http_t *http;

	http=(wtk_http_t*)p->layer;
	request=p->request;
	if(request && request->state == REQUEST_CONTENT_DONE)
	{
		//wtk_request_print(p->request);
		//if(request->global_streamid){wtk_log_log(c->net->log,"[recv: %*.*s]",request->global_streamid->pos,request->global_streamid->pos,request->global_streamid->data);}
		c->keep_alive=request->keep_alive;
		if(!request->keep_alive)
		{
			c->want_close=1;
		}
		//print_data(request->url.data,request->url.len);
		//if((request->url.len==1) && (request->speech||request->params.len>0))
		if(wtk_http_request_is_raise_able(http,request))
		{
			ret=wtk_http_parser_raise_request(p,request);
			if(ret!=0)
			{
				ret=wtk_http_parser_feedback_s(p,request,HTTP_BAD_REQUEST,"raise request failed.\n");
			}
		}else
		{
            //wtk_log_log(http->net->log,"req %*.*s.",request->url.len,request->url.len,request->url.data);
			ret=wtk_http_do_location(http,p->request);
			if(p->request)
			{
				if(ret!=0)
				{
					ret=wtk_http_parser_feedback_s(p,p->request,HTTP_BAD_REQUEST,"you are lost.:)\n");
				}else
				{
					ret=wtk_request_reply(p->request);
				}
			}
		}
        //reset handler.
        if(p->request)
        {
            wtk_request_reset(p->request);
        }
		p->handler=(wtk_parse_handler)wtk_http_parser_feed_hdr;
	}
}

int wtk_http_parser_done_request(wtk_http_parser_t *p,wtk_request_t *req)
{
	wtk_connection_t* c;
	wtk_http_t *http;

	wtk_request_touch_s(req,"http_send");
	wtk_response_write(req->response,req->c,req);
	http=(wtk_http_t*)p->layer;
	c=req->c;
	--c->active_count;
	if(http->cfg->log_req_route)
	{
		wtk_log_log(http->net->log,"req=%.*s: %p, active=%d,want_close=%d.",req->global_streamid->len,req->global_streamid->data,req,c->active_count,c->want_close);
	}
	wtk_http_push_request(http,req);
	//wtk_log_log(c->net->log,"want_close=%d,keep_alive=%d,active_count=%d.",c->want_close,c->keep_alive,c->active_count);
	//wtk_debug("want_close=%d,keep_alive=%d,active_count=%d,inqueue=%d.\n",c->want_close,c->keep_alive,c->active_count,c->event->in_queue);
    //if((c->want_close && !c->keep_alive && c->active_count<=0)||(c->want_close && c->active_count<=0 &&(!c->event->in_queue)))
	if(c->want_close && c->active_count<=0)
    {
    	//wtk_debug("try close ...\n");
        //connection is not valid anymore.
        wtk_connection_try_close(c);
    }else
    {
    	wtk_http_parser_update_connection_delay(p,c);
    }
	return 0;
}

int wtk_http_parser_done_unlink(wtk_http_parser_t *p)
{
	//wtk_debug("unlink parser (%p),ac=%d,ql=%d...\n",p,p->con->active_count,p->stream_queue.length);
	return wtk_connection_try_close(p->con);
}

void wtk_http_parser_hook_connection(wtk_http_parser_t *p,wtk_request_t *req)
{
	wtk_connection_t *c;

	c=req->c;
	c->busy=1;
    //this may be not safe,but in real, the active stream is not more than thousands in single connection.
	++c->active_count;
	//wtk_request_print(s->last_request);
}

void wtk_http_parser_hook_request(wtk_http_parser_t *p,wtk_request_t *req)
{
	wtk_http_parser_hook_connection(p,req);
	if(req==p->request)
	{
		p->request=0;
	}
}

int wtk_http_parser_raise_request(wtk_http_parser_t *p,wtk_request_t *req)
{
	wtk_http_t *http;
	int ret;

	wtk_http_parser_hook_connection(p,req);
	http=(wtk_http_t*)p->layer;
	wtk_request_touch_s(req,"http_recv");
	//wtk_debug("raise request: %p\n",req);
	if(http->cfg->log_req_route)
	{
		if(req->strhook)
		{
			wtk_log_log(http->net->log,"raise req %.*s, hook=%.*s, eof=%d.",req->global_streamid->len,req->global_streamid->data,
					req->strhook->len,req->strhook->data,req->stream_eof);
		}else
		{
			wtk_log_log(http->net->log,"raise req %.*s, hook=nil,eof=%d.",req->global_streamid->len,req->global_streamid->data,req->stream_eof);
		}
	}
	ret=wtk_http_raise_event(http,&(req->nk_to_vm_event));
	if(ret==0)
	{
		p->request=0;
		wtk_http_parser_update_connection_delay(p,req->c);
	}
	return ret;
}


int wtk_http_parser_push_stream(wtk_http_parser_t *p,wtk_queue_node_t *n)
{
	return wtk_queue_push(&(p->stream_queue),n);
}

int wtk_http_parser_remove_stream(wtk_http_parser_t *p,wtk_queue_node_t *n)
{
	return wtk_queue_remove(&(p->stream_queue),n);
}

wtk_queue_node_t* wtk_http_parser_pop_stream(wtk_http_parser_t *p)
{
	return wtk_queue_pop(&(p->stream_queue));
}

int wtk_http_parser_notify_close(wtk_http_parser_t *p)
{
	wtk_connection_t *c=p->con;
	wtk_http_t *http;
	int ret;

	//wtk_debug("notify close parser (%p),ac=%d,stream=%d...\n",p,c->active_count,p->stream_queue.length);
	if(c->active_count>0 || p->stream_queue.length>0)
	{
		if(p->nk_to_vm_event.used==0)
		{
			http=(wtk_http_t*)p->layer;
			//wtk_debug("raise type=%d,%s,nke=%p\n",p->nk_to_vm_event.type,c->name,&(p->nk_to_vm_event));
			p->nk_to_vm_event.used=1;
			wtk_http_raise_event(http,&(p->nk_to_vm_event));
		}
		if(c->active_count>0 || p->stream_queue.length>0)
		{
			ret=1;
		}else
		{
			ret=0;
		}
	}else
	{
		ret=0;
	}
	return ret;
}
