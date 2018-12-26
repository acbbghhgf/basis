#include "wtk_matrix.h"
#include <math.h>

qtk_blas_matrix_t* qtk_blas_matrix_new(int row,int col)
{
	qtk_blas_matrix_t *m;
	int ret;

	m=wtk_malloc(sizeof(*m));
	m->row=row;
	m->col=col;
    //m->m=(float*)memalign(align,row*col*sizeof(float));
	m->m=0;
	
	ret=posix_memalign((void**)&(m->m), 4096, row*col*sizeof(float));
//	m->m=(float*)wtk_malloc(row*col*sizeof(float));
	if(ret!=0)
	{
		wtk_free(m);
		m=0;
	}else
        {
            memset(m->m,0,row*col*sizeof(float));
        }

	return m;
}

void qtk_blas_matrix_zero(qtk_blas_matrix_t *m)
{
	memset(m->m,0,sizeof(float) * (m->col)*(m->row));
}

void qtk_blas_matrix_delete(qtk_blas_matrix_t *m)
{
	wtk_free(m->m);
	wtk_free(m);
}

void qtk_blas_matrix_print(qtk_blas_matrix_t *m)
{

	int i,j;
	wtk_debug("================ blas matrix ================\n");
	for(i=0;i<m->row;++i)
	{
		for(j=0;j<m->col;j++){
			printf("v[%d][%d]=%f\n",i,j,*(m->m+m->col*i+j));
		}
	}
}

double wtk_fast_exp(double y)
{
#define EXP_A (1048576/M_LN2)
#define EXP_C 60801
	union {
		double d;
		struct {
			int j, i;
		} n;
	} d2i;

	d2i.n.j=0;
	d2i.n.i = EXP_A*(y)+(1072693248-EXP_C);
	return d2i.d;
}

float wtk_fast_exp2(float y)
{
#define EXP_A (1048576/M_LN2)
#define EXP_C 60801
	union {
		double d;
		struct {
			int j, i;
		} n;
	} d2i;

	d2i.n.j=0;
	d2i.n.i = EXP_A*(y)+(1072693248-EXP_C);
	return d2i.d;
}

wtk_double_matrix_t* wtk_double_matrix_init(char *p,int nrows,int ncols)
{
	int csize;
	double **m;
	int i;

	m=(double**)p;
	*((int*)p)=nrows;
	csize=wtk_double_vector_bytes(ncols);
	p+=wtk_round_word((nrows+1)*sizeof(double*));
	for(i=1;i<=nrows;++i,p+=csize)
	{
		*((int*)p)=ncols;
		m[i]=(double*)p;
	}
	return m;
}

wtk_double_matrix_t* wtk_double_matrix_new(int nrows,int ncols)
{
	char *p;

	p=(char*)wtk_malloc(wtk_double_matrix_bytes(nrows,ncols));
	return wtk_double_matrix_init(p,nrows,ncols);
}

wtk_double_matrix_t* wtk_double_matrix_new_h(wtk_heap_t *heap,int nrows,int ncols)
{
	char *p;

	p=(char*)wtk_heap_malloc(heap,wtk_double_matrix_bytes(nrows,ncols));
	return wtk_double_matrix_init(p,nrows,ncols);
}

wtk_smatrix_t* wtk_smatrix_newh(wtk_heap_t *h,int nrows,int ncols)
{
	float** m;
	char *p;
	int csize,j;

	p=(char*)wtk_heap_malloc(h,wtk_smatrix_bytes(nrows,ncols))+2*sizeof(void**);
	m=(float**)((char*)p);//+2*sizeof(void**));
	*(int*)m=nrows;
	csize=wtk_vector_bytes(ncols);
	p+=(nrows+1)*sizeof(float*);
	for(j=1;j<=nrows;++j,p+=csize)
	{
		*(int*)p=ncols;
		m[j]=(float*)p;
	}
	wtk_set_hook((void**)m,0);
	wtk_set_use((void**)m,0);
	return m;
}

wtk_matrix_t* wtk_matrix_init(char *p,int nrows,int ncols)
{
	float **m;
	int csize;
	int i;

	m=(float**)p;
	*((int*)p)=nrows;
	csize=wtk_vector_bytes(ncols);
	p+=wtk_round_word((nrows+1)*sizeof(float*));
	for(i=1;i<=nrows;++i,p+=csize)
	{
		*((int*)p)=ncols;
		m[i]=(float*)p;
	}
	return m;
}

wtk_matrix_t* wtk_matrix_new(int nrows,int ncols)
{
	char *p;

	p=(char*)wtk_calloc(1,wtk_matrix_bytes(nrows,ncols));
	return wtk_matrix_init(p,nrows,ncols);
}

int wtk_matrix_bytes2(wtk_matrix_t *m)
{
	int r,c;

	r=wtk_matrix_rows(m);
	c=wtk_matrix_cols(m);
	return wtk_matrix_bytes(r,c);
}

wtk_matrix_t* wtk_matrix_newh(wtk_heap_t* h,int nrows,int ncols)
{
	char *p;

	p=(char*)wtk_heap_malloc(h,wtk_matrix_bytes(nrows,ncols));
	return wtk_matrix_init(p,nrows,ncols);
}


//0.119
void wtk_matrix_multi2(wtk_matrix_t *m,wtk_matrix_t *a,wtk_matrix_t *b)
{
	int rows=wtk_matrix_rows(m);
	int cols=wtk_matrix_cols(m);
	int ac=wtk_matrix_cols(a);
	int i,j,k;
	double t;
	float *p;
//#define DEBUG_MXY

	//wtk_debug("rows=%d,cols=%d,ac=%d\n",rows,cols,ac);
	for(i=1;i<=rows;++i)
	{
		p=a[i];
		for(j=1;j<=cols;++j)
		{
			for(t=0,k=1;k<=ac;++k)
			{
				t+=p[k]*b[k][j];
#ifdef DEBUG_MXY
				wtk_debug("v[%d]=%f*%f/%f\n",k,p[k],b[k][j],t);//,k,j);
				if(k==10)
				{
					//exit(0);
				}
#endif
			}
			m[i][j]=t;
#ifdef DEBUG_MXY
			wtk_debug("v[%d][%d]=%f\n",i,j,m[i][j]);
			exit(0);
#endif
		}
	}
}

