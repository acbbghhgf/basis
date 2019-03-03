#include "wtk_source.h"
#include <ctype.h>
#include <math.h>
#include <float.h>
#define SING_QUOTE '\''
#define DBL_QUOTE '"'
#define ESCAPE_CHAR '\\'

/*
*swap first char and second char of short value.
*byte1 byte2 --translate to->byte2 byte1
*/
//void SwapShort(short *p)
void wtk_swap_short(short *p)
{
	char temp,*q;

	q = (char*) p;
	temp = *q; *q = *(q+1); *(q+1) = temp;
}

/*
*	byte1 byte2 byte3 byte4
*	translate->
*	byte4 byte3 byte2 byte1
*/
//void SwapInt32(int *p)
void wtk_swap_int32_x(int *p)
{
	char temp,*q;

	q = (char*) p;
	temp = *q; *q = *(q+3); *(q+3) = temp;
	temp = *(q+1); *(q+1) = *(q+2); *(q+2) = temp;
}

void wtk_swap_int32(int *i)
{
	register char c;
	register char *p;

	p=(char*)i;
	c=p[0];
	p[0]=p[3];
	p[3]=c;

	c=p[1];
	p[1]=p[2];
	p[2]=c;
}



/* check big endian or little endian.*/
int wtk_is_little_endian()
{
	/*
	int d=0x100;
	return (*(char*)&d)==(char)d;
	*/
	short x, *px;
	unsigned char *pc;

	px = &x;
	pc = (unsigned char *) px;
	*pc = 1; *(pc+1) = 0;
	return x==1;
}

wtk_source_file_item_t* wtk_source_file_item_new(FILE *f)
{
	wtk_source_file_item_t *item;

	item=(wtk_source_file_item_t*)wtk_malloc(sizeof(wtk_source_file_item_t));
	item->f=f;
	item->alloc=4096;
	item->buf=(unsigned char*)wtk_malloc(item->alloc);
	item->cur=0;
	item->valid=0;

	item->eof=0;
	return item;
}

void wtk_source_file_item_delete(wtk_source_file_item_t *item)
{
	if(item->f)
	{
		fclose(item->f);
	}
	wtk_free(item->buf);
	wtk_free(item);
}

int wtk_source_file_item_get(wtk_source_file_item_t *item)
{
	int len;

	//wtk_debug("index=%d/%d eof=%d\n",item->index,item->len,item->eof);
	if(item->cur==item->valid)
	{
		if(item->eof)
		{
			return EOF;
		}
		item->cur=item->buf;
		len=fread(item->buf,1,item->alloc,item->f);
		item->valid=item->cur+len;
		if(len<item->alloc)
		{
			item->eof=1;
		}
		if(len<=0)
		{
			item->eof=1;
			return EOF;
		}
	}
	//printf("%c",*item->cur);
	return *(item->cur++);
}

int wtk_source_file_item_get_buf(wtk_source_file_item_t *item,char *buf,int bytes)
{
	int len;
	int ret;

	len=item->valid-item->cur;
	if(len>=bytes)
	{
		memcpy(buf,item->cur,bytes);
		item->cur+=bytes;
		ret=bytes;
	}else
	{
		if(len>0)
		{
			memcpy(buf,item->cur,len);
			item->cur=item->valid;
			buf+=len;
			bytes-=len;
		}
		ret=wtk_source_file_item_get(item);
		if(ret==EOF){return EOF;}
		--item->cur;
		ret=wtk_source_file_item_get_buf(item,buf,bytes);
		if(ret==EOF){return EOF;}
		ret+=len;
	}
	return ret;
}

int wtk_source_file_item_unget(wtk_source_file_item_t* item,int c)
{
	//wtk_debug("index=%d\n",c);
	if(item->cur>item->buf)
	{
		--item->cur;
		*item->cur=c;
	}else
	{
		wtk_debug("unget\n");
		exit(0);
		ungetc(c,item->f);
	}
	return 0;
}

void wtk_source_init(wtk_source_t *src)
{
	src->data=NULL;
	src->read_str=NULL;
	src->get=NULL;
	src->get_str=NULL;
	src->unget=NULL;
	src->get_file=NULL;
	//src->get_filesize=NULL;
	src->swap=wtk_is_little_endian();
}

