#include "wtk_mat.h"
//#define wtk_float_round(f) ((f)>0?(f+0.5):(f-0.5))
#define wtk_mati_prow_at(c,i,j) (*((c)->p+(i+(c)->p_row_x)*(c)->p_col+j+(c)->p_col_x))


float* wtk_matf_at2(wtk_matf_t *m,int row,int col)
{
	float *p;

	p=m->p+row*m->col+col;
	return p;
	//#define wtk_matf_at(a,i,j) ((a)->p+i*(a)->col+j)
	//float* wtk_matf_at(wtk_matf_t *m,int row,int col);
}

wtk_matdf_t* wtk_matdf_new(int row,int col)
{
	wtk_matdf_t *m;
	char *p;

	p=wtk_calloc(1,sizeof(wtk_matdf_t)+row*col*sizeof(double)+16);
	m=(wtk_matdf_t*)p;
	m->row=row;
	m->col=col;
	m->p=wtk_align_ptr(p+sizeof(wtk_matdf_t),16);
	return m;
}

void wtk_matdf_delete(wtk_matdf_t *m)
{
	wtk_free(m);
}

void wtk_matdf_cpy(wtk_matdf_t *src,wtk_matdf_t *dst)
{
	memcpy(dst->p,src->p,src->row*src->col*sizeof(double));
}

void wtk_matdf_zero(wtk_matdf_t *m)
{
	memset(m->p,0,m->row*m->col*sizeof(double));
}

void wtk_matdf_add(wtk_matdf_t *dst,wtk_matdf_t *a,wtk_matdf_t *b)
{
	int n=dst->row*dst->col;
	int i;

	for(i=0;i<n;++i)
	{
		dst->p[i]=a->p[i]+b->p[i];
	}
}



void wtk_matdf_add2(wtk_matdf_t *dst,wtk_matdf_t *b)
{
	int n=dst->row*dst->col;
	int i;

	for(i=0;i<n;++i)
	{
		dst->p[i]+=b->p[i];
	}
}

void wtk_matdf_scale(wtk_matdf_t *dst,wtk_matdf_t *src,double scale)
{
	int n=dst->row*dst->col;
	int i;

	for(i=0;i<n;++i)
	{
		//dst->p[i]=src->p[i]*scale;
		dst->p[i]=src->p[i]*scale;
	}
}

void wtk_matdf_normalize_row(wtk_matdf_t *m)
{
	int i,j;
	double z;

	for(i=0;i<m->row;++i)
	{
		z=0;
		for(j=0;j<m->col;++j)
		{
			z+=m->p[i*m->col+j];
		}
		for(j=0;j<m->col;++j)
		{
			m->p[i*m->col+j]/=z;
		}
	}
}

void wtk_float_normalize(float *fp,int n)
{
	int i;
	double z=0;

	for(i=0;i<n;++i)
	{
		z+=fp[i];
	}
	if(z!=0)
	{
		z=1/z;
		for(i=0;i<n;++i)
		{
			fp[i]*=z;
		}
	}
}

void wtk_matdf_print(wtk_matdf_t *f)
{
	int i,j;
	//char *px,*ppx;
	double *ppx;

	wtk_debug("============= mi=%p ===========\n",f);
	for(ppx=f->p,i=0;i<f->row;++i)
	{
		for(j=0;j<f->col;++j)
		{
			printf("v[%d][%d]=%.12f\n",i,j,*(ppx++));
			//printf("v[%d][%d]=%d\n",i,j,wtk_mati_prow_at(mc,i,j));
		}
	}
}


void wtk_matf_scale2(wtk_matf_t *dst,wtk_matf_t *src,double scale)
{
	int n=dst->row*dst->col;
	int i;

	for(i=0;i<n;++i)
	{
		//dst->p[i]=src->p[i]*scale;
		dst->p[i]=src->p[i]*scale;
	}
}

void wtk_matf_set_init(wtk_matf_t *m,float v)
{
	int n=m->row*m->col;
	int i;

	for(i=0;i<n;++i)
	{
		m->p[i]=v;
	}
}

void wtk_matf_add(wtk_matf_t *dst,wtk_matf_t *a,wtk_matf_t *b)
{
	int n=dst->row*dst->col;
	int i;

	for(i=0;i<n;++i)
	{
		dst->p[i]=a->p[i]+b->p[i];
	}
}

void wtk_matf_add_scale(wtk_matf_t *dst,float *a,float *b,float alpha,float beta)
{
	int n=dst->row*dst->col;
	int i;

	for(i=0;i<n;++i)
	{
		dst->p[i]=a[i]*alpha+b[i]*beta;
	}
}

void wtk_matf_add2_raw(wtk_matf_t *dst,wtk_matf_t *b)
{
	int n=dst->row*dst->col;
	int i;

	for(i=0;i<n;++i)
	{
		dst->p[i]+=b->p[i];
	}
}

void wtk_matf_add2(wtk_matf_t *dst,wtk_matf_t *b)
{
	int n=dst->row*dst->col;
	float *f1,*f2,*fe;

	f1=dst->p;
	fe=f1+n;
	f2=b->p;
	while(f1<fe)
	{
		*(f1++)+=*(f2++);
	}
}

void wtk_matf_normalize_row(wtk_matf_t *m)
{
	int i,j;
	double z;

	for(i=0;i<m->row;++i)
	{
		z=0;
		for(j=0;j<m->col;++j)
		{
			z+=m->p[i*m->col+j];
		}
		//wtk_debug("z=%.12f\n",z);
		if(z!=0)
		{
			for(j=0;j<m->col;++j)
			{
				m->p[i*m->col+j]/=z;
			}
		}
	}
}

wtk_range_t wtk_matf_range(wtk_matf_t *f)
{
	int i,n;
	wtk_range_t range;

	range.max=-1e6;
	range.min=1e6;
	n=f->row*f->col;
	for(i=0;i<n;++i)
	{
		if(f->p[i]>range.max)
		{
			range.max=f->p[i];
		}
		if(f->p[i]<range.min)
		{
			range.min=f->p[i];
		}
	}
	return range;
}

wtk_veci_t* wtk_veci_new(int len)
{
	wtk_veci_t *v;
	char *p;

	p=wtk_calloc(1,sizeof(wtk_veci_t)+len*sizeof(int)+16);
	v=(wtk_veci_t*)p;
	v->len=len;
	v->p=wtk_align_ptr(p+sizeof(wtk_veci_t),16);
	wtk_veci_zero(v);
	return v;
}

void wtk_veci_delete(wtk_veci_t *v)
{
	wtk_free(v);
}

void wtk_veci_zero(wtk_veci_t *v)
{
	memset(v->p,0,v->len*sizeof(int));
}

void wtk_veci_scale(wtk_veci_t *v,double scale)
{
	int *p,*pe;
	float f;

	p=v->p;
	pe=p+v->len;
	while(p<pe)
	{
		f=(*p)*scale;
		(*p)=wtk_float_round(f);
		++p;
	}
}

wtk_vecc_t* wtk_vecc_new(int len)
{
	wtk_vecc_t *v;
	char *p;

	p=wtk_calloc(1,sizeof(wtk_vecc_t)+len*sizeof(unsigned char)+16);
	v=(wtk_vecc_t*)p;
	v->len=len;
	v->p=wtk_align_ptr(p+sizeof(wtk_vecc_t),16);
	wtk_vecc_zero(v);
	//wtk_debug("v=%d/%d\n",v->len,len);
	return v;
}

void wtk_vecc_zero(wtk_vecc_t *v)
{
	memset(v->p,0,v->len);
}


void wtk_vecc_delete(wtk_vecc_t *v)
{
	wtk_free(v);
}

void wtk_vecc_init(wtk_vecc_t *v,float *pf,float scale)
{
	register float f;
	register char *px;
	int i;

	px=v->p;
	for(i=0;i<v->len;++i)
	{
		f=*(pf++)*scale;
		*(px++)=(char)(wtk_float_round(f));
	}
}

wtk_vecuc_t* wtk_vecuc_new(int len)
{
	wtk_vecuc_t *v;
	char *p;

	p=wtk_calloc(1,sizeof(wtk_vecuc_t)+len*sizeof(unsigned char)+16);
	v=(wtk_vecuc_t*)p;
	v->len=len;
	v->p=wtk_align_ptr(p+sizeof(wtk_vecuc_t),16);
	wtk_vecuc_zero(v);
	//wtk_debug("v=%d/%d\n",v->len,len);
	return v;
}

void wtk_vecuc_delete(wtk_vecuc_t *v)
{
	wtk_free(v);
}

void wtk_vecuc_zero(wtk_vecuc_t *v)
{
	memset(v->p,0,v->len);
}

void wtk_vecuc_init(wtk_vecuc_t *v,float *pf,float scale)
{
	register float f;
	register unsigned char *px;
	int i;

	px=v->p;
	for(i=0;i<v->len;++i)
	{
		f=*(pf++)*scale;
		*(px++)=(unsigned char)(wtk_float_round(f));
	}
}

void wtk_veci_init(wtk_veci_t *v,float *pf,float scale)
{
	register float f;
	register int *px;
	int i;

	px=v->p;
	for(i=0;i<v->len;++i)
	{
		f=*(pf++)*scale;
		*(px++)=(int)(wtk_float_round(f));
	}
}


wtk_vecf_t* wtk_vecf_new(int len)
{
	wtk_vecf_t *v;
	char *p;

	p=wtk_calloc(1,sizeof(wtk_vecf_t)+len*sizeof(float)+16);
	v=(wtk_vecf_t*)p;
	v->len=len;
	v->p=wtk_align_ptr(p+sizeof(wtk_vecf_t),16);
	wtk_vecf_zero(v);
	return v;
}

wtk_vecf_t* wtk_vecf_dup(wtk_vecf_t *src)
{
	wtk_vecf_t *dst;

	dst=wtk_vecf_new(src->len);
	memcpy(dst->p,src->p,sizeof(float)*src->len);
	return dst;
}

void wtk_vecf_cpy(wtk_vecf_t *dst,wtk_vecf_t *src)
{
	memcpy(dst->p,src->p,sizeof(float)*src->len);
}

void wtk_vecf_delete(wtk_vecf_t *v)
{
	wtk_free(v);
}

void wtk_vecf_zero(wtk_vecf_t *v)
{
	memset(v->p,0,v->len*sizeof(float));
}

void wtk_vecf_init(wtk_vecf_t *v,float f)
{
	int i;

	for(i=0;i<v->len;++i)
	{
		v->p[i]=f;
	}
}

void wtk_vecf_add(wtk_vecf_t *v,float *pf)
{
	int i;

	for(i=0;i<v->len;++i)
	{
		v->p[i]+=pf[i];
	}
}

void wtk_vecf_scale(wtk_vecf_t *v,float scale)
{
	int i;

	for(i=0;i<v->len;++i)
	{
		v->p[i]*=scale;
	}
}
#include <math.h>

float wtk_vecf_norm(wtk_vecf_t *v)
{
	float f;
	int i;
	float f1;

	f=0;
	for(i=0;i<v->len;++i)
	{
		f+=v->p[i]*v->p[i];
	}
	if(f==0)
	{
		return f;
	}
	f=sqrt(f);
	f1=1.0/f;
	for(i=0;i<v->len;++i)
	{
		v->p[i]*=f1;
	}
	return f;
}

float wtk_vecf_norm2(wtk_vecf_t *v,wtk_vecf_t *src)
{
	float f;
	int i;
	float f1;

	f=0;
	for(i=0;i<v->len;++i)
	{
		f+=src->p[i]*src->p[i];
	}
	if(f==0)
	{
		return f;
	}
	f=sqrt(f);
	f1=1.0/f;
	for(i=0;i<v->len;++i)
	{
		v->p[i]=src->p[i]*f1;
	}
	return f;
}

float wtk_vecf_cos_x(wtk_vecf_t *v1,wtk_vecf_t *v2)
{
	float f1,f2,f3;
	int i;

	f1=f2=f3=0;
	for(i=0;i<v1->len;++i)
	{
		f1+=v1->p[i]*v2->p[i];
		f2+=v1->p[i]*v1->p[i];
		f3+=v2->p[i]*v2->p[i];
	}
	return f1/(sqrt(f2)*sqrt(f3));
}

float wtk_vecf_value(wtk_vecf_t *v)
{
	float f=0;
	int i;

	for(i=0;i<v->len;++i)
	{
		f+=v->p[i]*v->p[i];
	}
	return sqrt(f);
}

void wtk_vecf_print(wtk_vecf_t *v)
{
	int i;

	wtk_debug("============= %d ===========\n",v->len);
	for(i=0;i<v->len;++i)
	{
		printf("v[%d]=%f\n",i,v->p[i]);
	}
}

float wtk_vecf_dist2(wtk_vecf_t *v1,wtk_vecf_t *v2)
{
	int i;
	float f,t;

	f=0;
	for(i=0;i<v1->len;++i)
	{
		t=v1->p[i]-v2->p[i];
		f+=t*t;
	}
	return sqrt(f);
}

float wtk_vecf_dist(wtk_vecf_t *v1,wtk_vecf_t *v2)
{
	int i;
	float f,t;
	float *fp1,*fp2;

	f=0;
	fp1=v1->p;
	fp2=v2->p;
	for(i=0;i<v1->len;++i)
	{
		t=fp1[i]-fp2[i];
		f+=t*t;
	}
	return f;
}

float wtk_vecf_cos(wtk_vecf_t *v1,wtk_vecf_t *v2)
{
	float f1;
	int i;

	f1=0;
	for(i=0;i<v1->len;++i)
	{
		f1+=v1->p[i]*v2->p[i];
	}
	return f1;
}

float wtk_vecf_cos2(wtk_vecf_t *v1,wtk_vecf_t *v2)
{
	float f1,f2,f3;
	float *fp1,*fp2,*fe;

	f1=f2=f3=0;
	fp1=v1->p;
	fe=fp1+v1->len;
	fp2=v2->p;
	while(fp1<fe)
	{
		f2+=*fp1 * *fp1;
		f3+=*fp2 * *fp2;
		f1+=*(fp1++) * *(fp2++);
	}
	return f1/(sqrt(f2)*sqrt(f3));
}

wtk_matf_t* wtk_matf_new(int row,int col)
{
	wtk_matf_t *m;
	char *p;

	p=wtk_calloc(1,sizeof(wtk_matf_t)+row*col*sizeof(float)+16);
	m=(wtk_matf_t*)p;
	m->row=row;
	m->col=col;
	m->p=wtk_align_ptr(p+sizeof(wtk_matf_t),16);
	return m;
}

void wtk_matf_cpy(wtk_matf_t *src,wtk_matf_t *dst)
{
	memcpy(dst->p,src->p,src->row*src->col*sizeof(float));
}

void wtk_matf_cpy2(wtk_matf_t *dst,float *fp,int n)
{
	memcpy(dst->p,fp,n*sizeof(float));
}