//0.133
void wtk_matrix_multi1(wtk_matrix_t *m,wtk_matrix_t *a,wtk_matrix_t *b)
{
	int rows=wtk_matrix_rows(m);
	int cols=wtk_matrix_cols(m);
	int ac=wtk_matrix_cols(a);
	int i,j,k;

	for(i=1;i<=rows;++i)
	{
		for(j=1;j<=cols;++j)
		{
			m[i][j]=0;
			for(k=1;k<=ac;++k)
			{
				m[i][j]+=a[i][k]*b[k][j];
				//wtk_debug("%d,%d,%d,%f,%f,%f\n",i,j,k,m[i][j],a[i][k],b[k][j]);
			}
		}
	}
}

//0.09
void wtk_matrix_multi5(wtk_matrix_t *m,wtk_matrix_t *a,wtk_matrix_t *b)
{
	int rows=wtk_matrix_rows(m);
	int cols=wtk_matrix_cols(m);
	int ac=wtk_matrix_cols(a);
	int i,k;
	float *pa,*pm;
	register float *tpm,*tpb;
	register float pak;
	register float *e;

	for(i=1;i<=rows;++i)
	{
		pa=a[i];pm=m[i];
		e=pm+cols;
		for(k=1;k<=ac;++k)
		{
			tpb=b[k];pak=pa[k];
			tpm=pm;
			//wtk_debug("%d/%d=%d\n",i,k,(int)(e-tpm));
			if(k==1)
			{
				while(tpm<e)
				{
					*(++tpm)=pak*(*(++tpb));
				}
			}else
			{
				while(tpm<e)
				{
					*(++tpm)+=pak*(*(++tpb));
				}
			}
		}
	}
}

#ifdef USE_ASM
#include <xmmintrin.h>


void wtk_matrix_multi(wtk_matrix_t *m,wtk_matrix_t *a,wtk_matrix_t *b)
{
	int rows=wtk_matrix_rows(m);
	int cols=wtk_matrix_cols(m);
	int ac=wtk_matrix_cols(a);
	int i,k;
	float *pa,*pm;
	register float *tpm,*tpb;
	register float pak;
	register float *e;
	register __m128 ma;

	//wtk_debug("m=%p a=%p b=%p\n",m,a,b);
	//wtk_debug("rows=%d cols=%d\n",rows,cols);
	for(i=1;i<=rows;++i)
	{
		pa=a[i];pm=m[i]+1;
		e=pm+cols;
		for(k=1;k<=ac;++k)
		{
			tpb=b[k]+1;pak=pa[k];
			tpm=pm;
			//wtk_debug("%d/%d=%d\n",i,k,(int)(e-tpm));
			ma=_mm_set_ps(pak,pak,pak,pak);
			if(k==1)
			{
				while(e-tpm>=4)
				{
					*((__m128*)tpb)=_mm_mul_ps(ma,*((__m128*)tpb));
					tpb+=4;
					tpm+=4;
					/*
					*(((__m128*)tpb)+1)=_mm_mul_ps(ma,*(((__m128*)tpb)+1));
					*(((__m128*)tpb)+2)=_mm_mul_ps(ma,*(((__m128*)tpb)+2));
					*(((__m128*)tpb)+3)=_mm_mul_ps(ma,*(((__m128*)tpb)+3));
					tpb+=16;
					tpm+=16;
					*/
				}
				while(tpm<e)
				{
					*(tpm++)=pak*(*(tpb++));
				}
			}else
			{
				while(e-tpm>=4)
				{
					*((__m128*)tpm)=_mm_add_ps(_mm_mul_ps(ma,*((__m128*)tpb)),*((__m128*)tpm));//*((__m128*)tpb));//,mb);
					tpb+=4;
					tpm+=4;
					/*
					*((__m128*)tpm+1)=_mm_add_ps(_mm_mul_ps(ma,*((__m128*)tpb+1)),*((__m128*)tpm+1));
					*((__m128*)tpm+2)=_mm_add_ps(_mm_mul_ps(ma,*((__m128*)tpb+2)),*((__m128*)tpm+2));
					*((__m128*)tpm+3)=_mm_add_ps(_mm_mul_ps(ma,*((__m128*)tpb+3)),*((__m128*)tpm+3));
					tpb+=16;
					tpm+=16;
					*/
				}
				while(tpm<e)
				{
					*(tpm++)+=pak*(*(tpb++));
				}
			}
		}
	}
}
#endif

void wtk_matrix_multi(wtk_matrix_t *m,wtk_matrix_t *a,wtk_matrix_t *b)
{
	int rows=wtk_matrix_rows(m);
	int cols=wtk_matrix_cols(m);
	int ac=wtk_matrix_cols(a);
	int i,k;
	float *pa,*pm;
	register float *tpm,*tpb;
	register float pak;
	register float *e;

	for(i=1;i<=rows;++i)
	{
		pa=a[i];pm=m[i];
		e=pm+cols;
		for(k=1;k<=ac;++k)
		{
			tpb=b[k];pak=pa[k];
			tpm=pm;
			//wtk_debug("%d/%d=%d\n",i,k,(int)(e-tpm));
			if(k==1)
			{

				while(e-tpm>=4)
				{
					*(++tpm)=pak*(*(++tpb));
					*(++tpm)=pak*(*(++tpb));
					*(++tpm)=pak*(*(++tpb));
					*(++tpm)=pak*(*(++tpb));
				}
				while(tpm<e)
				{
					*(++tpm)=pak*(*(++tpb));
				}
			}else
			{
				while(e-tpm>=4)
				{
					*(++tpm)+=pak*(*(++tpb));
					*(++tpm)+=pak*(*(++tpb));
					*(++tpm)+=pak*(*(++tpb));
					*(++tpm)+=pak*(*(++tpb));
				}
				while(tpm<e)
				{
					*(++tpm)+=pak*(*(++tpb));
				}
			}
		}
	}
}

