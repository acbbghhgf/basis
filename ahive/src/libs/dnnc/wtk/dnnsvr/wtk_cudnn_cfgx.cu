#include "wtk_cudnn_cfg.h"
#include "cuda.h"
#include "cuda_runtime_api.h"
#include "wtk_cudnn.h"
#include "cublas_v2.h"
#ifndef THREAD_PER_BLOCK
#define THREAD_PER_BLOCK 1024
#endif

#ifndef BLOCK_SIZE
#define BLOCK_SIZE 32
#endif


wtk_cudnn_gpu_cfg_t* wtk_cudnn_gpu_cfg_new(int idx)
{
	wtk_cudnn_gpu_cfg_t *cfg;

	cfg=(wtk_cudnn_gpu_cfg_t*)wtk_malloc(sizeof(wtk_cudnn_gpu_cfg_t));
	cfg->idx=idx;
	cfg->max_thread=0;
	cfg->nlayer=0;
	cfg->trans=NULL;
	cfg->layer=NULL;
	return cfg;
}

void wtk_cudnn_gpu_cfg_delete(wtk_cudnn_gpu_cfg_t *cfg)
{
	int i;

	if(cfg->trans)
	{
		wtk_cudnn_trans_delete_cuda(cfg->trans);
	}
	if(cfg->layer)
	{
		for(i=0;i<cfg->nlayer;++i)
		{
			wtk_cudnn_layer_delete_cuda(cfg->layer[i]);
		}
		wtk_free(cfg->layer);
	}
	wtk_free(cfg);
}


void print_float_cu(float *a,int n)
{
	float *v;

	v=(float*)wtk_malloc(n*sizeof(float));
	cudaMemcpy(v,a,n*sizeof(float),cudaMemcpyDeviceToHost);
	//print_float(v,m->row*m->col);
	print_float(v,n);
	wtk_free(v);
}

void wtk_cudnn_vector_delete_cuda(wtk_cudnn_vector_t *v)
{
	if(v->v)
	{
		cudaFree(v->v);
	}
	wtk_free(v);
}

void wtk_cudnn_trans_delete_cuda(wtk_cudnn_trans_t *v)
{
	if(v->b)
	{
		wtk_cudnn_vector_delete_cuda(v->b);
	}
	if(v->w)
	{
		wtk_cudnn_vector_delete_cuda(v->w);
	}
	wtk_free(v);
}

void wtk_cudnn_matrix_delete_cuda(wtk_cudnn_matrix_t *m)
{
	if(m->v)
	{
		cudaFree(m->v);
	}
	wtk_free(m);
}

void wtk_cuda_print_err()
{
	cudaError_t err;
	const char *msg;

	err=cudaGetLastError();
	msg=cudaGetErrorString(err);
	wtk_debug("cuda[%d]:%s\n",err,msg);
}

wtk_cudnn_matrix_t* wtk_cudnn_matrix_new_cuda(int row,int col)
{
	wtk_cudnn_matrix_t *m;
	int ret;

	m=(wtk_cudnn_matrix_t*)wtk_malloc(sizeof(wtk_cudnn_matrix_t));
	m->row=row;
	m->col=col;
	ret=cudaMalloc((void**)&(m->v),row*col*sizeof(float));
	if(ret!=0)
	{
		goto end;
	}
	ret=cudaMemset((m->v),0,row*col*sizeof(float));
	if(ret!=0)
	{
		wtk_cuda_print_err();
		goto end;
	}
end:
	if(ret!=0)
	{
		wtk_free(m);
		m=NULL;
	}
	return m;
}


void wtk_cudnn_layer_delete_cuda(wtk_cudnn_layer_t *layer)
{
	if(layer->b)
	{
		wtk_cudnn_vector_delete_cuda(layer->b);
	}
	if(layer->w)
	{
		wtk_cudnn_matrix_delete_cuda(layer->w);
	}
	wtk_free(layer);
}

wtk_cudnn_vector_t* wtk_cudnn_vector_host_to_cuda(wtk_cudnn_vector_t *vh)
{
	wtk_cudnn_vector_t *vd;
	int ret;
	int n=sizeof(float)*vh->len;

	vd=(wtk_cudnn_vector_t*)wtk_malloc(sizeof(wtk_cudnn_vector_t));
	vd->len=vh->len;
	vd->v=NULL;
	ret=cudaMalloc((void**)&(vd->v),n);
	if(ret!=0){goto end;}
	ret=cudaMemcpy(vd->v,vh->v,n,cudaMemcpyHostToDevice);
	if(ret!=0){goto end;}
end:
	if(ret!=0)
	{
		wtk_cudnn_vector_delete_cuda(vd);
		vd=NULL;
	}
	return vd;
}

