#include "wtk_request.h"
#include "wtk/http/nk/wtk_connection.h"
#include "ctype.h"
#include "wtk_response.h"

static wtk_request_str_to_content_type_f str_to_content=0;

void wtk_request_set_str_to_content_type_f_g(wtk_request_str_to_content_type_f f)
{
	str_to_content=f;
}

wtk_request_t* wtk_request_new(wtk_request_cfg_t *cfg)
{
	wtk_request_t* r;

	r=(wtk_request_t*)wtk_calloc(1,sizeof(*r));
	r->cfg=cfg;
	r->heap=wtk_heap_new(cfg->heap_size);
	r->tmp_buf=wtk_strbuf_new(cfg->buf_size,cfg->buf_rate);
	r->body=0;
	r->response=wtk_response_new();
	r->nk_to_vm_event.data=r;
	r->nk_to_vm_event.type=WTK_NK_REQUEST;
	if(cfg->use_times)
	{
		r->times=wtk_strbuf_new(256,1);
	}else
	{
		r->times=0;
	}
	return r;
}


int wtk_request_bytes(wtk_request_t *r)
{
	int b=sizeof(wtk_request_t);

	b+=wtk_strbuf_bytes(r->tmp_buf);
	if(r->body)
	{
		b+=wtk_strbuf_bytes(r->body);
	}
	//b+=wtk_strbuf_bytes(r->global_streamid);
	if(r->times)
	{
		b+=wtk_strbuf_bytes(r->times);
	}
	b+=wtk_heap_bytes(r->heap);
	b+=wtk_response_bytes(r->response);
	return b;
}

int wtk_request_delete(wtk_request_t *r)
{
	//wtk_debug("request delete %p\n",r);
	if(r->times)
	{
		wtk_strbuf_delete(r->times);
	}
	if(r->tmp_buf)
	{
		wtk_strbuf_delete(r->tmp_buf);
	}
	if(r->body)
	{
		wtk_strbuf_delete(r->body);
	}
	wtk_response_delete(r->response);
	//wtk_strbuf_delete(r->global_streamid);
	wtk_heap_delete(r->heap);
	wtk_free(r);
	return 0;
}

int wtk_request_update(wtk_request_t *r)
{
	//memset is the best way, but is not safe for develop.
	wtk_string_t* strs[]={
			&(r->url),
			&(r->params),
			&(r->key),
			&(r->err),
	};
	int i;

	r->state=REQUEST_START;
	r->method=HTTP_UNKNOWN;
	r->audio_type=AUDIO_UNKNOWN;
	r->is_stream=0;
	r->stream_eof=0;
	for(i=0;i<sizeof(strs)/sizeof(wtk_string_t*);++i)
	{
		wtk_string_set(strs[i],0,0);
	}
	r->global_streamid=0;
	r->local_streamid=0;
	r->strhook=0;
	r->content_length=0;

	r->content_type=0;
    r->user_agent  = 0;
	r->audio_tag=0;
	r->script=0;
	r->log=0;

	r->keep_alive=0;
	r->http1_1=0;
	r->client=NULL;
	return 0;
}

int wtk_request_init(wtk_request_t *r,struct wtk_connection* c)
{
	r->c=c;
	r->params.len=0;
	wtk_request_update(r);
	return 0;
}

int wtk_request_reset(wtk_request_t *r)
{
	r->data=0;
	if(r->times)
	{
		wtk_strbuf_reset(r->times);
	}
	wtk_response_reset(r->response);
	wtk_heap_reset(r->heap);
	wtk_strbuf_reset(r->tmp_buf);
	if(r->body)
	{
		wtk_strbuf_reset(r->body);
	}
	wtk_request_update(r);
	return 0;
}

