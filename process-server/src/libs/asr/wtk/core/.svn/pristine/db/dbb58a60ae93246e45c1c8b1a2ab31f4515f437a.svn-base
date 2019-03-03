#include "wtk_poseg.h" 
#include <ctype.h>

wtk_poseg_t* wtk_poseg_new(wtk_poseg_cfg_t *cfg)
{
	wtk_poseg_t *seg;

	seg=(wtk_poseg_t*)wtk_malloc(sizeof(wtk_poseg_t));
	seg->cfg=cfg;
	seg->heap=wtk_heap_new(4096);
	seg->chnpos=wtk_chnpos_new(&(cfg->pos));
	return seg;
}

void wtk_poseg_delete(wtk_poseg_t *seg)
{
	wtk_chnpos_delete(seg->chnpos);
	wtk_heap_delete(seg->heap);
	wtk_free(seg);
}

void wtk_poseg_reset(wtk_poseg_t *seg)
{
	wtk_heap_reset(seg->heap);
	wtk_chnpos_reset(seg->chnpos);
	seg->input=NULL;
	seg->wrds=NULL;
	seg->pos=NULL;
	seg->nwrd=0;
}

int wtk_poseg_is_valid_char(wtk_poseg_t *seg,char *data,int bytes)
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




void wtk_poseg_print_input(wtk_poseg_t *seg)
{
	wtk_string_t **input;
	int i,n;

	input=(wtk_string_t**)seg->input->slot;
	n=seg->input->nslot;
	for(i=0;i<n;++i)
	{
		printf("v[%d]=%.*s\n",i,input[i]->len,input[i]->data);
	}
}

void wtk_poseg_print_output(wtk_poseg_t *seg)
{
	wtk_string_t **input;
	int i,n;

	input=seg->wrds;
	n=seg->nwrd;
	wtk_debug("========================================\n");
	for(i=0;i<n;++i)
	{
		printf("v[%d]=%.*s/%.*s\n",i,input[i]->len,input[i]->data,seg->pos[i]?seg->pos[i]->len:0,seg->pos[i]?seg->pos[i]->data:0);
	}
}

int wtk_string_is_mult_char(wtk_string_t *str)
{
	int v;

	if(str->len>3)
	{
		return 1;
	}
	v=wtk_utf8_bytes(str->data[0]);
	return str->len>v?1:0;
}

int wtk_string_is_eng(wtk_string_t *str)
{
	char *s,*e;
	char c;

	s=str->data;
	e=s+str->len;
	while(s<e)
	{
		c=*s;
		if( c=='\'' || (c>='a' && c<='z') || (c>='A' && c<='Z'))
		{
		}else
		{
			return 0;
		}
		++s;
	}
	return 1;
}

int wtk_string_is_num(wtk_string_t *str)
{
	char *s,*e;
	char c;

	s=str->data;
	e=s+str->len;
	while(s<e)
	{
		c=*s;
		if((c>='0' && c<='9'))
		{
		}else
		{
			return 0;
		}
		++s;
	}
	return 1;
}

static wtk_string_t x=wtk_string("x");
static wtk_string_t m=wtk_string("m");
static wtk_string_t eng=wtk_string("eng");

