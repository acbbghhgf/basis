#include "wtk_median_filter.h" 

wtk_median_filter_t* wtk_median_filter_new(int window)
{
	wtk_median_filter_t* m;

	m=(wtk_median_filter_t*)wtk_malloc(sizeof(wtk_median_filter_t));
	m->win=window;
	m->idx=0;
	m->v=(short*)wtk_malloc(sizeof(short)*window);
	m->s=(short*)wtk_malloc(sizeof(short)*window);
	return m;
}

void wtk_median_filter_delete(wtk_median_filter_t *m)
{
	wtk_free(m->s);
	wtk_free(m->v);
	wtk_free(m);
}

float wtk_median_short_sort(void *ths,short *src,short *dst)
{
	return *src-*dst;
}

short wtk_median_update(wtk_median_filter_t *m)
{
	//print_short(m->v,m->win);
	memcpy(m->s,m->v,sizeof(short)*m->win);
	wtk_qsort2(m->s,m->win,sizeof(short),(wtk_qsort_cmp_f)wtk_median_short_sort,NULL);
	//print_short(m->s,m->win);
	return m->s[m->win/2];
}

void wtk_median_filter_feed(wtk_median_filter_t *m,short *v,int n,short *dst)
{
	int i,j;

	j=0;
	for(i=0;i<n;++i)
	{
		m->v[m->idx++]=v[i];
		if(m->idx==m->win)
		{
			dst[j++]=wtk_median_update(m);
			--m->idx;
			memmove(m->v,m->v+1,sizeof(short)*m->idx);
			//wtk_debug("v=%d\n",v[j-1]);
			//exit(0);
		}else
		{
			//dst[j++]=0;
		}
	}
}

void wtk_median_filter_process(short *input,short *output,int len,int win)
{
	wtk_median_filter_t *m;

	m=wtk_median_filter_new(win);
	wtk_median_filter_feed(m,input,len,output);
	wtk_median_filter_delete(m);
}