int wtk_source_init_fd(wtk_source_t *s,FILE *f,int pos)
{
	wtk_source_file_item_t *item;
	int ret;

	ret=fseek(f,pos,SEEK_SET);
	if(ret!=0)
	{
		wtk_debug("seek failed %d\n",pos);
		goto end;
	}
	item=wtk_source_file_item_new(f);
	wtk_source_init(s);
	s->data=item;
	s->get=(wtk_source_get_handler_t)wtk_source_file_item_get;
	s->unget=(wtk_source_unget_handler_t)wtk_source_file_item_unget;
	s->get_str=(wtk_source_get_str_f)wtk_source_file_item_get_buf;
	s->swap=wtk_is_little_endian();
	ret=0;
end:
	return ret;
}

int wtk_source_clean_fd(wtk_source_t *s)
{
	//wtk_strbuf_delete(s->buf);
	if(s->data)
	{
		((wtk_source_file_item_t*)s->data)->f=NULL;
		wtk_source_file_item_delete((wtk_source_file_item_t*)s->data);
	}
	return 0;
}

int wtk_source_init_file(wtk_source_t* s,char *fn)
{
	FILE* f;
	int ret;
	wtk_source_file_item_t *item;

	ret=-1;
	f=fopen(fn,"rb");
	if(!f){s->data=0;goto end;}
	item=wtk_source_file_item_new(f);
	wtk_source_init(s);
	s->data=item;
	s->get=(wtk_source_get_handler_t)wtk_source_file_item_get;
	s->unget=(wtk_source_unget_handler_t)wtk_source_file_item_unget;
	s->get_str=(wtk_source_get_str_f)wtk_source_file_item_get_buf;
	s->swap=wtk_is_little_endian();
	ret=0;
end:
	return ret;
}

int wtk_source_clean_file(wtk_source_t *s)
{
	//wtk_strbuf_delete(s->buf);
	if(s->data)
	{
		wtk_source_file_item_delete((wtk_source_file_item_t*)s->data);
	}
	return 0;
}

int wtk_file_unget(FILE* f,int c)
{
	return ungetc(c,f);
}

int wtk_source_init_file2(wtk_source_t* s,char *fn)
{
	FILE* f;
	int ret;

	ret=-1;
	f=fopen(fn,"rb");
	if(!f){s->data=0;goto end;}
	wtk_source_init(s);
	s->data=f;
	s->get=(wtk_source_get_handler_t)fgetc;
	s->unget=(wtk_source_unget_handler_t)wtk_file_unget;
	s->swap=wtk_is_little_endian();
	ret=0;
end:
	return ret;
}

int wtk_source_clean_file2(wtk_source_t *s)
{
	//wtk_strbuf_delete(s->buf);
	if(s->data)
	{
		fclose((FILE*)s->data);
	}
	return 0;
}



typedef struct wtk_source_str
{
	const unsigned char *data;
	int len;
	int pos;
}wtk_source_str_t;

void wtk_source_str_set(wtk_source_str_t *s,const char *data,int len)
{
	s->data=(const unsigned char*)data;
	s->len=len;
	s->pos=0;
}

wtk_source_str_t* wtk_source_str_new(const char *data,int len)
{
	wtk_source_str_t *s;

	s=(wtk_source_str_t*)wtk_malloc(sizeof(*s));
	wtk_source_str_set(s,data,len);
	return s;
}

int wtk_source_str_get(wtk_source_str_t *s)
{
	if(s->pos<s->len)
	{
		return s->data[s->pos++];
	}else
	{
		return EOF;
	}
}

int wtk_sources_str_unget(wtk_source_str_t *s,int c)
{
	if(s->pos>0 && c!=EOF)
	{
		--s->pos;
	}
	return 0;
}

int wtk_source_init_str(wtk_source_t *s,const char *data,int bytes)
{
	wtk_source_init(s);
	s->data=wtk_source_str_new(data,bytes);
	s->get=(wtk_source_get_handler_t)wtk_source_str_get;
	s->unget=(wtk_source_unget_handler_t)wtk_sources_str_unget;
	s->get_str=NULL;
	s->swap=wtk_is_little_endian();
	return 0;
}

void wtk_source_set_str(wtk_source_t *s,const char *data,int bytes)
{
	wtk_source_str_set((wtk_source_str_t*)s->data,data,bytes);
}