void wtk_matrix_multi4(wtk_matrix_t *m,wtk_matrix_t *a,wtk_matrix_t *b)
{
	int rows=wtk_matrix_rows(m);
	int cols=wtk_matrix_cols(m);
	int ac=wtk_matrix_cols(a);
	int i,k;
	float *pa,*pm;
	register float *tpm,*tpb;
	register float pak;
	register float *e;


	////|1*429|*|429*1500|=0;
	//wtk_debug("rows=%d,cols=%d,ac=%d\n",rows,cols,ac);
	//wtk_matrix_print(b);
	//wtk_debug("rows=%d,a=%d\n",rows,ac);
	for(i=1;i<=rows;++i)
	{
		pa=a[i];pm=m[i];
		e=pm+cols;
		for(k=1;k<=ac;++k)
		{
			tpb=b[k];pak=pa[k];
			//wtk_debug("sf=%d,pb=%d,pa=%d,pm=%d\n",(int)(sizeof(float)),(int)((long)pb%8),(int)((long)pa%8),(int)((long)pm%8));
			tpm=pm;
			while(tpm<e)
			{
				if(k==1)
				{
					*(++tpm)=pak*(*(++tpb));
				}else
				{
					*(++tpm)+=pak*(*(++tpb));
				}
			}
		}
	}
}


void wtk_matrix_multi3(wtk_matrix_t *m,wtk_matrix_t *a,wtk_matrix_t *b)
{
	int rows=wtk_matrix_rows(m);
	int cols=wtk_matrix_cols(m);
	int ac=wtk_matrix_cols(a);
	int i,j,k;
	register float *pi,*ai;
	float t;
	static int ki=0;

	//++ki;
	for(i=1;i<=rows;++i)
	{
		pi=m[i];ai=a[i];
		for(j=1;j<=cols;++j)
		{
			t=0;
			for(k=1;k<=ac;++k)
			{
				t+=ai[k]*b[k][j];
				//wtk_debug("%d,%d,%d,%f,%f,%f\n",i,j,k,m[i][j],a[i][k],b[k][j]);
				if(ki==2)
				{
					wtk_debug("v[%d]: %f*%f=%f\n",k,ai[k],b[k][j],t);
				}
			}
			pi[j]=t;
			if(ki==2)
			{
				wtk_debug("t=%f\n",t);
				exit(0);
			}
		}
	}
}

void wtk_matrix_add1(wtk_matrix_t *m,wtk_matrix_t *a)
{
	int rows=wtk_matrix_rows(m);
	int cols=wtk_matrix_cols(m);
	int i,j;

	for(i=1;i<=rows;++i)
	{
		for(j=1;j<=cols;++j)
		{
			//wtk_debug("%d,%d,%f,%f\n",i,j,m[i][j],a[i][j]);
			m[i][j]+=a[i][j];
		}
	}
}

void wtk_matrix_add(wtk_matrix_t *m,wtk_matrix_t *a)
{
	int rows=wtk_matrix_rows(m);
	int cols=wtk_matrix_cols(m);
	int i,j;
	float *pm,*pa;

	for(i=1;i<=rows;++i)
	{
		pm=m[i];pa=a[i];
		for(j=1;j<=cols;++j)
		{
			//wtk_debug("%d,%d,%f,%f\n",i,j,m[i][j],a[i][j]);
			pm[j]+=pa[j];
		}
	}
}

void wtk_matrix_add2(wtk_matrix_t *dst,wtk_matrix_t *src,float f1,float f2)
{
	int rows=wtk_matrix_rows(dst);
	int cols=wtk_matrix_cols(dst);
	int i,j;
	float *pm,*pa;

	for(i=1;i<=rows;++i)
	{
		pm=dst[i];pa=src[i];
		for(j=1;j<=cols;++j)
		{
			//wtk_debug("%d,%d,%f,%f\n",i,j,m[i][j],a[i][j]);
			pm[j]=pm[j]*f1+pa[j]*f2;
		}
	}
}

double wtk_matrix_max(wtk_matrix_t *m)
{
	int rows=wtk_matrix_rows(m);
	int cols=wtk_matrix_cols(m);
	int i,j;
	double max=-100000.0;

	for(i=1;i<=rows;++i)
	{
		for(j=1;j<=cols;++j)
		{
			if(m[i][j]>max)
			{
				max=m[i][j];
			}
		}
	}
	return max;
}


double wtk_matrix_min(wtk_matrix_t *m)
{
	int rows=wtk_matrix_rows(m);
	int cols=wtk_matrix_cols(m);
	int i,j;
	double min=100000.0;

	for(i=1;i<=rows;++i)
	{
		for(j=1;j<=cols;++j)
		{
			if(m[i][j]<min)
			{
				min=m[i][j];
			}
		}
	}
	return min;
}

float wtk_matrix_avg(wtk_matrix_t *m)
{
	int rows=wtk_matrix_rows(m);
	int cols=wtk_matrix_cols(m);
	int i;
	register float f,v;
	register float *p,*pe;

	f=0;
	for(i=1;i<=rows;++i)
	{
		p=m[i];
		pe=p+cols;
		while(p<pe)
		{
			v=*(++p);
			f+=v>=0?v:-v;
		}
	}
	return f/(rows*cols);
}

