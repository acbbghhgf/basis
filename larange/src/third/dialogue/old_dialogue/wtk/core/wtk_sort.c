#include "wtk_sort.h"


float wtk_float_cmp(void *ths,float *src,float *dst)
{
	return *src-*dst;
}

void wtk_qsort_float(float *f,int len)
{
	float t;

	wtk_qsort(f,f+len-1,sizeof(float),(wtk_qsort_cmp_f)wtk_float_cmp,NULL,&t);
}


float wtk_double_cmp(void *ths,double *src,double *dst)
{
	return *src-*dst;
}

void wtk_qsort_double(double *f,int len)
{
	double t;

	wtk_qsort(f,f+len-1,sizeof(double),(wtk_qsort_cmp_f)wtk_double_cmp,NULL,&t);
}

void wtk_qsort(void *s,void *e,size_t size,wtk_qsort_cmp_f cmp,void *app_data,void *tmp_elem)
{
	char *x;
	char *i,*j;
	float f;

	if(e<=s){return;}
	x=(char*)e;
	i=(char*)s-size;
	j=(char*)s;
	while(j<x)
	{
		f=cmp(app_data,(void*)j,(void*)x);
		//wtk_debug("v[%p/%p]=%f\n",j,x,f);
		if(f<=0)
		{
			i+=size;
			if(j!=i)
			{
				memcpy(tmp_elem,i,size);
				memcpy(i,j,size);
				memcpy(j,tmp_elem,size);
			}
		}
		j+=size;
	}
	i+=size;
	if(i!=x)
	{
		memcpy(tmp_elem,i,size);
		memcpy(i,x,size);
		memcpy(x,tmp_elem,size);
	}
	wtk_qsort(s,i-size,size,cmp,app_data,tmp_elem);
	wtk_qsort(i+size,e,size,cmp,app_data,tmp_elem);
}

void wtk_qsort2(void *base,size_t nmemb,size_t size,wtk_qsort_cmp_f cmp,void *app_data)
{
	void *tmp_elem;

	tmp_elem=malloc(size);
	wtk_qsort(base,(void*)((char*)base+(nmemb-1)*size),size,cmp,app_data,tmp_elem);
	free(tmp_elem);
}

void wtk_qsort3(void *base,size_t nmemb,size_t size,wtk_qsort_cmp_f cmp,void *app_data,void *tmp_elem)
{
	wtk_qsort(base,(void*)((char*)base+(nmemb-1)*size),size,cmp,app_data,tmp_elem);
}

void* wtk_inc_search(void *s,void *e,int size,wtk_search_cmp_f cmp,void *usr_data)
{
	void *p=0;
	int ret;

	while(s<=e)
	{
		ret=cmp(usr_data,s);
		if(ret==0)
		{
			p=s;
			break;
		}
		s=(void*)((char*)s+size);
	}
	return p;
}

void* wtk_binary_search(void *s,void *e,int size,wtk_search_cmp_f cmp,void *usr_data)
{
	void *p;
	int ret;

	//wtk_debug("s=%p-%p n=%d\n",s,e,1+(int)((e-s)/sizeof(void*)));
	if(s<e)
	{
		//wtk_debug("%d\n",(int)((((char*)e-(char*)s)/size - 1 )>>1));
		p=(void*)( (char*)s+ (( ((char*)e-(char*)s)/size - 1 )>>1)*size);
		//wtk_debug("s=%p,e=%p,p=%p,size=%#x\n",s,e,p,size);
		ret=cmp(usr_data,p);
		if(ret==0)
		{
			return p;
		}else if(ret<0)
		{
			return wtk_binary_search(s,(void*)((long)p-size),size,cmp,usr_data);
		}else
		{
			return wtk_binary_search((void*)((long)p+size),e,size,cmp,usr_data);
		}
	}else if(s==e)
	{
		ret=cmp(usr_data,s);
		return ret==0?s:0;
	}else
	{
		return 0;
	}
}

