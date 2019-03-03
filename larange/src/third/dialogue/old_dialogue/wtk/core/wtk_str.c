#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include "wtk_str.h"
#include "wtk/core/cfg/wtk_source.h"
#include "wtk/core/wtk_str_encode.h"
#include "wtk/core/wtk_strbuf.h"

wtk_string_t* wtk_string_new(int len)
{
	wtk_string_t *s;

	s=(wtk_string_t*)wtk_malloc(len+sizeof(*s));
	s->len=len;
	if(len>0)
	{
		s->data=(char*)s+sizeof(*s);
	}else
	{
		s->data=0;
	}
	return s;
}

int wtk_string_delete(wtk_string_t *s)
{
	wtk_free(s);
	return 0;
}

/*
wtk_string_t* wtk_string_new(int len)
{
	wtk_string_t *s;

	s=(wtk_string_t*)wtk_malloc(sizeof(*s));
	if(len>0)
	{
		s->len=len;
		s->data=(char*)wtk_malloc(len);
	}else
	{
		s->len=0;
		s->data=0;
	}
	return s;
}

int wtk_string_delete(wtk_string_t *s)
{
	wtk_free(s->data);
	wtk_free(s);
	return 0;
}
*/

char* wtk_string_to_str(wtk_string_t *s)
{
	return wtk_data_to_str(s->data,s->len);
}

wtk_string_t* wtk_string_dup_data(char* data,int len)
{
	wtk_string_t* s;

	s=wtk_string_new(len);
	if(s && data)
	{
		memcpy(s->data,data,len);
	}
	return s;
}

wtk_string_t* wtk_string_dup_data_pad0(char* data,int len)
{
	wtk_string_t* s;

	s=wtk_string_new(len+1);
	if(s && data)
	{
		memcpy(s->data,data,len);
	}
	s->data[len]=0;
	return s;
}

int wtk_string_cmp_withstart(wtk_string_t* str, char*s, int bytes)
{
	int ret;

	ret = str->len - bytes;
	if (ret < 0){goto end;}
	ret = strncmp(str->data, s, bytes);
end:
    return ret;
}

int wtk_str_end_with(char *data,int len,char *suf,int suf_bytes)
{
	int ret;

	ret=-1;
	if(len<suf_bytes){goto end;}
	//print_data(data+len-suf_bytes,suf_bytes);
	ret=strncmp(data+len-suf_bytes,suf,suf_bytes);
end:
	return ret==0?1:0;
}

int wtk_str_start_with(char *data,int len,char *suf,int suf_bytes)
{
	int ret;

	ret = len - suf_bytes;
	if (ret < 0){goto end;}
	ret = strncmp(data, suf, suf_bytes);
end:
	return ret==0?1:0;
}

int wtk_string_cmp(wtk_string_t *str,char* s,int bytes)
{
	int ret;

	ret=str->len-bytes;
	if(ret!=0){goto end;}
	ret=strncmp(str->data,s,bytes);
end:
	return ret;
}

int wtk_data_cmp(char *str,int str_bytes,char* s,int bytes)
{
	int ret;

	ret=str_bytes-bytes;
	if(ret!=0){goto end;}
	ret=strncmp(str,s,bytes);
end:
	return ret;
}

int wtk_string_cmp2(wtk_string_t *str1,wtk_string_t *str2)
{
	int b;

	if(str1!=str2)
	{
		if(str1 && str2)
		{
			if(wtk_string_cmp(str1,str2->data,str2->len)!=0)
			{
				b=-1;goto end;
			}
		}else
		{
			b=-1;goto end;
		}
	}
	b=0;
end:
	return b;
}

int wtk_string_is_char_in(wtk_string_t *s,char c)
{
	int i;
	int ret=0;

	for(i=0;i<s->len;++i)
	{
		if(s->data[i]==c)
		{
			ret=1;
			break;
		}
	}
	return ret;
}

int wtk_string_char_count(wtk_string_t *str,char c)
{
	char *s,*e;
	int n=0;

	s=str->data;e=s+str->len;
	while(s<e)
	{
		if(*s==c)
		{
			++n;
		}
		++s;
	}
	return n;
}

int wtk_str_char_count(const char *s,char c)
{
	int n=0;

	while(*s)
	{
		if(*s==c)
		{
			++n;
		}
		++s;
	}
	return n;
}

int wtk_string_array_has(wtk_string_t **strs,int n,wtk_string_t *s)
{
	int i;

	for(i=0;i<n;++i)
	{
		if(wtk_str_equal(s->data,s->len,strs[i]->data,strs[i]->len))
		{
			return 1;
		}
	}
	return 0;
}

unsigned int wtk_string_to_ord(wtk_string_t *str)
{
	char *s,*e;
	int i,n;
	unsigned int v,x;

	s=str->data;e=s+str->len;
	n=0;v=0;
	while(s<e)
	{
		for(x=*s,i=0;i<n;++i)
		{
			x<<=8;
		}
		v+=x;
		++n;++s;
	}
	return v;
}

typedef struct
{
	wtk_string_t k;
	int v;
}wtk_chnstr_item_t;

/*
 * I have to convert these character to hex because cl.exe with vs2005 doesn't support chinese
 * also I have to use pinyin to annotate these code.
 *                                                                     by Ciwi Shi, 2014.05.31
 */