void wtk_matf_zero(wtk_matf_t *m)
{
	memset(m->p,0,m->row*m->col*sizeof(float));
}

float* wtk_matf_row(wtk_matf_t *m,int row)
{
	return m->p+m->col*row;
}

void wtk_matf_scale(wtk_matf_t *m,float scale)
{
	float *f1,*fe;

	f1=m->p;
	fe=f1+m->row*m->col;
	while(f1<fe)
	{
		*(f1++)*=scale;
	}
}

void wtk_matf_delete(wtk_matf_t *m)
{
	wtk_free(m);
}

void wtk_matf_vm_raw(float *dst,float *src,wtk_matf_t *m)
{
	int i,j;
	//float f;
	static int ki=0;
	int xi;
	int nx=21;

	nx=0;
	++ki;
	//wtk_debug("ki=%d\n",ki);
	for(i=0;i<m->row;++i)
	{
		//f=0;
		dst[i]=0;
		xi=i*m->col;
		for(j=0;j<m->col;++j)
		{
			dst[i]+=(src[j]*m->p[xi+j]);
			if(i==nx)
			{
				//wtk_debug("v[%d/%d]=%.12f/%.12f/%.12f\n",i,j,src[j],m->p[xi+j],dst[i]);
			}
		}
		//dst[i]=f;
		if(i==21)
		{
			//wtk_debug("v[%d]=%.12f\n",i,dst[i]);
			//exit(0);
		}
		//dst[i]=f;
		//exit(0);
	}
}


void wtk_float_vm_raw(float *dst,float *src,float *m,int row,int col)
{
	int i,j;
	//float f;
	static int ki=0;
	int xi;

	++ki;
	//wtk_debug("ki=%d\n",ki);
	for(i=0;i<row;++i)
	{
		//f=0;
		dst[i]=0;
		xi=i*col;
		for(j=0;j<col;++j)
		{
			dst[i]+=src[j]*m[xi+j];
			if(ki==1 &&i==0)
			{
				//wtk_debug("v[%d/%d]=%.12f/%.12f/%.12f\n",i,j,src[j],m->p[i*m->col+j],dst[i]);
			}
		}
		if(ki==1 && i==0)
		{
			//wtk_debug("v[%d]=%.12f\n",i,dst[i]);
			//exit(0);
		}
		//dst[i]=f;
		//exit(0);
	}
}

void wtk_float_vm(float *dst,float *src,float *m,int row,int col)
{
	float f,g;
	float *fp,*f1,*fe,*fp2;
	float *fe2,*fe3;
	int rowx;

	fe=src+col;
	fe2=dst+row;
	rowx=(row>>1)<<1;
	fe3=dst+rowx;
	fp=m;
	while(dst<fe3)
	{
		f1=src;
		f=0;
		g=0;
		fp2=fp+col;
		while(f1<fe)
		{
			f+=*(f1) *(*(fp++));
			g+=*(f1++) * (*(fp2++));
		}
		fp=fp2;
		*(dst++)=f;
		*(dst++)=g;
	}
	while(dst<fe2)
	{
		f1=src;
		f=0;
		while(f1<fe)
		{
			f+=*(f1++) *(*(fp++));
		}
		*(dst++)=f;
	}
}


void wtk_float_vm_add(float *dst,float *src,float *m,int row,int col)
{
	float f,g;
	float *fp,*f1,*fe,*fp2;
	float *fe2,*fe3;
	int rowx;

	fe=src+col;
	fe2=dst+row;
	rowx=(row>>1)<<1;
	fe3=dst+rowx;
	fp=m;
	while(dst<fe3)
	{
		f1=src;
		f=0;
		g=0;
		fp2=fp+col;
		while(f1<fe)
		{
			f+=*(f1) *(*(fp++));
			g+=*(f1++) * (*(fp2++));
		}
		fp=fp2;
		*(dst++)+=f;
		*(dst++)+=g;
	}
	while(dst<fe2)
	{
		f1=src;
		f=0;
		while(f1<fe)
		{
			f+=*(f1++) *(*(fp++));
		}
		*(dst++)+=f;
	}
}


void wtk_matf_vm3(float *dst,float *src,wtk_matf_t *m)
{
	int i,j;
	int xi;
	float f;

	for(i=0;i<m->row;++i)
	{
		f=0;
		xi=i*m->col;
		for(j=0;j<m->col;++j)
		{
			f+=(src[j]*m->p[xi+j]);
		}
		dst[i]=f;
	}
}


void wtk_matf_vm2(float *dst,float *src,wtk_matf_t *m)
{
	int i,j;
	float f,g;
	float *fp,*fp2;
	int row;

	fp=m->p;
	row=(m->row>>1)<<1;
	for(i=0;i<row;i+=2)
	{
		f=0;
		g=0;
		fp2=fp+m->col;
		for(j=0;j<m->col;++j)
		{
			f+=src[j]*fp[j];
			g+=src[j]*fp2[j];
		}
		dst[i]=f;
		dst[i+1]=g;
		fp=fp2+m->col;
	}
	for(;i<m->row;++i)
	{
		f=0;
		for(j=0;j<m->col;++j)
		{
			f+=src[j]*fp[j];
		}
		dst[i]=f;
		fp+=m->col;
	}
}

void wtk_matf_vmx(float *dst,float *src,wtk_matf_t *m)
{
	float f,g;
	float *fp,*fp2;
	int col=m->col;
	int i,row,j;

	row=(m->row>>1)<<1;
	for(i=0,fp=m->p;i<row;i+=2)
	{
		f=0;
		g=0;
		fp2=fp+col;
		for(j=0;j<col;++j)
		{
			f+=src[j]*fp[j];
			g+=src[j]*fp2[j];
		}
		dst[i]=f;
		dst[i+1]=g;
		fp=fp2+col;
	}
	for(;i<m->row;++i)
	{
		f=0;
		for(j=0;j<col;++j)
		{
			f+=src[j]*fp[j];
		}
		dst[i]=f;
	}
}

void wtk_matf_vm(float *dst,float *src,wtk_matf_t *m)
{
	register float f,g,t;
	float *fp,*f1,*fe,*fp2;
	float *fe2,*fe3;
	int col=m->col;
	int row;

	fe=src+col;
	fe2=dst+m->row;
	row=(m->row>>1)<<1;
	fe3=dst+row;
	fp=m->p;
	while(dst<fe3)
	{
		f1=src;
		fp2=fp+col;
		f=0;
		g=0;
		while(f1<fe)
		{
			t=*(f1++);
			f+=t *(*(fp++));
			g+=t * (*(fp2++));
		}
		fp=fp2;
		*(dst++)=f;
		*(dst++)=g;
	}
	while(dst<fe2)
	{
		f1=src;
		f=0;
		while(f1<fe)
		{
			f+=*(f1++) *(*(fp++));
		}
		*(dst++)=f;
	}
}


void wtk_matf_vm_add2(float *dst,float *src,wtk_matf_t *m)
{
	float f,g;
	float *fp,*fp2;
	int col=m->col;
	int i,row,j;

	row=(m->row>>1)<<1;
	for(i=0,fp=m->p;i<row;i+=2)
	{
		f=0;
		g=0;
		fp2=fp+col;
		for(j=0;j<col;++j)
		{
			f+=src[j]*fp[j];
			g+=src[j]*fp2[j];
		}
		dst[i]+=f;
		dst[i+1]+=g;
		fp=fp2+col;
	}
	for(;i<m->row;++i)
	{
		f=0;
		for(j=0;j<col;++j)
		{
			f+=src[j]*fp[j];
		}
		dst[i]+=f;
	}
}

void wtk_matf_vm_add_raw(float *dst,float *src,wtk_matf_t *m)
{
	int i,j;
	float f;
	int xi;

	for(i=0;i<m->row;++i)
	{
		xi=i*m->col;
		f=0;
		for(j=0;j<m->col;++j)
		{
			f+=src[j]*m->p[xi+j];
		}
		dst[i]+=f;
	}
}

void wtk_float_add2(float *dst,float *src,int n)
{
	int i;

	for(i=0;i<n;++i)
	{
		dst[i]+=src[i];
	}
}

void wtk_float_add(register float *dst,register float *src,int n)
{
	int i;
	int n2;

	n2=(n>>3)<<3;
	for(i=0;i<n2;i+=8)
	{
		dst[i]+=src[i];
		dst[i+1]+=src[i+1];
		dst[i+2]+=src[i+2];
		dst[i+3]+=src[i+3];
		dst[i+4]+=src[i+4];
		dst[i+5]+=src[i+5];
		dst[i+6]+=src[i+6];
		dst[i+7]+=src[i+7];
	}
	for(;i<n;++i)
	{
		dst[i]+=src[i];
	}
}


void wtk_int_add(int *dst,int *src,int n)
{
	int col;
	int i;

	col=(n>>3)<<3;
	for(i=0;i<col;i+=8)
	{
		dst[i]+=src[i];
		dst[i+1]+=src[i+1];
		dst[i+2]+=src[i+2];
		dst[i+3]+=src[i+3];
		dst[i+4]+=src[i+4];
		dst[i+5]+=src[i+5];
		dst[i+6]+=src[i+6];
		dst[i+7]+=src[i+7];
	}
	for(;i<n;++i)
	{
		dst[i]+=src[i];
	}
}

void wtk_int_add_raw(int *dst,int *src,int n)
{
	int i;

	for(i=0;i<n;++i)
	{
		dst[i]+=src[i];
	}
}

void wtk_matf_vm_add(float *dst,float *src,wtk_matf_t *m)
{
	register float f,g,t;
	float *fp,*f1,*fe,*fp2;
	float *fe2,*fe3;
	int col=m->col;
	int row;

	fe=src+col;
	fe2=dst+m->row;
	row=(m->row>>1)<<1;
	fe3=dst+row;
	fp=m->p;
	while(dst<fe3)
	{
		f1=src;
		fp2=fp+col;
		f=0;
		g=0;
		while(f1<fe)
		{
			t=*(f1++);
			f+=t *(*(fp++));
			g+=t * (*(fp2++));
		}
		fp=fp2;
		*(dst++)+=f;
		*(dst++)+=g;
	}
	while(dst<fe2)
	{
		f1=src;
		f=0;
		while(f1<fe)
		{
			f+=*(f1++) *(*(fp++));
		}
		*(dst++)+=f;
	}
}

void wtk_matf_vm_hid_x(float *dst,float *src,wtk_matf_t *m,int add)
{
	int i,j;
	int xi;

	if(add==0)
	{
		memset(dst,0,sizeof(float)*m->row);
	}
	for(i=0;i<m->row;++i)
	{
		xi=i*m->col;
		for(j=0;j<m->col;++j)
		{
			dst[j]+=(src[i]*m->p[xi+j]);
		}
	}
}

#include <math.h>

void wtk_matf_vm_hid(float *dst,float *src,wtk_matf_t *m,int add)
{
	int i;
	float *fp,*fp2,*fp3,*fp4;
	float f,g,h,t;
	int row;
	int row2;
	int col=m->col;
	float *f1,*fe;

	fp=m->p;
	row=m->row;
	row2=(row>>2)<<2;
	if(add==0)
	{
		memset(dst,0,sizeof(float)*row);
	}
	fe=dst+col;
	for(i=0;i<row2;i+=4)
	{
		f=src[i];
		g=src[i+1];
		h=src[i+2];
		t=src[i+3];
		fp2=fp+col;
		fp3=fp2+col;
		fp4=fp3+col;
		f1=dst;
		while(f1<fe)
		{
			*(f1++)+=f* (*(fp++)) + g* (*(fp2++)) + h * (*(fp3++)) + t * (*(fp4++));
		}
		fp=fp4;
	}
	for(;i<row;++i)
	{
		f=src[i];
		f1=dst;
		while(f1<fe)
		{
			*(f1++)+=f* (*(fp++));
		}
	}
}

void wtk_matf_multvm_raw(wtk_matf_t *src_v,wtk_matf_t *m,wtk_matf_t *dst_v)
{
	float f;
	int i,j;

	//wtk_matf_print(m);
	//exit(0);
	for(i=0;i<dst_v->col;++i)
	{
		f=0;
		for(j=0;j<src_v->col;++j)
		{
			f+=src_v->p[j]*(*wtk_matf_at(m,j,i));
			//wtk_debug("v[%d/%d]=%f/%f/%f\n",i,j,src_v->p[j],*wtk_matf_at(m,j,i),f);
		}
		dst_v->p[i]=f;
	}
}


/**
 *   |x*N|*|y*N|=|x*y|
 * c=a*b
 */
void wtk_matf_mul(wtk_matf_t *a,wtk_matf_t *b,wtk_matf_t *c)
{
	float t;
	float *pf1,*pf2,*pf3;
	int i,j,k;
	int row=b->row;
	int col=b->col;

	c->row=a->row;
	pf1=a->p;
	pf3=c->p;
	for(i=0;i<a->row;++i)
	{
		pf2=b->p;
		for(j=0;j<row;++j)
		{
			t=0;
			for(k=0;k<col;++k)
			{
				//wtk_debug("v[%d]=%f\n",k,pf1[k]);
				t+=pf1[k]*(*(pf2++));
			}
			//wtk_debug("t=%f\n",t);
			*(pf3++)=t;
		}
		pf1+=col;
	}
}

void wtk_matf_multvm(wtk_matf_t *src_v,wtk_matf_t *m,wtk_matf_t *dst_v)
{
	float f;
	int i,j;
	float *f1,*f2;

	for(i=0;i<dst_v->col;++i)
	{
		f=0;
		for(j=0,f1=src_v->p,f2=m->p+i;j<src_v->col;++j,f2+=m->col)
		{
			f+=(*(f1++))*(*f2);
			//wtk_debug("v[%d/%d]=%f/%f/%f\n",i,j,src_v->p[j],*wtk_matf_at(m,j,i),f);
		}
		dst_v->p[i]=f;
	}
}


void wtk_matf_save(wtk_matf_t *m,FILE *f,char *sym)
{
	int i,j;
	float *fp;

	fp=m->p;
	fprintf(f,"%s row: %d col: %d\n",sym,m->row,m->col);
	for(i=0;i<m->row;++i)
	{
		for(j=0;j<m->col;++j)
		{
			if(j>0)
			{
				fprintf(f," %.6f",*(fp++));
			}else
			{
				fprintf(f,"%.6f",*(fp++));
			}
		}
		fprintf(f,"\n");
	}
	//fprintf(f,"\n");
}

wtk_matf_t* wtk_matf_transpose(wtk_matf_t *mf)
{
	wtk_matf_t *dst;
	float *p;
	int i,j;

	dst=wtk_matf_new(mf->col,mf->row);
	p=mf->p;
	for(i=0;i<mf->row;++i)
	{
		for(j=0;j<mf->col;++j)
		{
			*(wtk_matf_at(dst,j,i))=*(p++);
		}
	}
	return dst;
}