int wtk_request_update_content_length(wtk_request_t *r,char *v,int v_len)
{
	r->content_length=wtk_str_atoi(v,v_len);
    if(r->content_length>0)
    {
    	if(r->body)
    	{
    		if(r->content_length > r->body->length)
    		{
    			wtk_strbuf_delete(r->body);
    			r->body=0;
    		}
    	}
    	if(!r->body)
    	{
    		r->body=wtk_strbuf_new(wtk_round(r->content_length,16),r->cfg->body_buf_rate);
    	}else
    	{
    		wtk_strbuf_reset(r->body);
    	}
    }
    return 0;
}

void wtk_request_set_content_type_err(wtk_request_t *r)
{
	wtk_strbuf_t *buf;

	buf=r->c->net->tmp_buf;
	wtk_strbuf_reset(buf);
	wtk_strbuf_push_s(buf,"Unknown Content-Type:");
	if(r->content_type)
	{
		wtk_strbuf_push(buf,r->content_type->data,r->content_type->len);
	}
	r->err.data=wtk_heap_dup_data(r->heap,buf->data,buf->pos);
	r->err.len=buf->pos;
	wtk_log_log(r->c->net->log,"Unknown Content-Type: %.*s.",buf->pos,buf->data);
}

int wtk_request_update_user_agent(wtk_request_t *r, char *v, int v_len)
{
    r->user_agent = wtk_heap_dup_string(r->heap, v, v_len);

    return 0;
}

int wtk_request_update_client(wtk_request_t *r,char *v,int v_len)
{
    r->client = wtk_heap_dup_string(r->heap, v, v_len);
    //wtk_debug("[%.*s]\n",v_len,v);
    return 0;
}

int wtk_request_update_content_type(wtk_request_t *r,char *v,int v_len)
{
	wtk_string_t vx;
	int ret=0;

	wtk_string_set(&vx,v,v_len);
	r->content_type=wtk_heap_dup_string(r->heap,v,v_len);
	r->audio_type=wtk_audio_type_from_string(&vx);
	if(r->audio_type==AUDIO_UNKNOWN && str_to_content)
	{
		r->audio_type=str_to_content(v,v_len);
	}
	if(r->audio_type==AUDIO_UNKNOWN  && r->content_length>0)
	{
		wtk_request_set_content_type_err(r);
		ret=-1;
	}
	return ret;
}

int wtk_request_update_connection(wtk_request_t *r,char *v,int v_len)
{
	if(wtk_str_equal_s(v,v_len,"close"))
	{
		r->keep_alive=0;
	}else if(wtk_str_equal_s(v,v_len,"keep-alive"))
	{
		r->keep_alive=1;
	}
	return 0;
}

int wtk_request_update_stream_id(wtk_request_t *r,char *v,int v_len)
{
	r->local_streamid=wtk_heap_dup_string(r->heap,v,v_len);
	return 0;
}

int wtk_request_update_stream_mode(wtk_request_t *r,char *v,int v_len)
{
	r->stream_eof=wtk_str_equal_s(v,v_len,"EOF");
	return 0;
}

int wtk_request_update_script(wtk_request_t *r,char *v,int v_len)
{
	r->script=wtk_heap_dup_string(r->heap,v,v_len);
	return 0;
}

int wtk_request_update_audio_tag(wtk_request_t *r,char *v,int v_len)
{
	r->audio_tag=wtk_heap_dup_string(r->heap,v,v_len);
	return 0;
}

int wtk_request_update_log(wtk_request_t *r,char *v,int v_len)
{
	r->log=wtk_heap_dup_string(r->heap,v,v_len);
	return 0;
}


int wtk_request_update_hook(wtk_request_t *r,char *v,int v_len)
{
	r->strhook=wtk_heap_dup_string(r->heap,v,v_len);
	return 0;
}

