#include "wtk/core/wtk_str_encode.h"
#include "wtk_segmenter.h"

wtk_segmenter_t* wtk_segmenter_new(wtk_segmenter_cfg_t *cfg,wtk_rbin2_t *rbin)
{
	wtk_segmenter_t *m;

	m=(wtk_segmenter_t*)wtk_malloc(sizeof(*m));
	m->cfg=cfg;
	if(cfg->use_bin)
	{
		if(rbin)
		{
			m->fkv=wtk_fkv2_new2(rbin,cfg->dict_fn);
		}else
		{
			m->fkv=wtk_fkv2_new(cfg->dict_fn);
		}
	}else
	{
		m->fkv=NULL;
	}
	m->heap=wtk_heap_new(4096);
	m->buf=wtk_strbuf_new(256,1);
	wtk_segmenter_reset(m);
	return m;
}

void wtk_segmenter_delete(wtk_segmenter_t *seg)
{
	if(seg->fkv)
	{
		wtk_fkv2_delete(seg->fkv);
	}
	wtk_strbuf_delete(seg->buf);
	wtk_heap_delete(seg->heap);
	wtk_free(seg);
}

void wtk_segmenter_reset(wtk_segmenter_t *seg)
{
	seg->wrd_array=0;
	seg->wrd_array_n=0;
	wtk_heap_reset(seg->heap);
}


int wtk_segmenter_find_freq(wtk_segmenter_t *seg,char *k,int k_bytes,double *freq)
{
	wtk_segwrd_t *wrd;
	int ret=-1;

	//wtk_debug("[%.*s]\n",k_bytes,k);
	wrd=wtk_str_hash_find(seg->cfg->hash,k,k_bytes);
	if(!wrd){goto end;}
	*freq=wrd->prob;
	ret=0;
end:
	//wtk_debug("[%.*s]=%d\n",k_bytes,k,ret);
	return ret;
}

int wtk_segmenter_is_valid_char(wtk_segmenter_t *seg,char *data,int bytes)
{
	wtk_array_t *a=seg->cfg->filter;
	wtk_string_t **strs;
	int i;
	int is=1;

	//wtk_debug("a=%p\n",a);
	if(!a){goto end;}
	strs=(wtk_string_t **)a->slot;
	//wtk_debug("[%.*s]\n",bytes,data);
	for(i=0;i<a->nslot;++i)
	{
		//wtk_debug("[%.*s]=[%.*s]\n",strs[i]->len,strs[i]->data,bytes,data);
		if(wtk_string_cmp(strs[i],data,bytes)==0)
		{
			is=0;
			break;
		}
	}
end:
	return is;
}

#include <ctype.h>

