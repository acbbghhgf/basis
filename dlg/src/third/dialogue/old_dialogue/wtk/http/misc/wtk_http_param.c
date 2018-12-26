#include "wtk_http_param.h"

wtk_http_param_t *wtk_http_param_new(int nslot)
{
	wtk_http_param_t *param;

	param=(wtk_http_param_t*)wtk_malloc(sizeof(*param));
	param->hash=wtk_str_hash_new(nslot);
	param->buf=wtk_strbuf_new(4096,1);
	return param;
}

int wtk_http_param_bytes(wtk_http_param_t *p)
{
	return sizeof(*p)+wtk_strbuf_bytes(p->buf)+wtk_str_hash_bytes(p->hash);
}

int wtk_http_param_delete(wtk_http_param_t *p)
{
	wtk_strbuf_delete(p->buf);
	wtk_str_hash_delete(p->hash);
	wtk_free(p);
	return 0;
}

int wtk_http_param_reset(wtk_http_param_t *p)
{
	wtk_str_hash_reset(p->hash);
	return 0;
}

wtk_string_t* wtk_http_param_get(wtk_http_param_t *p,char* key,int bytes)
{
	return (wtk_string_t*)wtk_str_hash_find(p->hash,key,bytes);
}

int wtk_http_param_feed(wtk_http_param_t *param,char *p,int bytes)
{
	enum
	{
		WAIT_KEY=0,
		WATI_EQUAL,
		WAIT_VALUE,
	};
	wtk_string_t key,value,*v;
	int i,state;
	char c;
	wtk_strbuf_t *buf=param->buf;

	wtk_http_param_reset(param);
	state=WAIT_KEY;
	key.data=0;key.len=0;
	value.data=0;value.len=0;
	for(i=0;i<bytes;++i,++p)
	{
		c=*p;
		switch(state)
		{
		case WAIT_KEY:
			if(c!=' ')
			{
				key.data=p;
				state=WATI_EQUAL;
			}
			break;
		case WATI_EQUAL:
			if(c=='=')
			{
				key.len=p-key.data;
				value.data=p+1;
				state=WAIT_VALUE;
			}
			break;
		case WAIT_VALUE:
			if(c=='&' ||i==bytes-1)
			{
				if(c!='&')
				{
					value.len=p-value.data+1;
				}else
				{
					value.len=p-value.data;
				}
				wtk_strbuf_reset(buf);
				wtk_http_url_decode(buf,&value);
				v=wtk_heap_dup_string(param->hash->heap,buf->data,buf->pos);
				//print_data(key.data,key.len);
				//print_data(v->data,v->len);
				//wtk_debug("[%.*s]=[%.*s]\n",key.len,key.data,v->len,v->data);
				wtk_str_hash_add(param->hash,key.data,key.len,v);
				state=WAIT_KEY;
			}
			break;
		}
	}
	return state==WAIT_KEY?0:-1;
}