int wtk_request_update_hdr(wtk_request_t *r,wtk_connection_t* c)
{
	wtk_strbuf_t *buf;
	int ret;

	ret=-1;
	buf=r->tmp_buf;
	wtk_strbuf_reset(buf);
	if(c)
	{
		wtk_strbuf_push(buf,c->addr_text.data,c->addr_text.len);
	}
	r->is_stream=r->local_streamid?1:0;
	if(!r->is_stream)
	{
		r->stream_eof=1;
	}
	r->speech=1;
	if(r->local_streamid)
	{
		wtk_strbuf_push_s(buf,":");
		wtk_strbuf_push(buf,r->local_streamid->data,r->local_streamid->len);
	}else
	{
		if(!r->script)
		{
			if(!wtk_audio_type_is_audio(r->audio_type))
			{
				r->speech=0;
			}
		}
	}
	r->global_streamid=wtk_heap_dup_string(r->heap,buf->data,buf->pos);
	if(r->audio_type==AUDIO_UNKNOWN  && r->content_length>0)
	{
		wtk_request_set_content_type_err(r);
		ret=-1;
		goto end;
	}
	ret=0;
end:
	return ret;
}

void wtk_request_set_string(wtk_request_t *r,wtk_string_t *str,char *data,int bytes)
{
	str->data=wtk_heap_dup_data(r->heap,data,bytes);
	str->len=bytes;
}

int wtk_request_filter_hdr(wtk_request_t *r,char *key,int k_len,char *value,int v_len)
{
	wtk_request_hdr_parse_f hp;
	int ret;

	hp=wtk_request_cfg_find_hdr_parse(r->cfg,key,k_len);
	//wtk_debug("[%.*s]=[%.*s](hp=%p)\n",k_len,key,v_len,value,hp);
	if(hp)
	{
		ret=hp(r,value,v_len);
	}else
	{
		ret=0;
	}
	return ret;
}


/**
 * return 0 for continue, -1 failed, 1 hdr end.
 */
