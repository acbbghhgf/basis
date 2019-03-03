#include "wtk_cudnn_cfg.h" 
void wtk_cudnn_gpu_cfg_delete(wtk_cudnn_gpu_cfg_t *cfg);

wtk_cudnn_vector_t* wtk_cudnn_vector_new(int n)
{
	wtk_cudnn_vector_t *v;

	v=(wtk_cudnn_vector_t*)wtk_malloc(sizeof(wtk_cudnn_vector_t));
	v->len=n;
	v->v=(float*)wtk_calloc(n,sizeof(float));
	return v;
}

void wtk_cudnn_vector_delete(wtk_cudnn_vector_t *v)
{
	wtk_free(v->v);
	wtk_free(v);
}

void wtk_cudnn_trans_delete(wtk_cudnn_trans_t *v)
{
	if(v->b)
	{
		wtk_cudnn_vector_delete(v->b);
	}
	if(v->w)
	{
		wtk_cudnn_vector_delete(v->w);
	}
	wtk_free(v);
}

wtk_cudnn_layer_t* wtk_cudnn_layer_new()
{
	wtk_cudnn_layer_t *layer;

	layer=(wtk_cudnn_layer_t*)wtk_malloc(sizeof(wtk_cudnn_layer_t));
	layer->b=NULL;
	layer->w=NULL;
	layer->type=wtk_dnn_linear;
	return layer;
}

wtk_cudnn_matrix_t* wtk_cudnn_matrix_new(int row,int col)
{
	wtk_cudnn_matrix_t *m;

	m=(wtk_cudnn_matrix_t*)wtk_malloc(sizeof(wtk_cudnn_matrix_t));
	m->row=row;
	m->col=col;
	m->v=wtk_calloc(row*col,sizeof(float));
	return m;
}

void wtk_cudnn_matrix_delete(wtk_cudnn_matrix_t *m)
{
	wtk_free(m->v);
	wtk_free(m);
}

void wtk_cudnn_layer_delete(wtk_cudnn_layer_t *layer)
{
	if(layer->b)
	{
		wtk_cudnn_vector_delete(layer->b);
	}
	if(layer->w)
	{
		wtk_cudnn_matrix_delete(layer->w);
	}
	wtk_free(layer);
}

int wtk_cudnn_cfg_init(wtk_cudnn_cfg_t *cfg)
{
	cfg->use_linear_output=0;
	cfg->skip_frame=0;
	cfg->gpu=NULL;
	cfg->ngpu=0;
	cfg->cache_size=16;
	cfg->win=5;
	cfg->net_fn=NULL;
	cfg->trans_fn=NULL;
	cfg->nlayer=0;
	cfg->trans=NULL;
	cfg->layer=NULL;
	cfg->blk_size=16;
#ifdef	USE_GPU
	cfg->use_cuda=1;
#else
	cfg->use_cuda=0;
#endif
	cfg->use_single=1;
	cfg->debug=0;
	return 0;
}



int wtk_cudnn_cfg_clean(wtk_cudnn_cfg_t *cfg)
{
	int i;

	if(cfg->gpu)
	{
#ifdef USE_GPU
		for(i=0;i<cfg->ngpu;++i)
		{
			wtk_cudnn_gpu_cfg_delete(cfg->gpu[i]);
		}
		wtk_free(cfg->gpu);
#endif
	}
	if(cfg->trans)
	{
		if(cfg->use_cuda)
		{
#ifdef USE_GPU
			wtk_cudnn_trans_delete_cuda(cfg->trans);
#endif
		}else
		{
			wtk_cudnn_trans_delete(cfg->trans);
		}
	}
	if(cfg->layer)
	{
		for(i=0;i<cfg->nlayer;++i)
		{
			if(cfg->use_cuda)
			{
#ifdef USE_GPU
				wtk_cudnn_layer_delete_cuda(cfg->layer[i]);
#endif
			}else
			{
				wtk_cudnn_layer_delete(cfg->layer[i]);
			}
		}
		wtk_free(cfg->layer);
	}
	return 0;
}

int wtk_cudnn_cfg_update_local(wtk_cudnn_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_b(lc,cfg,debug,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_linear_output,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_single,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,net_fn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,trans_fn,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,cache_size,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,blk_size,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,win,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,skip_frame,v);
	return 0;
}

