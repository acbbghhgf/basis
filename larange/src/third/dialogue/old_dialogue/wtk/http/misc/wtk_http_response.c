#include <ctype.h>
#include <errno.h>
#include "wtk_http_response.h"
#include "wtk_http_chunk.h"
#include "wtk/os/wtk_fd.h"

wtk_http_response_t* wtk_http_response_new()
{
    return wtk_http_response_new2(1024,13,16384);
}

wtk_http_response_t* wtk_http_response_new2(int buf_size,int hdr_slot,int body_bytes)
{
    wtk_http_response_t *rep;

    rep=(wtk_http_response_t*)wtk_calloc(1,sizeof(*rep));
    rep->tmp_buf=wtk_strbuf_new(buf_size,1);
    rep->hash=wtk_str_hash_new(hdr_slot);
    rep->body=wtk_strbuf_new(body_bytes,1);
    rep->heap=rep->hash->heap;
    rep->unknown_content_length=0;
    rep->chunk=0;
    wtk_http_response_reset(rep);
    return rep;
}

int wtk_http_response_reset(wtk_http_response_t* rep)
{
	rep->is_chunk=0;
	if(rep->chunk)
	{
		wtk_http_chunk_reset(rep->chunk);
	}
    wtk_strbuf_reset(rep->tmp_buf);
    wtk_strbuf_reset(rep->body);
    wtk_str_hash_reset(rep->hash);
    rep->key.len=0;
    rep->status=0;
    rep->content_length=0;
    rep->state=RESPONSE_START;
    return 0;
}

int wtk_http_response_delete(wtk_http_response_t *rep)
{
	if(rep->chunk)
	{
		wtk_http_chunk_delete(rep->chunk);
	}
    wtk_strbuf_delete(rep->tmp_buf);
    wtk_strbuf_delete(rep->body);
    wtk_str_hash_delete(rep->hash);
    wtk_free(rep);
    return 0;
}

int wtk_http_response_process_plain_content(wtk_http_response_t *r,char *buf,int len,int *left)
{
	int buf_left,cpy;
	wtk_strbuf_t *body;

	//print_data(buf,10);
	body=r->body;
	if(r->content_length<0)
	{
		wtk_strbuf_push(body,buf,len);
		*left=0;
		return 0;
	}
    buf_left=r->content_length-body->pos;
    cpy=min(len,buf_left);
    wtk_strbuf_push(body,buf,cpy);
    if( body->pos >= r->content_length)
    {
        r->state = RESPONSE_CONTENT_DONE;
    }
    *left=len-cpy;
    return 0;
}

//int wtk_http_chunk_feed(wtk_http_chunk_t *chunk,char *data,int bytes,int *consumed,int *done);

int wtk_http_response_process_chunk(wtk_http_response_t *r,char *buf,int len,int *left)
{
	wtk_http_chunk_t *chunk=r->chunk;
	int ret=0;
	char *s,*e;
	int consumed,done;

	//print_data(buf,len);
	//wtk_debug("%.*s\n",len,buf);
	s=buf;e=s+len;
	while(s<e)
	{
		consumed=0;
		ret=wtk_http_chunk_feed(chunk,s,e-s,&consumed,&done);
		if(ret!=0){goto end;}
		s+=consumed;
		//wtk_debug("ret=%d,done=%d,consumed=%d,size=%d,feed=%d,len=%d\n",ret,done,consumed,chunk->chunk_size,chunk->buf->pos,e-s);
		if(done)
		{
			//wtk_debug("[%.*s]\n",chunk->buf->pos,chunk->buf->data);
			if(chunk->chunk_size>0)
			{
				wtk_strbuf_push(r->body,chunk->buf->data,chunk->buf->pos);
				wtk_http_chunk_reset(chunk);
			}else
			{
				r->state=RESPONSE_CONTENT_DONE;
				wtk_http_chunk_reset(chunk);
				goto end;
			}
		}
	}
end:
	*left=len-(s-buf);
	return ret;
}

int wtk_http_response_process_content(wtk_http_response_t *r,char *buf,int len,int *left)
{
	if(r->is_chunk)
	{
		return wtk_http_response_process_chunk(r,buf,len,left);
	}else
	{
		return wtk_http_response_process_plain_content(r,buf,len,left);
	}
}