int wtk_request_feed_hdr(wtk_request_t *r,char *data,int len,int *consumed)
{
	wtk_strbuf_t *buf=r->tmp_buf;
	char ch;
	char *s,*e;
	int ret=0;

	s=data;e=s+len;
	while(s<e)
	{
		ch=*s;
		switch(r->state)
		{
		case REQUEST_START:
			if ((ch < 'A' || ch > 'Z') && ch != '_')
			{
				//print_data(data,len);
				wtk_string_set_s(&(r->err),"request start with unexpected char");
				ret=-1;
				goto end;
			}
			r->state = REQUEST_METHOD;
			wtk_strbuf_reset(buf);
			wtk_strbuf_push_c(buf,ch);
			break;
		case REQUEST_METHOD:
			if(ch == ' ')
			{
				//print_data(r->strmethod.data,r->strmethod.len);
				if((buf->pos==4) && ngx_str4_cmp(buf->data,'P','O','S','T'))
				{
					r->method=HTTP_POST;
				}else if((buf->pos==3) && ngx_str3_cmp(buf->data,'G','E','T'))
				{
					r->method=HTTP_GET;
				}else
				{
					wtk_string_set_s(&(r->err),"only post,get method is support now");
					ret=-1;
					goto end;
				}
				r->state=REQUEST_URL_SPACE;
			}else
			{
				wtk_strbuf_push_c(buf,ch);
			}
			break;
		case REQUEST_URL_SPACE:
			if(ch!=' ')
			{
				wtk_strbuf_reset(buf);
				wtk_strbuf_push_c(buf,ch);
				r->state=REQUEST_URL;
			}
			break;
		case REQUEST_URL:
			if(ch==' ' || ch=='?')
			{
				wtk_request_set_string(r,&(r->url),buf->data,buf->pos);
				if(ch==' ')
				{
					r->state=REQUEST_WAIT_VERSION;
				}else
				{
					wtk_strbuf_reset(buf);
					r->state=REQUEST_URL_PARAM;
				}
			}else
			{
				wtk_strbuf_push_c(buf,ch);
			}
			break;
		case REQUEST_URL_PARAM:
			if(ch==' ')
			{
				wtk_request_set_string(r,&(r->params),buf->data,buf->pos);
				r->state=REQUEST_WAIT_VERSION;
			}else
			{
				wtk_strbuf_push_c(buf,ch);
			}
			break;
		case REQUEST_WAIT_VERSION:
			if(ch!=' ')
			{
				wtk_strbuf_reset(buf);
				wtk_strbuf_push_c(buf,ch);
				r->state=REQUEST_VERSION;
			}
			break;
		case REQUEST_VERSION:
			if(ch=='\r')
			{
				r->http1_1=wtk_str_equal_s(buf->data,buf->pos,"HTTP/1.1");
				if(r->http1_1)
				{
					r->keep_alive=1;
				}
				r->state=REQUEST_CR;
			}else
			{
				wtk_strbuf_push_c(buf,ch);
			}
			break;
		case REQUEST_CR:
			if(ch=='\n')
			{
				r->state=REQUEST_CL;
			}else
			{
				wtk_string_set_s(&(r->err),"except '\n'");
				return -1;
			}
			break;
		case REQUEST_CL:
			if(ch=='\r')
			{
				r->state=REQUEST_ALMOST_DONE;
			}else
			{
				wtk_strbuf_reset(buf);
				wtk_strbuf_push_c(buf,tolower(ch));
				r->state=REQUEST_KEY;
			}
			break;
		case REQUEST_KEY:
			if(ch==':')
			{
				wtk_request_set_string(r,&(r->key),buf->data,buf->pos);
				r->state=REQUEST_KEY_SPACE;
			}
			else
			{
				wtk_strbuf_push_c(buf,tolower(ch));
			}
			break;
		case REQUEST_KEY_SPACE:
			if(ch!=' ')
			{
				wtk_strbuf_reset(buf);
				wtk_strbuf_push_c(buf,ch);
				r->state=REQUEST_VALUE;
			}
			break;
		case REQUEST_VALUE:
			if(ch == '\r')
			{
				//wtk_debug("%*.*s: %*.*s\n",r->key.len,r->key.len,r->key.data,buf->pos,buf->pos,buf->data);
				ret=wtk_request_filter_hdr(r,r->key.data,r->key.len,buf->data,buf->pos);
				if(ret!=0){goto end;}
				r->state=REQUEST_CR;
			}else
			{
				wtk_strbuf_push_c(buf,ch);
			}
			break;
		case REQUEST_ALMOST_DONE:
			if(ch=='\n')
			{
				r->state=REQUEST_HDR_DONE;
				++s;
				ret=1;
				goto end;
			}
			break;
		default:
			wtk_debug("ah:never be here.\n");
			ret=-1;
			goto end;
		}
		++s;
	}
end:
	if(ret!=-1)
	{
		*consumed=s-data;
	}else
	{
		*consumed=0;
	}
	return ret;
}

void wtk_request_update_body(wtk_request_t *r)
{
	//wtk_log_log(r->c->net->log,"%.*s\n",r->c->addr_text.len,r->c->addr_text.data);
    if(r->method==HTTP_POST && r->params.len<=0 && r->body)
    {
    	r->params.data=r->body->data;
    	r->params.len=r->body->pos;
    }
}

int wtk_request_process_body(wtk_request_t *r,char* buf,int len,int *left)
{
	int buf_left,cpy;
	wtk_strbuf_t *body;

	//print_data(buf,10);
	body=r->body;
    buf_left=r->content_length-body->pos;
    cpy=min(len,buf_left);
    wtk_strbuf_push(body,buf,cpy);
    if( body->pos >= r->content_length)
    {
        r->state = REQUEST_CONTENT_DONE;
        wtk_request_update_body(r);
    }
    *left=len-cpy;
    return 0;
}

int wtk_request_process_hdr(wtk_request_t *r,char* buf,int len,int *left)
{
	int ret=0;
	char *p;
	int consumed;

	//print_data(buf,len);
	ret=wtk_request_feed_hdr(r,buf,len,&consumed);
	*left=len-consumed;
	if(ret!=1){goto end;}
	p=buf+consumed;
	ret=wtk_request_update_hdr(r,r->c);
	if(ret!=0){goto end;}
	if(r->body){wtk_strbuf_reset(r->body);}
	if(r->content_length==0)
	{
		r->state=REQUEST_CONTENT_DONE;
		goto end;
	}
	if(*left>0)
	{
		ret=wtk_request_process_body(r,p,*left,left);
	}else
	{
		ret=0;
	}
end:
	return ret;
}