int wtk_cudnn_cfg_update_prop(wtk_cudnn_cfg_t *cfg)
{
	cudaDeviceProp prop;
	int i,cnt;
	int ret;

	ret=cudaGetDeviceCount(&(cnt));
	if(ret!=0){goto end;}
	cfg->ngpu=cnt;
	cfg->gpu=(wtk_cudnn_gpu_cfg_t**)wtk_calloc(cnt,sizeof(wtk_cudnn_gpu_cfg_t*));
	for(i=0;i<cnt;++i)
	{
		ret=cudaGetDeviceProperties(&(prop),i);
		if(ret!=0){goto end;}
		cfg->gpu[i]=wtk_cudnn_gpu_cfg_new(i);
		cfg->gpu[i]->max_thread=prop.maxThreadsPerBlock;
	}
end:
	return ret;
}

wtk_cudnn_trans_t* wtk_cudnn_trans_new_cuda(wtk_cudnn_trans_t *trans)
{
	wtk_cudnn_trans_t *vt;
	int ret=-1;

	vt=(wtk_cudnn_trans_t*)wtk_malloc(sizeof(wtk_cudnn_trans_t));
	vt->b=NULL;
	vt->w=NULL;
	vt->b=wtk_cudnn_vector_host_to_cuda(trans->b);
	if(!vt->b){goto end;}
	vt->w=wtk_cudnn_vector_host_to_cuda(trans->w);
	if(!vt->w){goto end;}
	ret=0;
end:
	if(ret!=0)
	{
		wtk_cudnn_trans_delete_cuda(vt);
		vt=NULL;
	}
	return vt;
}

wtk_cudnn_matrix_t* wtk_cudnn_matrix_host_to_cuda(wtk_cudnn_matrix_t *mh,int blk_size)
{
	wtk_cudnn_matrix_t *md;
	int ret;
	int n;

	md=(wtk_cudnn_matrix_t*)wtk_malloc(sizeof(wtk_cudnn_matrix_t));
	md->row=mh->row;
	md->col=mh->col;
	md->v=NULL;
	n=((mh->row-1)/blk_size+1)*blk_size;
	//wtk_debug("row=%d/%d\n",m->row,n);
	n=n*mh->col*sizeof(float);
	ret=cudaMalloc((void**)&(md->v),n);
	if(ret!=0){goto end;}
	ret=cudaMemset(md->v,0,n);
	if(ret!=0){goto end;}
	n=mh->row*mh->col*sizeof(float);
	ret=cudaMemcpy(md->v,mh->v,n,cudaMemcpyHostToDevice);
	if(ret!=0){goto end;}
end:
	if(ret!=0)
	{
		wtk_cudnn_matrix_delete_cuda(md);
		md=NULL;
	}
	return md;
}

wtk_cudnn_vector_t* wtk_cudnn_vector_host_to_cuda2(wtk_cudnn_vector_t *vh,int blk_size)
{
	wtk_cudnn_vector_t *vd;
	int ret;
	int n;

	vd=(wtk_cudnn_vector_t*)wtk_malloc(sizeof(wtk_cudnn_vector_t));
	vd->len=vh->len;
	n=((vh->len-1)/blk_size+1)*blk_size;
	//wtk_debug("n=%d len=%d\n",n,v->len);
	n=sizeof(float)*n;//v->len;
	ret=cudaMalloc((void**)&(vd->v),n);
	if(ret!=0){goto end;}
	ret=cudaMemset(vd->v,0,n);
	if(ret!=0){goto end;}
	ret=cudaMemcpy(vd->v,vh->v,vh->len*sizeof(float),cudaMemcpyHostToDevice);
	if(ret!=0){goto end;}
end:
	if(ret!=0)
	{
		wtk_cudnn_vector_delete_cuda(vd);
		vd=NULL;
	}
	return vd;
}


