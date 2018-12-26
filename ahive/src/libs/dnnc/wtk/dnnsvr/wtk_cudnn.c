#include "wtk_cudnn.h"


wtk_cudnn_t* wtk_cudnn_new(wtk_cudnn_cfg_t *cfg)
{
	wtk_cudnn_t *dnn;
	int i;

	dnn=(wtk_cudnn_t*)wtk_malloc(sizeof(wtk_cudnn_t));
	dnn->cfg=cfg;
	if(cfg->trans)
	{
		dnn->feat_size=cfg->trans->b->len/(cfg->win*2+1);
	}else
	{
		dnn->feat_size=cfg->gpu[0]->trans->b->len/(cfg->win*2+1);
	}
	dnn->robin=wtk_robin_new(cfg->win*2+1);
	for(i=0;i<dnn->robin->nslot;++i)
	{
		dnn->robin->r[i]=(float*)wtk_malloc(sizeof(float)*dnn->feat_size);
	}
	dnn->input_size=dnn->feat_size*dnn->robin->nslot;
	dnn->pv=(float**)wtk_calloc(dnn->robin->nslot,sizeof(float*));
	dnn->input=wtk_cudnn_matrix_new(cfg->cache_size,dnn->input_size);
	//exit(0);
	if(cfg->use_single)
	{
		dnn->env=wtk_cudnn_env_new(cfg,cfg->gpu?cfg->gpu[0]:NULL);
	}else
	{
		dnn->env=NULL;
	}
	dnn->raise_ths=NULL;
	dnn->raise=NULL;
	wtk_cudnn_reset(dnn);
	return dnn;
}

void wtk_cudnn_delete(wtk_cudnn_t *dnn)
{
	int i;

	wtk_free(dnn->pv);
	wtk_cudnn_matrix_delete(dnn->input);
	for(i=0;i<dnn->robin->nslot;++i)
	{
		wtk_free(dnn->robin->r[i]);
	}
	wtk_robin_delete(dnn->robin);
	if(dnn->env)
	{
		wtk_cudnn_env_delete(dnn->env);
	}
	wtk_free(dnn);
}

void wtk_cudnn_set_raise(wtk_cudnn_t *dnn,void *ths,wtk_cudnn_raise_f raise)
{
	dnn->raise_ths=ths;
	dnn->raise=raise;
}

void wtk_cudnn_reset(wtk_cudnn_t *dnn)
{
	wtk_queue_init(&(dnn->recv_q));
	wtk_robin_reset(dnn->robin);
	dnn->pos=0;
	dnn->idx=0;
	dnn->recv_idx=0;
	dnn->is_end=0;
	dnn->cnt=0;
	dnn->want_delete=0;
	dnn->calc_cnt=0;
	dnn->pos2=0;
}

void wtk_cudnn_update_input(wtk_cudnn_t *dnn,float **pv)
{
	float *pf,*pf2;
	int step=dnn->robin->nslot;
	int i,j,k;
	int feat_size=dnn->feat_size;

	pf=dnn->input->v+dnn->pos*dnn->input_size;
	for(i=0;i<step;++i)
	{
		pf2=pv[i];
		for(j=0,k=i;j<feat_size;++j,k+=step)
		{
			pf[k]=pf2[j];
		}
	}
	++dnn->pos;
}

//1. trans file:  (x+bias)*window    |1*1320|*|1320*1320|
void wtk_cudnn_update_trans(wtk_cudnn_t *dnn)
{
	float *pb,*pw;
	int i,j,k;
	float *pv;

	pv=dnn->input->v;
	for(i=0,k=0;i<dnn->pos;++i)
	{
		pb=dnn->cfg->trans->b->v;
		pw=dnn->cfg->trans->w->v;
		for(j=0;j<dnn->input_size;++j,++k)
		{
			pv[k]=(pv[k]+pb[j])*pw[j];
		}
		//print_float(pv,k);
		//exit(0);
	}
}



//void wtk_cudnn_raise(wtk_cudnn_t *dnn,wtk_cudnn_matrix_t *output)
//{
//	if(dnn->raise)
//	{
//		dnn->raise(dnn->raise_ths,output);
//	}
//}