wtk_matf_t* wtk_matf_new2(wtk_matrix_t *m)
{
	int row,col;
	wtk_matf_t *mf;
	int i,j;
	float *px,*ppx,*fp;

	row=wtk_matrix_rows(m);
	col=wtk_matrix_cols(m);

	mf=wtk_matf_new(row,col);
	for(px=mf->p,i=0;i<row;++i,px+=col)
	{
		for(ppx=px,fp=m[i+1],j=0;j<col;++j)
		{
			*(ppx++)=*(++fp);
		}
	}
	return mf;
}

void wtk_matf_print(wtk_matf_t *f)
{
	int i,j;
	//char *px,*ppx;
	float *ppx;

	wtk_debug("============= mi=%p ===========\n",f);
	for(ppx=f->p,i=0;i<f->row;++i)
	{
		for(j=0;j<f->col;++j)
		{
			printf("v[%d][%d]=%.12f\n",i,j,*(ppx++));
			//printf("v[%d][%d]=%d\n",i,j,wtk_mati_prow_at(mc,i,j));
		}
	}
}

void wtk_matf_mul_raw(wtk_matf_t *m,wtk_matf_t *a,wtk_matf_t *b)
{
	int i,j,k;
	int t;
//#define DEBUG_MX

	//i=row of a j=col of b
	//m[i][j]=a[i][k]*b[k][j]
	for(i=0;i<a->row;++i)
	{
		for(j=0;j<b->col;++j)
		{
			for(t=0,k=0;k<a->col;++k)
			{
				t+=(*(a->p+i*a->col+k)) * (*(b->p+k*b->col+j));
			}
			//m[i][j]=t;
			*(m->p+i*m->col+j)=t;
		}
	}
}

wtk_matc_t* wtk_matc_new_align(int row,int col,int align)
{
	wtk_matc_t *m;
	int n;
	int col_bytes;

	n=wtk_round(sizeof(wtk_matc_t),align);
	col_bytes=wtk_round(col,align);
	n+=col_bytes*row;
	m=(wtk_matc_t*)wtk_malloc(n);
	m->row=row;
	m->col=col;
	m->align=align;
	m->col_bytes=col_bytes;
	m->p=(signed char*)m+wtk_round(sizeof(wtk_matc_t),align);
	//wtk_debug("n=%d/%d row=%d\n",(int)sizeof(wtk_matc_t),n,row*col);
	return m;
}

void wtk_matc_init_transpose(wtk_matc_t *dst,wtk_matc_t *src)
{
	signed char *p;
	int i,j;

	for(i=0;i<src->row;++i)
	{
		p=src->p+i*src->col_bytes;
		for(j=0;j<src->col;++j)
		{
			*(wtk_mat_at(dst,j,i))=*(p++);
		}
	}
}

wtk_matc_t* wtk_matc_transpose(wtk_matc_t *src)
{
	wtk_matc_t *dst;
	signed char *p;
	int i,j;

	dst=wtk_matc_new(src->col,src->row);
	p=src->p;
	for(i=0;i<src->row;++i)
	{
		for(j=0;j<src->col;++j)
		{
			*(wtk_matf_at(dst,j,i))=*(p++);
		}
	}
	return dst;
}

wtk_matc_t* wtk_matc_new(int row,int col)
{
	wtk_matc_t *m;
	char *p;

	p=wtk_malloc(sizeof(wtk_matc_t)+row*col*sizeof(char)+16);
	m=(wtk_matc_t*)p;
	m->row=row;
	m->col=col;
	m->align=0;
	m->col_bytes=col;
	m->p=wtk_align_ptr(p+sizeof(wtk_matc_t),16);
	return m;
}

void  wtk_matc_init_matf(wtk_matc_t *mc,wtk_matf_t *mf,float scale)
{
	int i,n;
	signed char *pc;
	float *pf;
	float f;

	pc=mc->p;
	pf=mf->p;
	n=mc->row*mc->col;
	for(i=0;i<n;++i)
	{
		f=pf[i]*scale;
		pc[i]=wtk_float_round(f);
	}
}


void wtk_matc_init(wtk_matc_t *cm,wtk_matrix_t *m,float scale)
{
	int row,col;
	int i,j;
	signed char *px;
	register signed char *ppx;
	register float *fp;
	//float scale;
	float f;

	//scale=((1<<7)-1)*1.0/max;
	row=wtk_matrix_rows(m);
	col=wtk_matrix_cols(m);
	cm->row=row;
	cm->col=col;
	for(px=cm->p,i=0;i<row;++i,px+=col)
	{
		for(ppx=px,fp=m[i+1],j=0;j<col;++j)
		{
			f=(*(++fp))*scale;
			*(ppx++)=(signed char)(wtk_float_round(f));
		}
	}
}

void wtk_matc_init2(wtk_matc_t *cm,wtk_matrix_t *m,float scale)
{
	int row,col;
	int i,j;
	signed char *px,*ppx;
	float *fp;
	//float scale;
	float f;

	//scale=((1<<7)-1)*1.0/max;
	row=wtk_matrix_rows(m);
	col=wtk_matrix_cols(m);
	cm->row=row;
	cm->col=col;
	for(px=cm->p,i=0;i<row;++i,px+=col)
	{
		for(ppx=px,fp=m[i+1],j=0;j<col;++j)
		{
			f=(*(++fp))*scale;
			*(ppx++)=(signed char)(wtk_float_round(f));
		}
	}
}

void wtk_matc_cpy(wtk_matc_t *dst,wtk_matc_t *src)
{
	int i;
	int n1,n2;

	n1=n2=0;
	for(i=0;i<dst->row;++i,n1+=src->col_bytes,n2+=dst->col_bytes)
	{
		memcpy(dst->p+n2,src->p+n1,src->col);
	}
}

wtk_matc_t* wtk_matc_new2(wtk_matrix_t *m,float scale)
{
	wtk_matc_t *cm;
	int row,col;
	int i,j;
	signed char *px,*ppx;
	float *fp;
	//float scale;
	float f;

	//scale=((1<<7)-1)*1.0/max;
	row=wtk_matrix_rows(m);
	col=wtk_matrix_cols(m);
	cm=wtk_matc_new(row,col);
	for(px=cm->p,i=0;i<row;++i,px+=col)
	{
		for(ppx=px,fp=m[i+1],j=0;j<col;++j)
		{
			f=(*(++fp))*scale;
			*(ppx++)=(signed char)(wtk_float_round(f));
		}
	}
	return cm;
}


/**
 * |---------- row -------------|
 *   |
 *  col
 *   |
 * |----------------------------|
 */
wtk_matc_t* wtk_matc_new3(wtk_matrix_t *m,float scale)
{
	wtk_matc_t *cm;
	int row,col;
	int i,j;
	signed char *p;
	//float scale;
	float f;

	//scale=((1<<7)-1)*1.0/max;
	row=wtk_matrix_rows(m);
	col=wtk_matrix_cols(m);
	cm=wtk_matc_new(row,col);
	for(p=cm->p,i=1;i<=col;++i)
	{
		for(j=1;j<=row;++j)
		{
			f=m[j][i]*scale;
			*(p++)=(signed char)(wtk_float_round(f));
		}
	}
	return cm;
}

void wtk_matc_delete(wtk_matc_t *m)
{
	wtk_free(m);
}

void wtk_matc_print(wtk_matc_t *mc)
{
	int i,j;
	//char *px,*ppx;
	signed char *px;

	wtk_debug("============= mi=%p ===========\n",mc);
	for(i=0;i<mc->row;++i)
	{
		px=mc->p+i*mc->col_bytes;
		for(j=0;j<mc->col;++j)
		{
			printf("v[%d][%d]=%d\n",i,j,*(px++));
			//printf("v[%d][%d]=%d\n",i,j,wtk_mati_prow_at(mc,i,j));
		}
		if(i==1)
		{
			exit(0);
		}
	}
}

wtk_matuc_t* wtk_matuc_new_align(int row,int col,int align)
{
	wtk_matuc_t *m;
	int n;
	int col_bytes;

	n=wtk_round(sizeof(wtk_matuc_t),align);
	col_bytes=wtk_round(col,align);
	n+=col_bytes*row;
	m=(wtk_matuc_t*)wtk_malloc(n);
	m->row=row;
	m->col=col;
	m->align=align;
	m->col_bytes=col_bytes;
	m->p=(unsigned char*)m+wtk_round(sizeof(wtk_matuc_t),align);
	//wtk_debug("n=%d/%d row=%d\n",(int)sizeof(wtk_matc_t),n,row*col);
	return m;
}

wtk_matuc_t* wtk_matuc_new3(wtk_matc_t *src,int *pmin)
{
	wtk_matuc_t *uc;
	int min=1000;
	int i,j;
	signed char *p;
	unsigned char *c;

	uc=wtk_matuc_new_align(src->col,src->row,4);
	uc->col=src->col;
	uc->row=src->row;
	min=1000;
	//wtk_debug("row=%d col=%d col_bytes=%d/%d\n",src->row,src->col,src->col_bytes,uc->col_bytes);
	//exit(0);
	for(i=0;i<src->col;++i)
	{
		p=src->p+i*src->col_bytes;
		for(j=0;j<src->row;++j)
		{
			if(p[j]<min)
			{
				min=p[j];
			}
		}
	}
	for(i=0;i<src->col;++i)
	{
		p=src->p+i*src->col_bytes;
		c=uc->p+i*uc->col_bytes;
		for(j=0;j<src->row;++j)
		{
			c[j]=p[j]-min;
		}
	}
	//exit(0);
	*pmin=min;
	return uc;
}


wtk_matuc_t* wtk_matuc_new(int row,int col)
{
	wtk_matuc_t *m;
	char *p;

	p=wtk_malloc(sizeof(wtk_matuc_t)+row*col*sizeof(char));
	m=(wtk_matuc_t*)p;
	m->row=row;
	m->col=col;
	m->align=0;
	m->col_bytes=col;
	m->p=(unsigned char*)(p+sizeof(wtk_matuc_t));
	return m;
}

void wtk_matuc_init(wtk_matuc_t* cm,wtk_matrix_t *m,float scale)
{
	int row,col;
	int i,j;
	unsigned char *px,*ppx;
	float *fp;
	//float scale;
	float f;

	//scale=((1<<8)-1)*1.0/max;
	row=wtk_matrix_rows(m);
	col=wtk_matrix_cols(m);
	for(px=cm->p,i=0;i<row;++i,px+=col)
	{
		for(ppx=px,fp=m[i+1],j=0;j<col;++j)
		{
			f=(*(++fp))*scale;
			*(ppx++)=(unsigned char)(wtk_float_round(f));
		}
	}
}

wtk_matuc_t* wtk_matuc_new2(wtk_matrix_t *m,float scale)
{
	wtk_matuc_t *cm;

	cm=wtk_matuc_new(wtk_matrix_rows(m),wtk_matrix_cols(m));
	wtk_matuc_init(cm,m,scale);
	return cm;
}


void wtk_matuc_delete(wtk_matuc_t *mc)
{
	wtk_free(mc);
}

void wtk_matuc_print(wtk_matuc_t *mc)
{
	int i,j;
	//char *px,*ppx;

	wtk_debug("============= mi=%p ===========\n",mc);
	for(i=0;i<mc->row;++i)
	{
		for(j=0;j<mc->col;++j)
		{
			//printf("v[%d][%d]=%d\n",i,j,*(ppx++));
			//printf("v[%d][%d]=%d\n",i,j,wtk_mati_prow_at(mc,i,j));
		}
	}
}


wtk_mati_t* wtk_mati_new(int row,int col)
{
	wtk_mati_t *m;
	char *p;

	p=wtk_malloc(sizeof(wtk_mati_t)+row*col*sizeof(int)+32);
	m=(wtk_mati_t*)p;
	m->row=row;
	m->col=col;
	m->p=(int*)wtk_align_ptr((p+sizeof(wtk_mati_t)),32);
	//wtk_debug("%p\n",m->p);
	//wtk_debug("row=%d col=%d\n",row,col);
	return m;
}

void wtk_mati_cpy(wtk_mati_t *src,wtk_mati_t *dst)
{
	memcpy(dst->p,src->p,dst->row*dst->col*sizeof(int));
}

void wtk_mati_zero(wtk_mati_t *m)
{
	memset(m->p,0,m->row*m->col*sizeof(int));
}

void wtk_mati_init(wtk_mati_t *im,wtk_matrix_t *m,float scale)
{
	int row,col;
	int i,j;
	int *px,*ppx;
	float *fp;
	//float scale;
	float f;

	//scale=(((unsigned)(1<<31))-1)*1.0/max;
	row=wtk_matrix_rows(m);
	col=wtk_matrix_cols(m);
	im->row=row;
	im->col=col;
	for(px=im->p,i=0;i<row;++i,px+=col)
	{
		for(ppx=px,fp=m[i+1],j=0;j<col;++j)
		{
			f=(*(++fp))*scale;
			*(ppx++)=(int)(wtk_float_round(f));
			//wtk_debug("%f=%d\n",f,(int)(wtk_float_round(f)));
			/*
			if(i==0 && j<5)
			{
				wtk_debug("%f=%d\n",f,(int)(wtk_float_round(f)));
			}*/
		}
	}
}

wtk_mati_t* wtk_mati_new2(wtk_matrix_t *m,float scale)
{
	wtk_mati_t *im;

	im=wtk_mati_new(wtk_matrix_rows(m),wtk_matrix_cols(m));
	wtk_mati_init(im,m,scale);
	return im;
}

wtk_mati_t* wtk_mati_new3(wtk_matrix_t *m,float scale)
{
	wtk_mati_t *im;
	int row,col;
	int i,j;
	int *p;
	//float scale;
	float f;

	//scale=((1<<7)-1)*1.0/max;
	row=wtk_matrix_rows(m);
	col=wtk_matrix_cols(m);
	im=wtk_mati_new(row,col);
	for(p=im->p,i=1;i<=col;++i)
	{
		for(j=1;j<=row;++j)
		{
			f=m[j][i]*scale;
			*(p++)=(int)(wtk_float_round(f));
		}
	}
	return im;
}

void wtk_mati_delete(wtk_mati_t *im)
{
	wtk_free(im);
}

void wtk_mati_scale(wtk_mati_t *mi,double scale)
{
	int n=mi->row*mi->col;
	int *p,*pe;
	float f;

	p=mi->p;
	pe=p+n;
	while(p<pe)
	{
		f=(*p)*scale;
		(*p)=wtk_float_round(f);
		++p;
	}
}

void wtk_mati_set(wtk_mati_t *mi,int row,int col,int v)
{
	*(mi->p+row*mi->col+col)=v;
}

int wtk_mati_get(wtk_mati_t *mi,int row,int col)
{
	return *(mi->p+row*mi->col+col);
}