int wtk_source_clean_str(wtk_source_t *s)
{
	if(s->data)
	{
		wtk_free(s->data);
	}
	return 0;
}

int wtk_source_peek(wtk_source_t *s)
{
	int c;

	c=wtk_source_get(s);
	wtk_source_unget(s,c);
	return c;
}

int wtk_source_read_line(wtk_source_t *s,wtk_strbuf_t *b)
{
	int ret;
	char c;

	wtk_strbuf_reset(b);
	while(1)
	{
		c=wtk_source_get(s);
		if(c=='\n'||c==EOF){goto end;}
		wtk_strbuf_push_c(b,c);
	}
end:
	//ret=b->pos<=0?-1:0;
	ret=0;
	return ret;
}

int wtk_source_read_line2(wtk_source_t *s,wtk_strbuf_t *b,int *eof)
{
	int ret;
	char c;

	if(eof)
	{
		*eof=0;
	}
	wtk_strbuf_reset(b);
	while(1)
	{
		c=wtk_source_get(s);
		if(c=='\n'||c==EOF)
		{
			if(eof && c==EOF)
			{
				*eof=1;
			}
			goto end;
		}
		wtk_strbuf_push_c(b,c);
	}
end:
	//ret=b->pos<=0?-1:0;
	ret=0;
	return ret;
}


int wtk_source_read_string(wtk_source_t *s,wtk_strbuf_t *b)
{
	wtk_strbuf_reset(b);
	return wtk_source_read_string2(s,b);
}

int wtk_source_read_normal_string(wtk_source_t *s,wtk_strbuf_t *b)
{
	int c,ret;
	//char t;

	wtk_strbuf_reset(b);
//	if(s->read_str)
//	{
//		return s->read_str(s->data,b);
//	}
	ret=-1;
	while(isspace(c=wtk_source_get(s)));
	if(c==EOF){goto end;}
	while(1)
	{
		//wtk_debug("%d:%c\n",c,c);
		if(c==EOF){ret=0;goto end;}
		if(c==EOF||isspace(c))
		{
			wtk_source_unget(s,c);
			break;
		}
		wtk_strbuf_push_c(b,c);
		c=wtk_source_get(s);
	}
	ret=0;
end:
	return ret;
}

int wtk_source_read_string2(wtk_source_t *s,wtk_strbuf_t *b)
{
	int isq,q=0,c,ret,n,i;
	//char t;

	if(s->read_str)
	{
		return s->read_str(s->data,b);
	}
	ret=-1;
	while(isspace(c=wtk_source_get(s)));
	if(c==EOF){goto end;}
	if(c==DBL_QUOTE||c==SING_QUOTE)
	{
		isq=1;q=c;
		c=wtk_source_get(s);
	}else
	{
		isq=0;
	}
	while(1)
	{
		//wtk_debug("%d:%c\n",c,c);
		if(c==EOF){ret=0;goto end;}
		if(isq)
		{
			if(c==q){break;}
		}else
		{
			if(c==EOF||isspace(c))
			{
				wtk_source_unget(s,c);
				break;
			}
		}
		if(c==ESCAPE_CHAR)
		{
			c=wtk_source_get(s);
			if(c==EOF){goto end;}
			if(c>='0' && c<='7')
			{
				n=c-'0';
				for(i=0;i<2;++i)
				{
					c=wtk_source_get(s);
					if(c==EOF||c<'0'||c>'7'){goto end;}
					n=(n<<3)+c-'0';
				}
				c=n;
			}
		}
		//t=c;
		wtk_strbuf_push_c(b,c);
		c=wtk_source_get(s);
	}
	ret=0;
end:
	return ret;
}

int wtk_source_read_wtkstr(wtk_source_t *s,wtk_strbuf_t *b)
{
	unsigned char bi;
	int ret;

	ret=wtk_source_fill(s,(char*)&bi,1);
	if(ret!=0)
	{
		//wtk_debug("read len failed\n");
		goto end;
	}
	wtk_strbuf_reset(b);
	wtk_strbuf_expand(b,bi);
	ret=wtk_source_fill(s,b->data,bi);
	if(ret!=0)
	{
		wtk_debug("read data failed bi=%d\n",bi);
		goto end;
	}
	b->pos=bi;
end:
	return ret;
}