int wtk_segmenter_parse2(wtk_segmenter_t *seg,char *data,int bytes)
{
	wtk_string_t *segchar,**strs;
	wtk_strbuf_t *buf=seg->buf;
	wtk_heap_t *heap=seg->heap;
	char *s,*e;
	int num;
	wtk_array_t *array;
	int i,j,k,ret;
	float prev_weight,cur_weight;
	float *best_prob;
	int *prev;
	double freq;
	int nx;
	int en;

	array=wtk_array_new_h(heap,bytes>>1,sizeof(wtk_string_t*));
	s=data;e=s+bytes;
	//wtk_debug("%d=[%.*s]\n",bytes,bytes,data);
	while(s<e)
	{
		num=wtk_utf8_bytes(*s);
		//wtk_debug("%d:[%.*s]\n",array->nslot,num,s);
		s+=num;
		if(s>e){break;}
		if(!wtk_segmenter_is_valid_char(seg,s-num,num)){continue;}
		segchar=wtk_heap_dup_string(heap,s-num,num);
		if(segchar->len==1)
		{
			if(seg->cfg->upper)
			{
				segchar->data[0]=toupper(segchar->data[0]);
			}else if(seg->cfg->lower)
			{
				segchar->data[0]=tolower(segchar->data[0]);
			}
		}
		*((wtk_string_t**)(wtk_array_push(array)))=segchar;
	}
	//wtk_debug("nslot: %d\n",array->nslot);
	num=array->nslot+1;
	best_prob=(float*)wtk_heap_malloc(heap,sizeof(float)*num);
	for(i=0;i<num;++i)
	{
		best_prob[i]=0;
	}
	prev=(int*)wtk_heap_zalloc(heap,sizeof(int)*num);
	strs=(wtk_string_t**)array->slot;
	for(i=0;i<array->nslot;++i)
	{
		wtk_strbuf_reset(buf);
		for(j=i+1,nx=0,en=0;j<=array->nslot;++j)
		{
			segchar=strs[j-1];
			//wtk_debug("%d/%d\n",array->nslot-i,seg->cfg->max_char);
			if(segchar->len>1)
			{
				++nx;
				en=0;
			}else
			{
				if(en==0)
				{
					++nx;
					en=1;
				}
			}
			if(seg->cfg->max_char>0 && segchar->len>1 && nx>seg->cfg->max_char)
			{
				break;
			}
			//wtk_debug("j=%d,%p,%d\n",j,segchar,array->nslot);
			wtk_strbuf_push(buf,segchar->data,segchar->len);
			ret=wtk_segmenter_find_freq(seg,buf->data,buf->pos,&freq);
			//wtk_debug("[%.*s]=%d\n",buf->pos,buf->data,ret);
			if(ret!=0)
			{
				continue;
			}
			//wtk_debug("[%.*s]=%f\n",buf->pos,buf->data,freq);
			prev_weight=0;
			if(i>0)
			{
				if(best_prob[i]>0)
				{
					prev_weight=best_prob[i];
				}else
				{
					int jx;

					jx=i;
					prev_weight=0;//seg->cfg->def_weight;
					while(jx>=0)
					{
						if(best_prob[jx]>0)
						{
							prev_weight+=best_prob[jx];
							break;
						}else
						{
							prev_weight+=seg->cfg->def_weight;
						}
						--jx;
					}
					//wtk_debug("default weight=%f\n",prev_weight);
				}
			}
			cur_weight=prev_weight+freq;
			//wtk_debug("cur_weight=%f,%f,%d\n",cur_weight,best_prob[j],prev[j]);
			//wtk_debug("j=%d prev=%d best=%f/%f %f/%f\n",j,prev[j],best_prob[j],cur_weight,prev_weight,freq);
			if(prev[j]==0 || best_prob[j] >cur_weight)
			{
				prev[j]=i+1;
				best_prob[j]=cur_weight;
			}
		}
	}
	seg->wrd_array_n=0;
	for(i=num-1;i>0;)
	{
		if(prev[i]==0)
		{
			i-=1;
		}else
		{
			i=prev[i]-1;
		}
		++seg->wrd_array_n;
	}
	seg->wrd_array=(wtk_string_t**)wtk_heap_malloc(heap,sizeof(wtk_string_t*)*seg->wrd_array_n);
	for(i=num-1,k=seg->wrd_array_n;i>0;)
	{
		wtk_strbuf_reset(buf);
		if(prev[i]==0)
		{
			wtk_strbuf_push(buf,strs[i-1]->data,strs[i-1]->len);
			i-=1;
		}else
		{
			for(j=prev[i];j<=i;++j)
			{
				wtk_strbuf_push(buf,strs[j-1]->data,strs[j-1]->len);
			}
			i=prev[i]-1;
		}
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		seg->wrd_array[--k]=wtk_heap_dup_string(heap,buf->data,buf->pos);
	}
	//wtk_debug("")
	return 0;
}