double wtk_matrix_max_abs(wtk_matrix_t *m)
{
	int rows=wtk_matrix_rows(m);
	int cols=wtk_matrix_cols(m);
	int i,j;
	float max=-100000.0;
	//float min=10000.0;
	float *ps,f;

	for(i=1;i<=rows;++i)
	{
		ps=m[i]+1;
		for(j=1;j<=cols;++j,++ps)
		{
			f=*ps;
			f=f>0?f:-f;
			if(f>max)
			{
				max=f;
			}
		}
	}
	return max;
}

void wtk_matrix_add3(wtk_matrix_t *m,wtk_matrix_t *a)
{
	int rows=wtk_matrix_rows(m);
	int cols=wtk_matrix_cols(m);
	int i;
	float *pm,*pa;
	float *p1,*p1e,*p2;

	for(i=1;i<=rows;++i)
	{
		pm=m[i];pa=a[i];
		p1=pm;
		p1e=p1+cols;
		p2=pa;
		while(p1<=p1e)
		{
			*(++p1)=*(++p2);
		}
	}
}

void wtk_matrix_transpose1(wtk_matrix_t *m,wtk_matrix_t *a)
{
	int rows=wtk_matrix_rows(m);
	int cols=wtk_matrix_cols(m);
	int i,j;

	for(i=1;i<=rows;++i)
	{
		for(j=1;j<=cols;++j)
		{
			m[i][j]=a[j][i];
		}
	}
}

#define LSMALL (-0.5E10)   /* log values < LSMALL are set to LZERO */

void wtk_matrix_scale(wtk_matrix_t *m,float scale)
{
	int r,c;
	int i,j;

	r=wtk_matrix_rows(m);
	c=wtk_matrix_cols(m);
	for(i=1;i<=r;++i)
	{
		for(j=1;j<=c;++j)
		{
			if(m[i][j]>LSMALL)
			{
				m[i][j]*=scale;
			}
		}
	}
}


void wtk_matrix_transpose(wtk_matrix_t *dst,wtk_matrix_t *src)
{
	int rows=wtk_matrix_rows(dst);
	int cols=wtk_matrix_cols(dst);
	int i,j;
	float *pm;

	for(i=1;i<=rows;++i)
	{
		pm=dst[i];
		for(j=1;j<=cols;++j)
		{
			pm[j]=src[j][i];
		}
	}
}

wtk_matrix_t* wtk_matrix_transpose2(wtk_matrix_t *a)
{
	wtk_matrix_t *b;

	b=wtk_matrix_new2(wtk_matrix_cols(a),wtk_matrix_rows(a));
	wtk_matrix_transpose(b,a);
	return b;
}

void wtk_matrix_to_vector(wtk_matrix_t *m,wtk_vector_t *v)
{
	int rows=wtk_matrix_rows(m);
	int cols=wtk_matrix_cols(m);
	int i,j,k;

	k=0;
	for(i=1;i<=rows;++i)
	{
		for(j=1;j<=cols;++j)
		{
			v[++k]=m[i][j];
		}
	}
}

void wtk_matrix_print(wtk_matrix_t *m)
{
	int i,rows;
	int j,cols;
	//int ki=0;

	rows=wtk_matrix_rows(m);
	cols=wtk_matrix_cols(m);
	for(i=1;i<=rows;++i)
	{
		for(j=1;j<=cols;++j)
		{
			//if(fabs(m[i][j]-0.003)>0.001)
			{
				printf("v[%d][%d]=%.6f\n",i,j,m[i][j]);
			}
		}
	}
}


void wtk_matrix_print2(wtk_matrix_t *m)
{
	int i,rows;

	rows=wtk_matrix_rows(m);
	for(i=1;i<=rows;++i)
	{
		wtk_vector_print(m[i]);
	}
}

void wtk_double_matrix_cpy(wtk_double_matrix_t *src,wtk_double_matrix_t *dst)
{
	int i,rows;

	rows=wtk_matrix_rows(src);
	for(i=1;i<=rows;++i)
	{
		wtk_double_vector_cpy(src[i],dst[i]);
	}
}

void wtk_matrix_cpy(wtk_matrix_t *src,wtk_matrix_t *dst)
{
	int i,rows;

	rows=wtk_matrix_rows(src);
	for(i=1;i<=rows;++i)
	{
		wtk_vector_cpy(src[i],dst[i]);
	}
}

void wtk_sub_matrix_cpy(wtk_sub_matrix_t *dst,wtk_sub_matrix_t *src)
{
	int i,j;
	float *a=dst->val;
	float *b=src->val;

	for(i=0;i<dst->row;i++)
	{
		for(j=0;j<dst->col+4;j++)
		{
			*(a+i*dst->col+j)=*(b+i*src->col+j);
		}
	}
}

void wtk_sub_matrix_cpy2(qtk_blas_matrix_t  *dst,qtk_blas_matrix_t *src,qtk_nnet3_submatrix_info_t *dst_info,qtk_nnet3_submatrix_info_t *src_info)
{
	int i,j;

	if(dst_info->num_cols==src_info->num_cols && dst_info->num_rows==src_info->num_rows)
	{
		for(i=0;i<dst_info->num_rows;i++)
		{
			for(j=0;j<dst_info->num_cols;j++)
			{
				//dst[1+dst_info->row_offset+i][1+dst_info->col_offset+j]=src[1+src_info->row_offset+i][1+src_info->col_offset+j];
				*(dst->m+(dst_info->row_offset+i)*(dst->col)+dst_info->col_offset+j)=*(src->m+(src_info->row_offset+i)*(src->col)+src_info->col_offset+j);
				//wtk_debug("dst: [%d][%d]  src: [%d][%d\n",1+dst_info->row_offset+i,1+dst_info->col_offset+j,1+src_info->row_offset+i,1+src_info->col_offset+j);
			}
		}
	}
}

