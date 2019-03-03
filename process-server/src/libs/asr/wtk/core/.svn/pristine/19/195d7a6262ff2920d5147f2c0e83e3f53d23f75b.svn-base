#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "wtk_param.h"
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

static char* wtk_param_data_dup(const char* data,size_t data_len,size_t alloc_len)
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


wtk_param_t* wtk_param_new(wtk_paramtype_t type)
{
	wtk_param_t *p;

	p=(wtk_param_t*)wtk_malloc(sizeof(*p));
	p->is_ref=0;
	p->type=type;
	p->free=free;
	return p;
}

wtk_param_t* wtk_param_dup(wtk_param_t *src)
{
	wtk_param_t *dst=0;
	int i,len;

	if(!src){goto end;}
	if(src->is_ref){dst=src;goto end;}
	switch(src->type)
	{
	case WTK_NIL:
		dst=wtk_param_new(src->type);
		break;
	case WTK_BIN:
		dst=wtk_param_new(src->type);
		if(src->value.bin.is_ref)
		{
			dst->value=src->value;
		}else
		{
			len=src->value.bin.len;
			dst->value.bin.data=wtk_param_data_dup(src->value.bin.data,len,len);
			dst->value.bin.len=len;
			dst->value.bin.is_ref=0;
		}
		break;
	case WTK_NUMBER:
		dst=wtk_param_new(src->type);
		dst->value.number=src->value.number;
		break;
	case WTK_STRING:
		dst=wtk_param_new(src->type);
		if(src->value.str.is_ref)
		{
			dst->value=src->value;
		}else
		{
			len=src->value.str.len;
			dst->value.str.data=wtk_param_data_dup(src->value.str.data,len,len);
			dst->value.str.len=len;
			dst->value.str.is_ref=0;
		}
		break;
	case WTK_OCT:
		dst=wtk_param_new(src->type);
		if(src->value.bin.is_ref)
		{
			dst->value = src->value;
		}else
		{
			len=src->value.bin.len;
			dst->value.bin.data=wtk_param_data_dup(src->value.bin.data,len,len);
			dst->value.bin.len=len;
			dst->value.bin.is_ref=0;
		}
		break;
	case WTK_ARRAY:
		len=src->value.array.len;
		dst=wtk_param_new_array(len);
		for(i=0;i<len;++i)
		{
			dst->value.array.params[i]=wtk_param_dup(src->value.array.params[i]);
		}
		break;
	}
end:
	return dst;
}

int wtk_param_bytes(wtk_param_t *p)
{
	int i;
	int b=sizeof(*p);

    if (NULL == p) return 0;

	switch(p->type)
	{
	case WTK_BIN:
		b+=p->value.bin.len;
		break;
	case WTK_OCT:
		b+=p->value.bin.len;
		break;
	case WTK_STRING:
		b+=p->value.str.len;
		break;
	case WTK_ARRAY:
		for(i=0;i<p->value.array.len;++i)
		{
			b+=wtk_param_bytes(p->value.array.params[i]);
		}
		break;
	default:
		break;
	}
	return b;
}

int wtk_param_delete(wtk_param_t* p)
{
    //param_print(param);

	if(p && !p->is_ref)
	{
		wtk_param_clean(p);
		if(p->free)
		{
			p->free(p);
		}
	}
    return 0;
}

int wtk_param_clean(wtk_param_t* p)
{
	int i;

    if(p && p->free)
    {
    	switch(p->type)
    	{
    	case WTK_BIN:
            if(p->value.bin.data && p->value.bin.is_ref==0)
            {
                p->free(p->value.bin.data);
                p->value.bin.data=0;
                p->value.bin.len=0;
            }
    		break;
    	case WTK_OCT:
            if(p->value.bin.data && p->value.bin.is_ref==0)
            {
                p->free(p->value.bin.data);
                p->value.bin.data=0;
                p->value.bin.len=0;
            }
    		break;
    	case WTK_STRING:
            if(p->value.str.data && p->value.str.is_ref==0)
            {
                p->free(p->value.str.data);
                p->value.str.data=0;
            }
    		break;
    	case WTK_ARRAY:
    		for(i=0;i<p->value.array.len;++i)
    		{
    			wtk_param_delete(p->value.array.params[i]);
    		}
    		p->free(p->value.array.params);
    		break;
    	default:
    		break;
    	}
    }
    return 0;
}