static wtk_chnstr_item_t chnstrs[]={
		{wtk_string("\xe9\x9b\xb6"),0},         // ling2
		{wtk_string("\xe4\xb8\x80"),1},         // yi1
		{wtk_string("\xe5\xb9\xba"),1},         // yao1
		{wtk_string("\xe4\xb8\xa4"),2},         // liang2
		{wtk_string("\xe4\xba\x8c"),2},         // er4
		{wtk_string("\xe4\xb8\x89"),3},         // san1
		{wtk_string("\xe5\x9b\x9b"),4},         // si4
		{wtk_string("\xe4\xba\x94"),5},         // wu3
		{wtk_string("\xe5\x85\xad"),6},         // liu4
		{wtk_string("\xe4\xb8\x83"),7},         // qi1
		{wtk_string("\xe5\x85\xab"),8},         // ba1
		{wtk_string("\xe4\xb9\x9d"),9},         // jiu3
		{wtk_string("\xe5\x8d\x81"),10},        // shi2
		{wtk_string("\xe7\x99\xbe"),100},       // bai3
		{wtk_string("\xe5\x8d\x83"),1000},      // qian1
		{wtk_string("\xe4\xb8\x87"),10000},     // wan4
		{wtk_string("\xe4\xba\xbf"),100000000}, // yi4
};

static wtk_string_t chnstrs2[]={
		wtk_string("\xe9\x9b\xb6"),
		wtk_string("\xe4\xb8\x80"),
		wtk_string("\xe4\xba\x8c"),
		wtk_string("\xe4\xb8\x89"),
		wtk_string("\xe5\x9b\x9b"),
		wtk_string("\xe4\xba\x94"),
		wtk_string("\xe5\x85\xad"),
		wtk_string("\xe4\xb8\x83"),
		wtk_string("\xe5\x85\xab"),
		wtk_string("\xe4\xb9\x9d"),
};

static wtk_chnstr_item_t chnstrs3[]={
		{wtk_string("\xe9\x9b\xb6"),0},
		{wtk_string("\xe4\xb8\x80"),1},
		{wtk_string("\xe5\xb9\xba"),1},
		{wtk_string("\xe4\xba\x8c"),2},
		{wtk_string("\xe4\xb8\x89"),3},
		{wtk_string("\xe5\x9b\x9b"),4},
		{wtk_string("\xe4\xba\x94"),5},
		{wtk_string("\xe5\x85\xad"),6},
		{wtk_string("\xe4\xb8\x83"),7},
		{wtk_string("\xe5\x85\xab"),8},
		{wtk_string("\xe4\xb9\x9d"),9},
};

int wtk_chnstr_atoi4(char *s,int len)
{
	int n=-1;
	int i;

	for(i=0;i<sizeof(chnstrs3)/sizeof(wtk_chnstr_item_t);++i)
	{
		if(wtk_string_cmp(&(chnstrs3[i].k),s,len)==0)
		{
			n=chnstrs3[i].v;
			goto end;
		}
	}
end:
	return n;
}

int wtk_strbuf_itoa2(wtk_strbuf_t *buf,int v,int base,char *unit,int unit_len)
{
	wtk_string_t *x;
	int t;
	int pos;

	pos=buf->pos;
	//wtk_debug("v=%d,pos=%d\n",v,buf->pos);
	if(v<=10)
	{
		x=&(chnstrs2[v]);
		wtk_strbuf_push(buf,x->data,x->len);
		v=0;
	}else
	{
		t=v/base;
		//wtk_debug("t=%d,base=%d\n",t,base);
		if(t>0)
		{
			if(t>10)
			{
				wtk_strbuf_itoa(buf,t);
			}else
			{
				if(t==1 && base==10 && pos==0)
				{
					//11
				}else
				{
					x=&(chnstrs2[t]);
					wtk_strbuf_push(buf,x->data,x->len);
				}
			}
			wtk_strbuf_push(buf,unit,unit_len);
		}
		v=v%base;
		//101
		if(t>0)
		{
			//wtk_debug("v=%d,base=%d\n",v,base);
			//111 110 120
			t=v/(base/10);
			if(v>0 && t==0 && base>10)
			{
				wtk_strbuf_push_s(buf,"\xe9\x9b\xb6");
			}
			if((v%10==0) && t==1 && pos>0)// &&(v/(base/10)==1))
			{
				wtk_strbuf_push_s(buf,"\xe4\xb8\x80");
			}
		}
	}
	return v;
}

int wtk_strbuf_itoa(wtk_strbuf_t *buf,int v)
{
	static wtk_chnstr_item_t strs[]={
		{wtk_string("\xe4\xba\xbf"),100000000},
		{wtk_string("\xe4\xb8\x87"),10000},
		{wtk_string("\xe5\x8d\x83"),1000},
		{wtk_string("\xe7\x99\xbe"),100},
		{wtk_string("\xe5\x8d\x81"),10},
	};
	int i;
	int vt;

	if(v<=10)
	{
		wtk_strbuf_itoa2(buf,v,10,0,0);
		return 0;
	}
	vt=v;
	for(i=0;i<sizeof(strs)/sizeof(wtk_chnstr_item_t);++i)
	{
		//wtk_debug("[%.*s]\n",strs[i].k.len,strs[i].k.data);
		vt=wtk_strbuf_itoa2(buf,vt,strs[i].v,strs[i].k.data,strs[i].k.len);
		if(vt<=0)
		{
			break;
		}
	}
	//wtk_debug("[%.*s]\n",buf->pos,buf->data);
	//exit(0);
	return 0;
}

void wtk_strbuf_atochn(wtk_strbuf_t *buf,char *data,int len)
{
	char *s,*e;
	int t;

	s=data;e=s+len;
	while(s<e)
	{
		t=*s-'0';
		if(t>=0 && t<=9)
		{
			wtk_strbuf_push(buf,chnstrs2[t].data,chnstrs2[t].len);
		}
		++s;
	}
}