void wtk_poseg_update_wrd_pos(wtk_poseg_t *seg,int last_idx,int i)
{
	wtk_chnpos_pth_t *pth;
	wtk_string_t *str;
	wtk_posdict_wrd_t *wrd;

	//wtk_debug("i=%d/%d\n",last_idx,i);
	if(i>last_idx)
	{
		while(i>last_idx)
		{
			//wtk_debug("i=%d/%d\n",i,last_idx);
			wtk_chnpos_parse(seg->chnpos,seg->wrds+last_idx,i-last_idx+1);
			if(seg->chnpos->inst_q.length==0)
			{
				seg->pos[last_idx]=&x;
				wtk_chnpos_reset(seg->chnpos);
				++last_idx;
			}else
			{
				break;
			}
		}
		if(seg->chnpos->max_inst && seg->chnpos->max_inst->pth)
		{
			pth=seg->chnpos->max_inst->pth;
			while(pth)
			{
				seg->pos[i--]=wtk_chnpos_model_get_pos_str(seg->chnpos->cfg->model,pth->pos);
				//wtk_debug("%.*s\n",seg->pos[i+1]->len,seg->pos[i+1]->data);
				pth=pth->prev;
			}
		}else
		{
			for(;last_idx<=i;++last_idx)
			{
				seg->pos[last_idx]=&x;
			}
		}
		wtk_chnpos_reset(seg->chnpos);
	}else
	{
		str=seg->wrds[last_idx];
		wrd=wtk_posdict_get(seg->cfg->dict,str->data,str->len,0);
		if(wrd)
		{
			seg->pos[last_idx]=wtk_heap_dup_string(seg->heap,wrd->pos.data,wrd->pos.len);//&(wrd->pos);
		}else
		{
			seg->pos[last_idx]=&(x);
		}
	}
}

void wtk_poseg_update_pos(wtk_poseg_t *seg)
{
	static wtk_string_t v=wtk_string("v");
	wtk_posdict_wrd_t *wrd;
	wtk_string_t *str;
	int i;
	int detect_pos=0;
	int last_idx=-1;

	seg->pos=(wtk_string_t**)wtk_heap_zalloc(seg->heap,sizeof(wtk_string_t*)*seg->nwrd);
	for(i=0;i<seg->nwrd;++i)
	{
		str=seg->wrds[i];
		//b=wtk_string_is_mult_char(str);
		//wtk_debug("v[%d/%d]=%.*s detect_pos=%d\n",i,seg->nwrd,str->len,str->data,detect_pos);
		if(detect_pos)
		{
			if(wtk_string_is_eng(str))
			{
				//wtk_debug("[%.*s]\n",str->len,str->data);
				wtk_poseg_update_wrd_pos(seg,last_idx,i-1);
				seg->pos[i]=&eng;
				detect_pos=0;
			}else if(wtk_string_is_num(str))
			{
				wtk_poseg_update_wrd_pos(seg,last_idx,i-1);
				seg->pos[i]=&m;
				detect_pos=0;
			}else if(str->len>3)
			{
				wtk_poseg_update_wrd_pos(seg,last_idx,i-1);
				wrd=wtk_posdict_get(seg->cfg->dict,str->data,str->len,0);
				//wtk_debug("[%.*s]=%p\n",str->len,str->data,wrd);
				if(wrd)
				{
					seg->pos[i]=wtk_heap_dup_string(seg->heap,wrd->pos.data,wrd->pos.len);
				}else
				{
					seg->pos[i]=&v;
				}
				//seg->pos[i]=&(wrd->pos);
				detect_pos=0;
			}
			else if(i==seg->nwrd-1)
			{
				wtk_poseg_update_wrd_pos(seg,last_idx,i);
			}else
			{
				wtk_chnpos_wrd_t *tw;

				tw=wtk_chnpos_model_get_wrd(seg->cfg->pos.model,str->data,str->len);
				if(!tw)
				{
					wtk_poseg_update_wrd_pos(seg,last_idx,i-1);
					seg->pos[i]=&(x);
					detect_pos=0;
				}
			}
		}else
		{
			//wtk_debug("[%.*s]\n",str->len,str->data);
			if(wtk_string_is_eng(str))
			{
				seg->pos[i]=&eng;
			}else if(wtk_string_is_num(str))
			{
				seg->pos[i]=&m;
			}else if(str->len>3)
			{
				wrd=wtk_posdict_get(seg->cfg->dict,str->data,str->len,0);
				if(wrd)
				{
					seg->pos[i]=wtk_heap_dup_string(seg->heap,wrd->pos.data,wrd->pos.len);//&(wrd->pos);
				}else
				{
					seg->pos[i]=&(x);
				}
			}else
			{
				wtk_chnpos_wrd_t *tw;

				tw=wtk_chnpos_model_get_wrd(seg->cfg->pos.model,str->data,str->len);
				if(tw)
				{
					last_idx=i;
					detect_pos=1;
					if(i==seg->nwrd-1)
					{
						wtk_poseg_update_wrd_pos(seg,last_idx,i);
					}
				}else
				{
					seg->pos[i]=&(x);
				}
			}
		}
	}
	//wtk_poseg_print_output(seg);
}

