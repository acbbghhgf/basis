#include "wtk_vector.h"

wtk_type_vector_new_imp(double);
wtk_type_vector_new_imp(short);
wtk_type_vector_newh_imp(short);
wtk_type_vector_newh_imp(double);

wtk_vector_t* wtk_vector_new_h(wtk_heap_t *h,int size)
{
	wtk_vector_t* v;

	v=(wtk_vector_t*)wtk_heap_malloc(h,wtk_vector_bytes(size));
	wtk_vector_init(v,size);
	return v;
}

wtk_int_vector_t* wtk_int_vector_new_h(wtk_heap_t *h,int size)
{
	wtk_int_vector_t *v;

	v=(wtk_int_vector_t*)wtk_heap_malloc(h,wtk_int_vector_bytes(size));
	wtk_vector_init(v,size);
	return v;
}

//#include <stdlib.h>
//#include <malloc.h>

wtk_vector_t *wtk_vector_new(int size)
{
	wtk_vector_t *v;

	v=(wtk_vector_t*)wtk_calloc(1,wtk_vector_bytes(size));
	wtk_vector_init(v,size);
	return v;
}

void wtk_vector_delete2(wtk_vector_t *v)
{
	int of;
	char *p;

	of=*(((int*)v)-1);
	p=(char*)v-of;
//	wtk_debug("p=%p\n",p);
	wtk_free(p);
}

wtk_vector_t *wtk_vector_new2(int size)
{
	wtk_vector_t *v;
	char *p;
	int of;

	v=(wtk_vector_t*)wtk_calloc(1,wtk_vector_bytes(size)+8);
	p=(char*)(v)+4;
	while(1)
	{
		if(((long)(p+4))%8==0)
		{
			break;
		}
		++p;
	}
	of=(int)(p-(char*)v);
	//wtk_debug("%p/%p of=%d\n",v,p,of);
	v=(wtk_vector_t*)p;
	*(((int*)v)-1)=of;
	//wtk_vector_delete2(v);
	//exit(0);
	//v=(wtk_vector_t*)memalign(4096,wtk_vector_bytes(size));
	wtk_vector_init(v,size);
	return v;
}


int wtk_vector_bytes2(wtk_vector_t *v)
{
	int n;

	n=wtk_vector_size(v);
	return wtk_vector_bytes(n);
}

void wtk_vector_cpy(wtk_vector_t *src,wtk_vector_t *dst)
{
	int n;

	n=wtk_vector_size(src);
	memcpy((char*)(dst+1),(char*)(src+1),n*sizeof(float));
//	for(i=1;i<=n;++i)
//	{
//		dst[i]=src[i];
//	}
}

void wtk_double_vector_cpy(wtk_double_vector_t *src,wtk_double_vector_t *dst)
{
	int i,n;

	n=wtk_vector_size(src);
	for(i=1;i<=n;++i)
	{
		dst[i]=src[i];
	}
}

wtk_vector_t *wtk_vector_dup(wtk_vector_t *src)
{
	wtk_vector_t *dst;
	int n;

	n=wtk_vector_size(src);
	dst=wtk_vector_new(n);
	wtk_vector_cpy(src,dst);
	return dst;
}

/* EXPORT->CreateSVector:  Shared version */
wtk_svector_t* wtk_svector_newh(wtk_heap_t* heap, int size)
{
	wtk_svector_t* v;
	void *p;

	p=(void*)wtk_heap_malloc(heap,wtk_svector_bytes(size));
	v=(wtk_svector_t*)((char*)p+sizeof(void*)*2);
	(*(int*)v)=size;
	wtk_set_hook((void**)v,0);
	wtk_set_use((void**)v,0);
	return v;
}

wtk_svector_t* wtk_svector_dup(wtk_heap_t* heap, wtk_svector_t *src)
{
	wtk_svector_t *dst;

	dst=wtk_svector_newh(heap,wtk_vector_size(src));
	wtk_vector_cpy(src,dst);
	return dst;
}

void wtk_set_use(void **m,int n)
{
	*((int*)(m-1))=n;
}

int wtk_get_use(void **m)
{
	return *((int*)(m-1));
}

void wtk_inc_use(void **m)
{
	++*((int*)(m-1));
}

void wtk_dec_use(void **m)
{
	--*((int*)(m-1));
}

void wtk_set_hook(void **m,void *h)
{
	*(m-2)=h;
	//*((void**)((char*)m-2*sizeof(void*)))=h;
}