void wtk_http_response_set_string(wtk_http_response_t *r,wtk_string_t *str,char *data,int bytes)
{
	str->data=wtk_heap_dup_data(r->heap,data,bytes);
	str->len=bytes;
}


/**
 * return 0 for continue, -1 failed, 1 hdr end.
 */
int wtk_http_response_feed_hdr(wtk_http_response_t *rep,char *data,int len,int *consumed)
{
	wtk_strbuf_t *buf=rep->tmp_buf;
	wtk_string_t *v;
	char *s,*e;
	char ch;
	int ret=0;

	s=data;e=data+len;
	while(s<e)
	{
		ch=*s;
		switch(rep->state)
		{
		case RESPONSE_START:
			//HTTP/1.1 #200 OK
			if(ch==' ')
			{
				//seek to 200
				wtk_strbuf_reset(buf);
				rep->state=RESPONSE_STATUS_CODE;
			}
			break;
		case RESPONSE_STATUS_CODE:
			if(ch==' ')
			{
				rep->status=wtk_str_atoi(buf->data,buf->pos);
				rep->state=RESPONSE_STATUS_INFO;
			}else
			{
				wtk_strbuf_push_c(buf,ch);
			}
			break;
		case RESPONSE_STATUS_INFO:
			if(ch==' '||ch=='\t')
			{
				rep->state=RESPONSE_WAIT_CR;
			}else if(ch=='\r')
			{
				rep->state=RESPONSE_CR;
			}
			break;
		case RESPONSE_WAIT_CR:
			if(ch=='\r')
			{
				rep->state=RESPONSE_CR;
			}
			break;
		case RESPONSE_CR:
			if(ch=='\n')
			{
				rep->state=RESPONSE_CL;
			}else
			{
				//wtk_debug("wait \\n but is %#x[%.*s]\n",ch,e-s,s);
				ret=-1;
				goto end;
			}
			break;
		case RESPONSE_CL:
			if(ch=='\r')
			{
				rep->state=RESPONSE_HDR_ALMOST_DONE;
			}else
			{
				wtk_strbuf_reset(buf);
				wtk_strbuf_push_c(buf,tolower(ch));
				rep->state=RESPONSE_KEY;
			}
			break;
		case RESPONSE_KEY:
			if(ch==':')
			{
				wtk_http_response_set_string(rep,&(rep->key),buf->data,buf->pos);
				rep->state=RESPONSE_KEY_SPACE;
			}else
			{
				wtk_strbuf_push_c(buf,tolower(ch));
			}
			break;
		case RESPONSE_KEY_SPACE:
			if(ch!=' ')
			{
				wtk_strbuf_reset(buf);
				wtk_strbuf_push_c(buf,ch);
				rep->state=RESPONSE_VALUE;
			}
			break;
		case RESPONSE_VALUE:
			if(ch=='\r')
			{
				hash_str_node_t *node;

				wtk_strbuf_strip(buf);
				node=wtk_str_hash_find_node(rep->hash,rep->key.data,rep->key.len,0);
				if(node)
				{
					v=(wtk_string_t*)node->value;
					wtk_strbuf_push_front_s(buf,";");
					wtk_strbuf_push_front(buf,v->data,v->len);
					wtk_heap_fill_string(rep->heap,v,buf->data,buf->pos);
				}else
				{
					v=wtk_heap_dup_string(rep->heap,buf->data,buf->pos);
					wtk_str_hash_add(rep->hash,rep->key.data,rep->key.len,v);
				}
				rep->state=RESPONSE_CR;
			}else
			{
				wtk_strbuf_push_c(buf,ch);
			}
			break;
		case RESPONSE_HDR_ALMOST_DONE:
			if(ch=='\n')
			{
				rep->state=RESPONSE_HDR_DONE;
				++s;
				ret=1;
				goto end;
			}
			break;
		default:
			wtk_debug("unexcepted state %d.\n",rep->state);
			ret=-1;
			goto end;
			break;
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

void wtk_http_response_update_hdr(wtk_http_response_t *rep)
{
	wtk_string_t *v;

	//print_data(rep->strstatus.data,rep->strstatus.len);
    v=(wtk_string_t*)wtk_str_hash_find_s(rep->hash,"content-length");
    if(!v)
    {
    	v=(wtk_string_t*)wtk_str_hash_find_s(rep->hash,"transfer-encoding");
    	if(v && (wtk_string_cmp_s(v,"chunked")==0))
    	{
    		if(!rep->chunk)
    		{
    			rep->chunk=wtk_http_chunk_new();
    		}
    		rep->is_chunk=1;
    		rep->content_length=-1;
    	}else
    	{
			if(rep->unknown_content_length)
			{
				rep->content_length=-1;
			}else
			{
				rep->content_length=0;
			}
    	}
    	//wtk_debug("v=%p,[%.*s],%d\n",v,v->len,v->data,rep->is_chunk);
    }else
    {
        rep->content_length=wtk_str_atoi(v->data,v->len);
    }
}

int wtk_http_response_process_hdr(wtk_http_response_t *r,char *buf,int len,int *left)
{
	int ret=0;
	char *p;
	int consumed;

	ret=wtk_http_response_feed_hdr(r,buf,len,&consumed);
	*left=len-consumed;
	if(ret!=1){goto end;}
	ret=0;
	p=buf+consumed;
	wtk_http_response_update_hdr(r);
	if(r->body){wtk_strbuf_reset(r->body);}
	if(r->content_length==0)
	{
		r->state=RESPONSE_CONTENT_DONE;
		goto end;
	}
	if(*left>0)
	{
		ret=wtk_http_response_process_content(r,p,*left,left);
	}else
	{
		ret=0;
	}
end:
	//if(ret!=0){wtk_debug("%*.*s\n",len,len,buf);}
	return ret;
}

int wtk_http_response_feed(wtk_http_response_t *rep,char *buf,int bytes,int *left)
{
	int ret;

	//print_data(buf,bytes);
	if(rep->state!=RESPONSE_HDR_DONE)
	{
		ret=wtk_http_response_process_hdr(rep,buf,bytes,left);
	}else
	{
		ret=wtk_http_response_process_content(rep,buf,bytes,left);
	}
	return ret;
}

int wtk_http_response_feed_fd(wtk_http_response_t *rep,int fd,wtk_strbuf_t *buf)
{
	wtk_fd_state_t s;
	int readed,left;
	int ret;

	wtk_http_response_reset(rep);
	while(1)
	{
		readed=0;
		s=wtk_fd_recv(fd,buf->data,buf->length,&readed);
		//wtk_debug("read=%d,%d,errno=%d\n",readed,buf->length,errno);
		if(s!=WTK_OK)
		{
			ret=-1;break;
		}
		if(readed==0)
		{
			ret=rep->unknown_content_length?0:-1;
			break;
		}
		buf->pos=readed;
		//print_data(buf->data,buf->pos);
		ret=wtk_http_response_feed(rep,buf->data,buf->pos,&left);
		//wtk_debug("ret=%d\n",ret);
		if((ret!=0) || (left!=0))
		{
			ret=-1;
			break;
		}
		if(rep->state==RESPONSE_CONTENT_DONE)
		{
			ret=0;break;
		}
		if(readed<=0)
		{
			ret=-1;break;
		}
	}
	return ret;
}

int wtk_http_response_is_finish(wtk_http_response_t *rep)
{
	return rep->state==RESPONSE_CONTENT_DONE;
}

int wtk_http_response_print_hdr(wtk_http_response_t *rep,hash_str_node_t *n)
{
	wtk_string_t *v;

	v=(wtk_string_t*)n->value;
	printf("[%.*s]=[%.*s]\n",n->key.len,n->key.data,v->len,v->data);
	return 0;
}

void wtk_http_response_print(wtk_http_response_t *rep)
{
	//print_data(rep->hdr->data,rep->hdr->pos);
	wtk_debug("================ response ================\n");
	printf("status: %d\n",rep->status);
	wtk_str_hash_walk(rep->hash,(wtk_walk_handler_t)wtk_http_response_print_hdr,rep);
	print_data(rep->body->data,rep->body->pos);
}