void wtk_poseg_update_input(wtk_poseg_t *seg,char *data,int bytes)
{
typedef enum
{
	WTK_POSEG_INIT,
	WTK_POSEG_NUM,
	WTK_POSEG_ENG,
}wtk_poseg_input_state_t;
	wtk_string_t *segchar;
	wtk_heap_t *heap=seg->heap;
	char *s,*e;
	int num;
	wtk_array_t *array;
	wtk_poseg_input_state_t state;
	wtk_strbuf_t *buf;

	buf=wtk_strbuf_new(256,1);
	state=WTK_POSEG_INIT;
	array=wtk_array_new_h(heap,bytes>>1,sizeof(wtk_string_t*));
	s=data;e=s+bytes;
	//wtk_debug("%d=[%.*s]\n",bytes,bytes,data);
	while(s<e)
	{
		num=wtk_utf8_bytes(*s);
		//wtk_debug("%d:[%.*s] state=%d\n",array->nslot,num,s,state);
STATE:
		switch(state)
		{
		case WTK_POSEG_INIT:
			if(!wtk_poseg_is_valid_char(seg,s,num)){break;;}
			//wtk_debug("[%.*s]\n",num,s);
			if(num>1)
			{
				segchar=wtk_heap_dup_string(heap,s,num);
				*((wtk_string_t**)(wtk_array_push(array)))=segchar;
			}else
			{
				if(isdigit(*s))
				{
					wtk_strbuf_reset(buf);
					wtk_strbuf_push(buf,s,num);
					//wtk_debug("[%.*s]\n",buf->pos,buf->data);
					state=WTK_POSEG_NUM;
				}else if(isalpha(*s))
				{
					wtk_strbuf_reset(buf);
					wtk_strbuf_push(buf,s,num);
					state=WTK_POSEG_ENG;
				}else
				{
					segchar=wtk_heap_dup_string(heap,s,num);
					*((wtk_string_t**)(wtk_array_push(array)))=segchar;
				}
			}
			break;
		case WTK_POSEG_NUM:
			if(num==1 && isdigit(*s))
			{
				wtk_strbuf_push(buf,s,num);
			}else
			{
				//wtk_debug("[%.*s]\n",buf->pos,buf->data);
				segchar=wtk_heap_dup_string(heap,buf->data,buf->pos);
				*((wtk_string_t**)(wtk_array_push(array)))=segchar;
				state=WTK_POSEG_INIT;
				goto STATE;
			}
			break;
		case WTK_POSEG_ENG:
			if(num==1 && isalpha(*s))
			{
				wtk_strbuf_push(buf,s,num);
			}else
			{
				segchar=wtk_heap_dup_string(heap,buf->data,buf->pos);
				*((wtk_string_t**)(wtk_array_push(array)))=segchar;
				state=WTK_POSEG_INIT;
				goto STATE;
			}
			break;
		}
		s+=num;
	}
	if(state!=WTK_POSEG_INIT && buf->pos>0)
	{
		segchar=wtk_heap_dup_string(heap,buf->data,buf->pos);
		*((wtk_string_t**)(wtk_array_push(array)))=segchar;
	}
	seg->input=array;
	wtk_strbuf_delete(buf);
	//wtk_poseg_print_input(seg);
	//exit(0);
}