int wtk_segmenter_parse3(wtk_segmenter_t *seg,char *data,int bytes)
{
	wtk_strbuf_t *buf=seg->buf;
	wtk_string_t *segchar,**strs;
	wtk_heap_t *heap=seg->heap;
	char *s,*e;
	int num;
	wtk_array_t *array;
	int i,j,k;
	float prev_weight,cur_weight;
	float *best_prob;
	int *prev;
	double freq;
	wtk_fkv_env_t env;
	wtk_fkv2_t *kv=seg->fkv;

	array=wtk_array_new_h(heap,bytes>>1,sizeof(wtk_string_t*));
	s=data;e=s+bytes;
	//wtk_debug("%d=[%.*s]\n",bytes,bytes,data);
	while(s<e)
	{
		num=wtk_utf8_bytes(*s);
		//wtk_debug("%d:[%.*s]\n",array->nslot,num,s);
		s+=num;
		if(s>e){break;}
		if(!wtk_segmenter_is_valid_char(seg,s-num,num)){continue;}
		segchar=wtk_heap_dup_string(heap,s-num,num);
		if(segchar->len==1)
		{
			if(seg->cfg->upper)
			{
				segchar->data[0]=toupper(segchar->data[0]);
			}else if(seg->cfg->lower)
			{
				segchar->data[0]=tolower(segchar->data[0]);
			}
		}
		*((wtk_string_t**)(wtk_array_push(array)))=segchar;
	}
	//wtk_debug("nslot: %d\n",array->nslot);
	num=array->nslot+1;
	best_prob=(float*)wtk_heap_malloc(heap,sizeof(float)*num);
	for(i=0;i<num;++i)
	{
		best_prob[i]=0;
	}
	prev=(int*)wtk_heap_zalloc(heap,sizeof(int)*num);
	strs=(wtk_string_t**)array->slot;
	for(i=0;i<array->nslot;++i)
	{
		wtk_fkv_env_init(kv,&(env));
		//wtk_debug("%d ===============>\n",i);
		for(j=i+1;j<=array->nslot;++j)
		{
			segchar=strs[j-1];
			//wtk_debug("%d/%d\n",array->nslot-i,seg->cfg->max_char);
			wtk_fkv2_get(kv,&(env),segchar->data,segchar->len);
			//wtk_debug("[%.*s] end=%d err=%d\n",segchar->len,segchar->data,env.is_end,env.is_err);
			if(env.is_err)
			{
				break;
			}else if(!env.is_end)
			{
				continue;
			}
			freq=env.v.f;
			//wtk_debug("freq=%f\n",freq);
			//wtk_debug("[%.*s]=%f\n",buf->pos,buf->data,freq);
			prev_weight=0;
			if(i>0)
			{
				if(best_prob[i]>0)
				{
					prev_weight=best_prob[i];
				}else
				{
					int jx;

					jx=i;
					prev_weight=0;//seg->cfg->def_weight;
					while(jx>=0)
					{
						if(best_prob[jx]>0)
						{
							prev_weight+=best_prob[jx];
							break;
						}else
						{
							prev_weight+=seg->cfg->def_weight;
						}
						--jx;
					}
					//wtk_debug("default weight=%f\n",prev_weight);
				}
			}
			cur_weight=prev_weight+freq;
			//wtk_debug("cur_weight=%f,%f,%d\n",cur_weight,best_prob[j],prev[j]);
			//wtk_debug("j=%d prev=%d best=%f/%f %f/%f\n",j,prev[j],best_prob[j],cur_weight,prev_weight,freq);
			if(prev[j]==0 || best_prob[j] >cur_weight)
			{
				prev[j]=i+1;
				best_prob[j]=cur_weight;
			}
		}
	}
	seg->wrd_array_n=0;
	for(i=num-1;i>0;)
	{
		if(prev[i]==0)
		{
			i-=1;
		}else
		{
			i=prev[i]-1;
		}
		++seg->wrd_array_n;
	}
	seg->wrd_array=(wtk_string_t**)wtk_heap_malloc(heap,sizeof(wtk_string_t*)*seg->wrd_array_n);
	for(i=num-1,k=seg->wrd_array_n;i>0;)
	{
		wtk_strbuf_reset(buf);
		if(prev[i]==0)
		{
			wtk_strbuf_push(buf,strs[i-1]->data,strs[i-1]->len);
			i-=1;
		}else
		{
			for(j=prev[i];j<=i;++j)
			{
				wtk_strbuf_push(buf,strs[j-1]->data,strs[j-1]->len);
			}
			i=prev[i]-1;
		}
		seg->wrd_array[--k]=wtk_heap_dup_string(heap,buf->data,buf->pos);
		//wtk_debug("[%.*s]=%p\n",buf->pos,buf->data,seg->wrd_array[k]);
	}
	return 0;
}

int wtk_segmenter_parse(wtk_segmenter_t *seg,char *data,int bytes,wtk_strbuf_t *output_buf)
{
	wtk_string_t **strs;
	int i;
	int ret;

	wtk_segmenter_reset(seg);
	if(output_buf)
	{
		wtk_strbuf_reset(output_buf);
	}
	//wtk_debug("use_bin=%d\n",seg->cfg->use_bin);
	if(seg->cfg->use_bin)
	{
		ret=wtk_segmenter_parse3(seg,data,bytes);
	}else
	{
		ret=wtk_segmenter_parse2(seg,data,bytes);
	}
	if(ret!=0){goto end;}
	if(output_buf)
	{
		strs=seg->wrd_array;
		for(i=0;i<seg->wrd_array_n;++i)
		{
			if(i>0)
			{
				wtk_strbuf_push_c(output_buf,' ');
			}
			wtk_strbuf_push(output_buf,strs[i]->data,strs[i]->len);
		}
	}
end:

	//wtk_debug("[%.*s]\n",output_buf->pos,output_buf->data);
	//exit(0);
	return 0;
}