int wtk_strbuf_atochn2(wtk_strbuf_t *buf,char *data,int len)
{
	char *s,*e;
	int t,num;
	char c;
	int n=0;

	s=data;e=s+len;
	while(s<e)
	{
		c=*s;
		num=wtk_utf8_bytes(c);
		if(num==1)
		{
			t=c-'0';
			//wtk_debug("c=%c\n",c);
			if(t>=0 && t<=9)
			{
				wtk_strbuf_push(buf,chnstrs2[t].data,chnstrs2[t].len);
				++n;
			}else
			{
				wtk_strbuf_push(buf,s,num);
			}
		}else
		{
			wtk_strbuf_push(buf,s,num);
		}
		s+=num;
	}
	return n;
}

wtk_string_t* wtk_chnstr_itoa(int v)
{
	wtk_string_t *vx=0;
	int i;

	for(i=0;i<sizeof(chnstrs)/sizeof(wtk_chnstr_item_t);++i)
	{
		if(chnstrs[i].v==v)
		{
			vx=&(chnstrs[i].k);
			goto end;
		}
	}
end:
	return vx;
}

int wtk_chnstr_atoi3(char *s,int len)
{
	int n=-1;
	int i;

	for(i=0;i<sizeof(chnstrs)/sizeof(wtk_chnstr_item_t);++i)
	{
		if(wtk_string_cmp(&(chnstrs[i].k),s,len)==0)
		{
			n=chnstrs[i].v;
			goto end;
		}
	}
end:
	return n;
}

int wtk_chnstr_atoi2(char *data,int len)
{
	//no check
	//if(len<=0){return 0;}
	if(wtk_utf8_bytes(data[0])==1)
	{
		return wtk_str_atoi(data,len);
	}else
	{
		return wtk_chnstr_atoi(data,len,0);
	}
}

int wtk_str_atoi2(char *data,int len,int *left)
{
	if(wtk_utf8_bytes(data[0])==1)
	{
		if(left)
		{
			*left=0;
		}
		return wtk_str_atoi(data,len);
	}else
	{
		return wtk_chnstr_atoi(data,len,left);
	}
}

int wtk_num_get_unit(int num)
{
	int u=1;

	while((num=num/10)>0)
	{
		u=u*10;
	}
	return u;
}

int wtk_is_all_digit(char *data,int len)
{
	char *s,*e;
	int num;
	char c;
	int b=0;

	//wtk_debug("[%.*s]\n",len,data);
	//wtk_utf8_bytes(strs[i]->data[0])==1 && isdigit(strs[i]->data[0])
	if(len<=0){goto end;}
	s=data;e=s+len;
	while(s<e)
	{
		c=*s;
		num=wtk_utf8_bytes(c);
		if(num!=1)
		{
			goto end;
		}
		if(c<'0'|| c>'9')
		{
			goto end;
		}
		s+=num;
	}
	b=1;
end:
	return b;
}



/**
 * 	p="\xe4\xb8\x80\xe7\x99\xbe\xe4\xb8\x83\xe5\x8d\x81\xe5\x85\xad\xe4\xb8\x87\xe4\xba\x94\xe5\x8d\x83\xe4\xba\x8c\xe5\x8d\x81";
 */
int wtk_chnstr_atoi(char *data,int len,int *left)
{
	char *s,*e;
	int n=0,num;
	//wtk_chnstr_num_state_t state;
	int inc=-1;
	int t,last_t;
	//int use_unit=1;
	int last_unit=0;

	s=data;e=s+len;
	//state=WTK_CHNSTR_NUM_WAIT;
	last_t=-1;
	while(s<e)
	{
		num=wtk_utf8_bytes(*s);
		//wtk_debug("[%.*s]\n",num,s);
		t=wtk_chnstr_atoi3(s,num);
		//wtk_debug("t=%d use_unit=%d inc=%d n=%d\n",t,use_unit,inc,n);
		//wtk_debug("t=%d\n",t);
		if(t==-1){n=-1;goto end;}
		/*
		if(!use_unit)
		{
			n=n*10+t;
			s+=num;
			continue;
		}*/
		if(t<10)
		{
			/*
			if(inc>0 && n==0 && use_unit)
			{
				use_unit=0;
				n=inc*10+t;
				s+=num;
				continue;
			}*/
			if(inc>0)
			{
				n+=inc;
				goto end;
			}
			if(s>data && n==0)
			{
				goto end;
			}
			if(n>0 && t==0)
			{
				last_unit=1;
			}
			inc=t;
			//wtk_debug("inc=%d\n",inc);
		}else
		{
			//wtk_debug("inc=%d,t=%d,n=%d\n",inc,t,n);
			if(inc==-1)
			{
				if(n==0)
				{
					n=t;
				}else
				{
					goto end;
					//n*=t;
				}
			}else
			{
				if(t<last_t)
				{
					inc*=t;
					n+=inc;
				}else
				{
					n+=inc;
					n*=t;
				}
			}
			inc=-1;
			last_t=t;
		}
		//wtk_debug("t=%d,inc=%d,num=%d\n",t,inc,n);
		s+=num;
		if(s>=e && inc>0)
		{
			if(!last_unit && last_t>0)
			{
				inc=inc*last_t/10;
			}
			n+=inc;
			//wtk_debug("last_t=%d\n",last_t);
		}
	}
end:
	if(left)
	{
		*left=data-s+len;
	}
	//wtk_debug("n=%d\n",n);
	//exit(0);
	return n;
}


long long wtk_str_atoi(char* s,int len)
{
	char *p,*end,c;
	long long v;
	int sign;

	v=0;
	if(len<=0){goto end;}
	p=s;end=s+len;
	while(p<end && isspace(*p))
	{
		++p;
	}
	if(*p=='-')
	{
		++p;
		sign=-1;
	}else
	{
		sign=1;
	}
	while(p<end)
	{
		c=*p;
		if(c< '0' || c > '9')
		{
			break;
		}
		v=v*10+c-'0';
		++p;
	}
	if(sign==-1)
	{
		v=-v;
	}
end:
	return v;
}