wtk_cudnn_layer_t* wtk_cudnn_layer_new_cuda(wtk_cudnn_layer_t *layer,int blk_size)
{
	wtk_cudnn_layer_t *ld;
	int ret;

	ld=(wtk_cudnn_layer_t*)wtk_malloc(sizeof(wtk_cudnn_layer_t));
	ld->type=layer->type;
	ld->b=NULL;
	ld->w=NULL;
	ld->b=wtk_cudnn_vector_host_to_cuda2(layer->b,blk_size);
	if(!ld->b){ret=-1;goto end;}
	ld->w=wtk_cudnn_matrix_host_to_cuda(layer->w,blk_size);
	if(!ld->w){ret=-1;goto end;}
	ret=0;
end:
	if(ret!=0)
	{
		wtk_cudnn_layer_delete_cuda(ld);
		ld=NULL;
	}
	return ld;
}


int wtk_cudnn_cfg_update_cuda(wtk_cudnn_cfg_t *cfg)
{
	wtk_cudnn_gpu_cfg_t *gpu;
	int ret;
	int i,j;

	ret=wtk_cudnn_cfg_update_prop(cfg);
	if(ret!=0){goto end;}
	if(cfg->blk_size>BLOCK_SIZE)
	{
		wtk_debug("blk_size %d/%d wrong.\n",cfg->blk_size,BLOCK_SIZE);
		ret=-1;
		goto end;
	}
	for(i=0;i<cfg->ngpu;++i)
	{
		cudaSetDevice(i);
		gpu=cfg->gpu[i];
		gpu->trans=wtk_cudnn_trans_new_cuda(cfg->trans);
		if(!gpu->trans){ret=-1;goto end;}
		gpu->nlayer=cfg->nlayer;
		gpu->layer=(wtk_cudnn_layer_t**)wtk_malloc(sizeof(wtk_cudnn_layer_t*)*cfg->nlayer);
		for(j=0;j<cfg->nlayer;++j)
		{
			gpu->layer[j]=wtk_cudnn_layer_new_cuda(cfg->layer[j],cfg->blk_size);
			if(!gpu->layer[j]){ret=-1;goto end;}
		}
	}
	if(cfg->trans)
	{
		wtk_cudnn_trans_delete(cfg->trans);
		cfg->trans=NULL;
	}
	if(cfg->layer)
	{
		for(i=0;i<cfg->nlayer;++i)
		{
			wtk_cudnn_layer_delete(cfg->layer[i]);
		}
		wtk_free(cfg->layer);
		cfg->layer=NULL;
	}
	ret=0;
end:
	return ret;
}


#include "wtk_cudnn_env.h"

wtk_cudnn_matrix_t* wtk_cudnn_matrix_new_cuda2(int row,int col,int blk_size)
{
	wtk_cudnn_matrix_t *md;
	int ret;
	int n;

	md=(wtk_cudnn_matrix_t*)wtk_malloc(sizeof(wtk_cudnn_matrix_t));
	md->row=row;
	md->col=col;
	md->v=NULL;
	n=((row-1)/blk_size+1)*blk_size;
	//wtk_debug("row=%d/%d\n",m->row,n);
	n=n*col*sizeof(float);
	ret=cudaMalloc((void**)&(md->v),n);
	if(ret!=0){goto end;}
	ret=cudaMemset(md->v,0,n);
end:
	if(ret!=0)
	{
		wtk_cudnn_matrix_delete_cuda(md);
		md=NULL;
	}
	return md;
}


