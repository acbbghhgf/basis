#include "wtk_prune.h" 
#include "wtk/core/math/wtk_math.h"

wtk_prune_t* wtk_prune_new(wtk_prune_cfg_t *cfg)
{
	wtk_prune_t *p;

	p=(wtk_prune_t*)wtk_malloc(sizeof(wtk_prune_t));
	p->cfg=cfg;
	p->bin_width_scale=1.0/cfg->bin_width;
	p->nbin=(int)((cfg->max_score-cfg->min_score)*p->bin_width_scale+0.5)+1;
	p->bins=(unsigned int*)wtk_malloc(sizeof(unsigned int)*p->nbin);
	p->min_x=-p->cfg->min_score*p->bin_width_scale+0.5;
	p->thresh=LZERO;
	wtk_prune_reset(p);
	return p;
}

void wtk_prune_delete(wtk_prune_t *p)
{
	wtk_free(p->bins);
	wtk_free(p);
}

void wtk_prune_reset(wtk_prune_t *p)
{
	p->count=0;
	p->min=1e6;
	p->max=-1e6;
	memset(p->bins,0,sizeof(unsigned int)*p->nbin);
}

void wtk_prune_plus(wtk_prune_t *dst,wtk_prune_t *src)
{
	int s,e;
	int i;

	dst->count+=src->count;
	if(dst->min>src->min)
	{
		dst->min=src->min;
	}
	if(dst->max<src->max)
	{
		dst->max=src->max;
	}
	s=(int)((src->min-src->cfg->min_score)*src->bin_width_scale+0.5);
	e=(int)((src->max-src->cfg->min_score)*src->bin_width_scale+0.5);
	for(i=s;i<=e;++i)
	{
		dst->bins[i]+=src->bins[i];
	}
}

void wtk_prune_add(wtk_prune_t *p,float f)
{
	int idx;

#ifndef DEBUG_PRUNE
	if(f<p->cfg->min_score || f>p->cfg->max_score)
	{
		wtk_debug("warning:value over range %f[%f-%f].\n",f,p->cfg->min_score,p->cfg->max_score);
		//exit(0);
		return;
	}
#endif
	if(f<p->min)
	{
		p->min=f;
	}
	if(f>p->max)
	{
		p->max=f;
	}
	idx=(int)(f*p->bin_width_scale+p->min_x);
//	if(idx<0 || idx>=p->nbin)
//	{
//		return;
//	}
	//idx=(int)((f-p->cfg->min_score)*p->bin_width_scale+0.5);
//	if(idx>=p->nbin)
//	{
//		wtk_debug("found bug=%f %d/%d\n",f,idx,p->nbin);
//		exit(0);
//	}
	++p->bins[idx];
	++p->count;
}

int wtk_prune_want_prune(wtk_prune_t *p)
{
//	if(p->count<=p->cfg->count || p->min<p->cfg->min_score || p->max>p->cfg->max_score)
//	{
//		return  0;
//	}else
//	{
//		return 1;
//	}
	if(p->count>p->cfg->count || (p->cfg->beam>0 && (p->max-p->min)<=(p->cfg->beam)))
	{
		return 1;
	}else
	{
		return 0;
	}
}

float wtk_prune_get_thresh(wtk_prune_t *p)
{
	return wtk_prune_get_thresh2(p,p->cfg->count);
}


float wtk_prune_get_thresh2(wtk_prune_t *p,int count)
{
	int s,e;
	int i,cnt;

	if(p->cfg->beam>0 && ((p->max-p->min)>(p->cfg->beam)))
	{
		return p->max-p->cfg->beam;
	}
	if(p->count<=count || p->min<p->cfg->min_score || p->max>p->cfg->max_score)
	//if(p->min<p->cfg->min_score || p->max>p->cfg->max_score)
	{
		return p->cfg->min_score;
	}
	s=(int)((p->min-p->cfg->min_score)*p->bin_width_scale+0.5);
	if(p->count<=count)
	{
		goto end;
	}
	e=(int)((p->max-p->cfg->min_score)*p->bin_width_scale+0.5);
	for(i=e,cnt=0;i>=s;--i)
	{
		cnt+=p->bins[i];
		if(cnt>=count)
		{
			///wtk_debug("return %f\n",(i-1)*p->cfg->bin_width);
			//wtk_debug("cnt=%d/%d/%d\n",cnt,p->cfg->count,p->count);
			return (i-1)*p->cfg->bin_width+p->cfg->min_score;//+p->bin_width_scale;
			//wtk_debug("cnt=%d %d %f\n",cnt,i,(i-1)*p->cfg->bin_width+p->cfg->min);
			//return (i-1-0.5)*p->cfg->bin_width+p->cfg->min;
		}
	}
end:
	return (s-1)*p->cfg->bin_width+p->cfg->min_score;//+p->bin_width_scale;
}

void wtk_prune_update_thresh(wtk_prune_t *p)
{
	p->thresh=wtk_prune_get_thresh2(p,p->cfg->count);
}

float wtk_prune_get_thresh2_x(wtk_prune_t *p,int count)
{
	int s,e;
	int i,cnt;

	if(p->count<=count || p->min<p->cfg->min_score || p->max>p->cfg->max_score)
	{
		return p->cfg->min_score;
	}
	s=(int)((p->min-p->cfg->min_score)*p->bin_width_scale+0.5);
	e=(int)((p->max-p->cfg->min_score)*p->bin_width_scale+0.5);
	for(i=e,cnt=0;i>=s;--i)
	{
		cnt+=p->bins[i];
		if(cnt>=count)
		{
			//wtk_debug("cnt=%d/%d/%d\n",cnt,p->cfg->count,p->count);
			return (i-1)*p->cfg->bin_width+p->cfg->min_score+p->bin_width_scale;
			//wtk_debug("cnt=%d %d %f\n",cnt,i,(i-1)*p->cfg->bin_width+p->cfg->min);
			//return (i-1-0.5)*p->cfg->bin_width+p->cfg->min;
		}
	}
	return p->cfg->min_score;
}