wtk_param_t* wtk_param_new_bin(char* data,int len) //,wtk_heap_t *heap)
{
	wtk_param_t *p;

	p=wtk_param_new(WTK_BIN);//,heap);
	p->value.bin.data=wtk_param_data_dup(data,len,len);//,heap);
	p->value.bin.len=len;
	p->value.bin.is_ref=0;
	return p;
}

wtk_param_t* wtk_param_new_bin2(char* data,int len)//,wtk_heap_t *heap)
{
	wtk_param_t *p;

	p=wtk_param_new(WTK_BIN);//,heap);
	p->value.bin.data=data;
	p->value.bin.len=len;
	p->value.bin.is_ref=0;
	return p;
}

wtk_param_t* wtk_param_new_oct(char* data, int len)
{
	wtk_param_t* p;

	p = wtk_param_new_bin(data,len);//,heap);
	p->type = WTK_OCT;

	return p;
}

wtk_param_t* wtk_param_new_str(char* data,int len) //,wtk_heap_t *heap)
{
	wtk_param_t* p;

	p=wtk_param_new_bin(data,len);//,heap);
	p->type=WTK_STRING;
	return p;
}

wtk_param_t* wtk_param_new_number(double number) //,wtk_heap_t *heap)
{
	wtk_param_t *p;

	p=wtk_param_new(WTK_NUMBER);//,heap);
	p->value.number=number;
	return p;
}

void wtk_param_set_ref_number(wtk_param_t *param,int v)
{
	param->free=0;
	param->is_ref=1;
	param->type=WTK_NUMBER;
	param->value.number=v;
}

void wtk_param_set_ref_str(wtk_param_t *p,char *msg,int msg_bytes)
{
	p->type=WTK_STRING;
	p->is_ref=1;
	p->value.str.is_ref=1;
	p->value.str.data=msg;
	p->value.str.len=msg_bytes;
}

wtk_param_t* wtk_param_new_array(int len) //,wtk_heap_t *heap)
{
	wtk_param_t *p;

	p=wtk_param_new(WTK_ARRAY);//,heap);
	p->value.array.params=(wtk_param_t**)wtk_calloc(len,sizeof(wtk_param_t*));
	/*
	if(heap)
	{
		p->value.array.params=(wtk_param_t**)wtk_heap_zalloc(heap,len*sizeof(wtk_param_t*));
	}else
	{
		p->value.array.params=(wtk_param_t**)wtk_calloc(len,sizeof(wtk_param_t*));
	}
	*/
	p->value.array.len=len;
	return p;
}

void wtk_param_print(wtk_param_t* p)
{
	int i;

	//printf("type: %d; \n",p->type);
	switch(p->type)
	{
	case WTK_NIL:
		printf("NIL: \n");
		break;
	case WTK_BIN:
		printf("BIN:\t %d\n",p->value.bin.len);
		//print_data((char*)param->value.bin.data,param->value.bin.len);
		break;
	case WTK_OCT:
		printf("STREAM:\t %d\n",p->value.bin.len);
		//print_data((char*)param->value.bin.data,param->value.bin.len);
		break;
	case WTK_STRING:
		printf("STRING:\t");
		i=p->value.str.len;
		if(i>1024)
		{
			i=1024;
			printf("%*.*s ...\n",i,i,p->value.str.data);
		}else
		{
			printf("%*.*s\n",i,i,p->value.str.data);
		}
		//printf("%d\n",param->value.str.len);
		break;
	case WTK_NUMBER:
		printf("Number:\t");
		printf("%f\n",p->value.number);
		break;
	case WTK_ARRAY:
		printf("Array:\n");
		for(i=0;i<p->value.array.len;++i)
		{
			wtk_param_print(p->value.array.params[i]);
		}
		break;
	}
}
