#include "wtk_strbuf.h"
#include "wtk_os.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

wtk_strbuf_t* wtk_strbuf_new(int init_len,float rate)
{
    wtk_strbuf_t *b;
    char *data;

    data=(char*)wtk_malloc(init_len);
    if(!data){b=0;goto end;}
    b=(wtk_strbuf_t*)wtk_malloc(sizeof(*b));
    b->data=data;
    b->length=init_len;
    b->pos=0;
    b->rate=1.0+rate;
end:
    return b;
}

int wtk_strbuf_bytes(wtk_strbuf_t *b)
{
	return sizeof(*b)+b->length;
}

int wtk_strbuf_delete(wtk_strbuf_t* b)
{
    wtk_free(b->data);
    wtk_free(b);
    return 0;
}

void wtk_strbuf_expand(wtk_strbuf_t *s,int bytes)
{
    int left,alloc;
    char *p;
    int t1,t2;

    left=s->length-s->pos;
    if(bytes>left)
    {
    	t1=s->length*s->rate;
    	t2=s->pos+bytes;
        alloc=max(t1,t2);//s->length*s->rate,s->pos+bytes);
        p=s->data;
        s->data=(char*)wtk_malloc(alloc);
        s->length=alloc;
        memcpy(s->data,p,s->pos);
        wtk_free(p);
    }
    return;
}

void wtk_strbuf_push(wtk_strbuf_t *s,const char *buf,int bytes)
{
    if(bytes<=0){return;}
    if(bytes>s->length-s->pos)
    {
        wtk_strbuf_expand(s,bytes);
    }
    if(buf)
    {
    	memcpy(s->data+s->pos,buf,bytes);
    }else
    {
    	memset(s->data+s->pos,0,bytes);
    }
    s->pos+=bytes;
    return;
}

void wtk_strbuf_push_word(wtk_strbuf_t *buf,char *data,int bytes)
{
	if(buf->pos>0 && (bytes==1 || wtk_utf8_bytes(data[0])==1) && (isalpha(data[0])))
	{
		wtk_strbuf_push_s(buf," ");
	}
	wtk_strbuf_push(buf,data,bytes);
}

void wtk_strbuf_push_front(wtk_strbuf_t *s,const char *buf,int bytes)
{
    int left;

    if(bytes<0){return;}
    left=s->length-s->pos;
    if(bytes>left)
    {
        wtk_strbuf_expand(s,bytes);
    }
    memmove(s->data+bytes,s->data,s->pos);
    if(buf)
    {
    	memcpy(s->data,buf,bytes);
    }else
    {
    	memset(s->data,0,bytes);
    }
    s->pos+=bytes;
    return;
}

/*
void wtk_strbuf_push_c(wtk_strbuf_t *s,char b)
{
	//wtk_strbuf_push(s,&b,1);
    if(s->length<=s->pos)
    {
        wtk_strbuf_expand(s,1);
    }
    s->data[s->pos++]=b;
}
*/

void wtk_strbuf_push_f(wtk_strbuf_t *b,const char *fmt,...)
{
	char buf[4096]={0};
	va_list ap;
	int n;

	va_start(ap,fmt);
	n=vsprintf(buf,fmt,ap);
	wtk_strbuf_push(b,buf,n);
	va_end(ap);
}

int wtk_strbuf_pop(wtk_strbuf_t *s,char* data,int bytes)
{
	int ret;

	if(s->pos<bytes)
	{
		ret=-1;goto end;
	}
	if(data)
	{
		memcpy(data,s->data,bytes);
	}
	s->pos-=bytes;
	if(s->pos>0)
	{
		memmove(s->data,&(s->data[bytes]),s->pos);
	}
	ret=0;
end:
	return ret;
}

char* wtk_strbuf_to_str(wtk_strbuf_t *s)
{
	char *p=0;

	if(s->pos<=0){goto end;}
	p=(char*)wtk_malloc(s->pos+1);
	memcpy(p,s->data,s->pos);
	p[s->pos]=0;
end:
	return p;
}

void wtk_strbuf_merge(wtk_strbuf_t *buf,wtk_string_t *p1,...)
{
    va_list ap;

    va_start(ap,p1);
    while(p1)
    {
    	wtk_strbuf_push(buf,p1->data,p1->len);
        p1=va_arg(ap,wtk_string_t*);
    }
    va_end(ap);
}