#define wtk_strbuf_push_http_hdr_s(buf,k,v,v_len) wtk_strbuf_push_http_hdr(buf,k,sizeof(k)-1,v,v_len)

void wtk_strbuf_push_http_hdr(wtk_strbuf_t *buf,char *k,int k_len,char *v,int v_len)
{
	wtk_strbuf_push(buf,k,k_len);
	wtk_strbuf_push_s(buf,": ");
	wtk_strbuf_push(buf,v,v_len);
	wtk_strbuf_push_s(buf,"\r\n");
}

void wtk_request_tobin(wtk_request_t *r,wtk_strbuf_t *buf,wtk_string_t *host,int hook)
{
	int n;
	char tmp[256];

	wtk_strbuf_reset(buf);
	if(r->method==HTTP_GET)
	{
		wtk_strbuf_push_s(buf,"GET ");
	}else
	{
		wtk_strbuf_push_s(buf,"POST ");
	}
	wtk_strbuf_push(buf,r->url.data,r->url.len);
	if(r->params.len>0)
	{
		wtk_strbuf_push_s(buf,"?");
		wtk_strbuf_push(buf,r->params.data,r->params.len);
	}
	wtk_strbuf_push_s(buf," HTTP/1.1\r\n");
	if(host)
	{
		wtk_strbuf_push_s(buf,"Host: ");
		wtk_strbuf_push(buf,host->data,host->len);
		wtk_strbuf_push_s(buf,"\r\n");
	}
	wtk_strbuf_push_s(buf,"Connection: keep-alive\r\nStream-Id: ");
	wtk_strbuf_push(buf,r->global_streamid->data,r->global_streamid->len);
	wtk_strbuf_push_s(buf,"\r\nContent-Length: ");
	n=sprintf(tmp,"%d\r\n",r->body?r->body->pos:0);
	wtk_strbuf_push(buf,tmp,n);
	n=sprintf(tmp,"Hook: %d\r\n",hook);
	wtk_strbuf_push(buf,tmp,n);
	if(r->log)
	{
		wtk_strbuf_push_http_hdr_s(buf,"Log",r->log->data,r->log->len);
	}
	if(r->script)
	{
		wtk_strbuf_push_http_hdr_s(buf,"Script",r->script->data,r->script->len);
	}
	if(r->audio_tag)
	{
		wtk_strbuf_push_http_hdr_s(buf,"Audio-Tag",r->audio_tag->data,r->audio_tag->len);
	}
	if(r->content_type)
	{
		wtk_strbuf_push_http_hdr_s(buf,"Content-Type",r->content_type->data,r->content_type->len);
	}
	//if(r->stream_eof ||  !r->local_streamid)
	if(r->stream_eof)
	{
		wtk_strbuf_push_s(buf,"Stream-Mode: EOF\r\n");
	}
	wtk_strbuf_push_s(buf,"\r\n");
}

void wtk_request_tobin2(wtk_request_t *r,wtk_strbuf_t *buf,wtk_string_t *host,int hook)
{
	wtk_request_tobin(r,buf,host,hook);
	if(r->body)
	{
		wtk_strbuf_push(buf,r->body->data,r->body->pos);
	}
}

int wtk_request_reply(wtk_request_t *r)
{
	return wtk_response_write(r->response,r->c,r);
}