int wtk_source_read_wtkstr2(wtk_source_t *s,wtk_strbuf_t *b,int bi)
{
	int ret;

	wtk_strbuf_reset(b);
	wtk_strbuf_expand(b,bi);
	ret=wtk_source_fill(s,b->data,bi);
	if(ret!=0){goto end;}
	b->pos=bi;
end:
	return ret;
}

int wtk_source_read_string3(wtk_source_t *s,wtk_strbuf_t *b)
{
	int isq,q=0,c,ret,n,i;
	//char t;

	wtk_strbuf_reset(b);
	if(s->read_str)
	{
		return s->read_str(s->data,b);
	}
	ret=-1;
	while(isspace(c=wtk_source_get(s)));
	if(c==EOF){goto end;}
	if(c==DBL_QUOTE||c==SING_QUOTE)
	{
		isq=1;q=c;
		c=wtk_source_get(s);
	}else
	{
		isq=0;
	}
	while(1)
	{
		//wtk_debug("%d:%c\n",c,c);
		if(c==EOF){ret=0;goto end;}
		if(isq)
		{
			if(c==q){break;}
		}else
		{
			if(c==EOF||isspace(c)||c=='\"' ||c=='\'')
			{
				wtk_source_unget(s,c);
				break;
			}
		}
		if(c==ESCAPE_CHAR)
		{
			c=wtk_source_get(s);
			if(c==EOF){goto end;}
			if(c>='0' && c<='7')
			{
				n=c-'0';
				for(i=0;i<2;++i)
				{
					c=wtk_source_get(s);
					if(c==EOF||c<'0'||c>'7'){goto end;}
					n=(n<<3)+c-'0';
				}
				c=n;
			}
		}
		//t=c;
		wtk_strbuf_push_c(b,c);
		c=wtk_source_get(s);
	}
	ret=0;
end:
	return ret;
}

int wtk_source_skip_sp(wtk_source_t *s,int *nl)
{
	int c,ret,n;

	ret=-1;n=0;
	while(1)
	{
		c=wtk_source_get(s);
		if(c==EOF){n=1;ret=0;goto end;}
		if(!isspace(c)){break;}
		if(c=='\n')
		{
			++n;
		}
	}
	ret=0;
end:
	if(c!=EOF){wtk_source_unget(s,c);}
	if(nl){*nl=n;}
	return ret;
}

int wtk_source_skip_sp2(wtk_source_t *s,int *nl,int *eof)
{
	int c,ret,n;

	if(eof)
	{
		*eof=0;
	}
	ret=-1;n=0;
	while(1)
	{
		c=wtk_source_get(s);
		if(c==EOF)
		{
			if(eof)
			{
				*eof=1;
			}
			n=1;ret=0;
			goto end;
		}
		if(!isspace(c)){break;}
		if(c=='\n')
		{
			++n;
		}
	}
	ret=0;
end:
	if(c!=EOF){wtk_source_unget(s,c);}
	if(nl){*nl=n;}
	return ret;
}

int wtk_source_skip_sp3(wtk_source_t *s,int *nl)
{
	int c,ret,n;

	ret=-1;n=0;
	c=wtk_source_get(s);
	if(c==EOF){n=1;ret=0;goto end;}
	if(!isspace(c)){goto end;}
	if(c=='\n')
	{
		++n;
	}

	ret=0;
end:
	if(ret==-1){wtk_source_unget(s,c);}
	if(nl){*nl=n;}
	return ret;
}

int wtk_source_fill(wtk_source_t* s,char* data,int len)
{
	int ret,i;
	int c;
	unsigned char*p;

	if(s->get_str)
	{
		ret=s->get_str(s->data,data,len);
		//print_hex(data,len);
		if(ret!=len)
		{
			ret=-1;
		}else
		{
			ret=0;
		}
	}else
	{
		p=(unsigned char*)data;
		ret=0;
		for(i=0;i<len;++i)
		{
			c=wtk_source_get(s);
			//wtk_debug("c=%x\n",c);
			if(c==EOF){ret=-1;break;}
			p[i]=c;
		}
	}
	return ret;
}