void wtk_strbuf_merge2(wtk_strbuf_t *buf,char *p1,...)
{
    va_list ap;

    va_start(ap,p1);
    while(p1)
    {
    	wtk_strbuf_push(buf,p1,strlen(p1));
        p1=va_arg(ap,char*);
    }
    va_end(ap);
}

/*
void wtk_strbuf_reset(wtk_strbuf_t *s)
{
    s->pos=0;
}
*/

void wtk_strbuf_print(wtk_strbuf_t *s)
{
    //print_data(s->data,s->pos);
    printf("%*s\n",s->pos,s->data);
}

int wtk_strbuf_read2(wtk_strbuf_t *buf,char *fn)
{
	FILE *f;
	int ret=-1;
	char tmp[4096];
	int n;

	f=fopen(fn,"rb");
	if(!f){goto end;}
	wtk_strbuf_reset(buf);
	while(1)
	{
		n=fread(tmp,1,sizeof(tmp),f);
		if(n>0)
		{
			wtk_strbuf_push(buf,tmp,n);
		}
		if(n<sizeof(tmp)){break;}
	}
	ret=0;
end:
	if(f){fclose(f);}
	return ret;
}

int wtk_strbuf_read(wtk_strbuf_t *buf,char *fn)
{
    FILE *f;
    int ret=-1;
    int n,len;

    f=fopen(fn,"rb");
    if(!f){goto end;}
    len=file_length(f);
    wtk_strbuf_reset(buf);
    wtk_strbuf_expand(buf,len);
    n=fread(buf->data,1,len,f);
    if(n>0)
    {
        buf->pos=n;
    }
    ret=(n==len)?0:-1;
end:
    if(f){fclose(f);}
    return ret;
}



void wtk_strbuf_strip(wtk_strbuf_t *buf)
{
	int i,n;

	for(i=0,n=0;i<buf->pos;++i)
	{
#ifdef WIN32
		if(isspace((unsigned char)buf->data[i]))
#else
		if(isspace(buf->data[i]))
#endif
		{
			++n;
		}else
		{
			break;
		}
	}
	//wtk_debug("[%d]\n",n);
	if(n>0)
	{
		wtk_strbuf_pop(buf,0,n);
	}
	for(i=buf->pos-1;i>=0;--i)
	{
#ifdef WIN32
		if(isspace((unsigned char)buf->data[i]))
#else
		if(isspace(buf->data[i]))
#endif
		{
			--buf->pos;
		}else
		{
			break;
		}
	}
}

void wtk_strbuf_pad0(wtk_strbuf_t *buf,int bytes)
{
	wtk_strbuf_expand(buf,bytes);
	memset(buf->data+buf->pos,0,bytes);
	buf->pos+=bytes;
}

void wtk_strbuf_replace(wtk_strbuf_t *buf,char *data,int data_len,char *src,int src_len,char *dst,int dst_len)
{
	char *s,*e;
	int i;

	wtk_strbuf_reset(buf);
	s=data;e=s+data_len;
	while(s<e)
	{
		i=wtk_str_str(s,e-s,src,src_len);
		if(i<0)
		{
			wtk_strbuf_push(buf,s,e-s);
			break;
		}
		if(i>0)
		{
			wtk_strbuf_push(buf,s,i);
		}
		wtk_strbuf_push(buf,dst,dst_len);
		s+=i+src_len;
	}
}

#ifndef BUF_USE_MACRO
void wtk_strbuf_push_c(wtk_strbuf_t *buf,char b)
{
    if(buf->length<=buf->pos)
    {
        wtk_strbuf_expand(buf,1);
    }
    buf->data[buf->pos++]=b;
}
#endif

void wtk_strbuf_push_skip_ws(wtk_strbuf_t *buf,char *data,int len)
{
	char *s,*e;
	char c;

	s=data;e=s+len;
	while(s<e)
	{
		c=*s;
		if(c==' '||c=='\t')
		{

		}else
		{
			wtk_strbuf_push_c(buf,c);
		}
		++s;
	}
}

void wtk_strbuf_push_skip_utf8_ws(wtk_strbuf_t *buf,char *data,int len)
{
	char *s,*e;
	char c;
	int n;
	int prev_n=-1;
	int space=0;

	s=data;e=s+len;
	while(s<e)
	{
		c=*s;
		n=wtk_utf8_bytes(c);
		if(n==1)
		{
			if(prev_n>1 && (c==' '||c=='\t'))
			{

			}else
			{
				if(c==' ' || c=='\t')
				{
					space=1;
				}else
				{
					if(space)
					{
						wtk_strbuf_push_c(buf,' ');
					}
					space=0;
					wtk_strbuf_push_c(buf,c);
				}
			}
		}else
		{
			space=0;
			wtk_strbuf_push(buf,s,n);
		}
		prev_n=n;
		s+=n;
	}
}

