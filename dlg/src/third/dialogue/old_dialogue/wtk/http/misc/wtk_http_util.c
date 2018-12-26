#include "wtk_http_util.h"
#include <stdlib.h>
#include <ctype.h>

int wtk_hex_to_value(char c)
{
	int v;

	if(c>='0'&&c<='9')
	{
		v=c-'0';
	}else if((c>='A'&&c<='Z'))
	{
		v=c-'A'+10;
	}else if(c>='a'&& c<='z')
	{
		v=c-'a'+10;
	}else
	{
		v=0;
	}
	return v;
}

int wtk_http_url_decode(wtk_strbuf_t *buf,wtk_string_t *p)
{
enum
{
	WTK_HTTP_URL_DECODE_NORMAL,
	WTK_HTTP_URL_DECODE_PER1,
	WTK_HTTP_URL_DECODE_PER2,
};
	char *s,*e;
	char c;
	int hex1=0,hex2;
	char *dst;
	int state;

	wtk_strbuf_reset(buf);
	wtk_strbuf_expand(buf,p->len);
	s=p->data;e=s+p->len;
	state=WTK_HTTP_URL_DECODE_NORMAL;
	//print_data(p->data,p->len);
	dst=buf->data;
	while(s<e)
	{
		c=*s;++s;
		//wtk_debug("%c\n",c);
		switch(state)
		{
		case WTK_HTTP_URL_DECODE_NORMAL:
			if(c=='%')
			{
				state=WTK_HTTP_URL_DECODE_PER1;
			}else
			{
				//printf("%c",c);
				if(c=='+')
				{
					c=' ';
				}
				*dst++=c;
			}
			break;
		case WTK_HTTP_URL_DECODE_PER1:
			hex1=wtk_hex_to_value(c);
			state=WTK_HTTP_URL_DECODE_PER2;
			break;
		case WTK_HTTP_URL_DECODE_PER2:
			hex2=wtk_hex_to_value(c);
			c=(hex1<<4)+hex2;
			*dst++=c;
			state=WTK_HTTP_URL_DECODE_NORMAL;
			break;
		}
	}
	buf->pos=dst - buf->data;
	//wtk_debug("%*.*s\n",buf->pos,buf->pos,buf->data);
	return 0;
}

void wtk_http_url_encode(wtk_strbuf_t *buf,char *s,int s_bytes)
{
	static const char *digits = "0123456789ABCDEF";
	char c;
	int len;
	char *dst;
	char *start,*end;

	//make sure there is enough data
	len=buf->pos+s_bytes*3;
	wtk_strbuf_expand(buf,len);
	dst=buf->data+buf->pos;
	start=s;
	end=s+s_bytes;
	while(start<end)
	{
		c=*start++;
		if((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_'||c=='.')
		{
			*dst++=c;
		}else if(c==' ')
		{
			*dst++='+';
		}else
		{
			*dst++='%';
			*dst++=digits[(c>>4) & 0x0F];
			*dst++=digits[c & 0x0F];
		}
	}
	buf->pos=dst - buf->data;
	//wtk_debug("%d: %c,%c,%c,%c\n",buf->pos,*(dst-1),*(dst),buf->data[buf->pos],buf->data[buf->pos-1]);
}


void wtk_http_url_encode2(wtk_strbuf_t *buf,char *s,int s_bytes)
{
	static const char *digits = "0123456789ABCDEF";
	char c;
	int len;
	char *dst;
	char *start,*end;

	//make sure there is enough data
	len=buf->pos+s_bytes*3;
	wtk_strbuf_expand(buf,len);
	dst=buf->data+buf->pos;
	start=s;
	end=s+s_bytes;
	while(start<end)
	{
		c=*start++;
		if(isascii(c))
		{
			if(c==' ')
			{
				*dst++='%';
				*dst++='2';
				*dst++='0';
			}else
			{
				*dst++=c;
			}
		}else
		{
			*dst++='%';
			*dst++=digits[(c>>4) & 0x0F];
			*dst++=digits[c & 0x0F];
		}
	}
	buf->pos=dst - buf->data;
	//wtk_debug("%d: %c,%c,%c,%c\n",buf->pos,*(dst-1),*(dst),buf->data[buf->pos],buf->data[buf->pos-1]);
}




void wtk_http_url_encode_kv(wtk_strbuf_t *buf,char *key,int key_bytes,char *v,int v_bytes)
{
	wtk_http_url_encode(buf,key,key_bytes);
	wtk_strbuf_push_s(buf,"=");
	wtk_http_url_encode(buf,v,v_bytes);
}

wtk_string_t* wtk_http_file2content(char *fn,int len)
{
	static wtk_string_t keys[]={
			wtk_string("js"),wtk_string("html"),
			wtk_string("css"),wtk_string("jpg"),
			wtk_string("gif"),wtk_string("png"),
			wtk_string("swf"),
            wtk_string("mp3"),
            wtk_string("wav"),
            wtk_string("ogg"),
	};
	static wtk_string_t vs[]={
			wtk_string("text/javascript;charset=utf-8"),wtk_string("text/html;charset=utf-8"),
			wtk_string("text/css"),wtk_string("image/jpeg"),
			wtk_string("image/gif"),wtk_string("image/png"),
			wtk_string("application/x-shockwave-flash"),
            wtk_string("audio/mp3"),
            wtk_string("audio/wav"),
            wtk_string("audio/ogg"),
	};
	char *e,*s;
	int i,n;
	wtk_string_t *v=0;

	e=fn+len;
	while(e>fn)
	{
		if(*e!=0){break;}
		--e;
	}
	s=e;
	while(s>fn)
	{
		if(*s=='.'){++s;break;}
		--s;
	}
	n=e-s+1;
	for(i=0;i<sizeof(keys)/sizeof(wtk_string_t);++i)
	{
		//wtk_debug("%*.*s=%*.*s\n",n,n,s,keys[i].len,keys[i].len,keys[i].data);
		if(wtk_string_cmp(&(keys[i]),s,n)==0)
		{
			v=&(vs[i]);
			break;
		}
	}
	if(!v)
	{
		v=vs+1;
	}
	return v;
}


void wtk_http_param_parse(wtk_strbuf_t *buf,char *p,int bytes,wtk_http_param_cb_f cb,void *cb_data)
{
	enum
	{
		WAIT_KEY=0,
		WATI_EQUAL,
		WAIT_VALUE,
	};
	wtk_string_t key,value;
	int i,state;
	char c;

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
				wtk_string_set(&(value),buf->data,buf->pos);
				cb(cb_data,&key,&value);
				//print_data(key.data,key.len);
				//print_data(v->data,v->len);
				state=WAIT_KEY;
			}
			break;
		}
	}
	//return state==WAIT_KEY?0:-1;
}