void wtk_poseg_update_merge(wtk_poseg_t *seg)
{
	wtk_array_t *w,*p;
	int i;
	wtk_strbuf_t *buf;
	int j;
	wtk_string_t *v;

	buf=wtk_strbuf_new(256,1);
	w=wtk_array_new_h(seg->heap,seg->nwrd,sizeof(void*));
	p=wtk_array_new_h(seg->heap,seg->nwrd,sizeof(void*));
	for(i=0;i<seg->nwrd;)
	{
		//wtk_debug("v[%d]=[%.*s]\n",i,seg->wrds[i]->len,seg->wrds[i]->data);
		if(wtk_string_cmp_s(seg->pos[i],"nr")==0)
		{
			wtk_strbuf_reset(buf);
			wtk_strbuf_push(buf,seg->wrds[i]->data,seg->wrds[i]->len);
			wtk_array_push2(p,&(seg->pos[i]));
			j=1;
			for(++i;i<seg->nwrd;++i)
			{
				if(wtk_string_cmp_s(seg->pos[i],"nr")==0)
				{
					++j;
					wtk_strbuf_push(buf,seg->wrds[i]->data,seg->wrds[i]->len);
				}else
				{
					break;
				}
			}
			if(j==1)
			{
				wtk_array_push2(w,&(seg->wrds[i-1]));
			}else
			{
				v=wtk_heap_dup_string(seg->heap,buf->data,buf->pos);
				wtk_array_push2(w,&(v));
			}
			//exit(0);
		}else
		{
			wtk_array_push2(w,&(seg->wrds[i]));
			wtk_array_push2(p,&(seg->pos[i]));
			++i;
		}

	}
	seg->wrds=(wtk_string_t**)(w->slot);
	seg->pos=(wtk_string_t**)(p->slot);
	seg->nwrd=w->nslot;
	wtk_strbuf_delete(buf);
	//exit(0);
}

void wtk_poseg_update_seg_merge(wtk_poseg_t *seg)
{
	wtk_heap_t *heap=seg->heap;
	wtk_array_t *a;
	wtk_string_t *v;
	wtk_string_t *pre,*nxt;
	int i;
	wtk_strbuf_t *buf;

	buf=wtk_strbuf_new(256,1);
	a=wtk_array_new_h(heap,seg->nwrd,sizeof(void*));
	for(i=1;i<seg->nwrd-1;++i)
	{
		v=seg->wrds[i];
		wtk_debug("v[%d]=[%.*s]\n",i,v->len,v->data);
		pre=seg->wrds[i-1];
		nxt=seg->wrds[i+1];
		//喜不喜欢 聪不聪明 来不来 去不去
		if(pre->len==3 && (nxt->len==6||nxt->len==3) && wtk_string_cmp_s(v,"不")==0)
		{
			if(wtk_string_cmp(pre,nxt->data,pre->len)==0)
			{
				wtk_strbuf_reset(buf);
				wtk_strbuf_push(buf,pre->data,pre->len);
				wtk_strbuf_push_s(buf,"不");
				wtk_strbuf_push(buf,nxt->data,nxt->len);
				v=wtk_heap_dup_string(heap,buf->data,buf->pos);
				wtk_array_push2(a,&v);
				wtk_debug("[%.*s]\n",v->len,v->data);
				i+=2;
//				if(i==seg->nwrd-2)
//				{
//					wtk_array_push2(a,&(seg->wrds[i]));
//				}
			}else
			{
				wtk_debug("[%.*s]\n",seg->wrds[i-1]->len,seg->wrds[i-1]->data);
				wtk_debug("[%.*s]\n",seg->wrds[i]->len,seg->wrds[i]->data);
				wtk_array_push2(a,&(seg->wrds[i-1]));
				//wtk_array_push2(a,&(seg->wrds[i]));
				++i;
			}
		}else
		{
			wtk_debug("[%.*s]\n",seg->wrds[i-1]->len,seg->wrds[i-1]->data);
			wtk_array_push2(a,&(seg->wrds[i-1]));
		}
	}
	wtk_debug("i=%d nwrd=%d\n",i,seg->nwrd);
	for(;i<=seg->nwrd;++i)
	{
		wtk_array_push2(a,&(seg->wrds[i-1]));
	}
	wtk_strbuf_delete(buf);
	if(a->nslot!=seg->nwrd)
	{
		seg->nwrd=a->nslot;
		seg->wrds=(wtk_string_t**)(a->slot);
	}
//	wtk_poseg_print_output(seg);
	//exit(0);
}