double wtk_str_atof(char *s,int len)
{
	wtk_source_t src;
	double v;

	v=0;
	wtk_source_init_str(&(src),s,len);
	wtk_source_atof(&(src),&v);
	wtk_source_clean_str(&(src));
	return v;
}

char* wtk_data_dup(const char* data,size_t data_len,size_t alloc_len)
{
    char* p;
    size_t count;

    if(alloc_len>0)
    {
    	p=(char*)wtk_malloc(alloc_len);
    	if(data)
    	{
    		count=min(data_len,alloc_len);
    		memcpy(p,data,count);
    	}
    }else
    {
    	p=0;
    }
    return p;
}

char* wtk_data_dup2(char *data,int bytes)
{
	return wtk_data_dup(data,bytes,bytes);
}

char* wtk_data_to_str(char *data,int len)
{
	char *p;

	p=wtk_data_dup(data,len,len+1);
	p[len]=0;
	return p;
}

char* wtk_str_dup_len(const char* str,size_t len)
{
    char* p=0;

    if(str)
    {
        p=(char*)wtk_malloc(len);
        strcpy(p,str);
    }
    return p;
}

char* wtk_str_dup(const char* str)
{
    return str?wtk_str_dup_len(str,strlen(str)+1):NULL;
}

void print_data2(char* data, int len)
{
    unsigned char c;
    int i;

    for (i = 0; i < len; ++i)
    {
        c = (unsigned char) data[i];
         printf("%c", c);
    }
}

void print_char(unsigned char *data,int len)
{
	int i;

	printf("{");
	for(i=0;i<len;++i)
	{
		if(i>0)
		{
			printf(",");
		}
		if(i%32==0)
		{
			printf("\n");
		}
		printf("%#x",(unsigned int)data[i]);
	}
	printf("};\n");
}



void print_data(char* data, int len)
{
	print_data_f(stdout,data,len);
}

void print_short(short *f,int len)
{
	int i;

	wtk_debug("============== short=%p =====================\n",f);
	printf("(%d,",len);
	for(i=0;i<len;++i)
	{
		if(i>0)
		{
			printf(",");
		}
		printf("%d",f[i]);
//		if(f[i]>1000 || f[i]<-1000)
//		{
//			exit(0);
//		}
		//printf("v[%d]=%d\n",i,f[i]);
	}
	printf(")\n");
}

void print_short2(short *f,int len)
{
	float x;
	int i;

	x=0;
	for(i=0;i<len;++i)
	{
		x+=f[i];
	}
	wtk_debug("============== short=%f =====================\n",x);
	//printf("(%d,",len);
	for(i=0;i<len;++i)
	{
		printf("v[%d]=%d\n",i,f[i]);
//		if(f[i]>127 || f[i]<-127)
//		{
//			exit(0);
//		}
	}
	wtk_debug("============== short=%f =====================\n",x);
	//printf(")\n");
}


void print_char2(signed char *f,int len)
{
	int i;

	//wtk_debug("============== int =====================\n");
	printf("(char=%d,",len);
	for(i=0;i<len;++i)
	{
		if(i>0)
		{
			printf(",");
		}
		printf("%d",f[i]);
	}
	printf(")\n");
//	for(i=0;i<len;++i)
//	{
//		printf("v[%d]=%d\n",i,f[i]);
//	}
}

void print_int(int *f,int len)
{
	int i;

	//wtk_debug("============== int =====================\n");
	printf("(int=%d,",len);
	for(i=0;i<len;++i)
	{
		if(i>0)
		{
			printf(",");
		}
		printf("%d",f[i]);
	}
	printf(")\n");
//	for(i=0;i<len;++i)
//	{
//		printf("v[%d]=%d\n",i,f[i]);
//	}
}

void print_uchar(unsigned char *f,int len)
{
	int i;

	//wtk_debug("============== int =====================\n");
	printf("(uchar=%d,",len);
	for(i=0;i<len;++i)
	{
		if(i>0)
		{
			printf(",");
		}
		printf("%d",f[i]);
		if(i%20==0)
		{
			printf("\n");
		}
	}
	printf(")\n");
//	for(i=0;i<len;++i)
//	{
//		printf("v[%d]=%d\n",i,f[i]);
//	}
}

void wtk_float_print2(float **f,int n1,int n2)
{
	int i,j;

	for(i=0;i<n1;++i)
	{
		for(j=0;j<n2;++j)
		{
			if(j>0)
			{
				printf(" ");
			}
			printf("%f",f[i][j]);
		}
		printf("\n");
	}
}

void print_float(float *f,int len)
{
	int i;
	float sum=0;

	for(i=0;i<len;++i)
	{
		sum+=f[i];
	}
	wtk_debug("============== float=%f =====================\n",sum);
	for(i=0;i<len;++i)
	{
		printf("v[%d]=%.6f\n",i,f[i]);
		//printf("v[%d]=%e\n",i,f[i]);
	}
	wtk_debug("============== float=%f =====================\n",sum);
}

void print_float2(float *f,int len)
{
	int i;

	printf("(%d,",len);
	for(i=0;i<len;++i)
	{
		if(i>0)
		{
			printf(" ");
		}
		//printf("%.6f",f[i]);
		printf("%.6f",f[i]);
	}
	printf("\n");
}

void float_nan_check(float *f,int len)
{
	int i;

	for(i=0;i<len;++i)
	{
		if(isnan(f[i]))
		{
			wtk_debug("found bug[%d]=%f\n",i,f[i]);
			exit(0);
		}
	}
}