wtk_mati_t* wtk_mati_transpose(wtk_mati_t *mi)
{
	wtk_mati_t *dst;
	int *p;
	int i,j;

	dst=wtk_mati_new(mi->col,mi->row);
	p=mi->p;
	for(i=0;i<mi->row;++i)
	{
		for(j=0;j<mi->col;++j)
		{
			wtk_mati_at(dst,j,i)=*(p++);
		}
	}
	return dst;
}


int wtk_mati_max(wtk_mati_t *m)
{
	register int *p,*pe;
	register int max,v;

	p=m->p-1;
	pe=p+m->row*m->col;
	max=*(++p);
	while(p<pe)
	{
		v=*(++p);
		if(v>max)
		{
			max=v;
		}
	}
	return max;
}

int wtk_mati_min(wtk_mati_t *m)
{
	int *p,*pe;
	int max=0;
	int set=0;

	p=m->p;
	pe=p+m->row*m->col;
	while(p<pe)
	{
		if(!set)
		{
			set=1;
			max=*p;
		}else if(*p<max)
		{

			max=*p;
		}
		++p;
	}
	return max;
}

int wtk_mati_value_count(wtk_mati_t *m,int v)
{
	int cnt;
	int *p,*pe;

	cnt=0;
	p=m->p;
	pe=p+m->row*m->col;
	while(p<pe)
	{
		if(*p==v)
		{
			++cnt;
		}
		++p;
	}
	return cnt;
}

int wtk_mati_range_count(wtk_mati_t *m,int f1,int f2)
{
	int cnt;
	int *p,*pe;

	cnt=0;
	p=m->p;
	pe=p+m->row*m->col;
	while(p<pe)
	{
		if(*p>=f1 && *p<=f2)
		{
			++cnt;
		}
		++p;
	}
	return cnt;
}

int wtk_mati_avg(wtk_mati_t *m)
{
	register int *p,*pe;
	register float f;
	register int v;
	int cnt;

	f=0;
	p=m->p-1;
	cnt=m->row*m->col;
	pe=p+cnt;
	while(p<pe)
	{
		v=*(++p);
		f+=v>=0?v:-v;
	}
	return (int)(f/(cnt));
}

void wtk_mati_prune(wtk_mati_t *m,int f1,int f2)
{
	int *p,*pe;

	p=m->p;
	pe=p+m->row*m->col;
	while(p<pe)
	{
		if(*p>=f1 && *p<=f2)
		{
			*p=0;
		}
		++p;
	}
}

void wtk_mati_print(wtk_mati_t *mi)
{
	int i,j;
	//char *px,*ppx;
	int *px;

	px=mi->p;
	wtk_debug("============= mi=%p ===========\n",mi);
	for(i=0;i<mi->row;++i)
	{
		for(j=0;j<mi->col;++j)
		{
			printf("v[%d][%d]=%d\n",i,j,*(px++));
			//printf("v[%d][%d]=%d\n",i,j,wtk_mati_prow_at(mi,i,j));
		}
	}
}

void wtk_mati_print2(wtk_mati_t *mi,int cnt)
{
	int i;

	for(i=0;i<cnt;++i)
	{
		wtk_debug("v[%d]=%d\n",i,mi->p[i]);
	}
}

#define MUL_X_b(j,epm,pm,N) \
if(j==0) \
{\
	while(epm-pm>=4) \
	{\
		*(pm++)=(*(pb++))<<N;\
		*(pm++)=(*(pb++))<<N;\
		*(pm++)=(*(pb++))<<N;\
		*(pm++)=(*(pb++))<<N;\
	} \
	while(epm>pm) \
	{\
		*(pm++)=(*(pb++))<<N;\
	}\
}else\
{\
	while(epm-pm>=4) \
	{\
		*(pm++)+=(*(pb++))<<N;\
		*(pm++)+=(*(pb++))<<N;\
		*(pm++)+=(*(pb++))<<N;\
		*(pm++)+=(*(pb++))<<N;\
	}\
	while(epm>pm)\
	{\
		*(pm++)+=(*(pb++))<<N;\
	}\
}


#define MUL_X3(j,epm,pm,N) \
if(j==0) \
{\
	while(epm-pm>=4) \
	{\
		t=*((int*)pb);\
		pb+=4;\
		*(pm++)=((char)((t>>8) & 0x00FF))<<N; \
		*(pm++)=((char)((t>>16) & 0x00FF))<<N; \
		*(pm++)=((char)((t>>24) & 0x00FF))<<N; \
		*(pm++)=((char)((t>>24) & 0x00FF))<<N; \
	} \
	while(epm>pm) \
	{\
		*(pm++)=(*(pb++))<<N;\
	}\
}else\
{\
	while(epm-pm>=4) \
	{\
		t=*((int*)pb);\
		pb+=4;\
		*(pm++)+=((char)(t & 0x00FF))<<N; \
		*(pm++)+=((char)((t>>8) & 0x00FF))<<N; \
		*(pm++)+=((char)((t>>16) & 0x00FF))<<N; \
		*(pm++)+=((char)((t>>24) & 0x00FF))<<N; \
	}\
	while(epm>pm)\
	{\
		*(pm++)+=(*(pb++))<<N;\
	}\
}

#define MUL_XT(j,epm,pm,N) \
if(j==0) \
{\
	nx1=8-N;\
	nx2=16-N;\
	nx3=24-N;\
	while(epm-pm>=4) \
	{\
		t=*((int*)pb);\
		pb+=4;\
		*(pm++)=(char)((t&0x00FF)<<N);\
		*(pm++)=(char)((t&0x00FF00)<<nx1);\
		*(pm++)=(char)((t&0x00FF0000)<<nx2);\
		*(pm++)=(char)((t&0x00FF000000)<<nx3);\
	} \
	while(epm>pm) \
	{\
		*(pm++)=(*(pb++))<<N;\
	}\
}else\
{\
	nx1=8-N;\
	nx2=16-N;\
	nx3=24-N;\
	while(epm-pm>=4) \
	{\
		t=*((int*)pb);\
		pb+=4;\
		*(pm++)+=(char)((t&0x00FF)<<N);\
		*(pm++)+=(char)((t&0x00FF00)<<nx1);\
		*(pm++)+=(char)((t&0x00FF0000)<<nx2);\
		*(pm++)+=(char)((t&0x00FF000000)<<nx3);\
	}\
	while(epm>pm)\
	{\
		*(pm++)+=(*(pb++))<<N;\
	}\
}

#define MUL_X(j,epm,pm,N) \
if(j==0) \
{\
	while(epm-pm>=4) \
	{\
		*(pm++)=(*(pb++))<<N;\
		*(pm++)=(*(pb++))<<N;\
		*(pm++)=(*(pb++))<<N;\
		*(pm++)=(*(pb++))<<N;\
	} \
	while(epm>pm) \
	{\
		*(pm++)=(*(pb++))<<N;\
	}\
}else\
{\
	while(epm-pm>=4) \
	{\
		*(pm++)+=(*(pb++))<<N;\
		*(pm++)+=(*(pb++))<<N;\
		*(pm++)+=(*(pb++))<<N;\
		*(pm++)+=(*(pb++))<<N;\
	}\
	while(epm>pm)\
	{\
		*(pm++)+=(*(pb++))<<N;\
	}\
}

char* wtk_mati_xxx_2(int j,register int pak,register int *pm,register int *epm,register char *pb)
{
	register int t,t1;

	if(j==0)
	{
		while(epm-pm>=4)
		{
			t=*((int*)pb);
			t1=pak*(t&0x00FF00FF);

			pm[0]=t1&0x00FFFF;
			pm[2]=(t1&0xFFFF000)>>16;

			t1=pak*((t>>8)&0x00FF00FF);
			pm[1]=t1&0x00FFFF;
			pm[3]=(t1&0xFFFF000)>>16;

			pb+=4;
			pm+=4;
			/*
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
			*/
		}
		while(epm>pm)
		{
			*(pm++)=pak*(*(pb++));
		}
	}else
	{
		while(epm-pm>=4)
		{
			t=*((int*)pb);
			t1=pak*(t&0x00FF00FF);

			pm[0]=t1&0x00FFFF;
			pm[2]=(t1&0xFFFF000)>>16;

			t1=pak*((t>>8)&0x00FF00FF);
			t1=pak*t;
			pm[1]=t1&0x00FFFF;
			pm[3]=(t1&0xFFFF000)>>16;

			pb+=4;
			pm+=4;
			/*
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));*/
		}
		while(epm>pm)
		{
			*(pm++)+=pak*(*(pb++));
		}
	}
	return pb;
}

char* wtk_mati_xxx2(int j,register int pak,register int *pm,register int *epm,register char *pb)
{
	if(j==0)
	{
		while(epm-pm>=8)
		{
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));

			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
		}
		while(epm-pm>=4)
		{
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
		}
		while(epm>pm)
		{
			*(pm++)=pak*(*(pb++));
		}
	}else
	{
		while(epm-pm>=8)
		{
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));

			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
		}
		while(epm-pm>=4)
		{
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
		}
		while(epm>pm)
		{
			*(pm++)+=pak*(*(pb++));
		}
	}
	return pb;
}

long int mat_mul_x(int pak,int *pm,int *epm,char *pb)
{
	register int t;

	while(epm-pm>=4)
	{
		t=*((short*)pb);
		t=((t&0x00FF)+( t&0x00FF00<<16))*pak;
		*(pm++)=t&0x00FFFF;
		*(pm++)=(t>>16)&0x00FFFF;

		t=*((short*)pb);
		t=((t&0x00FF)+( t&0x00FF00<<16))*pak;
		*(pm++)=t&0x00FFFF;
		*(pm++)=(t>>16)&0x00FFFF;
	}
	while(epm>pm)
	{
		*(pm++)=pak*(*(pb++));
	}
	return 0;
}

char* wtk_mati_xxx(int j,register int pak,int *pm,int *epm,char *pb)
{
//#define USE_PROFILE
#ifdef USE_PROFILE
	//int t;
#endif

	//wtk_debug("pm=%p epm=%p\n",pm,epm);
	//exit(0);
	if(j==0)
	{
		while(epm-pm>=4)
		{
#ifdef USE_PROFILE
			*pm=pak*(*pb);
			pm[1]=pak*(pb[1]);
			pm[2]=pak*(pb[2]);
			pm[3]=pak*(pb[3]);

			/*
			t=pak*(*((int*)pb));
			pm[0]=t;
			pm[1]=t;
			pm[2]=t;
			pm[3]=t;*/
			/*
			*(pm++)=t;
			*(pm++)=t;
			*(pm++)=t;
			*(pm++)=t;
			*(pm++)=t;*/
			pb+=4;
			pm+=4;
#else
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
#endif
		}
		while(epm>pm)
		{
			*(pm++)=pak*(*(pb++));
		}
	}else
	{
		while(epm-pm>=4)
		{
#ifdef USE_PROFILE
			*pm+=pak*(*pb);
			pm[1]+=pak*(pb[1]);
			pm[2]+=pak*(pb[2]);
			pm[3]+=pak*(pb[3]);

			/*
			t=pak*(*((int*)pb));
			pm[0]+=t;
			pm[1]+=t;
			pm[2]+=t;
			pm[3]+=t;*/
			/*
			*(pm++)+=t;
			*(pm++)+=t;
			*(pm++)+=t;
			*(pm++)+=t;
			*(pm++)=t;*/
			pb+=4;
			pm+=4;
#else
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
#endif
		}
		while(epm>pm)
		{
			*(pm++)+=pak*(*(pb++));
		}
	}
	return pb;
}

/**
 * |2*245|*|245*128|=|2*128|
 */
void wtk_mati_multi_x2(wtk_mati_t *m,wtk_matuc_t *a,wtk_matc_t *b)
{
	unsigned char *pa;
	register signed char *pb;
	register int pak;
	register int *pm,*epm;
	int *tpm;
	int i,j;
	//register int t;
	//int nx1,nx2,nx3;

	//wtk_debug("a=[%d*%d] b=[%d*%d]\n",a->row,a->col,b->row,b->col);
	for(tpm=m->p,pa=a->p,i=0;i<a->row;++i,tpm+=m->col)
	{
		for(pb=b->p,j=0;j<a->col;++j)
		{
			pak=*(pa++);//pa[j];
			//wtk_debug("pak=%d\n",pak);
			pm=tpm;
			epm=pm+b->col;
			if(j==0)
			{
				while(epm>pm)
				{
					*(pm++)=pak*(*(pb++));
				}
			}else
			{
				while(epm>pm)
				{
					*(pm++)+=pak*(*(pb++));
				}
			}
		}
	}
}

#ifdef USE_SIMD
#include <arm_neon.h>

void wtk_mati_multi2(wtk_mati_t *m,wtk_mati_t *a,wtk_matc_t *p)
{
	int *pa;
	register signed char *pb,*epb;
	register int pak;
	register int *pm;
	int *tpm;
	int i,j;
	int32x4_t fa,fb,fc;
	int buf[4];

	for(tpm=m->p,pa=a->p,i=0;i<a->row;++i,tpm+=m->col)
	{
		for(pb=p->p,j=0;j<a->col;++j)
		{
			pak=*(pa++);//pa[j];
			epb=pb+p->col;
			pm=tpm;
			fa=vdupq_n_s32(pak);
			if(j==0)
			{
				while(epb-pb>=4)
				{
					buf[0]=pb[0];
					buf[1]=pb[1];
					buf[2]=pb[2];
					buf[3]=pb[3];
					fb=vld1q_s32(buf);
					fc=vmulq_s32(fa,fb);
					vst1q_s32(pm,fc);
					pm+=4;
					pb+=4;
				}
				while(epb>pb)
				{
					*(pm++)=pak*(*(pb++));
				}
			}else
			{
				while(epb-pb>=4)
				{
					buf[0]=pb[0];
					buf[1]=pb[1];
					buf[2]=pb[2];
					buf[3]=pb[3];
					fb=vld1q_s32(buf);
					fc=vld1q_s32(pm);
					fc=vmlaq_s32(fc,fa,fb);
					vst1q_s32(pm,fc);
					pm+=4;
					pb+=4;
				}
				while(epb>pb)
				{
					*(pm++)+=pak*(*(pb++));
				}
			}
		}
	}
}