void wtk_strbuf_push_add_escape_str(wtk_strbuf_t *buf,char *data,int bytes)
{
	char *s,*e;
	char c;

	s=data;e=s+bytes;
	while(s<e)
	{
		c=*s;
#ifdef WIN32
		if(c=='\"' ||c=='\\')
#else
		if(c=='\'' || c=='\"' ||c=='\\')
#endif
		{
			wtk_strbuf_push_c(buf,'\\');
		}
		wtk_strbuf_push_c(buf,c);
		++s;
	}
}

void wtk_strbuf_string_to_str(wtk_strbuf_t *buf,char *data,int bytes)
{
	wtk_strbuf_reset(buf);
	wtk_strbuf_push(buf,data,bytes);
	wtk_strbuf_push_c(buf,0);
}

void wtk_strbuf_parse_quote(wtk_strbuf_t *buf,char *data,int bytes)
{
	char quote;
	char *s,*e;
	char c;
	int esc;

	s=data;e=s+bytes;
	wtk_strbuf_reset(buf);
	quote=*s;
	if(quote!='\'' && quote !='"')
	{
		wtk_strbuf_push(buf,data,bytes);
		return;
	}
	++s;
	esc=0;
	while(s<e)
	{
		c=*s;

		if(esc)
		{
			wtk_strbuf_push_c(buf,c);
			esc=0;
		}else
		{
			if(c=='\\')
			{
				esc=1;
			}else if(c==quote)
			{
				break;
			}else
			{
				wtk_strbuf_push_c(buf,c);
			}
		}
		++s;
	}

}

static wtk_string_t num[]={
		wtk_string("零"),
		wtk_string("一"),
		wtk_string("二"),
		wtk_string("三"),
		wtk_string("四"),
		wtk_string("五"),
		wtk_string("六"),
		wtk_string("七"),
		wtk_string("八"),
		wtk_string("九"),
};

void wtk_stochn(wtk_strbuf_t *buf,char *data,int len)
{
	char *s,*e;
	int v;

	wtk_strbuf_reset(buf);
	s=data;e=s+len;
	while(s<e)
	{
		v=*s-'0';
		if(v<10)
		{
			wtk_strbuf_push(buf,num[v].data,num[v].len);
		}
		++s;
	}
}

static wtk_string_t num2[]={
		wtk_string("零"),
		wtk_string("幺"),
		wtk_string("二"),
		wtk_string("三"),
		wtk_string("四"),
		wtk_string("五"),
		wtk_string("六"),
		wtk_string("七"),
		wtk_string("八"),
		wtk_string("九"),
};

void wtk_stotel(wtk_strbuf_t *buf,char *data,int len)
{
	char *s,*e;
	int v;

	wtk_strbuf_reset(buf);
	s=data;e=s+len;
	while(s<e)
	{
		v=*s-'0';
		if(v<10)
		{
			wtk_strbuf_push(buf,num2[v].data,num2[v].len);
		}
		++s;
	}
}