void wtk_cudnn_update_dnn(wtk_cudnn_t *dnn)
{
	int ret;

	dnn->input->row=dnn->pos;
	//wtk_debug("row=%d use_cuda=%d\n",dnn->input->row,dnn->cfg->use_cuda);
	if(dnn->cfg->use_single)
	{
		if(dnn->cfg->use_cuda)
		{
			ret=wtk_cudnn_env_process(dnn->env,dnn->input);
			if(ret==0)
			{
				if(dnn->raise)
				{
					dnn->raise(dnn->raise_ths,dnn,dnn->env->host_output);
				}
				//wtk_cudnn_raise(dnn,dnn->env->host_output);
			}
		}else
		{
			ret=wtk_cudnn_env_process_cpu(dnn->env,dnn->input);
			if(ret==0)
			{
				if(dnn->raise)
				{
					dnn->raise(dnn->raise_ths,dnn,dnn->env->output[dnn->cfg->nlayer-1]);
				}
				//wtk_cudnn_raise(dnn,dnn->env->output[dnn->cfg->nlayer-1]);
			}
		}
	}else
	{
		if(dnn->raise)
		{
			dnn->raise(dnn->raise_ths,dnn,dnn->input);
		}
		++dnn->idx;
	}
	dnn->pos=0;
}

void wtk_cudnn_flush(wtk_cudnn_t *dnn,int is_end)
{
	wtk_robin_t *rb=dnn->robin;
	float **pv;
	int pad;
	int i,j;
	float *pf;

	if(rb->used<=dnn->cfg->win)
	{
		return;
	}
	pv=dnn->pv;
	pad=rb->nslot-rb->used;
	i=0;
	if(pad>0 && !is_end)
	{
		//if not end, add pad to front.
		pf=(float*)wtk_robin_at(rb,0);
		for(;i<pad;++i)
		{
			pv[i]=pf;
		}
	}
	for(j=0;j<rb->used;++j)
	{
		pv[i++]=(float*)wtk_robin_at(rb,j);
	}
	if(pad>0 && is_end)
	{
		pf=(float*)wtk_robin_at(rb,rb->used-1);
		for(j=0;j<pad;++j)
		{
			pv[i++]=pf;
		}
	}
	++dnn->pos2;
	//	if( (0 == d->dnn->cfg->skip_frame) || (f->index%d->dnn->cfg->skip_frame==1) )
	if(dnn->cfg->skip_frame==0 || dnn->pos2%dnn->cfg->skip_frame==1)
	{
		wtk_cudnn_update_input(dnn,pv);
		//wtk_debug("pos=%d cache=%d\n",dnn->pos,dnn->cfg->cache_size);
		if(dnn->pos>=dnn->cfg->cache_size)
		{
			wtk_cudnn_update_dnn(dnn);
		}
	}
	//exit(0);
}


void wtk_cudnn_feed(wtk_cudnn_t *dnn,float *f,int len,int is_end)
{
	float *pf;
	wtk_robin_t *rb=dnn->robin;

	//wtk_debug("len=%d\n",len);
	if(f)
	{
		//print_float(f,len);
		++dnn->cnt;
		pf=wtk_robin_next(rb);
		memcpy(pf,f,dnn->feat_size*sizeof(float));
		wtk_cudnn_flush(dnn,0);
	}
	//exit(0);
	if(is_end)
	{
		wtk_robin_pop(rb);
		while(rb->used>dnn->cfg->win)
		{
			wtk_cudnn_flush(dnn,1);
			wtk_robin_pop(rb);
		}
		if(dnn->pos>0)
		{
			wtk_cudnn_update_dnn(dnn);
		}
	}
}

void wtk_cudnn_feed2(wtk_cudnn_t *cudnn,float *f,int len,int is_end)
{
	int i;
	//static int ki=0;

	//print_float(f,10);
	//exit(0);
	len-=cudnn->feat_size;
	for(i=0;i<=len;i+=cudnn->feat_size)
	{
		//++ki;
		//wtk_debug("============= xki=%d ============\n",ki);
		wtk_cudnn_feed(cudnn,f,cudnn->feat_size,0);
		f+=cudnn->feat_size;
	}
	if(is_end)
	{
		wtk_cudnn_feed(cudnn,NULL,0,1);
	}
	cudnn->is_end=is_end;
}