int wtk_source_atoi(wtk_source_t* s,int* value)
{
	int c,ret,i;

	//wtk_debug("=============\n");
	while(1)
	{
		c=wtk_source_get(s);
		//printf("%c",c);
		if(isspace(c))
		{
			continue;
		}else
		{
			if(isdigit(c)==0)
			{
				ret=-1;goto end;
			}
			break;
		}
	}
	i=0;
	do
	{
		i=(c-'0')+i*10;
		c=wtk_source_get(s);
		//printf("%c",c);
	}while(isdigit(c));
	wtk_source_unget(s,c);
	*value=i;ret=0;
end:
	//wtk_debug("i=%d\n",i);
	return ret;
}

#include <stdarg.h>
int wtk_source_atof(wtk_source_t* s,double *v)
{
	double number;
	int exponent;
	int negative;
	double p10;
	int n;
	int num_digits;
	int num_decimals;
	char c;
	int ret;

	while(1)
	{
		c=wtk_source_get(s);
		if(isspace(c)==0){break;}
	}
	negative=0;
	switch(c)
	{
	case '-':
		negative=1;
	case '+':
		c=wtk_source_get(s);
		break;
	}
	number = 0.;
	exponent = 0;
	num_digits = 0;
	num_decimals = 0;

	// Process string of digits
	while (isdigit(c))
	{
		number = number * 10. + (c - '0');
		c=wtk_source_get(s);
		++num_digits;
	}
	if (c == '.')
	{
		c=wtk_source_get(s);
		while (isdigit(c))
		{
			number = number * 10. + (c- '0');
			c=wtk_source_get(s);
			++num_digits;
			++num_decimals;
		}
		exponent -= num_decimals;
	}
    if (num_digits == 0)
	{
    	//wtk_debug("num_digits is 0\n");
    	ret=-1;goto end;
	}
    if (negative) number = -number;
	if (c == 'e' || c == 'E')
	{
		// Handle optional sign
		negative = 0;
		c=wtk_source_get(s);
		switch(c)
		{
		case '-':
			negative = 1;   // Fall through to increment pos
		case '+':
			c=wtk_source_get(s);
			break;
		}
		n = 0;
		while (isdigit(c))
		{
			n = n * 10 + (c - '0');
			c=wtk_source_get(s);
		}
		if (negative)
				exponent -= n;
		else
				exponent += n;
	}
	if (exponent < DBL_MIN_EXP  || exponent > DBL_MAX_EXP)
    {
		//wtk_debug("read exponent failed\n");
		ret=-1;goto end;
    }

	p10 = 10.;
	n = exponent;
	if (n < 0) n = -n;
	while (n)
	{
		if (n & 1)
		{
			if (exponent < 0)
					number /= p10;
			else
					number *= p10;
		}
		n >>= 1;
		p10 *= p10;
	}
	if (number == HUGE_VAL)
	{
		//wtk_debug("read number failed\n");
		ret=-1;goto end;
	}
	wtk_source_unget(s,c);
	*v=number;
	ret=0;
end:
    return ret;
}

int wtk_source_read_int(wtk_source_t *s,int* v,int n,int bin)
{
	int ret=0,x;
	int *p,*e;

	e=v+n;
	if(bin)
	{
		ret=wtk_source_fill(s,(char*)v,sizeof(int)*n);
		if(ret!=0 || !s->swap){goto end;}
		for(p=v;p<e;++p)
		{
			wtk_swap_int32(p);
		}
	}else
	{
		for(p=v;p<e;++p)
		{
			ret=wtk_source_atoi(s,&x);
			if(ret!=0){goto end;}
			*p=x;
		}
	}
end:
	return ret;
}


int wtk_source_read_char(wtk_source_t *s)
{
	/*
	char c;

	c=(s)->get((s)->data);
	//printf("%c",c);
	return c;
	*/
	return (s)->get((s)->data);
}

int wtk_source_read_utf8_char2(wtk_source_t *s,wtk_strbuf_t *buf)
{
	char c;
	int ret;
	int i,n;

	wtk_strbuf_reset(buf);
	ret=wtk_source_get(s);
	if(ret==EOF){ret=-1;goto end;}
	c=(char)ret;
	wtk_strbuf_push_c(buf,c);
	n=wtk_utf8_bytes(c);
	i=1;
	while(i<n)
	{
		ret=wtk_source_get(s);
		if(ret==EOF){ret=-1;goto end;}
		c=(char)ret;
		wtk_strbuf_push_c(buf,c);
		++i;
	}
	ret=0;
end:
	return ret;
}