void wtk_itochn2(wtk_strbuf_t *buf,int v,int last_u)
{
typedef struct
{
	int v;
	wtk_string_t s;
}wtk_iunit_t;
	static wtk_iunit_t unit1[]={
			{100000000,wtk_string("亿")},
			{10000,wtk_string("万")},
	};
	static wtk_iunit_t unit2[]={
			{1000,wtk_string("千")},
			{100,wtk_string("百")},
			{10,wtk_string("十")},
	};
	int x;
	int i;

	for(i=0;i<sizeof(unit1)/sizeof(wtk_iunit_t);++i)
	{
		x=v/(unit1[i].v);
		if(x>0)
		{
			//wtk_debug("x=%d\n",x);
			wtk_itochn2(buf,x,last_u);
			wtk_strbuf_push(buf,unit1[i].s.data,unit1[i].s.len);
			v=v%(unit1[i].v);
			//last_u=unit1[i].v;
			wtk_itochn2(buf,v,unit1[i].v);
			return;
		}
	}
	for(i=0;i<sizeof(unit2)/sizeof(wtk_iunit_t);++i)
	{
		x=v/(unit2[i].v);
		if(x>0 && x<10)
		{
			//wtk_debug("x=%d\n",x);
			//wtk_debug("last=%d/%d\n",last_u,unit2[i].v);
			if(last_u/unit2[i].v>10)
			{
				wtk_strbuf_push(buf,num[0].data,num[0].len);
			}
			if(x==1 && buf->pos==0 && unit2[i].v==10)
			{

			}else
			{
				wtk_strbuf_push(buf,num[x].data,num[x].len);
			}
			wtk_strbuf_push(buf,unit2[i].s.data,unit2[i].s.len);
			v=v%(unit2[i].v);
			last_u=unit2[i].v;
			//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		}
	}
	//wtk_debug("v=%d last_u=%d\n",v,last_u);
	if(v>0 && v<10)
	{
		if(last_u>10)
		{
			wtk_strbuf_push(buf,num[0].data,num[0].len);
		}
		wtk_strbuf_push(buf,num[v].data,num[v].len);
	}else if(v==0 && buf->pos==0)
	{
		wtk_strbuf_push(buf,num[v].data,num[v].len);
	}
	//wtk_debug("[%.*s]\n",buf->pos,buf->data);
	//wtk_debug("v=%d\n",v);
	//exit(0);
}

void wtk_itochn(wtk_strbuf_t *buf,int v)
{
	wtk_strbuf_reset(buf);
	wtk_itochn2(buf,v,0);
}

#include "wtk/core/wtk_larray.h"

void wtk_strbuf_real_path(wtk_strbuf_t *buf,char *name,int len)
{
enum
{
	WTK_RP_INIT,
};
	char *s,*e;
	int state;
	int n;
	char c;
	wtk_larray_t *a;
	wtk_heap_t *heap;
	wtk_string_t *v;
	wtk_string_t **strs;
	int i;

	//wtk_debug("[%.*s]\n",len,name);
	heap=wtk_heap_new(4096);
	a=wtk_larray_new(len/3,sizeof(void*));
	wtk_strbuf_reset(buf);
	s=name;
	e=s+len;
	state=WTK_RP_INIT;
	while(s<e)
	{
		c=*s;
		n=wtk_utf8_bytes(c);
		switch(state)
		{
		case WTK_RP_INIT:
			if(n==1 && *s=='/')
			{
				//wtk_debug("[%.*s]\n",buf->pos,buf->data);
				if(buf->pos>0)
				{
					if(wtk_str_equal_s(buf->data,buf->pos,".."))
					{
						if(a->nslot>0)
						{
							--a->nslot;
						}
					}else if(wtk_str_equal_s(buf->data,buf->pos,"."))
					{
						if(a->nslot==0)
						{
							v=wtk_heap_dup_string(heap,buf->data,buf->pos);
							wtk_larray_push2(a,&v);
						}
					}else
					{
						v=wtk_heap_dup_string(heap,buf->data,buf->pos);
						wtk_larray_push2(a,&v);
					}
					wtk_strbuf_reset(buf);
				}
			}else
			{
				wtk_strbuf_push(buf,s,n);
			}
			break;
		}
		s+=n;
	}
	if(buf->pos>0)
	{
		if(wtk_str_equal_s(buf->data,buf->pos,".."))
		{
			if(a->nslot>0)
			{
				--a->nslot;
			}
		}else if(wtk_str_equal_s(buf->data,buf->pos,"."))
		{
			if(a->nslot==0)
			{
				v=wtk_heap_dup_string(heap,buf->data,buf->pos);
				wtk_larray_push2(a,&v);
			}
		}else
		{
			v=wtk_heap_dup_string(heap,buf->data,buf->pos);
			wtk_larray_push2(a,&v);
		}
	}
	strs=(wtk_string_t**)(a->slot);
	n=a->nslot;
	//wtk_debug("n=%d\n",a->nslot);
	wtk_strbuf_reset(buf);
	for(i=0;i<n;++i)
	{
		//wtk_debug("[%.*s]\n",strs[i]->len,strs[i]->data);
		if(buf->pos>0)
		{
			wtk_strbuf_push_s(buf,"/");
		}
		wtk_strbuf_push(buf,strs[i]->data,strs[i]->len);
	}
	//wtk_debug("[%.*s]\n",buf->pos,buf->data);
	wtk_heap_delete(heap);
	wtk_larray_delete(a);
	//exit(0);
}

void wtk_strbuf_control_cache(wtk_strbuf_t *buf,int max_cache)
{
	char *data;

	if(buf->length>max_cache && buf->pos<=max_cache)
	{
		data=wtk_malloc(max_cache);
		memcpy(data,buf->data,buf->pos);
		wtk_free(buf->data);
		buf->data=data;
		buf->length=max_cache;
	}
}