void print_double(double *f,int len)
{
	int i;
	double t;

	t=0;
	wtk_debug("============== float =====================\n");
	for(i=0;i<len;++i)
	{
		t+=f[i];
		printf("v[%d]=%.12f\n",i,f[i]);
		//printf("v[%d]=%e\n",i,f[i]);
		//printf("%.12f\n",f[i]);
	}
	wtk_debug("tot=%f\n",t);
}

void print_double2(double *f,int len)
{
	int i;

	//wtk_debug("============== float =====================\n");
	for(i=0;i<len;++i)
	{
		if(i>0)
		{
			printf(" ");
		}
		printf("%.12f",f[i]);
	}
	printf("\n");
}


void print_double3(double *f,int len)
{
	int i;

	wtk_debug("============== float =====================\n");
	for(i=0;i<len;++i)
	{
		if(f[i]!=0)
		{
			printf("v[%d]=%.3f\n",i,f[i]);
		}
	}
	//printf("\n");
}


void print_data_f(FILE* f,char* data, int len)
{
	print_data_f2(f,data,len,0);
}

void print_hex(char *data,int len)
{
	int i;

	printf("(%d,",len);
	for (i = 0; i < len; ++i)
	{
		printf("\\x%02x", (unsigned char) data[i]);
	}
	printf(")\n");
}

void print_hex2(char *data,int len)
{
	int i;

	//printf("(%d,",len);
	for (i = 0; i < len; ++i)
	{
		printf("\\x%02x", (unsigned char) data[i]);
	}
	//printf(")\n");
}

void print_data_f2(FILE* f,char* data, int len,int cn)
{
	unsigned char c;
	int i;

	fprintf(f,"(%d,", len);
	for (i = 0; i < len; ++i)
	{
		c = (unsigned char) data[i];
		if (isprint(c))
		{
			fprintf(f,"%c", c);
		}
		else
		{
			fprintf(f,"\\x%02x", c);
		}
		if(cn && (c=='\n'))
		{
			fprintf(f,"\n");
		}
	}
	fprintf(f,")\n");
}

int zero_dispose(void *data)
{
    if(data){free(data);}
    return 0;
}

char* str_merge(char* arg1,...)
{
    va_list ap;
    char *p,*result;
    int count=0;

    //get data length.
    va_start(ap,arg1);
    count=0;result=0;
    p=arg1;
    while(p)
    {
        count+=strlen(p);
        p=va_arg(ap,char*);
    }
    va_end(ap);
    if(count<=0){goto end;}

    //copy string
    result=(char*)wtk_calloc(1,count+1);
    va_start(ap,arg1);
    p=arg1;
    while(p)
    {
        strcat(result,p);
        p=va_arg(ap,char*);

    }
    va_end(ap);
end:
    return result;
}

char * wtk_str_found(char *src,char *key,int key_bytes)
{
    char *p;
    int i;

    p=src;i=0;
    while(*p)
    {
        if(*p==key[i])
        {
            i+=1;
            if(i>=key_bytes)
            {
                ++p;
                break;
            }
        }else
        if(i>0)
        {
            i=0;
            continue;
        }
        ++p;
    }
    return (*p) ? p : 0;
}

char* wtk_str_chr(char* s,int slen,char c)
{
	char *p=0;
	char *e=s+slen;

	while(s<e)
	{
		if(*s==c)
		{
			p=s;
			break;
		}
		++s;
	}
	return p;
}

char* wtk_str_rchr(char *s,int len,char c)
{
	char *e;

	e=s+len-1;
	while(e>=s)
	{
		if(*e==c)
		{
			return e;
		}
		--e;
	}
	return NULL;
}

int wtk_str_str4(char *haystack,int haystack_len,char *needle,int needle_len)
{
	int buf[32];
	int i,j;
    int *next;
    int b;

    if(needle_len<=0)
    {
    	return -1;
    }else if(needle_len<32)
    {
    	b=0;
    	next=buf;
    }else
    {
    	b=1;
    	next=(int*)wtk_malloc(sizeof(int)*(needle_len));
    }
    next[0] = -1;
    for (i = 1, j = -1; i < needle_len; ++i)
    {
        while (j > -1 && needle[j + 1] != needle[i])
        {
        	j = next[j];
        }
        if (needle[j + 1] == needle[i])
        {
        	j++;
        }
        next[i] = j;
    }

    for (i = 0, j = -1; i<haystack_len; ++i)
    {
        while (j > -1 && needle[j + 1] != haystack[i])
        {
        	j = next[j];
        }
        if (needle[j + 1] == haystack[i])
        {
        	j++;
        }
        if (j == needle_len - 1)
        {
        	if(b)
        	{
        		wtk_free(next);
        	}
        	return  i + 1 - needle_len;
        }
    }
    if(b)
    {
    	wtk_free(next);
    }
    return -1;
}

int wtk_str_str3(char *src,int src_len,char *sub,int sub_len)
{
	register char *s,*e,*ps,*pe,*pa;
	char c;

	//wtk_debug("sub_len=%d\n",sub_len);
	s=src;
	if(sub_len==1)
	{
		c=*sub;
		e=s+src_len;
		while(s<e)
		{
			if(*(s++)==c)
			{
				return s-src-1;
			}
		}
	}else
	{
		e=src+src_len-sub_len+1;
		pe=sub+sub_len;
		ps=sub;
		while(s<e)
		{
			if(*s==*ps)
			{
				pa=s+1;
				++ps;
				while(ps<pe &&(*(ps)==*(pa)))
				{
					++ps;++pa;
				}
				if(ps==pe)
				{
					return s-src;
				}
				ps=sub;
				++s;
			}else
			{
				++s;
			}
		}
	}
	return -1;
}