void wtk_sub_matrix_cpy_rows(qtk_blas_matrix_t  *dst,qtk_blas_matrix_t *src,qtk_nnet3_submatrix_info_t *dst_info,qtk_nnet3_submatrix_info_t *src_info,int *index,int len)
{
	int i,src_index;
	//if(dst_info->num_cols==src_info->num_cols && dst_info->num_rows==src_info->num_rows)
	//{
		for(i=0;i<dst_info->num_rows;i++)
		{
			if(i<len)//if(i+1<=len)
			{
				src_index=*(index+i);
				memcpy((dst->m+(dst_info->row_offset+i)*dst->col+dst_info->col_offset),(src->m+src_index*src->col),sizeof(float)*dst_info->num_cols);
				//memcpy(&(dst[1+dst_info->row_offset+i][1+dst_info->col_offset]),&(src[src_index][1]),sizeof(float)*dst_info->num_cols);
			}
			else
			{
				memset((dst->m+(dst_info->row_offset+i)*dst->col+dst_info->col_offset),0,sizeof(float)*dst_info->num_cols);
				//memset(&(dst[1+dst_info->row_offset+i][1+dst_info->col_offset]),0,sizeof(float)*dst_info->num_cols);
			}
		}
	//}
}

void wtk_sub_matrix_cpy_relu(wtk_sub_matrix_t *src,wtk_sub_matrix_t *dst,float floor)
{
	{
		int i,j;
		float *a=dst->val;
		float *b=src->val;

		for(i=0;i<dst->row;i++)
		{
			for(j=0;i<dst->col;j++)
			{
				if(*(b+i*src->col+j)<floor)
					*(a+i*dst->col+j)=floor;
				else
					*(a+i*dst->col+j)=*(b+i*src->col+j);
			}
		}
	}
}

void wtk_sub_matrix_cpy_relu2(qtk_blas_matrix_t  *dst,qtk_blas_matrix_t *src,
		qtk_nnet3_submatrix_info_t *dst_info,qtk_nnet3_submatrix_info_t *src_info,float floor)
{

		int i,j;
		float tmp=0.0;

		if(dst_info->num_cols==src_info->num_cols && dst_info->num_rows==src_info->num_rows)
		{
			for(i=0;i<dst_info->num_rows;i++)
			{
				for(j=0;j<dst_info->num_cols;j++)
				{
					//tmp=src[1+src_info->row_offset+i][1+src_info->col_offset+j];
					tmp=*(src->m+(src_info->row_offset+i)*(src->col)+src_info->col_offset+j);
					if(tmp<floor)
					{
						//dst[1+dst_info->row_offset+i][1+dst_info->col_offset+j]=floor;
						*(dst->m+(dst_info->row_offset+i)*(dst->col)+dst_info->col_offset+j)=floor;
					}else
					{
						//dst[1+dst_info->row_offset+i][1+dst_info->col_offset+j]=tmp;
						*(dst->m+(dst_info->row_offset+i)*(dst->col)+dst_info->col_offset+j)=tmp;
					}
					//dst[1+dst_info->row_offset+i][1+dst_info->col_offset+j]=src[1+src_info->row_offset+i][1+src_info->col_offset+j];
					//wtk_debug("dst: [%d][%d]  src: [%d][%d\n",1+dst_info->row_offset+i,1+dst_info->col_offset+j,1+src_info->row_offset+i,1+src_info->col_offset+j);
				}
			}
		}
}

void wtk_sub_matrix_add(wtk_sub_matrix_t *src,wtk_sub_matrix_t *dst,float alpha)
{
	int i,j;
	float *a=dst->val;
	float *b=src->val;

	for(i=0;i<dst->row;i++)
	{
		for(j=0;i<dst->col;j++)
		{
			*(a+i*dst->col+j)=*(a+i*dst->col+j)*alpha+*(b+i*src->col+j);
		}
	}
}

void wtk_sub_matrix_add_rows(wtk_sub_matrix_t *src,wtk_sub_matrix_t *dst,float alpha)
{

}

void wtk_double_matrix_zero(wtk_double_matrix_t *m)
{
	int i,j,nr,nc;

	nr=wtk_matrix_rows(m);
	nc=wtk_matrix_cols(m);
	for(i=1;i<=nr;++i)
	{
		for(j=1;j<=nc;++j)
		{
			m[i][j]=0;
		}
	}
}

void wtk_matrix_zero(wtk_matrix_t *m)
{
	int i,j,nr,nc;

	nr=wtk_matrix_rows(m);
	nc=wtk_matrix_cols(m);
	for(i=1;i<=nr;++i)
	{
		for(j=1;j<=nc;++j)
		{
			m[i][j]=0;
		}
	}
}

void wtk_matrix_set_init_value(wtk_matrix_t *m,double f)
{
	int i,j,nr,nc;

	nr=wtk_matrix_rows(m);
	nc=wtk_matrix_cols(m);
	for(i=1;i<=nr;++i)
	{
		for(j=1;j<=nc;++j)
		{
			m[i][j]=f;
		}
	}
}


/* mat_id -- set A to being closest to identity matrix as possible
        -- i.e. A[i][j] == 1 if i == j and 0 otherwise */
void wtk_double_matrix_init_identity(wtk_double_matrix_t *A)
{
	int i,size;

	wtk_double_matrix_zero(A);
	size=min(wtk_matrix_rows(A),wtk_matrix_cols(A));
	for(i=1;i<=size;++i)
	{
		A[i][i]=1.0;
	}
}