void wtk_mati_multi(wtk_mati_t *m,wtk_matuc_t *a,wtk_matc_t *p)
{
	unsigned char *pa;
	register signed char *pb;
	register int pak;
	register int *pm,*epm;
	int *tpm;
	int i,j;
	//short buf[8];
	int16x8_t fa,fb,fc;
	short buf[8];

	//wtk_debug("a=[%d*%d] b=[%d*%d]\n",a->row,a->col,b->row,b->col);
	for(tpm=m->p,pa=a->p,i=0;i<a->row;++i,tpm+=m->col)
	{
		for(pb=p->p,j=0;j<a->col;++j)
		{
			pak=*(pa++);//pa[j];
			//wtk_debug("pak=%d\n",pak);
			pm=tpm;
			epm=pm+p->col;
			fa=vdupq_n_s16(pak);
			if(j==0)
			{
				while(epm-pm>=8)
				{
					fb=vmovl_s8(vld1_s8(pb));
					fc=vmulq_s16(fa,fb);
					vst1q_s16(buf,fc);
					*(pm++)=buf[0];
					*(pm++)=buf[1];
					*(pm++)=buf[2];
					*(pm++)=buf[3];
					*(pm++)=buf[4];
					*(pm++)=buf[5];
					*(pm++)=buf[6];
					*(pm++)=buf[7];
					pb+=8;
				}
				while(epm-pm>=4)
				{
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
				}
				while(epm>pm)
				{
					*(pm++)=pak*(*(pb++));
				}
			}else
			{
				while(epm-pm>=8)
				{
					fb=vmovl_s8(vld1_s8(pb));
					fc=vmulq_s16(fa,fb);
					vst1q_s16(buf,fc);
					*(pm++)+=buf[0];
					*(pm++)+=buf[1];
					*(pm++)+=buf[2];
					*(pm++)+=buf[3];
					*(pm++)+=buf[4];
					*(pm++)+=buf[5];
					*(pm++)+=buf[6];
					*(pm++)+=buf[7];
					pb+=8;
				}
				while(epm-pm>=4)
				{
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
				}
				while(epm>pm)
				{
					*(pm++)+=pak*(*(pb++));
				}
			}
		}
	}
}

#else

void wtk_mati_multi(wtk_mati_t *m,wtk_matuc_t *a,wtk_matc_t *b)
{
	unsigned char *pa;
	register signed char *pb;
	register int pak;
	register int *pm,*epm;
	int *tpm;
	int i,j;
	//register int t;
	//int nx1,nx2,nx3;

	//wtk_debug("a=[%d*%d] b=[%d*%d]\n",a->row,a->col,b->row,b->col);
	for(tpm=m->p,pa=a->p,i=0;i<a->row;++i,tpm+=m->col)
	{
		epm=tpm+b->col;
		for(pb=b->p,j=0;j<a->col;++j)
		{
			pak=*(pa++);//pa[j];
			//wtk_debug("pak=%d\n",pak);
			pm=tpm;
			//epm=pm+b->col;
			switch(pak)
			{
			case 0:
				if(j==0)
				{
					memset(pm,0,b->col<<2);
				}
				pb+=b->col;
				break;
			case 1:
				if(j==0)
				{
					while(epm-pm>=4)
					{
						*(pm++)=(*(pb++));
						*(pm++)=(*(pb++));
						*(pm++)=(*(pb++));
						*(pm++)=(*(pb++));
					}
					while(epm>pm)
					{
						*(pm++)=(*(pb++));
					}
				}else
				{
					while(epm-pm>=4)
					{
						*(pm++)+=(*(pb++));
						*(pm++)+=(*(pb++));
						*(pm++)+=(*(pb++));
						*(pm++)+=(*(pb++));
					}
					while(epm>pm)
					{
						*(pm++)+=(*(pb++));
					}
				}
				break;
			case 2:
				MUL_X(j,epm,pm,1);
				break;
			case 4:
				MUL_X(j,epm,pm,2);
				break;
			case 8:
				MUL_X(j,epm,pm,3);
				break;
			case 16:
				MUL_X(j,epm,pm,4);
				break;
			case 32:
				MUL_X(j,epm,pm,5);
				break;
			case 64:
				MUL_X(j,epm,pm,6);
				break;
			case 128:
				MUL_X(j,epm,pm,7);
				break;
			default:
				//pb=wtk_mati_xxx(j,pak,pm,epm,pb);
				if(j==0)
				{
					while(epm-pm>=4)
					{
						*(pm++)=pak*(*(pb++));
						*(pm++)=pak*(*(pb++));
						*(pm++)=pak*(*(pb++));
						*(pm++)=pak*(*(pb++));
					}
					while(epm>pm)
					{
						*(pm++)=pak*(*(pb++));
					}
				}else
				{
					while(epm-pm>=4)
					{
						*(pm++)+=pak*(*(pb++));
						*(pm++)+=pak*(*(pb++));
						*(pm++)+=pak*(*(pb++));
						*(pm++)+=pak*(*(pb++));
					}
					while(epm>pm)
					{
						*(pm++)+=pak*(*(pb++));
					}
				}
				break;
			}
		}
	}
}
#endif

void wtk_mati_multi_2x(wtk_mati_t *m,wtk_matuc_t *a,wtk_matc_t *b)
{
	unsigned char *pa;
	register signed  char *pb,*epb;
	register unsigned char pak;
	register int *pm;
	int *tpm;
	int i,j;

	for(tpm=m->p,pa=a->p,i=0;i<a->row;++i,tpm+=m->col)
	{
		for(pb=b->p,j=0;j<a->col;++j)
		{
			pak=*(pa++);
			epb=pb+b->col;
			pm=tpm;
			if(j==0)
			{
				while(epb-pb>=4)
				{
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
				}
				while(epb>pb)
				{
					*(pm++)=pak*(*(pb++));
				}
			}else
			{
				while(epb-pb>=4)
				{
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
				}
				while(epb>pb)
				{
					*(pm++)+=pak*(*(pb++));
				}
			}
		}
	}
}


void wtk_mati_multi_x_raw(wtk_mati_t *m,wtk_matuc_t *a,wtk_matc_t *b)
{
	int i,j,k;
	int t;
//#define DEBUG_MX

	//i=row of a j=col of b
	//m[i][j]=a[i][k]*b[k][j]
	for(i=0;i<a->row;++i)
	{
		for(j=0;j<b->col;++j)
		{
			for(t=0,k=0;k<a->col;++k)
			{
				t+=(*(a->p+i*a->col+k)) * (*(b->p+k*b->col+j));
				//t+=a[i][k]*b[k][j]
				wtk_debug("v[%d]=%d*%d=%d/%f\n",k+1,*(a->p+i*a->col+k),
						*(b->p+k*b->col+j),t,t*1.0/(12.5669*255.0));
			}
			//m[i][j]=t;
			*(m->p+i*m->col+j)=t;
			wtk_debug("t=%d/%f\n",t,t*1.0/(12.5669*255.0));
			exit(0);
		}
	}
}

void wtk_mati_multi_x_raw2(wtk_mati_t *m,wtk_matuc_t *a,wtk_matc_t *b)
{
	int i,j,k;
	int t;

	for(i=0;i<a->row;++i)
	{
		for(j=0;j<b->row;++j)
		{
			t=0;
			for(k=0;k<a->col;++k)
			{
				//t+=a[i][k]*b[k][j];
				t+=(*(a->p+i*a->col+k)) * (*(b->p+j*b->col+k));
				//wtk_debug("%d*%d=%d\n",*(a->p+i*a->col+k),*(b->p+i*b->col+k),t);
				//if(0)
				{
					wtk_debug("v[%d][%d]=[%d*%d/%d]\n",i,j,
						(*(a->p+i*a->col+k)),(*(b->p+j*b->col+k)),t);
					if(k>10)
					{
						exit(0);
					}
				}
			}
			//m[i][j]=t;
			*(m->p+i*m->col+j)=t;
			wtk_debug("v[%d][%d]=%d\n",i,j,t);
			if(j>3)
			{
				exit(0);
			}
		}
	}
}


void wtk_mati_multi_cu_x_scale(wtk_mati_t *m,wtk_matc_t *a,wtk_matuc_t *b,int min,float scale)
{
	int i,j;
	register int t;
	register signed char *tpa;
	register unsigned char *tpb;
	signed char *pa,*pae;
	//signed char *pae2;
	int *pm;
	//int col;
	int nx;

	//wtk_debug("a=%d/%d b=%d/%d/%d\n",a->row,a->col,b->row,b->col,b->col_bytes);
	//exit(0);
	//col=(a->col>>3)<<3;
	//exit(0);
	for(pm=m->p,pa=a->p,i=0;i<a->row;++i,pa=pae)
	{
		pae=pa+a->col;
		//pae2=pa+col;
		//wtk_debug("i=%d\n",i);
		for(j=0,nx=0;j<b->row;++j,nx+=b->col_bytes)
		{
			tpb=b->p+nx;
			//wtk_debug("j=%d\n",j);
			t=0;
			tpa=pa;
//			while(tpa<pae2)
//			{
//				t+=(tpa[0])*(tpb[0]+min);
//				t+=(tpa[1])*(tpb[1]+min);
//				t+=(tpa[2])*(tpb[2]+min);
//				t+=(tpa[3])*(tpb[3]+min);
//				t+=(tpa[4])*(tpb[4]+min);
//				t+=(tpa[5])*(tpb[5]+min);
//				t+=(tpa[6])*(tpb[6]+min);
//				t+=(tpa[7])*(tpb[7]+min);
//				tpa+=8;
//				tpb+=8;
//			}

			//wtk_debug("tpa=%p/%p/%p tpb=%p\n",tpa,pae2,pae,tpb);
			//wtk_debug("row=%d/%d a=%d\n",a->col,b->row,a->row);
			//exit(0);
			//wtk_debug("tpa=%p/%p/%p dt=%d min=%d\n",tpa,pae,pae2,(int)(pae-tpa),min);
			while(tpa<pae)
			{
				t+=(*(tpa++))*(*(tpb++)+min);
			}
//			if(tpb>(b->p+b->col*b->row))
//			{
//				wtk_debug("found bug\n");
//				exit(0);
//			}
			//wtk_debug("%d/%f/%f %p\n",t,scale,t*scale,pae2);
			//exit(0);
			*(pm++)=t*scale;
		}
	}
}


void wtk_mati_multi_cu_x(wtk_mati_t *m,wtk_matc_t *a,wtk_matuc_t *b,int min)
{
	int i,j;
	register int t;
	register signed char *tpa;
	register unsigned char *tpb;
	signed char *pa,*pae2,*pae;
	int *pm;
	int col;
	int nx;

	//wtk_debug("a=%d/%d b=%d/%d/%d\n",a->row,a->col,b->row,b->col,b->col_bytes);
	//exit(0);
	col=(a->col>>3)<<3;
	//exit(0);
	for(pm=m->p,pa=a->p,i=0;i<a->row;++i,pa=pae)
	{
		pae=pa+a->col;
		pae2=pa+col;
		//wtk_debug("i=%d\n",i);
		for(j=0,nx=0;j<b->row;++j,nx+=b->col_bytes)
		{
			tpb=b->p+nx;
			//wtk_debug("j=%d\n",j);
			t=0;
			tpa=pa;
			while(tpa<pae2)
			{
				t+=(tpa[0])*(tpb[0]+min);
				t+=(tpa[1])*(tpb[1]+min);
				t+=(tpa[2])*(tpb[2]+min);
				t+=(tpa[3])*(tpb[3]+min);
				t+=(tpa[4])*(tpb[4]+min);
				t+=(tpa[5])*(tpb[5]+min);
				t+=(tpa[6])*(tpb[6]+min);
				t+=(tpa[7])*(tpb[7]+min);
				tpa+=8;
				tpb+=8;
			}
			//wtk_debug("tpa=%p/%p/%p tpb=%p\n",tpa,pae2,pae,tpb);
			//wtk_debug("row=%d/%d a=%d\n",a->col,b->row,a->row);
			//exit(0);
			//wtk_debug("tpa=%p/%p/%p\n",tpa,pae,pae2);
			while(tpa<pae)
			{
				t+=(*(tpa++))*(*(tpb++)+min);
			}
//			if(tpb>(b->p+b->col*b->row))
//			{
//				wtk_debug("found bug\n");
//				exit(0);
//			}
			*(pm++)=t;
		}
	}
}



void wtk_mati_multi_cc_x(wtk_mati_t *m,wtk_matc_t *a,wtk_matc_t *b)
{
	int i,j;
	register int t;
	register signed char *tpa,*tpb;
	signed char *pa,*pae2,*pae;
	int *pm;
	int col;
	int nx;

	wtk_debug("a=%d/%d b=%d/%d\n",a->row,a->col,b->row,b->col);
	//exit(0);
	col=(a->col>>3)<<3;
	//exit(0);
	for(pm=m->p,pa=a->p,i=0;i<a->row;++i,pa=pae)
	{
		pae=pa+a->col;
		pae2=pa+col;
		//wtk_debug("i=%d\n",i);
		for(j=0,nx=0;j<b->col;++j,nx+=b->col_bytes)
		{
			tpb=b->p+nx;
			//wtk_debug("j=%d\n",j);
			t=0;
			tpa=pa;
			while(tpa<pae2)
			{
				t+=(tpa[0])*(tpb[0]);
				t+=(tpa[1])*(tpb[1]);
				t+=(tpa[2])*(tpb[2]);
				t+=(tpa[3])*(tpb[3]);
				t+=(tpa[4])*(tpb[4]);
				t+=(tpa[5])*(tpb[5]);
				t+=(tpa[6])*(tpb[6]);
				t+=(tpa[7])*(tpb[7]);
				tpa+=8;
				tpb+=8;
			}
			//wtk_debug("tpa=%p/%p/%p tpb=%p\n",tpa,pae2,pae,tpb);
			//wtk_debug("row=%d/%d a=%d\n",a->col,b->row,a->row);
			//exit(0);
			//wtk_debug("tpa=%p/%p/%p\n",tpa,pae,pae2);
			while(tpa<pae)
			{
				t+=(*(tpa++))*(*(tpb++));
			}
//			if(tpb>(b->p+b->col*b->row))
//			{
//				wtk_debug("found bug\n");
//				exit(0);
//			}
			*(pm++)=t;
		}
	}
}

void wtk_mati_multi_cc_x_scale(wtk_mati_t *m,wtk_matc_t *a,wtk_matc_t *b,float scale)
{
	int i,j;
	register int t;
	register signed char *tpa,*tpb;
	signed char *pa,*pae2,*pae;
	int *pm;
	int col;
	int nx;

	//wtk_debug("a=%d/%d b=%d/%d\n",a->row,a->col,b->row,b->col);
	col=(a->col>>3)<<3;
	//exit(0);
	for(pm=m->p,pa=a->p,i=0;i<a->row;++i,pa=pae)
	{
		pae=pa+a->col;
		pae2=pa+col;
		//wtk_debug("i=%d\n",i);
		for(j=0,nx=0;j<b->col;++j,nx+=b->col_bytes)
		{
			tpb=b->p+nx;
			//wtk_debug("j=%d\n",j);
			t=0;
			tpa=pa;
			while(tpa<pae2)
			{
				t+=(tpa[0])*(tpb[0]);
				t+=(tpa[1])*(tpb[1]);
				t+=(tpa[2])*(tpb[2]);
				t+=(tpa[3])*(tpb[3]);
				t+=(tpa[4])*(tpb[4]);
				t+=(tpa[5])*(tpb[5]);
				t+=(tpa[6])*(tpb[6]);
				t+=(tpa[7])*(tpb[7]);
				tpa+=8;
				tpb+=8;
			}
			//wtk_debug("tpa=%p/%p/%p tpb=%p\n",tpa,pae2,pae,tpb);
			//wtk_debug("row=%d/%d a=%d\n",a->col,b->row,a->row);
			//exit(0);
			//wtk_debug("tpa=%p/%p/%p\n",tpa,pae,pae2);
			while(tpa<pae)
			{
				t+=(*(tpa++))*(*(tpb++));
			}
//			if(tpb>(b->p+b->col*b->row))
//			{
//				wtk_debug("found bug\n");
//				exit(0);
//			}
			//wtk_debug("t=%f\n",t*scale);
			//exit(0);
			*(pm++)=t*scale;
		}
	}
}