wtk_cudnn_env_t* wtk_cudnn_env_new(wtk_cudnn_cfg_t *cfg,wtk_cudnn_gpu_cfg_t *gpu_cfg)
{
	wtk_cudnn_env_t *env;
	int i;

	env=(wtk_cudnn_env_t*)wtk_malloc(sizeof(wtk_cudnn_env_t));
	env->cu_cfg=cfg;
	env->gpu_cfg=gpu_cfg;
	if(gpu_cfg)
	{
		cudaSetDevice(gpu_cfg->idx);
	}
	env->output=(wtk_cudnn_matrix_t**)wtk_calloc(cfg->nlayer,sizeof(wtk_cudnn_matrix_t*));
	env->host_output=NULL;
	if(cfg->use_cuda)
	{
		env->input=wtk_cudnn_matrix_new_cuda(cfg->cache_size,gpu_cfg->trans->b->len);
		for(i=0;i<cfg->nlayer;++i)
		{
			//env->output[i]=wtk_cudnn_matrix_new_cuda2(cfg->cache_size,gpu_cfg->layer[i]->w->row,cfg->blk_size);
			env->output[i]=wtk_cudnn_matrix_new_cuda(cfg->cache_size,gpu_cfg->layer[i]->w->row);
			if(i==cfg->nlayer-1)
			{
				env->host_output=wtk_cudnn_matrix_new(cfg->cache_size,gpu_cfg->layer[i]->w->row);
			}
		}
	}else
	{
		env->input=NULL;//wtk_cudnn_matrix_new(cfg->cache_size,cfg->trans->b->len);
		for(i=0;i<cfg->nlayer;++i)
		{
			env->output[i]=wtk_cudnn_matrix_new(cfg->cache_size,cfg->layer[i]->w->row);//,cfg->layer[i]->w->col);
		}
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
	if(env->input)
	{
		wtk_cudnn_matrix_delete_cuda(env->input);
	}
	for(i=0;i<env->cu_cfg->nlayer;++i)
	{
		if(env->cu_cfg->use_cuda)
		{
			wtk_cudnn_matrix_delete_cuda(env->output[i]);
		}else
		{
			wtk_cudnn_matrix_delete(env->output[i]);
		}
	}
	wtk_free(env->output);
	wtk_free(env);
}



__global__ void wtk_cudnn_update_bias_window(float *c,float *b,float *w,int col)
{
	int i,j;
	int threads=blockDim.x;
	int idx=threadIdx.x;

	i=blockIdx.x*col+idx;
	c[i]=(c[i]+b[idx])*w[idx];
	//c[i]=(c[i]+b[j])*w[j];
	for(j=i+threads,i=idx+threads;i<col;j+=threads,i+=threads)
	{
		c[j]=(c[j]+b[i])*w[i];
	}
}

int wtk_cudnn_env_update_trans(wtk_cudnn_env_t *env)
{
	wtk_cudnn_trans_t *trans=env->gpu_cfg->trans;
	int thread=env->gpu_cfg->max_thread;

	wtk_cudnn_update_bias_window<<<env->input->row,thread>>>(env->input->v,trans->b->v,trans->w->v,env->input->col);
	return 0;
}

void wtk_cudnn_matrix_print_cuda(wtk_cudnn_matrix_t *m)
{
	float *v;
	int n;

	n=m->row*m->col*sizeof(float);
	v=(float*)wtk_malloc(n);
	cudaMemcpy(v,m->v,n,cudaMemcpyDeviceToHost);
	//print_float(v,m->row*m->col);
	print_float(v,1*m->col);
	wtk_free(v);
}


void print_int_cu(int *a,int n)
{
	int *v;

	v=(int*)wtk_malloc(n*sizeof(int));
	cudaMemcpy(v,a,n*sizeof(int),cudaMemcpyDeviceToHost);
	//print_float(v,m->row*m->col);
	print_int(v,n);
	wtk_free(v);
}


__global__ void wtk_cudnn_sigmoid_cuda(float *f,int n)
{
	int i=(blockIdx.x*blockDim.x+threadIdx.x);

	if(i<n)
	{
		f[i]=1.0/(1.0+expf(-f[i]));
	}
}

__global__ void wtk_cudnn_softmax_cuda(float *f,int row,int col)
{
	int j=blockIdx.x;
	int threads=blockDim.x;
	int idx=threadIdx.x;
	int i,ki;
	__shared__ float aux[THREAD_PER_BLOCK];
	float max,sum;

	//find max;
	f+=j*col;
	aux[idx]=f[idx];
	for(ki=idx+threads;ki<col;ki+=threads)
	{
		if(aux[idx]<f[ki])
		{
			aux[idx]=f[ki];
		}
	}
	__syncthreads();
	ki=threads;
	while(ki>1)
	{
		i=((1+ki)>>1);//divide by two;
		if(idx<i)
		{
			j=idx+i;
			if(j<ki && aux[idx]<aux[j])
			{
				aux[idx]=aux[j];
			}
		}
		__syncthreads();
		ki=i;
	}
	max=aux[0];
	//f[0]=max;
	//return;
	__syncthreads();
	aux[idx]=f[idx]=expf(f[idx]-max);
	for(ki=idx+threads;ki<col;ki+=threads)
	{
		aux[idx]+=f[ki]=expf(f[ki]-max);
	}
	//return;
	__syncthreads();
	ki=threads;
	while(ki>1)
	{
		i=((1+ki)>>1);//divide by two;
		if(idx<i)
		{
			j=idx+i;
			if(j<ki)
			{
				aux[idx]+=aux[j];
			}
		}
		__syncthreads();
		ki=i;
	}
	sum=1.0/aux[0];
	//f[0]=sum;
	//return;
	__syncthreads();
	for(ki=idx;ki<col;ki+=threads)
	{
		f[ki]=log(f[ki]*sum);
	}
}

void wtk_cudnn_softmax2(float *f,int n)
{
	float max,sum;
	float *p,*e;

	max=wtk_math_max(f,n);
	//wtk_debug("max=%f\n",max);
	sum=0;
	p=f;e=p+n;
	while(p<e)
	{
		*p=expf(*p-max);
		//wtk_debug("%f\n",*p);
		sum+=*p;
		++p;
	}
	sum=1.0f/sum;
	//wtk_debug("sum=%f\n",sum);
	p=f;e=p+n;
	while(p<e)
	{
		*(p)=log(*p*sum);
		++p;
	}
}

#define wtk_cudnn_blk_mul(f,pa2,pb2,blk_size) \
		switch(blk_size) \
		{ \
		case 2: \
			f+=pa2[0]*pb2[0]; \
			f+=pa2[1]*pb2[1]; \
			break; \
		case 4: \
			f+=pa2[0]*pb2[0]; \
			f+=pa2[1]*pb2[1]; \
			f+=pa2[2]*pb2[2]; \
			f+=pa2[3]*pb2[3]; \
			break; \
		case 8: \
			f+=pa2[0]*pb2[0]; \
			f+=pa2[1]*pb2[1]; \
			f+=pa2[2]*pb2[2]; \
			f+=pa2[3]*pb2[3]; \
			f+=pa2[4]*pb2[4]; \
			f+=pa2[5]*pb2[5]; \
			f+=pa2[6]*pb2[6]; \
			f+=pa2[7]*pb2[7]; \
			break; \
		case 16: \
			f+=pa2[0]*pb2[0]; \
			f+=pa2[1]*pb2[1]; \
			f+=pa2[2]*pb2[2]; \
			f+=pa2[3]*pb2[3]; \
			f+=pa2[4]*pb2[4]; \
			f+=pa2[5]*pb2[5]; \
			f+=pa2[6]*pb2[6]; \
			f+=pa2[7]*pb2[7]; \
			f+=pa2[8]*pb2[8]; \
			f+=pa2[9]*pb2[9]; \
			f+=pa2[10]*pb2[10]; \
			f+=pa2[11]*pb2[11]; \
			f+=pa2[12]*pb2[12]; \
			f+=pa2[13]*pb2[13]; \
			f+=pa2[14]*pb2[14]; \
			f+=pa2[15]*pb2[15]; \
			break; \
		case 32: \
			f+=pa2[0]*pb2[0]; \
			f+=pa2[1]*pb2[1]; \
			f+=pa2[2]*pb2[2]; \
			f+=pa2[3]*pb2[3]; \
			f+=pa2[4]*pb2[4]; \
			f+=pa2[5]*pb2[5]; \
			f+=pa2[6]*pb2[6]; \
			f+=pa2[7]*pb2[7]; \
			f+=pa2[8]*pb2[8]; \
			f+=pa2[9]*pb2[9]; \
			f+=pa2[10]*pb2[10]; \
			f+=pa2[11]*pb2[11]; \
			f+=pa2[12]*pb2[12]; \
			f+=pa2[13]*pb2[13]; \
			f+=pa2[14]*pb2[14]; \
			f+=pa2[15]*pb2[15]; \
			f+=pa2[16]*pb2[16]; \
			f+=pa2[17]*pb2[17]; \
			f+=pa2[18]*pb2[18]; \
			f+=pa2[19]*pb2[19]; \
			f+=pa2[20]*pb2[20]; \
			f+=pa2[21]*pb2[21]; \
			f+=pa2[22]*pb2[22]; \
			f+=pa2[23]*pb2[23]; \
			f+=pa2[24]*pb2[24]; \
			f+=pa2[25]*pb2[25]; \
			f+=pa2[26]*pb2[26]; \
			f+=pa2[27]*pb2[27]; \
			f+=pa2[28]*pb2[28]; \
			f+=pa2[29]*pb2[29]; \
			f+=pa2[30]*pb2[30]; \
			f+=pa2[31]*pb2[31]; \
			break; \
		default: \
			{ \
				int k; \
				for(k=0;k<blk_size;++k) \
				{ \
					f+=pa2[k]*pb2[k]; \
				} \
			} \
		}


__global__ void wtk_cudnn_update_layer_wb(float *output,float *input,float *w,float *b,int col,int row,int mrow)
{
	int n,ki,kj,i;
	float f;

	n=(blockIdx.x*blockDim.x+threadIdx.x);
	ki=n/row;
	if(ki>=mrow)
	{
		return;
	}
	input+=ki*col;
	kj=n-ki*row;
	w+=kj*col;
	f=0;
	for(i=0;i<col;++i)
	{
		f+=input[i]*w[i];
	}
	//f+=b[j];
	//output[i][j]=f+b[j];
	output[ki*row+kj]=f+b[kj];
}


__global__ void wtk_cudnn_mat_mult_cuda_bias2(float *c,float *a,float *b,int blk_size,int acol,int arow,int brow,float *bias)
{
	int bx=blockIdx.x;
	int by=blockIdx.y;
	int tx=threadIdx.x;
	int ty=threadIdx.y;
	float f;
	int i,j,step;
    __shared__ float As[BLOCK_SIZE][BLOCK_SIZE];
    __shared__ float Bs[BLOCK_SIZE][BLOCK_SIZE];
    int ky,kx1;
    float *pa1,*pb1,*pa2,*pb2;

	step=acol*ty+tx;
	kx1=blk_size*by;
    ky=kx1+ty;
	i=acol*kx1+step;
    kx1=blk_size*bx;
    //kx=kx1+ty;
	j=acol*kx1+step;
	kx1+=tx;
	pa1=As[ty]+tx;
	pb1=Bs[ty]+tx;
	pa2=As[ty];
	pb2=Bs[tx];
	f=0;
	//if(ky<arow || kx<brow || ((acol/step)*step!=acol))
	if(acol%blk_size>0) //((acol/step)*step!=acol))
	{
		for(step=0;step<acol;step+=blk_size)
		{
			if(tx+step<acol)
			{
				*pa1=a[i+step];
				*pb1=b[j+step];
			}else
			{
				*pa1=0;
				*pb1=0;
			}
			__syncthreads();
			wtk_cudnn_blk_mul(f,pa2,pb2,blk_size)
			__syncthreads();
		}
	}else
	{
		for(step=0;step<acol;step+=blk_size)
		{
			*pa1=a[i+step];
			*pb1=b[j+step];
			__syncthreads();
			wtk_cudnn_blk_mul(f,pa2,pb2,blk_size)
			__syncthreads();
		}
		//c[brow*ky+blk_size*bx+tx]=f+bias[kx];
	}
	if(ky<arow && kx1<brow)
	{
		c[brow*ky+kx1]=f+bias[kx1];
	}
}

__global__ void wtk_cudnn_mat_mult_cuda_bias3(float *c,float *a,float *b,int blk_size,int acol,int arow,int brow,float *bias)
{
    __shared__ float As[BLOCK_SIZE][BLOCK_SIZE];
    __shared__ float Bs[BLOCK_SIZE][BLOCK_SIZE];
    int tx=threadIdx.x;
    int ty=threadIdx.y;
    int bx=blockIdx.x;
    int by=blockIdx.y;
    int ky,kx;
    int step;
    float f;
    float *pa1,*pb1;
    float *pa2,*pb2;
    int i,j;

    pa1=As[ty]+tx;
    pb1=Bs[ty]+tx;
    i=acol*(blk_size*by+ty)+tx;
    j=acol*(blk_size*bx+ty)+tx;
    pa2=As[ty];
    pb2=Bs[tx];
    f=0;
	if(acol%blk_size>0) //((acol/step)*step!=acol))
	{
		for(step=0;step<acol;step+=blk_size)
		{
			if(tx+step<acol)
			{
				*pa1=a[i+step];
				*pb1=b[j+step];
			}else
			{
				*pa1=0;
				*pb1=0;
			}
			__syncthreads();
			wtk_cudnn_blk_mul(f,pa2,pb2,blk_size)
			__syncthreads();
		}
	}else
	{
		for(step=0;step<acol;step+=blk_size)
		{
			*pa1=a[i+step];
			*pb1=b[j+step];
			__syncthreads();
			wtk_cudnn_blk_mul(f,pa2,pb2,blk_size)
			__syncthreads();
		}
	}
    ky=blk_size*by+ty;
    if(ky<arow)
    {
    	kx=blk_size*bx+tx;
    	if(kx<brow)
    	{
    		c[brow*ky+kx]=f+bias[kx];
    	}
    }
}

__global__ void wtk_cudnn_mat_mult_cuda_bias4(float *c,float *a,float *b,int blk_size,int acol,int arow,int brow,float *bias)
{
    __shared__ float As[BLOCK_SIZE][BLOCK_SIZE];
    __shared__ float Bs[BLOCK_SIZE][BLOCK_SIZE];
    int tx=threadIdx.x;
    int ty=threadIdx.y;
    int bx=blockIdx.x;
    int by=blockIdx.y;
    int ky,kx;

    float f;
    float *pa1,*pb1;
    float *pa2,*pb2;
    int i,j;
    int endi,endi2;

    pa1=As[ty]+tx;
    pb1=Bs[ty]+tx;
    ky=blk_size*by+ty;
    i=acol*ky+tx;
    endi=i+acol;
	//kx=blk_size*bx+tx;
    kx=blk_size*bx;
    j=acol*(kx+ty)+tx;
    kx+=tx;
    pa2=As[ty];
    pb2=Bs[tx];
    f=bias[kx];
	if(acol%blk_size>0) //((acol/step)*step!=acol))
	{
		endi2=endi-tx;
		for(;i<endi;i+=blk_size,j+=blk_size)
		{
			if(i<endi2)
			{
				*pa1=a[i];
				*pb1=b[j];
			}else
			{
				*pa1=0;
				*pb1=0;
			}
			__syncthreads();
			wtk_cudnn_blk_mul(f,pa2,pb2,blk_size)
			__syncthreads();
		}
	}else
	{
		for(;i<endi;i+=blk_size,j+=blk_size)
		{
			*pa1=a[i];
			*pb1=b[j];
			__syncthreads();
			wtk_cudnn_blk_mul(f,pa2,pb2,blk_size)
			__syncthreads();
		}
	}
    if(ky<arow && kx<brow)
    {
    	c[brow*ky+kx]=f;
    }
}

int wtk_cudnn_process_layer_cuda(wtk_cudnn_env_t *env,wtk_cudnn_layer_t *layer,wtk_cudnn_matrix_t *input,wtk_cudnn_matrix_t *output)
{
	int thread=env->gpu_cfg->max_thread;
	int nx,ny;
	dim3 blk,grid;
	int blk_size=env->cu_cfg->blk_size;
	int arow,brow;

	output->row=input->row;
	arow=input->row;
	brow=layer->w->row;
	blk.z=1;
	blk.x=blk_size;
	blk.y=blk_size;
	grid.z=1;
	grid.x=(brow-1)/blk_size+1;
	grid.y=(arow-1)/blk_size+1;
	//wtk_cudnn_update_layer_wb<<<ny,nx>>>(output->v,input->v,layer->w->v,layer->b->v,input->col,layer->w->row,input->row);
	//wtk_debug("col=%d row=%d/%d\n",input->col,arow,brow);
	//cudaMemset(output->v,0,output->row*output->col*sizeof(float));
	wtk_cudnn_mat_mult_cuda_bias4<<<grid,blk>>>(output->v,input->v,layer->w->v,blk_size,input->col,arow,brow,layer->b->v);
	switch(layer->type)
	{
	case wtk_dnn_sigmoid:
		//wtk_debug("row=%d col=%d\n",wtk_matrix_rows(output),wtk_matrix_cols(output));
		//wtk_cudnn_sigmoid(pf3-w->row,w->row);
		nx=output->row*output->col;
		ny=(int)((nx-1)/thread+1);
		//wtk_debug("nx=%d ny=%d thread=%d\n",nx,ny,thread);
		wtk_cudnn_sigmoid_cuda<<<ny,thread>>>(output->v,nx);
		//print_float_cu(output->v,layer->w->row);
		break;
	case wtk_dnn_softmax:
		//wtk_cudnn_softmax(pf3-w->row,w->row);
		//nx=(int)((output->col-1)/thread+1);
		//print_float_cu(output->v,layer->w->row);
		if(!env->cu_cfg->use_linear_output)
		{
			wtk_cudnn_softmax_cuda<<<output->row,thread>>>(output->v,output->row,output->col);
		}
		//print_float_cu(output->v,layer->w->row);
		//exit(0);
		break;
	case wtk_dnn_linear:
		break;
	default:
        wtk_debug("layer->type not in list. %d\n", layer->type);
        break;
	}
	//exit(0);
	return 0;
}


int wtk_cudnn_env_process(wtk_cudnn_env_t *env,wtk_cudnn_matrix_t *input)
{
	int n;
	int ret;
	int i;
	wtk_cudnn_matrix_t *output;
	wtk_cudnn_gpu_cfg_t *cfg=env->gpu_cfg;

	n=input->row*input->col*sizeof(float);
	ret=cudaMemcpy(env->input->v,input->v,n,cudaMemcpyHostToDevice);
	if(ret!=0){goto end;}
	env->input->row=input->row;
	//wtk_debug("n=%d/%d n=%d\n",input->row,input->col,n);
	//print_float(input->v+input->col*13,20);
	//print_float_cu(env->input->v+input->col*13,20);
	ret=wtk_cudnn_env_update_trans(env);
	if(ret!=0){goto end;}
	//print_float_cu(env->input->v,10);//env->input->col);
	input=env->input;
	for(i=0;i<cfg->nlayer;++i)
	{
		output=env->output[i];
		ret=wtk_cudnn_process_layer_cuda(env,cfg->layer[i],input,output);
		if(ret!=0){goto end;}
		//wtk_debug("========= i=%d ==========\n",i)
		//print_float_cu(output->v,10);//output->col);
		//exit(0);
		input=output;
	}
	env->host_output->row=output->row;
	ret=cudaMemcpy(env->host_output->v,output->v,output->row*output->col*sizeof(float),cudaMemcpyDeviceToHost);
	if(ret!=0){goto end;}
	//print_float(env->host_output->v,10);
	//exit(0);
	ret=0;
end:
	//exit(0);
	return ret;
}

void wtk_cudnn_test_cuda2()
{
#define N 30
	float pf[N];
	int i;
	float *dev_f;
	int ret;
	int nx;

	for(i=0;i<N;++i)
	{
		pf[i]=i;
	}
	ret=cudaMalloc((void**)&(dev_f),N*sizeof(float));
	if(ret!=0){goto end;}
	ret=cudaMemcpy(dev_f,pf,N*sizeof(float),cudaMemcpyHostToDevice);
	if(ret!=0){goto end;}
	//3*10
	//print_float(pf,N);
	nx=4;
	//nx=1;
	//step=(int)(10.0/nx+0.5);
	//wtk_debug("step=%d\n",step);
	wtk_cudnn_softmax_cuda<<<3,nx>>>(dev_f,1,10);
	//print_float_cu(dev_f,N);
	for(i=0;i<3;++i)
	{
		wtk_cudnn_softmax2(pf+i*10,10);
	}
	//exit(0);
	//print_float(pf,N);
end:
	//exit(0);
	return;
}

char* wtk_cuda_dup_data(void *a,int n)
{
	char *dev_a=NULL;
	int ret;

	ret=cudaMalloc((void**)&(dev_a),n);
	if(ret!=0){goto end;}
	if(a)
	{
		ret=cudaMemcpy(dev_a,a,n,cudaMemcpyHostToDevice);
		if(ret!=0){goto end;}
	}else
	{
		ret=cudaMemset(dev_a,0,n);
		if(ret!=0){goto end;}
	}
end:
	if(ret!=0)
	{
		if(dev_a)
		{
			cudaFree(dev_a);
		}
		dev_a=NULL;
	}
	return dev_a;
}

char* wtk_cuda_dup_dev_data(void *a,int n)
{
	char *v;

	v=(char*)wtk_malloc(n);
	cudaMemcpy(v,a,n,cudaMemcpyDeviceToHost);
	return v;
}


void wtk_cudnn_mat_mult(float *c,float *a,float *b,int arow,int acol,int brow)
{
	int i,j,k;
	float *pa,*pb;
	float f;

	for(i=0;i<arow;++i)
	{
		pa=a+i*acol;
		for(j=0;j<brow;++j)
		{
			pb=b+j*acol;
			f=0;
			for(k=0;k<acol;++k)
			{
				f+=pa[k]*pb[k];
			}
			//c[i][j]=f;
			c[i*brow+j]=f;
		}
	}
}