int wtk_matrix_16_bytes(int r,int col)
{
	return sizeof(float*)*(r+1)+16+(sizeof(float)*(col+1)+16)*r;
	/*
	int bytes;

	bytes=sizeof(float*)*(r+1)+16+(sizeof(float)*(col+1)+16)*r;
	{
		int i,t;

		t=s+sizeof(float*)*(r+1);
		for(i=0;i<r;++i)
		{
			t=wtk_round(t+sizeof(float),16);
			wtk_debug("t=%d/%d\n",t,t%16);
			if(t%16 !=0 )
			{
				exit(0);
			}
			t+=sizeof(float)*col;
		}
		wtk_debug("t=%d/%d r=%d/%d bytes=%d %d\n",t,s+bytes,r,col,bytes,t-s);
		if(s+bytes<t)
		{
			exit(0);
		}
	}
	//exit(0);
	return bytes;
	*/
}

void wtk_matrix_16_check()
{
	int i,t,s,r,col,bytes;

	r=13;
	col=26;
	for(s=0;s<16;++s)
	{
		bytes=wtk_matrix_16_bytes(r,col);
		wtk_debug("====== end %d ==========\n",s);

		t=s+sizeof(float*)*(r+1);
		for(i=0;i<r;++i)
		{
			t=wtk_round(t+sizeof(float),16);
			wtk_debug("t=%d/%d\n",t,t%16);
			if(t%16 !=0 )
			{
				exit(0);
			}
			t+=sizeof(float)*col;
		}
		wtk_debug("t=%d/%d r=%d/%d bytes=%d %d\n",t,s+bytes,r,col,bytes,t-s);
		if(s+bytes<t)
		{
			exit(0);
		}
	}
	exit(0);
}


wtk_matrix_t* wtk_matrix_new2(int r,int col)
{
	char *p;
	float **m;
	int i;
	int bytes;
	int col_bytes;

	bytes=wtk_matrix_16_bytes(r,col);
	p=wtk_malloc(bytes);
	m=(float**)p;
	*((int*)p)=r;
	p+=sizeof(float*)*(r+1);
	col_bytes=sizeof(float)*(col+1);
	for(i=1;i<=r;++i)
	{
		//wtk_debug("%p\n",wtk_align_ptr(p+sizeof(float),16));
#ifdef WIN32
		p = (char*)wtk_align_ptr(p + sizeof(float), 16) - sizeof(float);
#else
		p = wtk_align_ptr(p + sizeof(float), 16) - sizeof(float);
#endif
		//wtk_debug("p=%p=m=%p\n",p,p+sizeof(float));
		*((int*)p)=col;
		m[i]=(float*)p;
		//wtk_debug("v[%d]=%p:%p\n",i,m,p);
		//wtk_debug("v[%d]=%p/%p\n",i,m,&(m[i][1]));
		/*
		if(((long)(&(m[1][1])))%16!=0)
		{
			wtk_debug("[%ld]=%p\n",((long)(m[1]+1))%16,p);
			exit(0);
		}*/
		p+=col_bytes;
	}
	return m;
}


wtk_int_matrix_t* wtk_int_matrix_new(int r,int c)
{
	int **i;
	char *p;
	int j;
	int col_bytes;

	p=wtk_malloc((r+1)*sizeof(int*)+16+((c+1)*sizeof(int)+16)*r);
	i=(int**)p;
	*((int*)p)=r;
	p+=sizeof(int*)*(r+1);
	col_bytes=sizeof(int)*(c+1);
	for(j=1;j<=r;++j)
	{
#ifdef WIN32
		p = (char*)wtk_align_ptr(p + sizeof(float), 16) - sizeof(float);
#else
		p = wtk_align_ptr(p + sizeof(float), 16) - sizeof(float);
#endif
		*((int*)p)=c;
		i[j]=(int*)p;
		p+=col_bytes;
	}
	return i;
}

void wtk_int_matrix_print(wtk_int_matrix_t *m)
{
	int row,col;
	int i,j;

	row=wtk_matrix_rows(m);
	col=wtk_matrix_cols(m);
	//wtk_debug("row=%d col=%d\n",row,col);
	for(i=1;i<=row;++i)
	{
		for(j=1;j<=col;++j)
		{
			printf("v[%d][%d]=%d\n",i,j,m[i][j]);
		}
	}
}


void wtk_matrix_set_random(wtk_matrix_t *m,wtk_matrix_random_f random)
{
	int rows=wtk_matrix_rows(m);
	int cols=wtk_matrix_cols(m);
	int i,j;

	for(i=1;i<=rows;++i)
	{
		for(j=1;j<=cols;++j)
		{
			m[i][j]=random();
		}
	}
//	for(j=1;j<=cols;++j)
//	{
//		for(i=1;i<=rows;++i)
//		{
//
//			{
//				m[i][j]=random();
//			}
//		}
//	}
}


void wtk_matrix_sigmoid(wtk_matrix_t *m)
{
	int i,j,nr,nc;
	float f;

	nr=wtk_matrix_rows(m);
	nc=wtk_matrix_cols(m);
	for(i=1;i<=nr;++i)
	{
		for(j=1;j<=nc;++j)
		{
			f=m[i][j];
			if(f>50){f=50;}
			if(f<-50){f=-50;}
			f=-f;
			m[i][j]=1/(1+FAST_EXP(f));
		}
	}
}