int wtk_str_str(char *src,int src_len,char *sub,int sub_len)
{
	register char *s,*e,c;

	//wtk_debug("sub_len=%d\n",sub_len);
	s=src;
	c=*sub;
	switch(sub_len)
	{
	case 1:
		e=s+src_len;
		while(s<e)
		{
			if(*(s++)==c)
			{
				return s-src-1;
			}
		}
		break;
	case 2:
		e=s+src_len-1;
		while(s<e)
		{
			if(s[0]==c && s[1]==sub[1])
			{
				//wtk_debug("[%.*s]\n",2,s);
				return s-src;
			}else
			{
				if(s[1]!=c)
				{
					s+=2;
				}else
				{
					if(s[2]==sub[1])
					{
						//wtk_debug("[%.*s]\n",2,s+1);
						return s+1-src;
					}else
					{
						s+=3;
					}
				}
			}
		}
		break;
	case 3:
		e=s+src_len-2;
		while(s<e)
		{
			if(s[0]==c && s[1]==sub[1] && s[2]==sub[2])
			{
				return s-src;
			}else
			{
				if(s[1]!=c)
				{
					s+=2;
				}else
				{
					++s;
				}
			}
		}
		break;
	case 4:
		e=s+src_len-2;
		while(s<e)
		{
			if(s[0]==c && s[1]==sub[1] && s[2]==sub[2] && s[3]==sub[3])
			{
				return s-src;
			}else
			{
				if(s[1]!=c)
				{
					s+=2;
				}else
				{
					if(s[1]!=c)
					{
						s+=2;
					}else
					{
						++s;
					}
				}
			}
		}
		break;
	default:
		if(sub_len>2)
		{
			e=s+src_len-sub_len+1;
			sub+=2;
			sub_len-=2;
			while(s<e)
			{
				if(s[0]==c && s[1]==sub[-1] && strncmp(s+2,sub,sub_len)==0)
				{
					return s-src;
				}else
				{
					if(s[1]!=c)
					{
						s+=2;
					}else
					{
						++s;
					}
				}
			}
		}
		break;
	}
//	if(sub_len>2)
//	{
//		c2=sub[1];
//		e=s+src_len-sub_len+1;
//		sub+=2;
//		sub_len-=2;
//		while(s<e)
//		{
//			if(s[0]==c && s[1]==c2 && strncmp(s+2,sub,sub_len)==0)
//			{
//				return s-src;
//			}else
//			{
//				if(s[1]!=c)
//				{
//					s+=2;
//				}else
//				{
//					++s;
//				}
//			}
//		}
//	}else if(sub_len==1)
//	{
//		e=s+src_len;
//		while(s<e)
//		{
//			if(*(s++)==c)
//			{
//				return s-src-1;
//			}
//		}
//	}else if(sub_len==2)
//	{
//		c2=sub[1];
//		e=s+src_len-1;
//		while(s<e)
//		{
//			if(s[0]==c && s[1]==c2)
//			{
//				return s-src;
//			}else
//			{
//				++s;
//			}
//		}
//	}
	return -1;
}

int wtk_str_str_x(char *src,int src_len,char *sub,int sub_len)
{
	register char *s,*e,c;

	s=src;e=s+src_len-sub_len+1;
	//wtk_debug("[%.*s]\n",(int)(e-s),s);
	c=*sub;
	if(sub_len==1)
	{
		while(s<e)
		{
			if(*(s++)==c)
			{
				return s-src-1;
			}
		}
	}else
	{
		++sub;--sub_len;
		while(s<e)
		{
			if(*(s++)==c && strncmp(s,sub,sub_len)==0)
			{
				return	s-src-1;
			}
		}
	}
	return -1;
}


int wtk_str_str2(char *src,int src_len,char *sub,int bytes)
{
	char *s,*e;
	int i,index;

	//print_data(src,src_len);
	//print_data(sub,bytes);
	s=sub;e=s+bytes;
	for(i=0,index=0;i<src_len;++i)
	{
		//wtk_debug("i=%d,index=%d,[%c]\n",i,index,*s);
		if(src[i]==*s)
		{
			++s;
			if(s>=e)
			{
				return index;
			}
		}else
		{
			//wtk_debug("i=%d,len=%d\n",i,s-sub);
			i=i-(s-sub);
			index=i+1;
			s=sub;
		}
	}
	return -1;
}


int wtk_string_str(wtk_string_t *str,char *sub,int bytes)
{
	return wtk_str_str(str->data,str->len,sub,bytes);
}

int wtk_char_to_hex(char c)
{
	int v;

	if(c>='0' && c<='9')
	{
		v=c-'0';
	}else if(c>='A' && c<='F')
	{
		v=c-'A'+10;
	}else if(c>='a' && c<='f')
	{
		v=c-'a'+10;
	}else
	{
		v=-1;
	}
	return v;
}

uint32_t hash_string_value(char* name)
{
	uint32_t hashval;

	hashval = 0;
	if (name)
	{
		for (; *name != 0; ++name)
		{
			hashval = *name + (hashval << 4) - 1;
		}
	}
	return hashval;
}

uint32_t hash_string(char* name, uint32_t hash_size)
{
	uint32_t hashval;

	hashval = hash_string_value(name);
	return hashval % hash_size;
}


uint32_t hash_string_value_len2(char* data,int len,int hash_size)
{
	uint32_t hashval;
	char *p,*end;

	hashval = 0;
	p=data;end=p+len;
	while(p<end)
	{
		hashval=*(p++)+(hashval<<4)-1;
	}
	return hashval % hash_size;
}