int wtk_source_read_utf8_char(wtk_source_t *s,wtk_strbuf_t *b)
{
	char bi;
	int ret;
	int len;

	wtk_strbuf_reset(b);
	ret=wtk_source_fill(s,(char*)&bi,1);
	if(ret!=0)
	{
		//wtk_debug("read len failed\n");
		goto end;
	}
	wtk_strbuf_push_c(b,bi);
	len=wtk_utf8_bytes(bi);
	if(len>1)
	{
		ret=wtk_source_fill(s,b->data+1,len-1);
		if(ret!=0)
		{
			wtk_debug("read data failed bi=%d\n",bi);
			goto end;
		}
	}
	b->pos=len;
end:
	return ret;
}


int wtk_source_read_short(wtk_source_t* s,short* v,int n,int bin)
{
	int ret=0,x;
	short *p,*e;

	e=v+n;
	if(bin)
	{
		ret=wtk_source_fill(s,(char*)v,sizeof(short)*n);
		if(ret!=0 || !s->swap){goto end;}
		for(p=v;p<e;++p)
		{
			wtk_swap_short(p);
		}
	}else
	{
		for(p=v;p<e;++p)
		{
			ret=wtk_source_atoi(s,&x);
			if(ret!=0){goto end;}
			*p=x;
		}
	}
end:
	return ret;
}

int wtk_source_read_ushort(wtk_source_t* s,unsigned short* v,int n,int bin)
{
	int ret=0,x;
	unsigned short *p,*e;

	e=v+n;
	if(bin)
	{
		ret=wtk_source_fill(s,(char*)v,sizeof(unsigned short)*n);
		if(ret!=0 || !s->swap){goto end;}
		for(p=v;p<e;++p)
		{
			wtk_swap_short((short*)p);
		}
	}else
	{
		for(p=v;p<e;++p)
		{
			ret=wtk_source_atoi(s,&x);
			if(ret!=0){goto end;}
			*p=x;
		}
	}
end:
	return ret;
}


int wtk_source_read_float(wtk_source_t *s,float *f,int n,int bin)
{
	int ret=0;
	float *p,*e;
	double d;

	e=f+n;
	if(bin)
	{
		ret=wtk_source_fill(s,(char*)f,n*sizeof(float));
		if(ret!=0 || !s->swap){goto end;}
		for(p=f;p<e;++p)
		{
			wtk_swap_int32((int*)p);
		}
	}else
	{
		//wtk_debug("---------- n=%d ------------\n",n);
		for(p=f;p<e;++p)
		{
			ret=wtk_source_atof(s,&d);
			//wtk_debug("d=%f\n",d);
			if(ret!=0)
			{
				goto end;
			}
			*p=d;
//			if(isnan(*p) || isinf(*p))
//			{
//				wtk_debug("found err\n");
//				exit(0);
//			}
		}
	}
end:
	return ret;
}

int wtk_source_read_double(wtk_source_t *s,double *f,int n)
{
	int ret=0;
	double *p,*e;
	//double d;

	e=f+n;
	for(p=f;p<e;++p)
	{
		ret=wtk_source_atof(s,p);
		if(ret!=0){goto end;}
		//*p=d;
	}
end:
	return ret;
}

int wtk_file_write_float(FILE *file,float *f,int n,int bin,int swap)
{
	int ret=0;
	float *p,*e;

	e=f+n;
	if(bin)
	{
		if(swap)
		{
			for(p=f;p<e;++p)
			{
				wtk_swap_int32((int*)p);
			}
		}
		ret=fwrite(f,sizeof(float),n,file);
		ret=ret==n?0:-1;
		if(swap)
		{
			for(p=f;p<e;++p)
			{
				wtk_swap_int32((int*)p);
			}
		}
	}else
	{
		for(p=f;p<e;++p)
		{
			fprintf(file," %e",*p);
		}
	}
	return ret;
}


int wtk_source_seek_to(wtk_source_t *src,char *data,int len)
{
	char *s,*e;
	int ret;
	char c;

	ret=-1;
	s=data;e=s+len;
	while(1)
	{
		c=wtk_source_get(src);
		if(c==EOF){goto end;}
		if(c==*s)
		{
			++s;
			if(s>=e)
			{
				ret=0;
				break;
			}
		}else
		{
			s=data;
		}
	}
end:
	return ret;
}