void wtk_request_print(wtk_request_t *r)
{
	printf("################ request ###############\n");
	//print_data(r->hdr->data,r->hdr->pos);
	printf("url: %.*s\n",r->url.len,r->url.data);
	printf("param: %.*s\n",r->params.len,r->params.data);
	printf("keep-alive:\t%d\n",r->keep_alive);
	printf("content-length:\t%ld\n",r->content_length);
	printf("audio-type:\t%d\n",r->audio_type);
	printf("global-stream:\t%*.*s\n",r->global_streamid->len,r->global_streamid->len,r->global_streamid->data);
	if(r->local_streamid)
	{
		printf("local-stream:\t%*.*s\n",r->local_streamid->len,r->local_streamid->len,r->local_streamid->data);
	}
	printf("stream eof:\t%d\n",r->stream_eof);
	if(r->strhook)
	{
		printf("hook:\t%.*s\n",r->strhook->len,r->strhook->data);
	}
	if(r->body)
	{
		if(r->audio_type==AUDIO_TEXT)
		{
			printf("%.*s\n",r->body->pos,r->body->data);
		}
		//print_data(r->body->data,r->body->pos);
	}
	fflush(stdout);
}

void wtk_request_print_hash_node(wtk_request_t *r,hash_str_node_t *n)
{
	wtk_string_t *v=(wtk_string_t*)n->value;

	printf("%*.*s: %*.*s\n",n->key.len,n->key.len,n->key.data,v->len,v->len,v->data);
}

int wtk_request_feed_back(wtk_request_t *r,int status,char *b,int bytes)
{
	return wtk_response_feedback(r->response,status,r->c,r,b,bytes);
}

void wtk_request_touch(wtk_request_t *r,char *s,int bytes)
{
	wtk_strbuf_t *buf=r->times;

	if(buf)
	{
		wtk_strbuf_push(buf,s,bytes);
		wtk_strbuf_push_f(buf,"=%.0f;",time_get_ms());
		//wtk_debug("%.*s\n",buf->pos,buf->data);
	}
}

void wtk_request_detach_body(wtk_request_t *r)
{
	if(r->body)
	{
		wtk_strbuf_delete(r->body);
		r->body=0;
	}
}

void wtk_request_set_response_body(wtk_request_t *r,const char *data,int bytes)
{
	char *p;

	p=wtk_heap_dup_data(r->heap,data,bytes);
	wtk_response_set_body(r->response,p,bytes);
}

void wtk_request_set_response_audio_url(wtk_request_t *r,const char *audio_url,int bytes)
{
	r->response->audio_url=wtk_heap_dup_string(r->heap,(char*)audio_url,bytes);
}

void wtk_request_set_response_hdr2(wtk_request_t *r,wtk_string_t *key,char *v,int v_len)
{
	wtk_response_t *response=r->response;
	wtk_response_hdr_t *hdr;
	wtk_heap_t *heap;

	heap=r->heap;
	if(!response->custom_hdr_array)
	{
		response->custom_hdr_array=wtk_array_new_h(heap,2,sizeof(wtk_response_hdr_t));
	}
	hdr=(wtk_response_hdr_t*)wtk_array_push(response->custom_hdr_array);
	hdr->k=key;
	hdr->v=wtk_heap_dup_string(heap,v,v_len);
	//wtk_response_print_hdr_array(response);
}

void wtk_request_set_response_hdr(wtk_request_t *r,char *k,int k_len,char *v,int v_len)
{
	wtk_string_t *key;

	key=(wtk_string_t*)wtk_heap_dup_string(r->heap,k,k_len);
	wtk_request_set_response_hdr2(r,key,v,v_len);
}

void wtk_request_update_response_from_http_response(wtk_request_t *req,wtk_http_response_t *response)
{
	wtk_array_t *a=req->cfg->redirect_custom_hdr;
	wtk_request_fromto_t **strs;
	wtk_string_t *v;
	int i;

	if(!a){return;}
	strs=(wtk_request_fromto_t**)a->slot;
	for(i=0;i<a->nslot;++i)
	{
		v=(wtk_string_t*)wtk_str_hash_find(response->hash,strs[i]->from->data,strs[i]->from->len);
		//wtk_debug("%.*s=%p\n",strs[i]->from->len,strs[i]->from->data,v);
		if(v)
		{
			wtk_request_set_response_hdr2(req,strs[i]->to,v->data,v->len);
		}
	}
}