void wtk_matrix_softmax(wtk_matrix_t *m)
{
//	int i,j,nr,nc;
//	float f;
//	double sum=0;
//
//	nr=wtk_matrix_rows(m);
//	nc=wtk_matrix_cols(m);
//	for(i=1;i<=nr;++i)
//	{
//		for(j=rnn->voc_size+1;j<=nc;++j)
//		{
//			f=m[i][j];
//			if(f>50){f=50;}
//			if(f<-50){f=-50;}
//			f=FAST_EXP(f);
//			m[i][j]=f;
//			sum+=f;
//		}
//	}
//	sum=1.0/sum;
//	for(i=1;i<=nr;++i)
//	{
//		for(j=rnn->voc_size+1;j<=nc;++j)
//		{
//			m[i][j]*=sum;
//		}
//	}
}

void wtk_vector_mult_matrix_2(wtk_vector_t *dst,wtk_vector_t *src,wtk_matrix_t *m)
{
	int n=wtk_vector_size(src);
	int n2=wtk_vector_size(dst);
	int i,j;
	double f;

	for(i=1;i<=n2;++i)
	{
		f=0;
		for(j=1;j<=n;++j)
		{
			f+=(src[j])*(m[j][i]);
		}
		dst[i]=f;
	}
}

void wtk_vector_mult_matrix_4(wtk_vector_t *dst,wtk_vector_t *src,wtk_matrix_t *m)
{
	register float f;
	register float *pdst,*pdst_end;
	register float *pm;
	float *psrc,*psrc_end;
	float *ppdst;
	int i;

	wtk_vector_zero(dst);
	psrc=src+1;
	psrc_end=psrc+wtk_vector_size(src);
	ppdst=dst+1;
	pdst_end=ppdst+wtk_vector_size(dst);
	i=1;
	while(psrc<psrc_end)
	{
		f=*(psrc++);
		if(f!=0)
		{
			pdst=ppdst;
			pm=m[i]+1;
			//wtk_debug("pm=%p\n",pm);
			while(pdst<pdst_end-8)
			{
				*(pdst++)+=f*(*(pm++));
				*(pdst++)+=f*(*(pm++));
				*(pdst++)+=f*(*(pm++));
				*(pdst++)+=f*(*(pm++));
				*(pdst++)+=f*(*(pm++));
				*(pdst++)+=f*(*(pm++));
				*(pdst++)+=f*(*(pm++));
				*(pdst++)+=f*(*(pm++));
			}
			while(pdst<pdst_end)
			{
				*(pdst++)+=f*(*(pm++));
			}
		}
		++i;
	}
}

/*
#include <xmmintrin.h>

void wtk_vector_mult_matrix(wtk_vector_t *dst,wtk_vector_t *src,wtk_matrix_t *m)
{
	float f;
	float *pdst,*pdst_end;
	float *pm;
	float *psrc,*psrc_end;
	float *ppdst;
	float **pmx;
	__m128 mf;
	float *pdst_end2;

	wtk_vector_zero(dst);
	psrc=src+1;
	psrc_end=psrc+wtk_vector_size(src);
	ppdst=dst+1;
	pdst_end=ppdst+wtk_vector_size(dst);
	pdst_end2=psrc_end-8;
	pmx=m;
	while(psrc<psrc_end)
	{
		f=*(psrc++);
		if(f==0)
		{
			continue;
		}
		pdst=ppdst;
		pm=(*(++pmx))+1;
		//wtk_debug("pm=%p\n",pm);
		mf=_mm_set_ps(f,f,f,f);
		while(pdst<pdst_end2)
		{
//			ma=_mm_load_ps(pm);
//			mb=_mm_mul_ps(mf,ma);
//			ma=_mm_load_ps(pdst);
//			ma=_mm_add_ps(ma,mb);
//			_mm_store_ps(pdst,ma);
			*((__m128*)pdst)=_mm_add_ps(_mm_mul_ps(mf,*(__m128*)pm),*(__m128*)pdst);
			//wtk_debug("%.12f/%.12f/%.12f/%.12f\n",pdst[0],pdst[1],pdst[2],pdst[3]);
			pdst+=4;
			pm+=4;

			*((__m128*)pdst)=_mm_add_ps(_mm_mul_ps(mf,*(__m128*)pm),*(__m128*)pdst);
			//wtk_debug("%.12f/%.12f/%.12f/%.12f\n",pdst[0],pdst[1],pdst[2],pdst[3]);
			pdst+=4;
			pm+=4;
		}
		while(pdst<pdst_end)
		{
			*(pdst++)+=f*(*(pm++));
		}
	}
}
*/

void wtk_vector_mult_matrix(wtk_vector_t *dst,wtk_vector_t *src,wtk_matrix_t *m)
{
	float f;
	float *pdst,*pdst_end;
	float *pm;
	float *psrc,*psrc_end;
	float *ppdst;
	float **pmx;

	wtk_vector_zero(dst);
	psrc=src+1;
	psrc_end=psrc+wtk_vector_size(src);
	ppdst=dst+1;
	pdst_end=ppdst+wtk_vector_size(dst);
	pmx=m;
	while(psrc<psrc_end)
	{
		f=*(psrc++);
		if(f==0)
		{
			continue;
		}
		pdst=ppdst;
		pm=(*(++pmx))+1;
		while(pdst<pdst_end)
		{
			*(pdst++)+=f*(*(pm++));
		}
	}
}

void wtk_vector_mult_matrix_5(wtk_vector_t *dst,wtk_vector_t *src,wtk_matrix_t *m)
{
//	register float f;
//	register float *pdst,*pdst_end;
//	register float *pm;
	float f;
	float *pdst,*pdst_end;
	float *pm;
	float *psrc,*psrc_end,*pdst_end2;
	float *ppdst;
	float **pmx;

	wtk_vector_zero(dst);
	psrc=src+1;
	psrc_end=psrc+wtk_vector_size(src);
	ppdst=dst+1;
	pdst_end=ppdst+wtk_vector_size(dst);
	pdst_end2=pdst_end-8;
	pmx=m;
	while(psrc<psrc_end)
	{
		f=*(psrc++);
		pdst=ppdst;
		pm=(*(++pmx))+1;
		while(pdst<pdst_end2)
		{
			pdst[0]+=f*pm[0];
			pdst[1]+=f*pm[1];
			pdst[2]+=f*pm[2];
			pdst[3]+=f*pm[3];
			pdst[4]+=f*pm[4];
			pdst[5]+=f*pm[5];
			pdst[6]+=f*pm[6];
			pdst[7]+=f*pm[7];
			pdst+=8;
			pm+=8;
		}
		while(pdst<pdst_end)
		{
			*(pdst++)+=f*(*(pm++));
		}
	}
}

