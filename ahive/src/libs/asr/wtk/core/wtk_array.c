#include "wtk_array.h"

wtk_array_t* wtk_array_new_h(wtk_heap_t* h,uint32_t n,uint32_t size)
{
	wtk_array_t* a;

	a=(wtk_array_t*)wtk_heap_malloc(h,sizeof(*a));
	a->slot_alloc=n;
	a->slot_size=size;
	a->nslot=0;
	a->heap=h;
	a->slot=wtk_heap_malloc(h,n*size);
	return a;
}

int wtk_array_dispose(wtk_array_t* a)
{
	wtk_heap_free(a->heap,a);
	return 0;
}

void wtk_array_reset(wtk_array_t *a)
{
	a->nslot=0;
}

void* wtk_array_push_n(wtk_array_t* a,uint32_t n)
{
	uint32_t alloc;
	void *s;

	if(a->nslot+n > a->slot_alloc)
	{
		alloc=2*max(n,a->slot_alloc);
		s=wtk_heap_malloc(a->heap,alloc*a->slot_size);
		memcpy(s,a->slot,a->slot_size*a->nslot);
		a->slot=s;
		a->slot_alloc=alloc;
	}
	s=(char*)a->slot+a->slot_size*a->nslot;
	a->nslot+=n;
	return s;
}

void* wtk_array_push(wtk_array_t* a)
{
	return wtk_array_push_n(a,1);
}

void wtk_array_push2(wtk_array_t *a,void *src)
{
	void *dst;

	dst=wtk_array_push_n(a,1);
	//print_data(dst,4);
	memcpy(dst,src,a->slot_size);
}

/*
void wtk_array_append(wtk_array_t *a,void *v)
{
	void *p;

	p=wtk_array_push_n(a,1);
	memcpy(p,v,a->slot_size);
}
*/

void wtk_array_print_int(wtk_array_t *a)
{
	int i;
	int *v;

	v=(int*)a->slot;
	for(i=0;i<a->nslot;++i)
	{
		printf("v[%d]=%d\n",i,v[i]);
	}
}

void wtk_array_print_float(wtk_array_t *a)
{
	int i;
	float *v;

	v=(float*)a->slot;
	for(i=0;i<a->nslot;++i)
	{
		printf("v[%d]=%f\n",i,v[i]);
	}
}

void wtk_array_print_string(wtk_array_t *a)
{
	int i;
	wtk_string_t **v;

	v=(wtk_string_t**)a->slot;
	for(i=0;i<a->nslot;++i)
	{
		wtk_debug("v[%d]=%.*s\n",i,v[i]->len,v[i]->data);
	}
}

int wtk_array_str_in(wtk_array_t *a,char *s,int s_bytes)
{
	wtk_string_t **strs;
	int i;

	if(!a){return 0;}
	strs=(wtk_string_t**)a->slot;
	for(i=0;i<a->nslot;++i)
	{
		if(wtk_string_cmp(strs[i],s,s_bytes)==0)
		{
			return 1;
		}
	}
	return 0;
}

int wtk_array_str_has(wtk_array_t *a,char *s,int s_bytes)
{
	wtk_string_t **strs;
	int i;
	int pos;

	if(!a){return 0;}
	strs=(wtk_string_t**)a->slot;
	for(i=0;i<a->nslot;++i)
	{
		//		if(wtk_string_cmp(strs[i],s,s_bytes)==0)
		//		{
		//			return 1;
		//		}
		pos=wtk_str_str(s,s_bytes,strs[i]->data,strs[i]->len);
		if(pos>=0)
		{
			return 1;
		}
	}
	return 0;
}

int wtk_array_str_end_with(wtk_array_t *a,char *s,int s_bytes)
{
	wtk_string_t **strs;
	int i;

	if(!a){return 0;}
	//wtk_debug("[%.*s]\n",s_bytes,s);
	strs=(wtk_string_t**)a->slot;
	for(i=0;i<a->nslot;++i)
	{
		//wtk_debug("[%.*s]=[%.*s]\n",strs[i]->len,strs[i]->data,strs[i]->len,s+s_bytes-strs[i]->len);
		if(s_bytes>=strs[i]->len && (wtk_string_cmp(strs[i],s+s_bytes-strs[i]->len,strs[i]->len)==0))
		{
			return 1;
		}
	}
	return 0;
}

int wtk_array_str_start_with(wtk_array_t *a,char *s,int s_bytes)
{
	wtk_string_t **strs;
	int i;

	if(!a){return 0;}
	//wtk_debug("[%.*s]\n",s_bytes,s);
	strs=(wtk_string_t**)a->slot;
	for(i=0;i<a->nslot;++i)
	{
		//wtk_debug("[%.*s]=[%.*s]\n",strs[i]->len,strs[i]->data,strs[i]->len,s+s_bytes-strs[i]->len);
		if(s_bytes>=strs[i]->len && (wtk_string_cmp(strs[i],s,strs[i]->len)==0))
		{
			return 1;
		}
	}
	return 0;
}

void wtk_str_to_array_split(wtk_array_t *a,char *item,int len,int index)
{
	wtk_string_t *v;

	v=wtk_heap_dup_string(a->heap,item,len);
	wtk_array_push2(a,&v);
}

wtk_array_t* wtk_str_to_array(wtk_heap_t *heap,char *data,int bytes,char sep)
{
	wtk_array_t *a;

	a=wtk_array_new_h(heap,bytes/2,sizeof(wtk_string_t*));
	wtk_str_split(data,bytes,sep,a,(wtk_str_split_f)wtk_str_to_array_split);
	return a;
}

#include <ctype.h>
#include "wtk/core/wtk_str_encode.h"

int wtk_str_to_array_is_sep(wtk_array_t *a,char c)
{
	return isspace(c);
}

wtk_array_t* wtk_str_to_array2(wtk_heap_t *heap,char *data,int bytes)
{
	wtk_array_t *a;

	a=wtk_array_new_h(heap,bytes/2,sizeof(wtk_string_t*));
	wtk_str_split2(data,bytes,a,(wtk_str_split_f)wtk_str_to_array_split,
			(wtk_str_split_is_sep_f)wtk_str_to_array_is_sep);
	return a;
}

wtk_array_t* wtk_str_to_chars(wtk_heap_t *heap,char *data,int bytes)
{
	char *s,*e;
	int len;
	wtk_array_t *a;
	wtk_string_t *v;

	a=wtk_array_new_h(heap,bytes*2/3,sizeof(wtk_string_t*));
	s=data;e=s+bytes;
	while(s<e)
	{
		len=wtk_utf8_bytes(*s);
		v=wtk_heap_dup_string(heap,s,len);
		wtk_array_push2(a,&v);
		s+=len;
	}
	return a;
}