wtk_cudnn_vector_t* wtk_cudnn_load_trans_vector(wtk_source_t *src,wtk_strbuf_t *buf,
		char *name,int bin)
{
	wtk_cudnn_vector_t *v=0;
	int i;
	int ret;

	ret=wtk_source_read_string(src,buf);
	if(ret!=0){goto end;}
	//wtk_debug("[%.*s]\n",buf->pos,buf->data);
	if(strncmp(name,buf->data,buf->pos)!=0)
	{
		wtk_debug("[%s]!=[%.*s]\n",name,buf->pos,buf->data);
		ret=-1;goto end;
	}
	if(bin)
	{
		wtk_source_skip_sp3(src,NULL);
	}else
	{
		wtk_source_skip_sp(src,NULL);
	}
	ret=wtk_source_read_int(src,&i,1,bin);
	if(ret!=0){goto end;}
	ret=wtk_source_read_int(src,&i,1,bin);
	if(ret!=0){goto end;}
	ret=wtk_source_read_string(src,buf);
	if(ret!=0){goto end;}
	if(bin)
	{
		wtk_source_skip_sp3(src,NULL);
	}else
	{
		wtk_source_skip_sp(src,NULL);
	}
	ret=wtk_source_read_int(src,&i,1,bin);
	if(ret!=0){goto end;}
	//wtk_debug("i=%d\n",i);
	v=wtk_cudnn_vector_new(i);
	ret=wtk_source_read_float(src,v->v,v->len,bin);
	if(ret!=0){goto end;}
	//v->bytes=v->len*sizeof(float);
end:
	if(ret!=0 && v)
	{
		wtk_cudnn_vector_delete(v);
		v=0;
	}
	return v;
}

int wtk_cudnn_cfg_load_trans(wtk_cudnn_cfg_t *cfg,wtk_source_t *src)
{
	wtk_cudnn_trans_t *trans;
	wtk_strbuf_t *buf;
	int ret=-1;

	buf=wtk_strbuf_new(256,1);
	trans=(wtk_cudnn_trans_t*)wtk_calloc(1,sizeof(wtk_cudnn_trans_t));
	trans->b=wtk_cudnn_load_trans_vector(src,buf,"<bias>",0);
	if(!trans->b){goto end;}
	trans->w=wtk_cudnn_load_trans_vector(src,buf,"<window>",0);
	if(!trans->b){goto end;}
	ret=0;
end:
	if(ret!=0)
	{
		wtk_cudnn_trans_delete(trans);
	}else
	{
		cfg->trans=trans;
	}
	wtk_strbuf_delete(buf);
	return ret;
}



wtk_cudnn_layer_t* wtk_cudnn_cfg_load_layer(wtk_cudnn_cfg_t *cfg,wtk_source_t *src)
{
	wtk_cudnn_layer_t *l=0;
	wtk_strbuf_t *buf;
	int row,col,v;
	int ret=-1;
	int is_bin=0;

	buf=wtk_strbuf_new(256,1);
	l=wtk_cudnn_layer_new();
	//<biasedlinearity> 2048 429
	ret=wtk_source_read_string(src,buf);
	if(ret!=0){goto end;}
	if(!wtk_str_equal_s(buf->data,buf->pos,"<biasedlinearity>"))
	{
		wtk_debug("[%.*s] not support.\n",buf->pos,buf->data);
		ret=-1;
		goto end;
	}
	if(is_bin)
	{
		wtk_source_skip_sp3(src,NULL);
	}else
	{
		wtk_source_skip_sp(src,NULL);
	}
	ret=wtk_source_read_int(src,&row,1,is_bin);
	if(ret!=0){goto end;}
	ret=wtk_source_read_int(src,&col,1,is_bin);
	if(ret!=0){goto end;}
	//wtk_debug("row=%d,col=%d\n",row,col);
	ret=wtk_source_read_string(src,buf);
	if(ret!=0){goto end;}
	//wtk_debug("[%.*s]\n",buf->pos,buf->data);
	if(!wtk_str_equal_s(buf->data,buf->pos,"m"))
	{
		wtk_debug("[%.*s] not support.\n",buf->pos,buf->data);
		ret=-1;
		goto end;
	}
	if(is_bin)
	{
		wtk_source_skip_sp3(src,NULL);
	}else
	{
		wtk_source_skip_sp(src,NULL);
	}
	ret=wtk_source_read_int(src,&v,1,is_bin);
	if(ret!=0){goto end;}
	//wtk_debug("v=%d\n",v);
	if(v!=row)
	{
		wtk_debug("row[%d]!=v[%d]\n",row,v);
		ret=-1;
		goto end;
	}
	ret=wtk_source_read_int(src,&v,1,is_bin);
	if(ret!=0){goto end;}
	//wtk_debug("v=%d\n",v);
	if(v!=col)
	{
		wtk_debug("col[%d]!=v[%d]\n",col,v);
		ret=-1;
		goto end;
	}
	//wtk_debug("row=%d col=%d\n",row,col);
	l->w=wtk_cudnn_matrix_new(row,col);
	ret=wtk_source_read_float(src,l->w->v,row*col,is_bin);
	if(ret!=0)
	{
		wtk_debug("load matrix failed.\n");
		goto end;
	}
	wtk_source_read_string(src,buf);
	if(!wtk_str_equal_s(buf->data,buf->pos,"v"))
	{
		wtk_debug("[%.*s] not support.\n",buf->pos,buf->data);
		ret=-1;goto end;
	}
	if(is_bin)
	{
		wtk_source_skip_sp3(src,NULL);
	}else
	{
		wtk_source_skip_sp(src,NULL);
	}
	ret=wtk_source_read_int(src,&v,1,is_bin);
	if(ret!=0){goto end;}
	if(v!=row){ret=-1;goto end;}
	//wtk_debug("v=%d\n",v);
	l->b=wtk_cudnn_vector_new(v);
	ret=wtk_source_read_float(src,l->b->v,v,is_bin);
	if(ret!=0){goto end;}
	wtk_source_read_string(src,buf);
	if(is_bin)
	{
		wtk_source_skip_sp3(src,NULL);
	}else
	{
		wtk_source_skip_sp(src,NULL);
	}
	if(wtk_str_equal_s(buf->data,buf->pos,"<sigmoid>"))
	{
		l->type=wtk_dnn_sigmoid;
	}else if(wtk_str_equal_s(buf->data,buf->pos,"<softmax>"))
	{
		l->type=wtk_dnn_softmax;
	}else if(wtk_str_equal_s(buf->data,buf->pos,"<linear>"))
	{
		l->type=wtk_dnn_linear;
	}else
	{
		wtk_debug("[%.*s] not support\n",buf->pos,buf->data);
		ret=-1;
		goto end;
	}
	ret=wtk_source_read_int(src,&v,1,is_bin);
	if(ret!=0){goto end;}
	ret=wtk_source_read_int(src,&v,1,is_bin);
	if(ret!=0){goto end;}
end:
	if(ret!=0)
	{
		wtk_cudnn_layer_delete(l);
		l=0;
	}
	wtk_strbuf_delete(buf);
	return l;
}