void wtk_vector_mult_matrix_3(wtk_vector_t *dst,wtk_vector_t *src,wtk_matrix_t *m)
{
	register float f;
	register float *pdst,*pdst_end;
	register float *pm;
	float *psrc;
	float *ppdst;
	int i;
	int n;

	wtk_vector_zero(dst);
	n=wtk_vector_size(src);
	ppdst=dst+1;
	pdst_end=ppdst+wtk_vector_size(dst);
	for(i=1,psrc=src+1;i<=n;++i)
	{
		f=*(psrc++);
		if(f!=0)
		{
			pdst=ppdst;
			pm=m[i]+1;
			//wtk_debug("pm=%p\n",pm);
			while(pdst<pdst_end)
			{
				*(pdst++)+=f*(*(pm++));
			}
		}
	}
}

/**
 * [s,e)
 */
void wtk_vector_mult_matrix2(wtk_vector_t *dst,wtk_vector_t *src,wtk_matrix_t *m,int s,int e)
{
	int n=wtk_vector_size(src);
	//int n2=wtk_vector_size(dst);
	int i,j;
	float f;
	float *pf;

	for(i=s;i<e;++i)
	{
		f=0;
		for(j=1,pf=src+1;j<=n;++j)
		{
			f+=*(pf++) * m[j][i];
		}
		dst[i]=f;
	}
}

/**
 *	|dst * src| *|src*1|=|dst*1|
 */
void wtk_vector_mult_matrix_rev_2(wtk_vector_t *dst,wtk_vector_t *src,wtk_matrix_t *m,int add)
{
	int n,n2;
	int i;
	double f;
	float *pm;
	float *psrc;
	float *ppsrc;
	float *psrc_end;
	float *psrc_end2;
	float *pdst;

	n=wtk_vector_size(src);
	n2=wtk_vector_size(dst);
	if(add)
	{
		ppsrc=src+1;
		psrc_end=ppsrc+n;
		psrc_end2=psrc_end-8;
		pdst=dst+1;
		for(i=1;i<=n2;++i)
		{
			f=0;
			psrc=ppsrc;
			pm=m[i]+1;
			while(psrc<psrc_end2)
			{
				f+=(*(pm++))*(*(psrc++));
				f+=(*(pm++))*(*(psrc++));
				f+=(*(pm++))*(*(psrc++));
				f+=(*(pm++))*(*(psrc++));
				f+=(*(pm++))*(*(psrc++));
				f+=(*(pm++))*(*(psrc++));
				f+=(*(pm++))*(*(psrc++));
				f+=(*(pm++))*(*(psrc++));
			}
			while(psrc<psrc_end)
			{
				f+=(*(pm++))*(*(psrc++));
			}
			//dst[i]=f;
			*(pdst++)+=f;
		}
	}else
	{
		ppsrc=src+1;
		psrc_end=ppsrc+n;
		psrc_end2=psrc_end-8;
		pdst=dst+1;
		for(i=1;i<=n2;++i)
		{
			f=0;
			psrc=ppsrc;
			pm=m[i]+1;
			while(psrc<psrc_end2)
			{
				f+=(*(pm++))*(*(psrc++));
				f+=(*(pm++))*(*(psrc++));
				f+=(*(pm++))*(*(psrc++));
				f+=(*(pm++))*(*(psrc++));
				f+=(*(pm++))*(*(psrc++));
				f+=(*(pm++))*(*(psrc++));
				f+=(*(pm++))*(*(psrc++));
				f+=(*(pm++))*(*(psrc++));
			}
			while(psrc<psrc_end)
			{
				f+=(*(pm++))*(*(psrc++));
			}
			//dst[i]=f;
			*(pdst++)=f;
		}
	}
}

void wtk_vector_mult_matrix_rev(wtk_vector_t *dst,wtk_vector_t *src,wtk_matrix_t *m,int add)
{
	int n,n2;
	int i,j;
	double f;

	n=wtk_vector_size(src);
	n2=wtk_vector_size(dst);
	for(i=1;i<=n2;++i)
	{
		f=0;
		for(j=1;j<=n;++j)
		{
			f+=m[i][j]*src[j];
			if(i==1)
			{
				//wtk_debug("v[%d/%d]=%.12f/%.12f/%.12f\n",i,j,m[i][j],src[j],f);
			}
		}
		if(i==1)
		{
			//wtk_debug("v[%d]=%.12f/%.12f\n",i,dst[i],f);
		}
		if(add)
		{
			dst[i]+=f;
		}else
		{
			dst[i]=f;
		}
	}
}

void wtk_vector_mult_matrix_rev2(wtk_vector_t *dst,wtk_vector_t *src,wtk_matrix_t *m,int s,int e)
{
	int n2;
	int i,j;
	double f;

	n2=wtk_vector_size(dst);
	for(i=1;i<=n2;++i)
	{
		f=0;
		for(j=s;j<e;++j)
		{
			f+=m[i][j]*src[j];
			if(i==1)
			{
				//wtk_debug("v[%d/%d]=%.12f/%.12f/%.12f\n",j,i,f,m[i][j],src[j]);
			}
		}
		dst[i]=f;
	}
}