void wtk_mati_multi_uc_x(wtk_mati_t *m,wtk_matuc_t *a,wtk_matc_t *b)
{
	int i,j;
	register int t;
	signed char *tpb;
	unsigned char *tpa,*pa,*pae2,*pae;
	int *pm;
	int col;
	int nx;
	//int xt;

	//wtk_debug("a=%d/%d b=%d/%d\n",a->row,a->col,b->row,b->col);
	//exit(0);
	col=(a->col>>3)<<3;
	//exit(0);
	for(pm=m->p,pa=a->p,i=0;i<a->row;++i,pa=pae)
	{
		pae=pa+a->col;
		pae2=pa+col;
		//wtk_debug("i=%d\n",i);
		for(j=0,nx=0;j<b->row;++j,nx+=b->col_bytes)
		{
			tpb=b->p+nx;
			//wtk_debug("tpb=%p nx=%d\n",tpb,nx);
			//wtk_debug("j=%d\n",j);
			t=0;
			tpa=pa;
			while(tpa<pae2)
			{
				t+=(tpa[0])*(tpb[0]);
				t+=(tpa[1])*(tpb[1]);
				t+=(tpa[2])*(tpb[2]);
				t+=(tpa[3])*(tpb[3]);
				t+=(tpa[4])*(tpb[4]);
				t+=(tpa[5])*(tpb[5]);
				t+=(tpa[6])*(tpb[6]);
				t+=(tpa[7])*(tpb[7]);
				tpa+=8;
				tpb+=8;
			}
			//wtk_debug("tpa=%p/%p/%p tpb=%p\n",tpa,pae2,pae,tpb);
			//wtk_debug("row=%d/%d a=%d\n",a->col,b->row,a->row);
			//exit(0);
			//wtk_debug("tpa=%p/%p/%p\n",tpa,pae,pae2);
			//xt=0;
			while(tpa<pae)
			{
				//wtk_debug("v[%d]=%d/%d\n",++xt,*tpa,*tpb);
				t+=(*(tpa++))*(*(tpb++));
			}
			*(pm++)=t;
		}
	}
}

void wtk_mati_multi_uc_x_scale(wtk_mati_t *m,wtk_matuc_t *a,wtk_matc_t *b,double scale,double min)
{
	int i,j;
	register int t;
	signed char *tpb;
	unsigned char *tpa,*pa,*pae2,*pae;
	int *pm;
	int col;
	int nx;
	//int vx;
	int xmin;

	xmin=min;
	//wtk_debug("a=%d/%d b=%d/%d\n",a->row,a->col,b->row,b->col);
	col=(a->col>>3)<<3;
	//exit(0);
	for(pm=m->p,pa=a->p,i=0;i<a->row;++i,pa=pae)
	{
		pae=pa+a->col;
		pae2=pa+col;
		//wtk_debug("i=%d\n",i);
		//a*scale+min
		for(j=0,nx=0;j<b->col;++j,nx+=b->col_bytes)
		{
			tpb=b->p+nx;
			//wtk_debug("j=%d\n",j);
			t=0;
			tpa=pa;
			//vx=0;
			while(tpa<pae2)
			{
				t+=(tpa[0]+xmin)*(tpb[0]);
				t+=(tpa[1]+xmin)*(tpb[1]);
				t+=(tpa[2]+xmin)*(tpb[2]);
				t+=(tpa[3]+xmin)*(tpb[3]);
				t+=(tpa[4]+xmin)*(tpb[4]);
				t+=(tpa[5]+xmin)*(tpb[5]);
				t+=(tpa[6]+xmin)*(tpb[6]);
				t+=(tpa[7]+xmin)*(tpb[7]);
				//vx+=tpb[0]+tpb[1]+tpb[2]+tpb[3]+tpb[4]+tpb[5]+tpb[6]+tpb[7];
				tpa+=8;
				tpb+=8;
			}
			//wtk_debug("tpa=%p/%p/%p tpb=%p\n",tpa,pae2,pae,tpb);
			//wtk_debug("row=%d/%d a=%d\n",a->col,b->row,a->row);
			//exit(0);
			//wtk_debug("tpa=%p/%p/%p\n",tpa,pae,pae2);
			while(tpa<pae)
			{
				//vx+=*tpb;
				t+=(*(tpa++)+xmin)*(*(tpb++));
			}
//			if(tpb>(b->p+b->col*b->row))
//			{
//				wtk_debug("found bug\n");
//				exit(0);
//			}
			//wtk_debug("t=%f\n",t*scale+min*vx);
			//exit(0);
			//wtk_debug("t=%f scale=%f vx=%d min=%f\n",t*scale,scale,vx,min);
			//wtk_debug("t=%d min=%f vx=%d scale=%f\n",t,min,vx,scale);
			//t=(t+min*vx)*scale;
			t=t*scale;
			//wtk_debug("t=%d\n",t);
			//exit(0);
			//exit(0);
			*(pm++)=t;
		}
	}
}

void wtk_mati_multi_uc_x_scale2(wtk_mati_t *m,wtk_matuc_t *a,wtk_matc_t *b,double scale,double min)
{
	int i,j;
	register int t;
	signed char *tpb;
	unsigned char *tpa,*pa,*pae2,*pae;
	int *pm;
	int col;
	int nx;

	//wtk_debug("a=%d/%d b=%d/%d\n",a->row,a->col,b->row,b->col);
	col=(a->col>>3)<<3;
	//exit(0);
	for(pm=m->p,pa=a->p,i=0;i<a->row;++i,pa=pae)
	{
		pae=pa+a->col;
		pae2=pa+col;
		//wtk_debug("i=%d\n",i);
		//a*scale+min
		for(j=0,nx=0;j<b->col;++j,nx+=b->col_bytes)
		{
			tpb=b->p+nx;
			//wtk_debug("j=%d\n",j);
			t=0;
			tpa=pa;
			while(tpa<pae2)
			{
				t+=(tpa[0]+min)*(tpb[0]);
				t+=(tpa[1]+min)*(tpb[1]);
				t+=(tpa[2]+min)*(tpb[2]);
				t+=(tpa[3]+min)*(tpb[3]);
				t+=(tpa[4]+min)*(tpb[4]);
				t+=(tpa[5]+min)*(tpb[5]);
				t+=(tpa[6]+min)*(tpb[6]);
				t+=(tpa[7]+min)*(tpb[7]);
				tpa+=8;
				tpb+=8;
			}
			//wtk_debug("tpa=%p/%p/%p tpb=%p\n",tpa,pae2,pae,tpb);
			//wtk_debug("row=%d/%d a=%d\n",a->col,b->row,a->row);
			//exit(0);
			//wtk_debug("tpa=%p/%p/%p\n",tpa,pae,pae2);
			while(tpa<pae)
			{
				t+=(*(tpa++)+min)*(*(tpb++));
			}
			*(pm++)=t*scale;
		}
	}
}


void wtk_mati_multi_ic_x(wtk_mati_t *m,wtk_mati_t *a,wtk_matc_t *b)
{
	int i,j;
	register int t;
	signed char *tpb;
	int *tpa,*pa,*pae2,*pae;
	int *pm;
	int col;
	int nx;

	//wtk_debug("a=%d/%d b=%d/%d\n",a->row,a->col,b->row,b->col);
	col=(a->col>>3)<<3;
	//exit(0);
	for(pm=m->p,pa=a->p,i=0;i<a->row;++i,pa=pae)
	{
		pae=pa+a->col;
		pae2=pa+col;
		//wtk_debug("i=%d\n",i);
		for(j=0,nx=0;j<b->row;++j,nx+=b->col_bytes)
		{
			tpb=b->p+nx;
			//wtk_debug("j=%d\n",j);
			t=0;
			tpa=pa;
			while(tpa<pae2)
			{
				t+=(tpa[0])*(tpb[0]);
				t+=(tpa[1])*(tpb[1]);
				t+=(tpa[2])*(tpb[2]);
				t+=(tpa[3])*(tpb[3]);
				t+=(tpa[4])*(tpb[4]);
				t+=(tpa[5])*(tpb[5]);
				t+=(tpa[6])*(tpb[6]);
				t+=(tpa[7])*(tpb[7]);
				tpa+=8;
				tpb+=8;
			}
			//wtk_debug("tpa=%p/%p/%p tpb=%p\n",tpa,pae2,pae,tpb);
			//wtk_debug("row=%d/%d a=%d\n",a->col,b->row,a->row);
			//exit(0);
			//wtk_debug("tpa=%p/%p/%p\n",tpa,pae,pae2);
			while(tpa<pae)
			{
				t+=(*(tpa++))*(*(tpb++));
			}
//			if(tpb>(b->p+b->col*b->row))
//			{
//				wtk_debug("found bug\n");
//				exit(0);
//			}
			*(pm++)=t;
		}
	}
}




void wtk_mati_multi2_x(wtk_mati_t *m,wtk_mati_t *a,wtk_matc_t *b)
{
	int *pa;
	register signed char *pb,*epb;
	register int pak;
	register int *pm;
	int *tpm;
	int i,j;

	for(tpm=m->p,pa=a->p,i=0;i<a->row;++i,tpm+=m->col)
	{
		for(pb=b->p,j=0;j<a->col;++j)
		{
			pak=*(pa++);//pa[j];
			epb=pb+b->col;
			pm=tpm;
			if(j==0)
			{
				while(epb-pb>=4)
				{
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
				}
				while(epb>pb)
				{
					*(pm++)=pak*(*(pb++));
				}
			}else
			{
				while(epb-pb>=4)
				{
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
				}
				while(epb>pb)
				{
					*(pm++)+=pak*(*(pb++));
				}
			}
		}
	}
}

void wtk_mati_add(wtk_mati_t *a,wtk_mati_t *b)
{
	register int *pa,*epa,*pb;

	pa=a->p;
	pb=b->p;
	epa=pa+a->row*a->col;
	while(epa-pa>=4)
	{
		*(pa++)+=*(pb++);
		*(pa++)+=*(pb++);
		*(pa++)+=*(pb++);
		*(pa++)+=*(pb++);
	}
	while(epa>pa)
	{
		*(pa++)+=*(pb++);
	}
}


void wtk_mati_multi3(wtk_mati_t *m,wtk_mati_t *a,wtk_mati_t *b)
{
	int *pa;
	register int *pb,*epb2;
	int *epb;
	register int pak;
	register int *pm;
	int *tpm;
	int i,j;
	int col2;

	col2=(b->col>>3)<<3;
	//wtk_debug("col=%d/%d\n",b->col,col2);
	for(tpm=m->p,pa=a->p,i=0;i<a->row;++i,tpm+=m->col)
	{
		for(pb=b->p,j=0;j<a->col;++j)
		{
			pak=*(pa++);//pa[j];
			epb2=pb+col2;
			epb=pb+b->col;
			pm=tpm;
			if(j==0)
			{
				while(pb<epb2)
				{
					pm[0]=pak*pb[0];
					pm[1]=pak*pb[1];
					pm[2]=pak*pb[2];
					pm[3]=pak*pb[3];
					pm[4]=pak*pb[4];
					pm[5]=pak*pb[5];
					pm[6]=pak*pb[6];
					pm[7]=pak*pb[7];
					pm+=8;
					pb+=8;
				}
				while(epb>pb)
				{
					*(pm++)=pak*(*(pb++));
				}
			}else
			{
				while(pb<epb2)
				{
					pm[0]+=pak*pb[0];
					pm[1]+=pak*pb[1];
					pm[2]+=pak*pb[2];
					pm[3]+=pak*pb[3];
					pm[4]+=pak*pb[4];
					pm[5]+=pak*pb[5];
					pm[6]+=pak*pb[6];
					pm[7]+=pak*pb[7];
					pm+=8;
					pb+=8;
				}
				while(epb>pb)
				{
					*(pm++)+=pak*(*(pb++));
				}
			}
		}
	}
}


void wtk_mati_multi3_raw(wtk_mati_t *m,wtk_mati_t *a,wtk_mati_t *b)
{
	int *pa;
	register int *pb,*epb;
	register int pak;
	register int *pm;
	int *tpm;
	int i,j;

	for(tpm=m->p,pa=a->p,i=0;i<a->row;++i,tpm+=m->col)
	{
		for(pb=b->p,j=0;j<a->col;++j)
		{
			pak=*(pa++);//pa[j];
			epb=pb+b->col;
			pm=tpm;
			if(j==0)
			{
				while(epb-pb>=4)
				{
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
				}
				while(epb>pb)
				{
					*(pm++)=pak*(*(pb++));
				}
			}else
			{
				while(epb-pb>=4)
				{
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
				}
				while(epb>pb)
				{
					*(pm++)+=pak*(*(pb++));
				}
			}
		}
	}
}

typedef struct
{
	int t1;
	int t2;
	int t3;
	int t4;
}wtk_mati_v_t;

void wtk_mati_multi4(wtk_mati_t *m,wtk_matc_t *a,wtk_matc_t *b)
{
	signed char *pa;
	register signed char *pb,*epb;
	register char pak;
	register int *pm;
	register int t1,t2;
	int *tpm;
	int i,j;
	//int t[4];
	wtk_mati_v_t t;

	for(tpm=m->p-1,pa=a->p,i=0;i<a->row;++i,tpm+=m->col)
	{
		for(pb=b->p,j=0;j<a->col;++j)
		{
			pak=*(pa++);
			epb=pb+b->col;
			pm=tpm;
			if(j==0)
			{
				while(epb-pb>=4)
				{
					t1=pak*((*(pb)<<16)+pb[1]);
					pb+=2;
					t2=pak*((*(pb)<<16)+pb[1]);
					pb+=2;

					t.t1=(t1>>16)&0x00FFFF;
					t.t2=t1&0x00FFFF;
					t.t3=(t2>>16)&0x00FFFF;
					t.t4=t2&0x00FFFF;
					//memcpy(pm,t,16);

					*((wtk_mati_v_t*)pm)=t;
					pm+=4;
					//*(++pm)=t1&0x00FFFF;
					/*
					*(++pm)=(t1>>16)&0x00FFFF;
					*(++pm)=t1&0x00FFFF;

					*(++pm)=(t2>>16)&0x00FFFF;
					*(++pm)=t2&0x00FFFF;
					*(++pm)=t2&0x00FFFF;
					*/
				}
				while(epb>pb)
				{
					*(++pm)=pak*(*(pb++));
				}
			}else
			{
				while(epb-pb>=4)
				{
					t1=pak*((*(pb)<<16)+pb[1]);
					pb+=2;
					t2=pak*((*(pb)<<16)+pb[1]);
					pb+=2;

					t=*((wtk_mati_v_t*)pm);
					t.t1+=(t1>>16)&0x00FFFF;
					t.t2+=t1&0x00FFFF;
					t.t3+=(t2>>16)&0x00FFFF;
					t.t4+=t2&0x00FFFF;
					//memcpy(pm,t,16);
					*((wtk_mati_v_t*)pm)=t;
					/*
					if((((long)pm)%16)!=0)
					{
						wtk_debug("pm=%p\n",pm);
						exit(0);
					}*/
					pm+=4;
					//*(++pm)+=t1&0x00FFFF;
					/*
					*(++pm)+=(t1>>16)&0x00FFFF;
					*(++pm)+=t1&0x00FFFF;

					*(++pm)+=(t2>>16)&0x00FFFF;
					*(++pm)+=t2&0x00FFFF;*/
				}
				while(epb>pb)
				{
					*(++pm)+=pak*(*(pb++));
				}
			}
		}
	}
}