int wtk_request_is_speech(wtk_request_t *request)
{
	if((request->url.len==1) && (request->speech||request->params.len>0))
	{
		return 1;
	}else
	{
		return 0;
	}
}

/*
void wtk_request_dup_result(wtk_request_t *r,wtk_param_t *src)
{
	wtk_heap_t *heap=r->hash->heap;
	wtk_param_t *dst;

	if(src)
	{
		dst=wtk_param_dup(src,heap);
	}else
	{
		dst=0;
	}
	wtk_response_set_result(r->response,dst);
}
*/

void wtk_request_update_response(wtk_request_t *request,wtk_http_response_t *response)
{
	wtk_response_t *rep=request->response;
	wtk_str_hash_t *hash=response->hash;
	wtk_heap_t *heap;
	wtk_string_t *v;
	wtk_param_t *p;

	heap=request->heap;
	request->response->status=response->status;
	wtk_request_update_response_from_http_response(request,response);
	v=(wtk_string_t*)wtk_str_hash_find_s(hash,"lua");
	if(v)
	{
		v=wtk_string_dup_data(v->data,v->len);
		wtk_response_set_lua_hdr(rep,v);
	}
	v=(wtk_string_t*)wtk_str_hash_find_s(hash,"status-info");
	if(v)
	{
		rep->stream_status.data=wtk_heap_dup_data(heap,v->data,v->len);
		rep->stream_status.len=v->len;
	}
	v=(wtk_string_t*)wtk_str_hash_find_s(hash,"status-id");
	if(v)
	{
		rep->stream_status_id=wtk_str_atoi(v->data,v->len);
	}
	if(request->times)
	{
		v=(wtk_string_t*)wtk_str_hash_find_s(hash,"times");
		if(v)
		{
			wtk_strbuf_push(request->times,v->data,v->len);
		}
	}
	wtk_request_touch_s(request,"upserver_recv");
	v=(wtk_string_t*)wtk_str_hash_find_s(hash,"stream-length");
	if(v)
	{
		rep->stream_data_received=wtk_str_atoi(v->data,v->len);
	}else
	{
		rep->stream_data_received=0;
	}
	//print_data(rep->host.data,rep->host.len);
	if(response->body->pos>0)
	{
		char *data;

		v=(wtk_string_t*)wtk_str_hash_find_s(hash,"content-type");
		p=(wtk_param_t*)wtk_heap_malloc(heap,sizeof(wtk_param_t));
		p->is_ref=1;
		data=wtk_heap_dup_data(heap,response->body->data,response->body->pos);
		if(v && (wtk_string_cmp_s(v,"audio/mpeg")==0))
		{
			p->type=WTK_BIN;
			p->value.bin.data=data;
			p->value.bin.len=response->body->pos;
		}else
		{
			p->type=WTK_STRING;
			p->value.str.data=data;
			p->value.str.len=response->body->pos;
		}
		wtk_response_set_result(rep,p);
	}
	v=(wtk_string_t*)wtk_str_hash_find_s(hash,"stream-mode");
	if(v && (wtk_string_cmp_s(v,"EOF")==0))
	{
		request->stream_eof=1;
		request->response->eof=1;
	}
}

void wtk_request_set_disconnect_err(wtk_request_t *req)
{
	static wtk_string_t txt=wtk_string("text/plain");

	req->response->status=500;
	req->response->content_type=&txt;
	wtk_response_set_body_s(req->response,"upstream disconnected.");
}

int wtk_request_redirect(wtk_request_t *req,int fd,wtk_strbuf_t *buf)
{
	int writed;
	int ret;

	wtk_strbuf_reset(buf);
	wtk_request_tobin2(req,buf,0,0);
	ret=wtk_fd_send(fd,buf->data,buf->pos,&writed);
	if(ret==0 && (writed != buf->pos))
	{
		ret=-1;
	}
	return ret;
}