int wtk_cudnn_cfg_load_net(wtk_cudnn_cfg_t *cfg,wtk_source_t *src)
{
	wtk_cudnn_layer_t *layer;
	wtk_larray_t *a;

	a=wtk_larray_new(20,sizeof(wtk_cudnn_layer_t*));
	while(1)
	{
		layer=wtk_cudnn_cfg_load_layer(cfg,src);
		if(!layer){break;}
		//wtk_debug("layer=%p\n",layer);
		wtk_larray_push2(a,&layer);
	}
	if(a->nslot>0)
	{
		cfg->layer=(wtk_cudnn_layer_t**)wtk_calloc(a->nslot,sizeof(wtk_cudnn_layer_t*));
		memcpy(cfg->layer,a->slot,a->nslot*a->slot_size);
		cfg->nlayer=a->nslot;
	}
	//wtk_debug("nslot=%d\n",a->nslot);
	//exit(0);
	wtk_larray_delete(a);
	return 0;
}


int wtk_cudnn_cfg_update(wtk_cudnn_cfg_t *cfg)
{
	int ret=-1;

	if(!cfg->trans_fn || !cfg->net_fn){goto end;}
	ret=wtk_source_load_file(cfg,(wtk_source_load_handler_t)wtk_cudnn_cfg_load_trans,cfg->trans_fn);
	if(ret!=0){goto end;}
	ret=wtk_source_load_file(cfg,(wtk_source_load_handler_t)wtk_cudnn_cfg_load_net,cfg->net_fn);
	if(ret!=0){goto end;}
	if(cfg->skip_frame!=0)
	{
		cfg->skip_frame+=1;
	}
	if(cfg->use_cuda)
	{
#ifdef USE_GPU
		/* ret=wtk_cudnn_cfg_update_cuda(cfg); */
		/* if(ret!=0){goto end;} */
#endif
	}
	ret=0;
end:
	return ret;
}

void wtk_cudnn_cfg_print(wtk_cudnn_cfg_t *cfg)
{
	printf("use_cuda: %d\n",cfg->use_cuda);
	printf("net_fn; %s\n",cfg->net_fn);
	printf("trans_fn: %s\n",cfg->trans_fn);
	printf("skip_frame: %d\n",cfg->skip_frame);
	printf("use_linear_output: %d\n",cfg->use_linear_output);
}