void wtk_http_url_print(wtk_http_url_t *url)
{
	printf("http://%.*s:%.*s%.*s\n",url->ip.len,url->ip.data,url->port.len,url->port.data,url->uri.len,url->uri.data);
}

//http://58.210.177.12:9005/res/music/Schnappi - Schnappi.mp3
int wtk_http_url_decode_http(wtk_http_url_t *url,char *data,int len)
{
typedef enum
{
	WTK_HTTP_URL_STATE_INIT,
	WTK_HTTP_URL_STATE_HTTP,
	WTK_HTTP_URL_IP_SLASH1,
	WTK_HTTP_URL_IP_SLASH2,
	WTK_HTTP_URL_IP,
	WTK_HTTP_URL_PORT,
	WTK_HTTP_URL_URI,
}wtk_http_url_state_t;
	char *s,*e;
	int n;
	wtk_http_url_state_t state;
	wtk_string_t k;//,v;
	int ret=-1;

	wtk_string_set(&(url->ip),0,0);
	wtk_string_set_s(&(url->port),"80");
	wtk_string_set_s(&(url->uri),"/");
	//wtk_debug("[%.*s]\n",len,data);
	s=data;
	e=s+len;
	state=WTK_HTTP_URL_STATE_INIT;
	while(s<e)
	{
		n=wtk_utf8_bytes(*s);
		//wtk_debug("n=%d\n",n);
		switch(state)
		{
		case WTK_HTTP_URL_STATE_INIT:
			if(n==1 && *s=='h')
			{
				k.data=s;
				state=WTK_HTTP_URL_STATE_HTTP;
			}
			break;
		case WTK_HTTP_URL_STATE_HTTP:
			if(*s==':')
			{
				k.len=s-k.data;
				//wtk_debug("[%.*s]\n",k.len,k.data);
				k.data=s+n;
				state=WTK_HTTP_URL_IP_SLASH1;
				//exit(0);
			}
			break;
		case WTK_HTTP_URL_IP_SLASH1:
			if(*s=='/')
			{
				state=WTK_HTTP_URL_IP_SLASH2;
			}else
			{
				goto end;
			}
			break;
		case WTK_HTTP_URL_IP_SLASH2:
			if(*s=='/')
			{
				k.data=s+n;
				state=WTK_HTTP_URL_IP;
			}else
			{
				goto end;
			}
			break;
		case WTK_HTTP_URL_IP:
			if(*s==':')
			{
				k.len=s-k.data;
				//wtk_debug("[%.*s]\n",k.len,k.data);
				url->ip=k;
				state=WTK_HTTP_URL_PORT;
				k.data=s+n;
			}else if(*s=='/')
			{
				k.len=s-k.data;
				//wtk_debug("[%.*s]\n",k.len,k.data);
				url->ip=k;
				k.data=s;
				state=WTK_HTTP_URL_URI;
			}
			break;
		case WTK_HTTP_URL_PORT:
			if(*s=='/')
			{
				k.len=s-k.data;
				//wtk_debug("[%.*s]\n",k.len,k.data);
				url->port=k;
				k.data=s;
				state=WTK_HTTP_URL_URI;
			}
			break;
		case WTK_HTTP_URL_URI:
			s=e;
			continue;
			break;
		}
		s+=n;
	}
	k.len=s-k.data;
	if(state==WTK_HTTP_URL_IP)
	{
		//wtk_debug("IP=[%.*s]\n",k.len,k.data);
		url->ip=k;
	}else if(state==WTK_HTTP_URL_PORT)
	{
		//wtk_debug("PORT=[%.*s]\n",k.len,k.data);
		url->port=k;
	}else if(state==WTK_HTTP_URL_URI)
	{
		url->uri=k;
		//wtk_debug("URI=[%.*s]\n",k.len,k.data);
	}else
	{
		goto end;
	}
	ret=0;
end:
	//wtk_http_url_print(url);
	//exit(0);
	return ret;
}