uint32_t hash_string_value_len_seed(unsigned char* p,int len,int hash_size)
{
    //static unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
	uint32_t hashval=0;
	unsigned char *e;

	//wtk_debug("[%.*s]=%d\n",len,p,hash_size);
	e=p+len;
	while(p<e)
	{
		hashval = (hashval*131) + (*p++);
	}
	return (hashval & 0x7FFFFFFF) % hash_size;
}

char* wtk_itoa(int n)
{
	char buf[64];
	int len;

	len=sprintf(buf,"%d",n);
	return wtk_data_to_str(buf,len);
}


int wtk_str_split(char *data,int len,char sep,void *split_ths,wtk_str_split_f split)
{
	char *s,*e;
	char *prev;
	int index=0;

	prev=data;
	s=data;e=s+len;
	while(s<=e)
	{
		if(s==e||*s==sep)
		{
			if(s>prev)
			{
				split(split_ths,prev,s-prev,index++);
				//wtk_debug("[%.*s]\n",(int)(s-prev),prev);
			}
			prev=s+1;
			//wtk_debug("%c\n",*s);
		}
		++s;
	}
	return index;
}


int wtk_str_split2(char *data,int len,void *split_ths,wtk_str_split_f split,wtk_str_split_is_sep_f is_sep)
{
	char *s,*e;
	char *prev;
	int index=0;

	prev=data;
	s=data;e=s+len;
	while(s<=e)
	{
		if(s==e||is_sep(split_ths,*s))
		{
			if(s>prev)
			{
				split(split_ths,prev,s-prev,index++);
				//wtk_debug("[%.*s]\n",(int)(s-prev),prev);
			}
			prev=s+1;
			//wtk_debug("%c\n",*s);
		}
		++s;
	}
	return index;
}

int wtk_str_attr_parse2(char *data,int bytes,void *ths,wtk_str_attr_f attr_notify,int *consume)
{
typedef enum
{
	WTK_STR_WAIT_BRACE,
	WTK_STR_WAIT_KEY,
	WTK_STR_KEY,
	WTK_STR_WAIT_EQ,
	WTK_STR_WAIT_VALUE,
	WTK_STR_VALUE,
	WTK_STR_ESC_VALUE,
	WTK_STR_WAIT_COMMA,
	WTK_STR_END,
	WTK_STR_QUOTE_ESC,
}wtk_str_attr_state_t;
	char *s,*e;
	wtk_str_attr_state_t state;
	char c;
	int num;
	wtk_string_t k,v;
	char end_c=0;
	wtk_strbuf_t *buf;

	buf=wtk_strbuf_new(256,1);
	wtk_string_set(&(k),0,0);
	wtk_string_set(&(v),0,0);
	state=WTK_STR_WAIT_BRACE;
	s=data;e=s+bytes;
	while(s<e)
	{
		c=*s;
		num=wtk_utf8_bytes(c);
		//wtk_debug("[%.*s]\n",num,s);
		switch(state)
		{
		case WTK_STR_WAIT_BRACE:
			if(num==1 && (c=='[' ||c=='{'))
			{
				switch(c)
				{
				case '[':
					end_c=']';
					break;
				case '{':
					end_c='}';
					break;
				}
				state=WTK_STR_WAIT_KEY;
				break;
			}else
			{
				s-=num;
				end_c=-1;
				state=WTK_STR_WAIT_KEY;
			}
			break;
		case WTK_STR_WAIT_KEY:
			if(num!=1 || !isspace(c))
			{
				if(c==end_c)
				{
					state=WTK_STR_END;
					goto end;
				}
				k.data=s;
				state=WTK_STR_KEY;
			}
			break;
		case WTK_STR_KEY:
			if(num==1 && (isspace(c) || c==','||c==end_c||c=='='))
			{
				k.len=s-k.data;
				if(c=='=')
				{
					state=WTK_STR_WAIT_VALUE;
				}else if(c==',')
				{
					attr_notify(ths,&k,0);
					state=WTK_STR_WAIT_KEY;
				}else if(c==end_c)
				{
					attr_notify(ths,&k,0);
					state=WTK_STR_END;
					goto end;
				}else
				{
					state=WTK_STR_WAIT_EQ;
				}
			}
			break;
		case WTK_STR_WAIT_EQ:
			if(num==1 && (c=='='||c==','||c==end_c))
			{
				if(c=='=')
				{
					state=WTK_STR_WAIT_VALUE;
				}else if(c==',')
				{
					attr_notify(ths,&k,0);
					state=WTK_STR_WAIT_KEY;
				}else if(c==end_c)
				{
					attr_notify(ths,&k,0);
					state=WTK_STR_END;
					goto end;
				}
			}
			break;
		case WTK_STR_WAIT_VALUE:
			if(num!=1 || !isspace(c))
			{
				if(num==1 && c=='\"')
				{
					//v.data=s+1;
					wtk_strbuf_reset(buf);
					state=WTK_STR_ESC_VALUE;
				}else
				{
					v.data=s;
					state=WTK_STR_VALUE;
				}
				break;
			}
			break;
		case WTK_STR_ESC_VALUE:
			if(num==1)
			{
				if(c=='\\')
				{
					state=WTK_STR_QUOTE_ESC;
				}else if(c=='\"')
				{
					//v.len=s-v.data;
					v.data=buf->data;
					v.len=buf->pos;
					attr_notify(ths,&k,&v);
					state=WTK_STR_WAIT_COMMA;
				}else
				{
					wtk_strbuf_push(buf,s,num);
				}
			}else
			{
				wtk_strbuf_push(buf,s,num);
			}
			break;
		case WTK_STR_QUOTE_ESC:
			wtk_strbuf_push(buf,s,num);
			state=WTK_STR_ESC_VALUE;
			break;
		case WTK_STR_VALUE:
			if(num==1 && (isspace(c) || c==','||c==end_c))
			{
				v.len=s-v.data;
				//wtk_debug("[%d/%d]\n",)
				if(v.data[0]=='\"' && v.data[v.len-1]=='\"')
				{
					v.data+=1;
					v.len-=2;
				}
				if(c==',')
				{
					attr_notify(ths,&k,&v);
					state=WTK_STR_WAIT_KEY;
				}else if(c==end_c)
				{
					attr_notify(ths,&k,&v);
					state=WTK_STR_END;
					goto end;
				}else
				{
					attr_notify(ths,&k,&v);
					state=WTK_STR_WAIT_COMMA;
				}
			}else if(s+num>=e)
			{
				v.len=e-v.data;
				//wtk_debug("[%d/%d]\n",)
				if(v.data[0]=='\"' && v.data[v.len-1]=='\"')
				{
					v.data+=1;
					v.len-=2;
				}
				attr_notify(ths,&k,&v);
				state=WTK_STR_WAIT_COMMA;
			}
			break;
		case WTK_STR_WAIT_COMMA:
			if(num==1)
			{
				if(c==',')
				{
					state=WTK_STR_WAIT_KEY;
				}else if(c==end_c)
				{
					state=WTK_STR_END;
				}else if(!isspace(c))
				{
					state=WTK_STR_KEY;
					k.data=s;
				}
			}else
			{
				state=WTK_STR_KEY;
				k.data=s;
			}
			break;
		case WTK_STR_END:
			break;
		}
		s+=num;
	}
end:
	wtk_strbuf_delete(buf);
	if(consume)
	{
		*consume=s-data;
	}
	//exit(0);
	return 0;
}

