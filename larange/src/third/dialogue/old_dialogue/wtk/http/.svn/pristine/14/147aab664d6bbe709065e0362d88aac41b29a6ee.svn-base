#include <ctype.h>
#include "wtk/http/misc/wtk_http_chunk.h"


wtk_http_chunk_t* wtk_http_chunk_new()
{
	wtk_http_chunk_t *c;

	c=(wtk_http_chunk_t*)wtk_malloc(sizeof(*c));
	c->buf=wtk_strbuf_new(1024,1);
	wtk_http_chunk_reset(c);
	return c;
}

void wtk_http_chunk_delete(wtk_http_chunk_t *c)
{
	wtk_strbuf_delete(c->buf);
	wtk_free(c);
}

void wtk_http_chunk_reset(wtk_http_chunk_t *c)
{
	c->state=WTK_HTTP_CHUNK_WAIT_SIZE;
	c->chunk_size=0;
	wtk_strbuf_reset(c->buf);
}

void wtk_http_chunk_feed_hdr(wtk_http_chunk_t *chunk,char *data,int bytes,int *consumed)
{
	wtk_strbuf_t *buf=chunk->buf;
	char c;
	char *s,*e;

	//print_data(data,bytes);
	s=data;e=data+bytes;
	while(s<e)
	{
		c=*s;
		switch(chunk->state)
		{
		case WTK_HTTP_CHUNK_WAIT_SIZE:
			if(!isspace(c))
			{
				wtk_strbuf_reset(buf);
				wtk_strbuf_push_c(buf,c);
				chunk->state=WTK_HTTP_CHUNK_SIZE;
			}
			break;
		case WTK_HTTP_CHUNK_SIZE:
			if(isspace(c))
			{
				wtk_strbuf_push_c(buf,0);
				chunk->chunk_size=strtol(buf->data,0,16);
				//wtk_debug("chunk=[%s],%d\n",buf->data,chunk->chunk_size);
				chunk->state=c=='\r'?WTK_HTTP_CHUNK_WAIT_LF:WTK_HTTP_CHUNK_WAIT_CR;
			}else
			{
				wtk_strbuf_push_c(buf,c);
			}
			break;
		case WTK_HTTP_CHUNK_WAIT_CR:
			if(c=='\r')
			{
				chunk->state=WTK_HTTP_CHUNK_WAIT_LF;
			}
			break;
		case WTK_HTTP_CHUNK_WAIT_LF:
			if(c=='\n')
			{
				//wtk_stack_reset(r->content_stk);
				chunk->state=WTK_HTTP_CHUNK_HDR_DONE;
				wtk_strbuf_reset(chunk->buf);
				++s;
				goto end;
			}
			break;
		default:
			goto end;
			break;
		}
		++s;
	}
end:
	*consumed=s-data;
	return;
}

void wtk_http_chunk_feed_chunk0(wtk_http_chunk_t *chunk,char *data,int bytes,int *consumed)
{
	char *s,*e;
	char c;

	s=data;e=s+bytes;
	while(s<e)
	{
		c=*s;
		switch(chunk->state)
		{
		case WTK_HTTP_CHUNK_0_WAIT_CR:
			if(c=='\r')
			{
				chunk->state=WTK_HTTP_CHUNK_0_WAIT_LF;
			}
			break;
		case WTK_HTTP_CHUNK_0_WAIT_LF:
			if(c=='\n')
			{
				//chunk->state=WTK_HTTP_CHUNK_0_LAST_WAIT_CR;
				chunk->state=WTK_HTTP_CHUNK_DONE;
				++s;
				goto end;
			}
			break;
		/*
		case WTK_HTTP_CHUNK_0_LAST_WAIT_CR:
			if(c=='\r')
			{
				chunk->state=WTK_HTTP_CHUNK_0_LAST_WAIT_LF;
			}
			break;
		case WTK_HTTP_CHUNK_0_LAST_WAIT_LF:
			if(c=='\n')
			{
				chunk->state=WTK_HTTP_CHUNK_DONE;
				++s;
				goto end;
			}
			break;
		*/
		default:
			goto end;
			break;
		}
		++s;
	}
end:
	*consumed=s-data;
	return;
}

int wtk_http_chunk_feed(wtk_http_chunk_t *chunk,char *data,int bytes,int *consumed,int *done)
{
	int ret=0,c,step;

	//wtk_debug("bytes=%d/%d\n",bytes,chunk->chunk_size);
	//print_data(data,bytes);
	*consumed=0;*done=0;
	if(chunk->state<WTK_HTTP_CHUNK_HDR_DONE)
	{
		wtk_http_chunk_feed_hdr(chunk,data,bytes,&c);
		data+=c;bytes-=c;*consumed+=c;
		if(bytes<=0){goto end;}
		if(chunk->chunk_size==0)
		{
			chunk->state=WTK_HTTP_CHUNK_0_WAIT_CR;
		}
	}
	//print_data(data,bytes);
	if(chunk->chunk_size==0)
	{
		wtk_http_chunk_feed_chunk0(chunk,data,bytes,&c);
		data+=c;bytes-=c;*consumed+=c;
	}else
	{
		c=chunk->chunk_size-chunk->buf->pos;
		step=min(c,bytes);
		if(step>0)
		{
			wtk_strbuf_push(chunk->buf,data,step);
			*consumed+=step;
		}
		if(c<=step)
		{
			chunk->state=WTK_HTTP_CHUNK_DONE;
		}
		*done=c<=step?1:0;
		//wtk_debug("chunk_size=%d,done=%d\n",chunk->chunk_size,*done);
	}
	*done=chunk->state==WTK_HTTP_CHUNK_DONE;
end:
	//wtk_http_chunk_print(chunk);
	return ret;
}


//==================== print section ==================
void wtk_http_chunk_print(wtk_http_chunk_t *c)
{
	printf("chunk size: %d\n",c->chunk_size);
	printf("feed bytes: %d\n",c->buf->pos);
}