void wtk_poseg_process(wtk_poseg_t *seg,char *data,int bytes)
{
	wtk_string_t **input,**output;
	wtk_strbuf_t *buf;
	int i,n,j,k,jx;
	float *best_prob;
	int *prev;
	wtk_posdict_wrd_t *wrd;
	float prev_weight,f;
	wtk_heap_t *heap=seg->heap;

	buf=wtk_strbuf_new(256,1);
	wtk_poseg_update_input(seg,data,bytes);
	input=(wtk_string_t**)seg->input->slot;
	n=seg->input->nslot;
	best_prob=(float*)wtk_heap_malloc(heap,sizeof(float)*n);
	prev=(int*)wtk_heap_malloc(heap,sizeof(int)*n);
	for(i=0;i<n;++i)
	{
		best_prob[i]=0;
		prev[i]=-1;
	}
	for(i=0;i<n;++i)
	{
		wtk_strbuf_reset(buf);
		//wtk_strbuf_push(buf,input[i]->data,input[i]->len);
		for(j=i,k=0;j<n;++j,++k)
		{
			wtk_strbuf_push(buf,input[j]->data,input[j]->len);
			wrd=wtk_posdict_get(seg->cfg->dict,buf->data,buf->pos,0);
			//wtk_debug("[%.*s]=%p feq=%f\n",buf->pos,buf->data,wrd,wrd?wrd->freq:0);
			if(wrd)
			{
				//wtk_debug("[%.*s]=%f\n",buf->pos,buf->data,wrd->freq);
				if(i>0)
				{
					if(best_prob[i-1]!=0)
					{
						prev_weight=best_prob[i-1];
					}else
					{
						prev_weight=0;
						jx=i-1;
						while(jx>=0)
						{
							if(best_prob[jx]!=0)
							{
								prev_weight+=best_prob[jx];
								break;
							}else
							{
								prev_weight+=seg->cfg->def_weight;
							}
							--jx;
						}
					}
					//wtk_debug("best_prob[%d]=%f/%f\n",i,best_prob[i],prev_weight);
					//exit(0);
				}else
				{
					prev_weight=0;
				}
				f=prev_weight+wrd->freq;
				//wtk_debug("f[%d/%d]=%f/%f [%.*s] %f\n",i,j,f,best_prob[j],buf->pos,buf->data,wrd->freq);
				if(best_prob[j]==0 || f>best_prob[j])
				{
					//wtk_debug("set best %d prev=%d f=%f/%f\n",j,i,f,best_prob[j]);
					prev[j]=i;
					best_prob[j]=f;
				}
			}
			if(k>=seg->cfg->max_char)
			{
				break;
			}
		}
	}
	//wtk_debug("prev=%f idx=%d\n",best_prob[n-1],prev[n-1]);
	k=0;
	for(i=n-1;i>=0;)
	{
		if(prev[i]==-1)
		{
			--i;
		}else
		{
			i=prev[i]-1;
		}
		++k;
	}
	//wtk_poseg_print_input(seg);
	output=(wtk_string_t**)wtk_heap_malloc(heap,sizeof(wtk_string_t*)*k);
	seg->wrds=output;
	seg->nwrd=k;
	//wtk_debug("n=%d\n",n);
	for(i=n-1;i>=0;)
	{
		if(prev[i]==-1)
		{
			//wtk_debug("[%.*s]\n",input[i]->len,input[i]->data);
			output[--k]=input[i];
			--i;
		}else
		{
			wtk_strbuf_reset(buf);
			//wtk_debug("prev[%d]=%d\n",i,prev[i]);
			for(j=prev[i];j<=i;++j)
			{
				wtk_strbuf_push(buf,input[j]->data,input[j]->len);
			}
			//wtk_debug("[%.*s]\n",buf->pos,buf->data);
			//exit(0);
			i=prev[i]-1;
			output[--k]=wtk_heap_dup_string(heap,buf->data,buf->pos);
		}
	}
	wtk_strbuf_delete(buf);
	//wtk_poseg_update_seg_merge(seg);
	wtk_poseg_update_pos(seg);
	wtk_poseg_update_merge(seg);
	//wtk_poseg_print_output(seg);
	//exit(0);
}