void wtk_mati_multi4_4(wtk_mati_t *m,wtk_matc_t *a,wtk_matc_t *b)
{
	signed char *pa;
	register signed char *pb,*epb;
	register char pak;
	register int *pm;
	register int64_t t;
	int *tpm;
	int i,j;

	for(tpm=m->p-1,pa=a->p,i=0;i<a->row;++i,tpm+=m->col)
	{
		for(pb=b->p,j=0;j<a->col;++j)
		{
			pak=*(pa++);
			epb=pb+b->col;
			pm=tpm;
			if(j==0)
			{
				while(epb-pb>=4)
				{
					t=pak*(((int64_t)pb[0]<<48)+((int64_t)pb[1]<<32)+(pb[2]<<16)+pb[3]);
					pb+=4;
					*(++pm)=t>>48;
					*(++pm)=(t>>32)&0x00FFFF;
					*(++pm)=(t>>16)&0x00FFFF;
					*(++pm)=t&0x00FFFF;
				}
				while(epb>pb)
				{
					*(++pm)=pak*(*(pb++));
				}
			}else
			{
				while(epb-pb>=4)
				{
					t=pak*(((int64_t)pb[0]<<48)+((int64_t)pb[1]<<32)+(pb[2]<<16)+pb[3]);
					pb+=4;
					*(++pm)=t>>48;
					*(++pm)=(t>>32)&0x00FFFF;
					*(++pm)=(t>>16)&0x00FFFF;
					*(++pm)=t&0x00FFFF;
				}
				while(epb>pb)
				{
					*(++pm)+=pak*(*(pb++));
				}
			}
		}
	}
}


void wtk_mati_multi4_3(wtk_mati_t *m,wtk_matc_t *a,wtk_matc_t *b)
{
	signed char *pa;
	register signed char *pb,*epb;
	register char pak;
	register int *pm;
	register int64_t t1,t2,t3,t4;
	int *tpm;
	int i,j;

	for(tpm=m->p-1,pa=a->p,i=0;i<a->row;++i,tpm+=m->col)
	{
		for(pb=b->p,j=0;j<a->col;++j)
		{
			pak=*(pa++);
			epb=pb+b->col;
			pm=tpm;
			if(j==0)
			{
				while(epb-pb>=16)
				{
					/*
					*(++pm)=pak*(*(++pb));
					*(++pm)=pak*(*(++pb));
					*(++pm)=pak*(*(++pb));
					*(++pm)=pak*(*(++pb));
					*/
					/*
					t=pak*(((int64_t)pb[0]<<48)+((int64_t)pb[1]<<32)+(pb[2]<<16)+pb[3]);
					*(++pm)=t>>48;
					*(++pm)=(t>>32)&0x00FFFF;
					*(++pm)=(t>>16)&0x00FFFF;
					*(++pm)=t&0x00FFFF;
					pb+=4;
					*/
					t1=pak*(((int64_t)pb[0]<<48)+((int64_t)pb[1]<<32)+(pb[2]<<16)+pb[3]);
					pb+=4;
					t2=pak*(((int64_t)pb[0]<<48)+((int64_t)pb[1]<<32)+(pb[2]<<16)+pb[3]);
					pb+=4;
					t3=pak*(((int64_t)pb[0]<<48)+((int64_t)pb[1]<<32)+(pb[2]<<16)+pb[3]);
					pb+=4;
					t4=pak*(((int64_t)pb[0]<<48)+((int64_t)pb[1]<<32)+(pb[2]<<16)+pb[3]);
					pb+=4;

					*(++pm)=t1>>48;
					*(++pm)=(t1>>32)&0x00FFFF;
					*(++pm)=(t1>>16)&0x00FFFF;
					*(++pm)=t1&0x00FFFF;

					*(++pm)=t2>>48;
					*(++pm)=(t2>>32)&0x00FFFF;
					*(++pm)=(t2>>16)&0x00FFFF;
					*(++pm)=t2&0x00FFFF;

					*(++pm)=t3>>48;
					*(++pm)=(t3>>32)&0x00FFFF;
					*(++pm)=(t3>>16)&0x00FFFF;
					*(++pm)=t3&0x00FFFF;

					*(++pm)=t4>>48;
					*(++pm)=(t4>>32)&0x00FFFF;
					*(++pm)=(t4>>16)&0x00FFFF;
					*(++pm)=t4&0x00FFFF;
					pb+=16;
				}
				while(epb>pb)
				{
					*(++pm)=pak*(*(pb++));
				}
			}else
			{
				while(epb-pb>=16)
				{
					/*
					*(++pm)+=pak*(*(++pb));
					*(++pm)+=pak*(*(++pb));
					*(++pm)+=pak*(*(++pb));
					*(++pm)+=pak*(*(++pb));
					*/
					/*
					t=pak*(((int64_t)pb[0]<<48)+((int64_t)pb[1]<<32)+(pb[2]<<16)+pb[3]);
					*(++pm)+=t>>48;
					*(++pm)+=(t>>32)&0x00FFFF;
					*(++pm)+=(t>>16)&0x00FFFF;
					*(++pm)+=t&0x00FFFF;
					pb+=4;
					*/

					t1=pak*(((int64_t)pb[0]<<48)+((int64_t)pb[1]<<32)+(pb[2]<<16)+pb[3]);
					pb+=4;
					t2=pak*(((int64_t)pb[0]<<48)+((int64_t)pb[1]<<32)+(pb[2]<<16)+pb[3]);
					pb+=4;
					t3=pak*(((int64_t)pb[0]<<48)+((int64_t)pb[1]<<32)+(pb[2]<<16)+pb[3]);
					pb+=4;
					t4=pak*(((int64_t)pb[0]<<48)+((int64_t)pb[1]<<32)+(pb[2]<<16)+pb[3]);
					pb+=4;
					/*
					t2=pak*(((int64_t)pb[4]<<48)+((int64_t)pb[5]<<32)+(pb[6]<<16)+pb[7]);
					t3=pak*(((int64_t)pb[8]<<48)+((int64_t)pb[9]<<32)+(pb[10]<<16)+pb[11]);
					t4=pak*(((int64_t)pb[12]<<48)+((int64_t)pb[13]<<32)+(pb[14]<<16)+pb[15]);
					*/

					*(++pm)=t1>>48;
					*(++pm)=(t1>>32)&0x00FFFF;
					*(++pm)=(t1>>16)&0x00FFFF;
					*(++pm)=t1&0x00FFFF;

					*(++pm)=t2>>48;
					*(++pm)=(t2>>32)&0x00FFFF;
					*(++pm)=(t2>>16)&0x00FFFF;
					*(++pm)=t2&0x00FFFF;

					*(++pm)=t3>>48;
					*(++pm)=(t3>>32)&0x00FFFF;
					*(++pm)=(t3>>16)&0x00FFFF;
					*(++pm)=t3&0x00FFFF;

					*(++pm)=t4>>48;
					*(++pm)=(t4>>32)&0x00FFFF;
					*(++pm)=(t4>>16)&0x00FFFF;
					*(++pm)=t4&0x00FFFF;
				}
				while(epb>pb)
				{
					*(++pm)+=pak*(*(pb++));
				}
			}
		}
	}
}


void wtk_mati_multi4_2(wtk_mati_t *m,wtk_matc_t *a,wtk_matc_t *b)
{
	signed char *pa;
	register signed char *pb,*epb;
	register char pak;
	register int *pm;
	int *tpm;
	int i,j;

	for(tpm=m->p,pa=a->p,i=0;i<a->row;++i,tpm+=m->col)
	{
		for(pb=b->p,j=0;j<a->col;++j)
		{
			pak=*(pa++);
			epb=pb+b->col;
			pm=tpm;
			if(j==0)
			{
				while(epb-pb>=4)
				{
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
				}
				while(epb>pb)
				{
					*(pm++)=pak*(*(pb++));
				}
			}else
			{
				while(epb-pb>=4)
				{
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
				}
				while(epb>pb)
				{
					*(pm++)+=pak*(*(pb++));
				}
			}
		}
	}
}


//#include <xmmintrin.h>

/*
static void* wtk_memcpy(void* dest, const void* src, size_t bytes)
{
    size_t dwords = bytes/4;
    bytes %= 4;
    if(dwords>0)
    {
    	__asm__ volatile("cld\n" "rep movsl" : : "S" (src), "D" (dest), "c" (dwords));
    }
    if(bytes>0)
    {
    	__asm__ volatile(        "rep movsb" : : "c" (bytes));
    }
    return(dest);
}*/

/*
void wtk_int_matrix_multi(wtk_int_matrix_t *m,wtk_matc_t *a,wtk_matc_t *b)
{
typedef struct
{
	int t1;
	int t2;
	int t3;
	int t4;

	int t5;
	int t6;
	int t7;
	int t8;
}wtk_mati_vx_t;
	char *pa;
	register char *pb,*epb;
	register char pak;
	register int *pm;
	int t1,t2,t3,t4;
	int *tpm;
	int i,j;
	//int t[4];
	//wtk_mati_vx_t t;

	//register __m128i mi;

	for(pa=a->p,i=0;i<a->row;++i)
	{
		tpm=m[i+1]+1;
		for(pb=b->p,j=0;j<a->col;++j)
		{
			pak=*(pa++);
			epb=pb+b->col;
			pm=tpm;
			if(j==0)
			{
				while(epb-pb>=8)
				{
					t1=pak*((*(pb)<<16)+pb[1]);
					t2=pak*((pb[2]<<16)+pb[3]);
					t3=pak*((pb[4]<<16)+pb[5]);
					t4=pak*((pb[6]<<16)+pb[7]);
					pb+=8;

					*(pm++)=(t1>>16)&0x00FFFF;
					*(pm++)=t1&0x00FFFF;
					*(pm++)=(t2>>16)&0x00FFFF;
					*(pm++)=t2&0x00FFFF;

					*(pm++)=(t3>>16)&0x00FFFF;
					*(pm++)=t3&0x00FFFF;
					*(pm++)=(t4>>16)&0x00FFFF;
					*(pm++)=t4&0x00FFFF;
				}
				while(epb>pb)
				{
					*(pm++)=pak*(*(pb++));
				}
			}else
			{
				while(epb-pb>=8)
				{
					t1=pak*((*(pb)<<16)+pb[1]);
					t2=pak*((pb[2]<<16)+pb[3]);
					t3=pak*((pb[4]<<16)+pb[5]);
					t4=pak*((pb[6]<<16)+pb[7]);
					pb+=8;

					*(pm++)+=(t1>>16)&0x00FFFF;
					*(pm++)+=t1&0x00FFFF;
					*(pm++)+=(t2>>16)&0x00FFFF;
					*(pm++)+=t2&0x00FFFF;

					*(pm++)+=(t3>>16)&0x00FFFF;
					*(pm++)+=t3&0x00FFFF;
					*(pm++)+=(t4>>16)&0x00FFFF;
					*(pm++)+=t4&0x00FFFF;
				}
				while(epb>pb)
				{
					*(pm++)+=pak*(*(pb++));
				}
			}
		}
	}
}
*/


void wtk_int_matrix_multi(wtk_int_matrix_t *m,wtk_matc_t *a,wtk_matc_t *b)
{
//typedef struct
//{
//	int t1;
//	int t2;
//	int t3;
//	int t4;
//
//	int t5;
//	int t6;
//	int t7;
//	int t8;
//}wtk_mati_vx_t;
	signed char *pa;
	register signed char *pb,*epb;
	register int pak;
	register int *pm;
	//int t1,t2,t3,t4;
	int *tpm;
	int i,j;
	//int t[4];

	//register __m128i mi;

	for(pa=a->p,i=0;i<a->row;++i)
	{
		tpm=m[i+1]+1;
		for(pb=b->p,j=0;j<a->col;++j)
		{
			pak=*(pa++);
			epb=pb+b->col;
			pm=tpm;
			if(j==0)
			{
				while(epb-pb>=16)
				{
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));

					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));


					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));

					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
				}
				while(epb-pb>=8)
				{
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));

					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
				}
				while(epb>pb)
				{
					*(pm++)=pak*(*(pb++));
				}
			}else
			{
				while(epb-pb>=16)
				{
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));

					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));

					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));

					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
				}
				while(epb-pb>=8)
				{
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));

					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
				}
				while(epb>pb)
				{
					*(pm++)+=pak*(*(pb++));
				}
			}
		}
	}
}

void wtk_int_matrix_multi5(wtk_int_matrix_t *m,wtk_matc_t *a,wtk_matc_t *b)
{
	signed char *pa;
	register signed char *pb,*epb;
	register char pak;
	register int *pm;
	//register int t1,t2;
	register int64_t t1;
	int *tpm;
	int i,j;
	//int t[4];
	wtk_mati_v_t t;

	for(pa=a->p,i=0;i<a->row;++i)
	{
		tpm=m[i+1]+1;
		for(pb=b->p,j=0;j<a->col;++j)
		{
			pak=*(pa++);
			epb=pb+b->col;
			pm=tpm;
			if(j==0)
			{
				while(epb-pb>=4)
				{
					t1=pak*(((int64_t)pb[0]<<48)+((int64_t)pb[1]<<32)+(pb[2]<<16)+pb[3]);
					pb+=4;
					t.t1=(t1>>48)&0x00FFFF;
					t.t2=(t1>>32)&0x00FFFF;
					t.t3=(t1>>16)&0x00FFFF;
					t.t4=t1&0x00FFFF;
					*((wtk_mati_v_t*)pm)=t;
					pm+=4;
				}
				while(epb>pb)
				{
					*(pm++)=pak*(*(pb++));
				}
			}else
			{
				while(epb-pb>=4)
				{
					t1=pak*(((int64_t)pb[0]<<48)+((int64_t)pb[1]<<32)+(pb[2]<<16)+pb[3]);
					pb+=4;

					t=*((wtk_mati_v_t*)pm);
					t.t1+=(t1>>48)&0x00FFFF;
					t.t2+=(t1>>32)&0x00FFFF;
					t.t3+=(t1>>16)&0x00FFFF;
					t.t4+=t1&0x00FFFF;
					//memcpy(pm,t,16);
					*((wtk_mati_v_t*)pm)=t;

					/*
					if((((long)pm)%16)!=0)
					{
						wtk_debug("pm=%p\n",pm);
						exit(0);
					}*/
					pm+=4;
				}
				while(epb>pb)
				{
					*(pm++)+=pak*(*(pb++));
				}
			}
		}
	}
}

