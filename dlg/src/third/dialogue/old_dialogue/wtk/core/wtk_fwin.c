#include "wtk_fwin.h" 
#include "wtk/core/math/wtk_math.h"
#include "wtk/core/math/wtk_math.h"

wtk_fwin_t* wtk_fwin_new(int len)
{
	wtk_fwin_t *w;

	w=(wtk_fwin_t*)wtk_malloc(sizeof(wtk_fwin_t));
	w->dat=(float*)wtk_malloc(len*sizeof(float));
	w->pos=0;
	w->len=len;
	return w;
}

void wtk_fwin_delete(wtk_fwin_t *w)
{
	wtk_free(w->dat);
	wtk_free(w);
}

void wtk_fwin_reset(wtk_fwin_t *w)
{
	w->pos=0;
}

int wtk_fwin_push(wtk_fwin_t *w,float *p,int n)
{
	int left;

	left=w->len-w->pos;
	left=min(left,n);
	if(left>0)
	{
		memcpy(w->dat+w->pos,p,left*sizeof(float));
		w->pos+=left;
	}
	return left;
}

void wtk_fwin_mul(wtk_fwin_t *w,float xmx)
{
	int i;
	float *p;

	p=w->dat;
	for(i=0;i<w->pos;++i)
	{
		p[i]*=xmx;
	}
}

void wtk_fwin_pop(wtk_fwin_t *w,int n)
{
	w->pos-=n;
	memmove(w->dat,w->dat+n,w->pos*sizeof(float));
}