int wtk_str_attr_parse(char *data,int bytes,void *ths,wtk_str_attr_f attr_notify)
{
	return wtk_str_attr_parse2(data,bytes,ths,attr_notify,NULL);
}


void wtk_str_parse_chnwrd(char *data,int len,void *ths,wtk_str_chnwrd_f wrd)
{
	char *s,*e;
	int n;
	int is_chn=1;
	wtk_string_t eng;

	s=data;e=s+len;
	while(s<e)
	{
		n=wtk_utf8_bytes(*s);
		if(is_chn)
		{
			if(n>1)
			{
				wrd(ths,s,n);
				//wtk_debug("[%.*s]\n",n,s);
			}else if(!isspace(*s))
			{
				eng.data=s;
				is_chn=0;
			}
		}else
		{
			if(n>1)
			{
				is_chn=1;
				eng.len=s-eng.data;
				wrd(ths,eng.data,eng.len);
				wrd(ths,s,n);
				//wtk_debug("[%.*s]\n",eng.len,eng.data);
				//wtk_debug("[%.*s]\n",n,s);
			}else if(isspace(*s))
			{
				is_chn=1;
				eng.len=s-eng.data;
				wrd(ths,eng.data,eng.len);
				//wtk_debug("[%.*s]\n",eng.len,eng.data);
			}else if(s+n>=e)
			{
				is_chn=1;
				eng.len=e-eng.data;
				wrd(ths,eng.data,eng.len);
				//wtk_debug("[%.*s]\n",eng.len,eng.data);
			}
		}
		s+=n;
	}
}

void wtk_str_chnwrd_iter_init(wtk_str_chnwrd_iter_t *iter,char *data,int len)
{
	iter->s=data;
	iter->e=data+len;
}

wtk_string_t wtk_str_chnwrd_iter_next(wtk_str_chnwrd_iter_t *iter)
{
	wtk_string_t v;
	int is_chn=1;
	int n;

	while(iter->s<iter->e)
	{
		n=wtk_utf8_bytes(*iter->s);
		if(is_chn)
		{
			if(n>1)
			{
				wtk_string_set(&(v),iter->s,n);
				iter->s+=n;
				return v;
			}else if(!isspace(*iter->s))
			{
				v.data=iter->s;
				is_chn=0;
			}
		}else
		{
			if(n>1)
			{
				v.len=iter->s-v.data;
				return v;
			}else if(isspace(*iter->s))
			{
				v.len=iter->s-v.data;
				iter->s+=n;
				return v;
			}else if(iter->s+n>=iter->e)
			{
				v.len=iter->e-v.data;
				iter->s+=n;
				return v;
			}
		}
		iter->s+=n;
	}
	wtk_string_set(&(v),0,0);
	return v;
}

void wtk_str_spwrd_iter_init(wtk_str_spwrd_iter_t *iter,char *data,int len)
{
	iter->s=data;
	iter->e=data+len;
}

wtk_string_t wtk_str_spwrd_iter_next(wtk_str_spwrd_iter_t *iter)
{
	wtk_string_t v;
	int init=1;
	int n;

	while(iter->s<iter->e)
	{
		n=wtk_utf8_bytes(*iter->s);
		//wtk_debug("[%.*s]=%d %d\n",n,iter->s,init,(int)(iter->e-iter->s));
		if(init)
		{
			if(n>1 || !isspace(*iter->s))
			{
				v.data=iter->s;
				if(iter->s+n>=iter->e)
				{
					v.len=iter->e-v.data;
					iter->s+=n;
					goto end;
				}else
				{
					init=0;
				}
			}
		}else
		{
			if(n==1 && isspace(*iter->s))
			{
				v.len=iter->s-v.data;
				iter->s+=n;
				goto end;
			}else if(iter->s+n>=iter->e)
			{
				v.len=iter->e-v.data;
				iter->s=iter->e;
				goto end;
			}
		}
		iter->s+=n;
	}
	wtk_string_set(&(v),0,0);
end:
	//wtk_debug("[%.*s]\n",v.len,v.data);
	return v;
}