#ifdef DEBUG_XXX
#include <xmmintrin.h>

void wtk_mati_multi2x(wtk_mati_t *m,wtk_mati_t *a,wtk_matc_t *p)
{
	int *pa;
	register char *pb;
	register int pak;
	register int *pm,*epm;
	int *tpm;
	int i,j;
	int nx;
	__m128i ax;//,bx;//,cx;// b;

	nx=p->col<<2;
	for(tpm=m->p,pa=a->p,i=0;i<a->row;++i,tpm+=m->col)
	{
		for(pb=p->p,j=0;j<a->col;++j)
		{
			pak=*(pa++);//pa[j];
			pm=tpm;
			switch(pak)
			{
			case 0:
				if(j==0)
				{
					memset(pm,0,nx);
				}
				pb+=p->col;
				break;
			case 1:
				if(j==0)
				{
					memcpy(pm,pb,nx);
					pb+=p->col;
				}else
				{
					epm=pm+p->col;
					while(epm-pm>=4)
					{
						*(pm++)+=(*(pb++));
						*(pm++)+=(*(pb++));
						*(pm++)+=(*(pb++));
						*(pm++)+=(*(pb++));
					}
					while(epm>pm)
					{
						*(pm++)+=(*(pb++));
					}
				}
				break;
			default:
				epm=pm+p->col;
				if(j==0)
				{
					while(((((long)pm)%16)!=0) && (epm>pm))
					{
						*(pm++)=pak*(*(pb++));
					}
					if(epm-pm>=4)
					{
						ax=_mm_set_epi32(pak,pak,pak,pak);
						//while(epm-pm>=4)
						do
						{
							*((__m128i*)pm)=_mm_mulhi_epi16(ax,_mm_set_epi32(pb[0],pb[1],pb[2],pb[3]));
							pb+=4;
							pm+=4;
						}while(epm-pm>=4);
					}
					while(epm>pm)
					{
						*(pm++)=pak*(*(pb++));
					}
				}else
				{
					while(((((long)pm)%16)!=0) && (epm>pm))
					{
						*(pm++)+=pak*(*(pb++));
					}
					if(epm-pm>=4)
					{
						ax=_mm_set_epi32(pak,pak,pak,pak);
						//while(epm-pm>=4)
						do
						{
							*((__m128i*)pm)= _mm_add_epi16(*((__m128i*)pm),_mm_mulhi_epi16(ax,_mm_set_epi32(pb[0],pb[1],pb[2],pb[3])));
							//bx=_mm_mulhi_epi16(ax,_mm_set_epi32(pb[0],pb[1],pb[2],pb[3]));
							//*((__m128i*)pm)= _mm_add_epi16(*((__m128i*)pm),bx);
							pb+=4;
							pm+=4;
						}while(epm-pm>=4);
					}
					while(epm>pm)
					{
						*(pm++)+=pak*(*(pb++));
					}
				}
				break;
			}
		}
	}
}
#endif


#ifdef USE_SIMD
#else
void wtk_mati_multi2(wtk_mati_t *m,wtk_mati_t *a,wtk_matc_t *b)
{
	int *pa;
	register signed char *pb;
	register int pak;
	register int *pm,*epm;
	int *tpm;
	int i,j;

	//wtk_debug("a=[%d*%d] b=[%d*%d]\n",a->row,a->col,b->row,b->col);
	for(tpm=m->p,pa=a->p,i=0;i<a->row;++i,tpm+=m->col)
	{
		for(pb=b->p,j=0;j<a->col;++j)
		{
			pak=*(pa++);//pa[j];
			//wtk_debug("pak=%d\n",pak);
			pm=tpm;
			switch(pak)
			{
			case 0:
				if(j==0)
				{
					memset(pm,0,b->col<<2);
				}
				pb+=b->col;
				break;
			case 1:
				epm=pm+b->col;
				if(j==0)
				{
					while(epm-pm>=4)
					{
						*(pm++)=(*(pb++));
						*(pm++)=(*(pb++));
						*(pm++)=(*(pb++));
						*(pm++)=(*(pb++));
					}
					while(epm>pm)
					{
						*(pm++)=(*(pb++));
					}
				}else
				{
					while(epm-pm>=4)
					{
						*(pm++)+=(*(pb++));
						*(pm++)+=(*(pb++));
						*(pm++)+=(*(pb++));
						*(pm++)+=(*(pb++));
					}
					while(epm>pm)
					{
						*(pm++)+=(*(pb++));
					}
				}
				break;
			default:
				epm=pm+b->col;
				if(j==0)
				{
					//pb=wtk_mati_mx(pm,epm,pb,pak);
					while(epm-pm>=4)
					{
						*(pm++)=pak*(*(pb++));
						*(pm++)=pak*(*(pb++));
						*(pm++)=pak*(*(pb++));
						*(pm++)=pak*(*(pb++));
					}
					while(epm>pm)
					{
						*(pm++)=pak*(*(pb++));
					}
				}else
				{
					//pb=wtk_mati_mx2(pm,epm,pb,pak);
					while(epm-pm>=4)
					{
						*(pm++)+=pak*(*(pb++));
						*(pm++)+=pak*(*(pb++));
						*(pm++)+=pak*(*(pb++));
						*(pm++)+=pak*(*(pb++));
					}
					while(epm>pm)
					{
						*(pm++)+=pak*(*(pb++));
					}
				}
				break;
			}
		}
	}
}
#endif

void wtk_mati_multi_x4(wtk_mati_t *m,wtk_mati_t *a,wtk_mati_t *b)
{
	int *pa;
	register int *pb;
	register int pak;
	register int *pm,*epm;
	int *tpm;
	int i,j;
	//static int ki=0;
	//static int ky=0;
	int nx;

	nx=b->col*sizeof(int);
	for(tpm=m->p,pa=a->p,i=0;i<a->row;++i,tpm+=m->col)
	{
		for(pb=b->p,j=0;j<a->col;++j)
		{
			pak=*(pa++);//pa[j];
			pm=tpm;
			switch(pak)
			{
			case 0:
				if(j==0)
				{
					memset(pm,0,nx);
				}
				pb+=b->col;
				break;
			case 1:
				if(j==0)
				{
					memcpy(pm,pb,nx);
					pb+=b->col;
				}else
				{
					epm=pm+b->col;
					while(epm-pm>=4)
					{
						*(pm++)+=(*(pb++));
						*(pm++)+=(*(pb++));
						*(pm++)+=(*(pb++));
						*(pm++)+=(*(pb++));
					}
					while(epm>pm)
					{
						*(pm++)+=(*(pb++));
					}
				}
				break;
			default:
				epm=pm+b->col;
				if(j==0)
				{
					while(epm-pm>=4)
					{
						*(pm++)=pak*(*(pb++));
						*(pm++)=pak*(*(pb++));
						*(pm++)=pak*(*(pb++));
						*(pm++)=pak*(*(pb++));
					}
					while(epm>pm)
					{
						*(pm++)=pak*(*(pb++));
					}
				}else
				{
					while(epm-pm>=4)
					{
						*(pm++)+=pak*(*(pb++));
						*(pm++)+=pak*(*(pb++));
						*(pm++)+=pak*(*(pb++));
						*(pm++)+=pak*(*(pb++));
					}
					while(epm>pm)
					{
						*(pm++)+=pak*(*(pb++));
					}
				}
				break;
			}
		}
	}
}


//-----------------------------

wtk_mats_t *wtk_mats_new(int row,int col)
{
	wtk_mats_t *m;
	char *p;

	p=wtk_calloc(1,sizeof(wtk_mats_t)+row*col*sizeof(short)+16);
	m=(wtk_mats_t*)p;
	m->row=row;
	m->col=col;
	m->scale=0;
	m->p=wtk_align_ptr(p+sizeof(wtk_mats_t),16);
	return m;
}

void wtk_mats_delete(wtk_mats_t *m)
{
	wtk_free(m);
}

void  wtk_mats_fix_matf(wtk_mats_t *mc,wtk_matf_t *mf,float max)
{
	float f,t;
	int i,n;
	short *pc;
	float *pf;

	n=mf->row*mf->col;
	f=wtk_float_abs_max(mf->p,n);
	f=max*1.0/f;
	mc->scale=1.0/f;
	pc=mc->p;
	pf=mf->p;
	n=mc->row*mc->col;
	for(i=0;i<n;++i)
	{
		t=pf[i]*f;
		//wtk_debug("v[%d]=%f/%f/%f\n",i,pf[i],f,t);
		pc[i]=wtk_float_round(t);
	}
	//wtk_debug("max=%f\n",max);
	//print_short2(pc,20);
	//exit(0);
}

wtk_matb_t *wtk_matb_new(int row,int col)
{
	wtk_matb_t *m;
	char *p;

	p=wtk_calloc(1,sizeof(wtk_matb_t)+row*col*sizeof(char)+16);
	m=(wtk_matb_t*)p;
	m->row=row;
	m->col=col;
	m->scale=0;
	m->p=wtk_align_ptr(p+sizeof(wtk_matb_t),16);
	return m;
}

void wtk_matb_delete(wtk_matb_t *m)
{
	wtk_free(m);
}

void  wtk_matb_fix_matf(wtk_matb_t *mc,wtk_matf_t *mf,float max)
{
	float f,t;
	int i,n;
	signed char *pc;
	float *pf;

	n=mf->row*mf->col;
	f=wtk_float_abs_max(mf->p,n);
	f=max*1.0/f;
	mc->scale=1.0/f;
	pc=mc->p;
	pf=mf->p;
	n=mc->row*mc->col;
	for(i=0;i<n;++i)
	{
		t=pf[i]*f;
		pc[i]=wtk_float_round(t);
	}
}



void wtk_matb_fix(wtk_matb_t *m,wtk_matf_t *src)
{
	wtk_range_t range;
	float f;
	int i,n;

	n=m->row*m->col;
	range=wtk_matf_range(src);
	f=range.max;
	if(range.min<0 && -range.min>f)
	{
		f=-range.min;
	}
	m->scale=127.0/f;
	wtk_debug("min=%f max=%f scale=%f\n",range.min,range.max,m->scale);
	for(i=0;i<n;++i)
	{
		f=src->p[i]*m->scale;
		m->p[i]=wtk_float_round(f);
	}
	//exit(0);
}

void wtk_matb_print(wtk_matb_t *m)
{
	int i,j;
	signed char *pf;

	pf=m->p;
	for(i=0;i<m->row;++i)
	{
		for(j=0;j<m->col;++j)
		{
			wtk_debug("v[%d][%d]=%d\n",i,j,*(pf++));
		}
	}
}

wtk_vecb_t* wtk_vecb_new(int len)
{
	wtk_vecb_t *v;
	char *p;

	p=wtk_calloc(1,sizeof(wtk_vecb_t)+len+16);
	v=(wtk_vecb_t*)p;
	v->len=len;
	v->p=wtk_align_ptr(p+sizeof(wtk_vecb_t),16);
	return v;
}

void wtk_vecb_delete(wtk_vecb_t *v)
{
	wtk_free(v);
}

wtk_vecs_t* wtk_vecs_new(int len)
{
	wtk_vecs_t *v;
	char *p;

	p=wtk_calloc(1,sizeof(wtk_vecs_t)+len*sizeof(short)+16);
	v=(wtk_vecs_t*)p;
	v->len=len;
	v->p=wtk_align_ptr(p+sizeof(wtk_vecs_t),16);
	return v;
}

wtk_vecs_t* wtk_vecs_new_qlas(int len,int c_size)
{
	wtk_vecs_t *v;
	char *p;
	int length;

	length=len*c_size;
	p=wtk_calloc(1,sizeof(wtk_vecs_t)+length*sizeof(short)+16);
	v=(wtk_vecs_t*)p;
	v->len=len*c_size;
	v->p=wtk_align_ptr(p+sizeof(wtk_vecs_t),16);
	v->c_scale=(float*)wtk_malloc(sizeof(float)*c_size);
	return v;
}

void wtk_vecs_delete(wtk_vecs_t *v)
{
	if(v->c_scale)
	{
		wtk_free(v->c_scale);
	}
	wtk_free(v);
}

void  wtk_vecs_fix_vecf_qlas(wtk_vecs_t *mc,wtk_vecf_t *mf,float max,int len,int cache_size)
{
	float f,t;
	int i,n,j,k;
	short *pc;
	float *pf;
	float *scale;

	n=len;
	scale=mc->c_scale;

	for(j=0;j<cache_size;j++)
	{
		k=j*n;
		f=wtk_float_abs_max(mf->p+k,n);
		//printf("max float: %.12f   max:%f\n",f,max);
		f=max*1.0/f;
		//printf("max float: %.12f   max:%f\n",f,max);
		*scale=1.0/f;//TODO
		//printf("max float: %.12f   max:%f\n",*scale,max);
		pc=mc->p+k;
		pf=mf->p+k;
		for(i=0;i<n;++i)
		{
			t=pf[i]*f;
			pc[i]=wtk_float_round(t);
		}
		scale++;
	}
}

void  wtk_vecs_fix_vecf(wtk_vecs_t *mc,wtk_vecf_t *mf,float max)
{
	float f,t;
	int i,n;
	short *pc;
	float *pf;

	n=mc->len;
	f=wtk_float_abs_max(mf->p,n);
	f=max*1.0/f;
	mc->scale=1.0/f;
	pc=mc->p;
	pf=mf->p;
	for(i=0;i<n;++i)
	{
		t=pf[i]*f;
		pc[i]=wtk_float_round(t);
	}
}

void  wtk_vecb_fix_vecf(wtk_vecb_t *mc,wtk_vecf_t *mf,float max)
{
	float f,t;
	int i,n;
	char *pc;
	float *pf;

	n=mc->len;
	f=wtk_float_abs_max(mf->p,n);
	f=max*1.0/f;
	mc->scale=1.0/f;
	pc=mc->p;
	pf=mf->p;
	for(i=0;i<n;++i)
	{
		t=pf[i]*f;
		pc[i]=wtk_float_round(t);
	}
}