void* wtk_get_hook(void **m)
{
	return *(m-2);
}

void wtk_vector_zero2(wtk_vector_t *v)
{
	int i,n;

	n=wtk_vector_size(v);
	for(i=1;i<=n;++i)
	{
		v[i]=0;
	}
}

void wtk_vector_zero(wtk_vector_t *v)
{
	int n;

	n=wtk_vector_size(v);
	memset(v+1,0,sizeof(float)*n);
//	for(i=1;i<=n;++i)
//	{
//		v[i]=0;
//	}
}

void wtk_vector_set_init_value(wtk_vector_t *v,float f)
{
	int i,n;

	n=wtk_vector_size(v);
	for(i=1;i<=n;++i)
	{
		v[i]=f;
	}
}

void wtk_double_vector_zero(wtk_double_vector_t *v)
{
	int i,n;

	n=wtk_vector_size(v);
	for(i=1;i<=n;++i)
	{
		v[i]=0;
	}
}

float wtk_vector_max_abs(wtk_vector_t *v)
{
	float min=10000;
	float max=-10000;
	int i,n;

	n=wtk_vector_size(v);
	for(i=1;i<n;++i)
	{
		if(v[i]>max)
		{
			max=v[i];
		}
		if(v[i]<min)
		{
			min=v[i];
		}
	}
	if(min<0)
	{
		min=-min;
	}
	if(max<0)
	{
		max=-max;
	}
	if(max>min)
	{
		return max;
	}else
	{
		return min;
	}
}

float wtk_vector_sum(wtk_vector_t* v)
{
	float sum=0;
	wtk_vector_t *s,*e;

	s=v;e=v+wtk_vector_size(v);
	while((e-s)>=4)
	{
		sum+=*(++s);
		sum+=*(++s);
		sum+=*(++s);
		sum+=*(++s);
	}
	while(s<e)
	{
		sum+=*(++s);
	}
	return sum;
}

float wtk_math_max(float *a,int len)
{
	float max;
	float *s,*e;

	max=a[0];
	s=a+1;
	e=s+len-1;
	while(s<e)
	{
		if(*s>max)
		{
			max=*s;
			//wtk_debug("max=%f\n",max);
		}
		++s;
	}
	return max;
}

float wtk_math_max2(float *a,int len)
{
	int i;
	float max;

	max=a[0];
	for(i=1;i<len;++i)
	{
		if(a[i]>max)
		{
			max=a[i];
		}
	}
	return max;
}

void wtk_vector_print(wtk_vector_t* v)
{
	float t;

	wtk_debug("========== vector ==========\n");
#ifdef INLINE
	wtk_vector_do_i(v,t=,;printf("%.3f %s",t,i%10==0?"\n":""));
	printf("\n");
	/*
	if(wtk_vector_size(v)!=39)
	{
		wtk_debug("found invalid %d.\n",wtk_vector_size(v));
		exit(0);
	}
	*/
	//wtk_debug("%d\n",wtk_vector_size(v));
	//exit(0);
#else
	wtk_vector_do_i(v,t=,;if(t!=0){printf("v[%d]=%.6f\n",i,t);});
	//wtk_vector_do_i(v,t=,;printf("v[%d]=%f\n",i,t));
	//wtk_vector_do_i(v,t=,;printf("%f\n",t));
#endif
	//exit(0);
}

void wtk_short_vector_print(wtk_short_vector_t* v)
{
	int i,count;

	count=wtk_short_vector_size(v);
	for(i=1;i<=count;++i)
	{
		printf("%d\n",v[i]);
	}
}


void wtk_vector_fix_scale2(wtk_vector_t *v,float scale)
{
	int *pi;
	int i,n;
	float f;

	pi=(int*)(v);
	n=wtk_vector_size(v);
	for(i=1;i<=n;++i)
	{
		f=v[i]*scale;
		pi[i]=wtk_float_round(f);
		//wtk_debug("v[%d]=%d\n",i,pi[i]);
	}
	//exit(0);
}

void wtk_vector_fix_scale(wtk_vector_t *v,float scale)
{
	float f;
	int *pi,*pe;
	int n;
	//int i=0;

	n=wtk_vector_size(v);
	pi=((int*)v)+1;
	pe=pi+n;
	while(pi<pe)
	{
		f=(*((float*)pi))*scale;
		*pi=wtk_float_round(f);
//		++i;
//		wtk_debug("v[%d]=%d\n",i,*pi);
		++pi;
	}
	//exit(0);
}


