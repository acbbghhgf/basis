#include "wtk_cudnn_env.h"

#ifdef USE_GPU
#else
wtk_cudnn_env_t* wtk_cudnn_env_new(wtk_cudnn_cfg_t *cfg,wtk_cudnn_gpu_cfg_t *gpu_cfg)
{
	wtk_cudnn_env_t *env;
	int i;

	env=(wtk_cudnn_env_t*)wtk_malloc(sizeof(wtk_cudnn_env_t));
	env->cu_cfg=cfg;
	env->gpu_cfg=gpu_cfg;
	env->output=(wtk_cudnn_matrix_t**)wtk_calloc(cfg->nlayer,sizeof(wtk_cudnn_matrix_t*));
	env->host_output=NULL;
	env->input=NULL;//wtk_cudnn_matrix_new(cfg->cache_size,cfg->trans->b->len);
	for(i=0;i<cfg->nlayer;++i)
	{
		env->output[i]=wtk_cudnn_matrix_new(cfg->cache_size,cfg->layer[i]->w->row);//,cfg->layer[i]->w->col);
	}
	return env;
}

void wtk_cudnn_env_delete(wtk_cudnn_env_t *env)
{
	int i;

	if(env->host_output)
	{
		wtk_cudnn_matrix_delete(env->host_output);
	}
	for(i=0;i<env->cu_cfg->nlayer;++i)
	{
		wtk_cudnn_matrix_delete(env->output[i]);
	}
	wtk_free(env->output);
	wtk_free(env);
}

int wtk_cudnn_env_process(wtk_cudnn_env_t *env,wtk_cudnn_matrix_t *input)
{
	return -1;
}
#endif

void wtk_cudnn_env_update_cpu_trans(wtk_cudnn_env_t *env,wtk_cudnn_matrix_t *input)
{
	float *pb,*pw;
	int i,j,k;
	float *pv;

	pv=input->v;
	for(i=0,k=0;i<input->row;++i)
	{
		pb=env->cu_cfg->trans->b->v;
		pw=env->cu_cfg->trans->w->v;
		for(j=0;j<input->col;++j,++k)
		{
			pv[k]=(pv[k]+pb[j])*pw[j];
		}
		//print_float(pv,k);
		//exit(0);
	}
}

#include <math.h>

void wtk_cudnn_sigmoid(float *f,int n)
{
	int i;

	for(i=0;i<n;++i)
	{
		//wtk_debug("v[%d]=%f/%f\n",i,f[i],1.0+expf(-f[i]));
		//exit(0);
		f[i]=1.0/(1.0+expf(-f[i]));
	}
}

void wtk_cudnn_softmax(float *f,int n)
{
	float max,sum;
	float *p,*e;

	max=wtk_math_max(f,n);
	//print_float(f,n);
	sum=0;
	p=f;e=p+n;
	while(p<e)
	{
		*p=expf(*p-max);
		sum+=*p;
		++p;
	}
	//wtk_debug("======== sum=%f n=%d max=%f ==========\n",sum,n,max);
	sum=1.0f/sum;
	p=f;e=p+n;
	while(p<e)
	{
		*(p)=log(*p*sum);
		++p;
	}
}


void wtk_cudnn_env_process_layer(wtk_cudnn_env_t *dnn,wtk_cudnn_layer_t *layer,wtk_cudnn_matrix_t *input,wtk_cudnn_matrix_t *output)
{
	int i,j,k;
	float f;
	float *pf1,*pf2,*pf3,*pf4;
	wtk_cudnn_matrix_t *w;

	//wtk_debug("|%d/%d|*|%d/%d|=|%d/%d|\n",input->row,input->col,layer->w->row,layer->w->col,output->row,output->col);
	pf1=input->v;
	pf3=output->v;
	w=layer->w;
	pf4=layer->b->v;
	output->row=input->row;
	for(i=0;i<input->row;++i)
	{
		pf2=layer->w->v;
		for(j=0;j<w->row;++j)
		{
			f=0;
			for(k=0;k<w->col;++k)
			{
				f+=pf1[k]*pf2[k];
			}
			//output[i][j]=f;
			*(pf3++)=f+pf4[j];
			pf2+=w->col;
		}
		pf1+=input->col;
		//print_float(pf3-w->row,10);
		//exit(0);
		switch(layer->type)
		{
		case wtk_dnn_sigmoid:
			//wtk_debug("row=%d col=%d\n",wtk_matrix_rows(output),wtk_matrix_cols(output));
			//print_float(pf3-w->row,10);
			wtk_cudnn_sigmoid(pf3-w->row,w->row);
			//print_float(pf3-w->row,10);
			//exit(0);
//			if(i<2)
//			{
//				print_float(pf3-w->row,10);
//			}
			break;
		case wtk_dnn_softmax:
			//print_float(pf3-w->row,20);
			if(!dnn->cu_cfg->use_linear_output)
			{
				wtk_cudnn_softmax(pf3-w->row,w->row);
			}
//			print_float(pf3-w->row,10);
//			if(i<2)
//			{
//				exit(0);
//			}
			break;
		case wtk_dnn_linear:
			break;
		default:
	        wtk_debug("layer->type not in list. %d\n", layer->type);
	        break;
		}
		//print_float(pf3-w->row,w->row);
		//exit(0);
	}
	//exit(0);
}

int wtk_cudnn_env_process_cpu(wtk_cudnn_env_t *env,wtk_cudnn_matrix_t *input)
{
	wtk_cudnn_cfg_t *cfg=env->cu_cfg;
	int i;
	wtk_cudnn_matrix_t *output;

	//print_float(dnn->input->v+dnn->input->col*13,20);
	//print_float(input->v,10);
	//wtk_debug("input row=%d col=%d\n",input->row,input->col);
	//print_float(input->v,10);//input->col);
	//print_float(input->v+input->col,10);//input->col);
	//print_float(input->v+input->col,10);
	wtk_cudnn_env_update_cpu_trans(env,input);
	//print_float(input->v,10);//input->col);
	//print_float(input->v+input->col,10);//input->col);
	//exit(0);
	//print_float(input->v,10);//input->col);
	//print_float(input->v+input->col,10);
	//print_float(dnn->input->v+dnn->input->col*1+1020,20);
	for(i=0;i<cfg->nlayer;++i)
	{
		output=env->output[i];
		wtk_cudnn_env_process_layer(env,cfg->layer[i],input,output);
		input=output;
		//wtk_debug("output->col=%d\n",output->col);
//		wtk_debug("============= i=%d ================\n",i);
		//print_float(output->v,10);//output->col);
		//exit(0);
//		print_float(output->v+output->col,10);//output->col);
//		if(i==cfg->nlayer-1)
//		{
//			exit(0);
//		}
		//exit(0);
	}
	//print_float(output->v,10);//output->col);
	//print_float(output->v+output->col,10);//output->col);
	//exit(0);
	//exit(0);
//	{
//		static double f=0;
//		static int ki=0;
//		int i;
//
//		++ki;
//		for(i=0;i<output->row*output->col;++i)
//		{
//			f+=output->v[i];
//		}
//		//f+=wtk_float_sum(output->v,output->row*output->col);
//		wtk_debug("v[%d]=%f\n",ki,f);
//	}
	//exit(0);
	return 0;
}