int wtk_source_seek_to2(wtk_source_t *src,char *data,int len,wtk_strbuf_t *buf)
{
	char *s,*e;
	int ret;
	char c;

	ret=-1;
	s=data;e=s+len;
	if(buf)
	{
		wtk_strbuf_reset(buf);
	}
	while(1)
	{
		c=wtk_source_get(src);
		if(c==EOF){goto end;}
		if(c==*s)
		{
			++s;
			if(s>=e)
			{
				ret=0;
				break;
			}
		}else
		{
			if(buf)
			{
				if(s-data)
				{
					wtk_strbuf_push(buf,data,s-data);
				}
				wtk_strbuf_push_c(buf,c);
			}
			s=data;
		}
	}
end:
	return ret;
}

int wtk_source_load_file(void *data,wtk_source_load_handler_t loader,char *fn)
{
	wtk_source_t s,*ps=&s;
	int ret;

	ret=wtk_source_init_file(ps,fn);
	if(ret!=0){goto end;}
	ret=loader(data,ps);
	wtk_source_clean_file(ps);
end:
	if(ret!=0)
	{
		wtk_debug("load %s failed.\n",fn);
		exit(0);
	}
	return ret;
}

int wtk_source_load_file_v(void *hook,void *data,wtk_source_load_handler_t loader,char *fn)
{
	return wtk_source_load_file(data,loader,fn);
}

int wtk_source_loader_load(wtk_source_loader_t *l,void *data,wtk_source_load_handler_t loader,char *fn)
{
	return l->vf(l->hook,data,loader,fn);
}

void wtk_source_loader_init_file(wtk_source_loader_t *sl)
{
	sl->hook=NULL;
	sl->vf=wtk_source_load_file_v;
}


int wtk_source_get_lines(int *nw,wtk_source_t *s)
{
	int n=0;
	char c;
	int newline;

	if(!s){goto end;}
	newline=1;
	while(1)
	{
		c=wtk_source_get(s);
		if(c == EOF)
		{
			goto end;
		}
		else if(c=='\n')
		{
			newline = 1;
		}
		else
		{
			if(newline)
			{
				++n;
				newline=0;
			}
		}
	}
end:
	*nw=n;
	return 0;
}

int wtk_source_loader_file_lines(wtk_source_loader_t *sl,char *fn)
{
	int line;
	int ret;

	line=0;
	ret=wtk_source_loader_load(sl, &line,(wtk_source_load_handler_t)wtk_source_get_lines, fn);
	if(ret==0)
	{
		return line;
	}else
	{
		return -1;
	}
}


void wtk_source_read_file2(wtk_source_t *src,wtk_strbuf_t *buf)
{
	char c;

	wtk_strbuf_reset(buf);
	while(1)
	{
		c=wtk_source_get(src);
		if(c==EOF){break;}
		wtk_strbuf_push_c(buf,c);
	}
}

wtk_string_t* wtk_source_read_file(wtk_source_t *src)
{
	wtk_strbuf_t *buf;
	wtk_string_t *data;
	char c;

	buf=wtk_strbuf_new(256,1);
	while(1)
	{
		c=wtk_source_get(src);
		if(c==EOF){break;}
		wtk_strbuf_push_c(buf,c);
	}
	data=wtk_string_dup_data(buf->data,buf->pos);
	wtk_strbuf_delete(buf);
	return data;
}

#include "wtk/core/wtk_larray.h"

float* wtk_file_read_float(char *fn,int *n)
{
	float *p=NULL;
	int ret;
	wtk_source_t src;
	wtk_larray_t *a;
	float f;

	if(n)
	{
		*n=0;
	}
	ret=wtk_source_init_file(&(src),fn);
	if(ret!=0){goto end;}
	a=wtk_larray_new(100,sizeof(float));
	while(1)
	{
		ret=wtk_source_read_float(&(src),&f,1,0);
		if(ret!=0){break;}
		wtk_larray_push2(a,&(f));
	}
	p=(float*)wtk_calloc(a->nslot,sizeof(float));
	memcpy(p,a->slot,a->nslot*sizeof(float));
	if(n)
	{
		*n=a->nslot;
	}
	wtk_larray_delete(a);
end:
	wtk_source_clean_file(&(src));
	return p;
}
