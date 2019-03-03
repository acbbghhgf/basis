#include "wtk_complex.h" 

int isinvalid(double f)
{
	return isnan(f) || isinf(f);
}

void wtk_complex_check(wtk_complex_t *c,int n)
{
	int i;

	for(i=0;i<n;++i)
	{
		if(isinvalid(c[i].a) || isinvalid(c[i].b))
		{
			wtk_debug("v[%d]=%f+%fj\n",i,c[i].a,c[i].b);
			exit(0);
		}
	}
}

float*** wtk_float_new_p3(int n1,int n2,int n3)
{
	float ***c;
	int i,j;

	c=(float***)wtk_calloc(n1,sizeof(float***));
	for(i=0;i<n1;++i)
	{
		c[i]=(float**)wtk_calloc(n2,sizeof(float*));
		for(j=0;j<n2;++j)
		{
			c[i][j]=(float*)wtk_calloc(n3,sizeof(float));
		}
	}
	return c;
}

double*** wtk_double_new_p3(int n1,int n2,int n3)
{
	double ***c;
	int i,j;

	c=(double***)wtk_calloc(n1,sizeof(double***));
	for(i=0;i<n1;++i)
	{
		c[i]=(double**)wtk_calloc(n2,sizeof(double*));
		for(j=0;j<n2;++j)
		{
			c[i][j]=(double*)wtk_calloc(n3,sizeof(double));
		}
	}
	return c;
}

void wtk_float_p3_set(float ***p,int n1,int n2,int n3,float v)
{
	int i,j,k;
	float **p2;
	float *p1;

	for(i=0;i<n1;++i)
	{
		p2=p[i];
		for(j=0;j<n2;++j)
		{
			p1=p2[j];
			for(k=0;k<n3;++k)
			{
				p1[k]=v;
			}
		}
	}
}


void wtk_double_p3_set(double ***p,int n1,int n2,int n3,float v)
{
	int i,j,k;
	double **p2;
	double *p1;

	for(i=0;i<n1;++i)
	{
		p2=p[i];
		for(j=0;j<n2;++j)
		{
			p1=p2[j];
			for(k=0;k<n3;++k)
			{
				p1[k]=v;
			}
		}
	}
}


void wtk_float_delete_p3(float ***pf,int n1,int n2)
{
	int i,j;

	for(i=0;i<n1;++i)
	{
		for(j=0;j<n2;++j)
		{
			wtk_free(pf[i][j]);
		}
		wtk_free(pf[i]);
	}
	wtk_free(pf);
}

void wtk_double_delete_p3(double ***pf,int n1,int n2)
{
	int i,j;

	for(i=0;i<n1;++i)
	{
		for(j=0;j<n2;++j)
		{
			wtk_free(pf[i][j]);
		}
		wtk_free(pf[i]);
	}
	wtk_free(pf);
}

void wtk_float_delete_p2(float **pf,int n1)
{
	int i;

	for(i=0;i<n1;++i)
	{
		wtk_free(pf[i]);
	}
	wtk_free(pf);
}

void wtk_double_delete_p2(double **pf,int n1)
{
	int i;

	for(i=0;i<n1;++i)
	{
		wtk_free(pf[i]);
	}
	wtk_free(pf);
}


float** wtk_float_new_p2(int n1,int n2)
{
	float **c;
	int i;

	c=(float**)wtk_calloc(n1,sizeof(float*));
	for(i=0;i<n1;++i)
	{
		c[i]=(float*)wtk_calloc(n2,sizeof(float));
	}
	return c;
}

void wtk_float_p2_zero(float **p,int n1,int n2)
{
	int i,nx;

	nx=n2*sizeof(float);
	for(i=0;i<n1;++i)
	{
		memset(p[i],0,nx);
	}
}

void wtk_double_p2_zero(double **p,int n1,int n2)
{
	int i,nx;

	nx=n2*sizeof(double);
	for(i=0;i<n1;++i)
	{
		memset(p[i],0,nx);
	}
}


double** wtk_double_new_p2(int n1,int n2)
{
	double **c;
	int i;

	c=(double**)wtk_calloc(n1,sizeof(double*));
	for(i=0;i<n1;++i)
	{
		c[i]=(double*)wtk_calloc(n2,sizeof(double));
	}
	return c;
}

wtk_complex_t** wtk_complex_new_p2(int n1,int n2)
{
	wtk_complex_t **c;
	int i;

	c=(wtk_complex_t**)wtk_calloc(n1,sizeof(wtk_complex_t*));
	for(i=0;i<n1;++i)
	{
		c[i]=(wtk_complex_t*)wtk_calloc(n2,sizeof(wtk_complex_t));
	}
	return c;
}

wtk_dcomplex_t** wtk_dcomplex_new_p2(int n1,int n2)
{
	wtk_dcomplex_t **c;
	int i;

	c=(wtk_dcomplex_t**)wtk_calloc(n1,sizeof(wtk_dcomplex_t*));
	for(i=0;i<n1;++i)
	{
		c[i]=(wtk_dcomplex_t*)wtk_calloc(n2,sizeof(wtk_dcomplex_t));
	}
	return c;
}

void wtk_complex_eye(wtk_complex_t **p,int n,wtk_complex_t v)
{
	int i;
	int nx;

	nx=n*sizeof(wtk_complex_t);
	for(i=0;i<n;++i)
	{
		memset(p[i],0,nx);
		p[i][i]=v;
	}
}

void wtk_complex_eye2(wtk_complex_t *p,int n)
{
	int i;
	int nx;
	wtk_complex_t v={1,0};

	nx=n*sizeof(wtk_complex_t);
	for(i=0;i<n;++i)
	{
		memset(p,0,nx);
		p[i]=v;
		p+=n;
	}
}

void wtk_dcomplex_eye(wtk_dcomplex_t **p,int n,wtk_dcomplex_t v)
{
	int i;
	int nx;

	nx=n*sizeof(wtk_dcomplex_t);
	for(i=0;i<n;++i)
	{
		memset(p[i],0,nx);
		p[i][i]=v;
	}
}

void wtk_complex_delete_p2(wtk_complex_t **p2,int n1)
{
	int i;

	for(i=0;i<n1;++i)
	{
		wtk_free(p2[i]);
	}
	wtk_free(p2);
}

void wtk_dcomplex_delete_p2(wtk_dcomplex_t **p2,int n1)
{
	int i;

	for(i=0;i<n1;++i)
	{
		wtk_free(p2[i]);
	}
	wtk_free(p2);
}


void wtk_complex_delete_p3(wtk_complex_t ***p3,int n1,int n2)
{
	int i,j;

	for(i=0;i<n1;++i)
	{
		for(j=0;j<n2;++j)
		{
			wtk_free(p3[i][j]);
		}
		wtk_free(p3[i]);
	}
	wtk_free(p3);
}


wtk_complex_t*** wtk_complex_new_p3(int n1,int n2,int n3)
{
	wtk_complex_t ***c;
	wtk_complex_t **p;
	int i,j;

	c=(wtk_complex_t***)wtk_calloc(n1,sizeof(wtk_complex_t***));
	for(i=0;i<n1;++i)
	{
		p=c[i]=(wtk_complex_t**)wtk_calloc(n2,sizeof(wtk_complex_t*));
		for(j=0;j<n2;++j)
		{
			//c[i][j]=(wtk_complex_t*)wtk_calloc(n3,sizeof(wtk_complex_t));
			p[j]=(wtk_complex_t*)wtk_calloc(n3,sizeof(wtk_complex_t));
		}
	}
	return c;
}

void wtk_complex_zero_p3(wtk_complex_t ***p,int n1,int n2,int n3)
{
	int i,j;
	int t;
	wtk_complex_t **p2;

	t=n3*sizeof(wtk_complex_t);
	for(i=0;i<n1;++i)
	{
		p2=p[i];
		for(j=0;j<n2;++j)
		{
			memset(p2[j],0,t);
		}
	}
}

void wtk_dcomplex_zero_p3(wtk_dcomplex_t ***p,int n1,int n2,int n3)
{
	int i,j;
	int t;
	wtk_dcomplex_t **p2;

	t=n3*sizeof(wtk_dcomplex_t);
	for(i=0;i<n1;++i)
	{
		p2=p[i];
		for(j=0;j<n2;++j)
		{
			memset(p2[j],0,t);
		}
	}
}

void wtk_complex_cpy_p2(wtk_complex_t **dst,wtk_complex_t **src,int n1,int n2)
{
	int i;
	int t;

	t=n2*sizeof(wtk_complex_t);
	for(i=0;i<n1;++i)
	{
		memcpy(dst[i],src[i],t);
	}
}

void wtk_complex_cpy_p3(wtk_complex_t ***dst,wtk_complex_t ***src,int n1,int n2,int n3)
{
	int i,j;
	int t;

	t=n3*sizeof(wtk_complex_t);
	for(i=0;i<n1;++i)
	{
		for(j=0;j<n2;++j)
		{
			memcpy(dst[i][j],src[i][j],t);
		}
	}
}

void wtk_dcomplex_cpy_p3(wtk_dcomplex_t ***dst,wtk_complex_t ***src,int n1,int n2,int n3)
{
	int i,j,k;
	//wtk_complex_t **src1,*src2;
	//wtk_dcomplex_t **dst1,*dst2;

	for(i=0;i<n1;++i)
	{
		//src1=src[i];
		for(j=0;j<n2;++j)
		{
			//src2=src1[j];
			for(k=0;k<n3;++k)
			{
				dst[i][j][k].a=src[i][j][k].a;
				dst[i][j][k].b=src[i][j][k].b;
			}
		}
	}
}


void wtk_complex_cpy_p4(wtk_complex_t ***dst,wtk_dcomplex_t ***src,int n1,int n2,int n3)
{
	int i,j,k;

	for(i=0;i<n1;++i)
	{
		for(j=0;j<n2;++j)
		{
			for(k=0;k<n3;++k)
			{
				dst[i][j][k].a=src[i][j][k].a;
				dst[i][j][k].b=src[i][j][k].b;
			}
		}
	}
}

void wtk_complex_zero_p2(wtk_complex_t **p,int n1,int n2)
{
	int i,t;

	t=n2*sizeof(wtk_complex_t);
	for(i=0;i<n1;++i)
	{
		memset(p[i],0,t);
	}
}

void wtk_dcomplex_zero_p2(wtk_dcomplex_t **p,int n1,int n2)
{
	int i;
	int t;

	t=n2*sizeof(wtk_dcomplex_t);
	for(i=0;i<n1;++i)
	{
		memset(p[i],0,t);
	}
}

int wtk_dcomplex_guass_elimination(wtk_dcomplex_t **a,wtk_dcomplex_t **b,int n1,int n2)
{
	int i,j,k;
	double f;
	wtk_dcomplex_t *pa,*pb;
	double fa,fb,fa1,fb1;

	fb=0;
	for(i=0;i<n1;++i)
	{
		//(a+bi)/(c+di)=(a+bi)(c-di)/(c*c+d*d)=(ac+bd)+i(-ad+bc)/(c*c+d*d)
		pa=a[i]+i;
		if(pa->b==0)
		{
			if(pa->a==0)
			{
				return -1;
			}
			fa=1.0/pa->a;
			pa->a=1;
			pa->b=0;
			j=i+1;
			++pa;
			for(;j<n1;++j,++pa)
			{
				pa->a*=fa;
				pa->b*=fa;
			}
			pa=b[i];
			for(j=0;j<n2;++j,++pa)
			{
				pa->a*=fa;
				pa->b*=fa;
			}
		}else
		{
			f=(pa->a*pa->a+pa->b*pa->b);
			if(f==0)
			{
				return -1;
			}
			fa=pa->a/f;
			fb=-pa->b/f;
			pa->a=1;
			pa->b=0;
			j=i+1;
			++pa;
			for(;j<n1;++j,++pa)
			{
				//d*c=(a+bi)(c+di)=(ac-bd)+i(ad+bc);
				fa1=pa->a;
				fb1=pa->b;
				pa->a=fa1*fa -fb1*fb;
				pa->b=fa1*fb+fb1*fa;
			}
			pa=b[i];
			for(j=0;j<n2;++j,++pa)
			{
				fa1=pa->a;
				fb1=pa->b;
				pa->a=fa1*fa -fb1*fb;
				pa->b=fa1*fb+fb1*fa;
			}
		}
		for(j=i+1;j<n1;++j)
		{
			fa=a[j][i].a;
			fb=a[j][i].b;
			if(fb==0)
			{
				pa=a[i]+i;
				pb=a[j]+i;
				for(k=i;k<n1;++k,++pa,++pb)
				{
					pb->a-=pa->a*fa;
					pb->b-=pa->b*fa;
				}
				pa=b[i];
				pb=b[j];
				for(k=0;k<n2;++k,++pa,++pb)
				{
					pb->a-=pa->a*fa;
					pb->b-=pa->b*fa;
				}
			}else
			{
				pa=a[i]+i;
				pb=a[j]+i;
				for(k=i;k<n1;++k,++pa,++pb)
				{
					pb->a-=pa->a*fa -pa->b*fb;
					pb->b-=pa->a*fb+pa->b*fa;
				}
				pa=b[i];
				pb=b[j];
				for(k=0;k<n2;++k,++pa,++pb)
				{
					pb->a-=pa->a*fa -pa->b*fb;
					pb->b-=pa->a*fb+pa->b*fa;
				}
			}
		}
	}
	for(i=n1-1;i>=0;--i)
	{
		for(j=0;j<i;++j)
		{
			fa=a[j][i].a;
			fb=a[j][i].b;
			if(fb==0)
			{
				pa=b[i];
				pb=b[j];
				for(k=0;k<n2;++k,++pa,++pb)
				{
					pb->a-=pa->a*fa;
					pb->b-=pa->b*fa;
				}
			}else
			{
				pa=b[i];
				pb=b[j];
				for(k=0;k<n2;++k,++pa,++pb)
				{
					pb->a-=pa->a*fa -pa->b*fb;
					pb->b-=pa->a*fb+pa->b*fa;
				}
			}
		}
	}
	return 0;
}

int wtk_complex_guass_elimination_p1(wtk_complex_t *a,wtk_complex_t *b,int n1,int n2)
{
	int i,j,k;
	double f;
	wtk_complex_t *pa,*pb;
	float fa,fb;
	float fa1,fb1;

	for(i=0;i<n1;++i)
	{
		pa=a+i*(n1+1);
		if(pa->b==0)
		{
			if(pa->a==0)
			{
				return -1;
			}
			fa=1.0/pa->a;
			pa->a=1;
			j=i+1;
			++pa;
			for(;j<n1;++j,++pa)
			{
				pa->a*=fa;
				pa->b*=fa;
			}
			pa=b+i*n2;
			for(j=0;j<n2;++j,++pa)
			{
				pa->a*=fa;
				pa->b*=fa;
			}
		}else
		{
			f=(pa->a*pa->a+pa->b*pa->b);
			if(f==0)
			{
				return -1;
			}
			fa=pa->a/f;
			fb=-pa->b/f;
			pa->a=1;
			pa->b=0;
			j=i+1;
			++pa;
			for(;j<n1;++j,++pa)
			{
				fa1=pa->a;
				fb1=pa->b;
				pa->a=fa1*fa -fb1*fb;
				pa->b=fa1*fb+fb1*fa;
			}
			pa=b+i*n2;
			for(j=0;j<n2;++j,++pa)
			{
				fa1=pa->a;
				fb1=pa->b;
				pa->a=fa1*fa -fb1*fb;
				pa->b=fa1*fb+fb1*fa;
			}
		}
		for(j=i+1;j<n1;++j)
		{
			fa=a[j*n1+i].a;
			fb=a[j*n1+i].b;
			if(fb==0)
			{
				pa=a+i*n1+i;
				pb=a+j*n1+i;
				for(k=i;k<n1;++k,++pa,++pb)
				{
					pb->a-=pa->a*fa;
					pb->b-=pa->b*fa;
				}
				pa=b+i*n2;
				pb=b+j*n2;
				for(k=0;k<n2;++k,++pa,++pb)
				{
					pb->a-=pa->a*fa;
					pb->b-=pa->b*fa;
				}
			}else
			{
				pa=a+i*n1+i;
				pb=a+j*n1+i;
				for(k=i;k<n1;++k,++pa,++pb)
				{
					pb->a-=pa->a*fa -pa->b*fb;
					pb->b-=pa->a*fb+pa->b*fa;
				}
				pa=b+i*n2;
				pb=b+j*n2;
				for(k=0;k<n2;++k,++pa,++pb)
				{
					pb->a-=pa->a*fa -pa->b*fb;
					pb->b-=pa->a*fb+pa->b*fa;
				}
			}
		}
	}
	for(i=n1-1;i>=0;--i)
	{
		for(j=0;j<i;++j)
		{
			fa=a[j*n1+i].a;
			fb=a[j*n1+i].b;
			if(fb==0)
			{
				pa=b+i*n2;
				pb=b+j*n2;
				for(k=0;k<n2;++k,++pa,++pb)
				{
					pb->a-=pa->a*fa;
					pb->b-=pa->b*fa;
				}
			}else
			{
				pa=b+i*n2;
				pb=b+j*n2;
				for(k=0;k<n2;++k,++pa,++pb)
				{
					pb->a-=pa->a*fa -pa->b*fb;
					pb->b-=pa->a*fb+pa->b*fa;
				}
			}
		}
	}
	return 0;
}

void wtk_complex_guass_elimination(wtk_complex_t **a,wtk_complex_t **b,int n1,int n2)
{
	int i,j,k;
	double f;
	wtk_complex_t *pa,*pb;
	float fa,fb;
	float fa1,fb1;

	fb=0;
	for(i=0;i<n1;++i)
	{
		//(a+bi)/(c+di)=(a+bi)(c-di)/(c*c+d*d)=(ac+bd)+i(-ad+bc)/(c*c+d*d)
		pa=a[i]+i;
		if(pa->b==0)
		{
			fa=1.0/pa->a;
			pa->a=1;
			j=i+1;
			++pa;
			for(;j<n1;++j,++pa)
			{
				pa->a*=fa;
				pa->b*=fa;
			}
			pa=b[i];
			for(j=0;j<n2;++j,++pa)
			{
				pa->a*=fa;
				pa->b*=fa;
			}
		}else
		{
			f=(pa->a*pa->a+pa->b*pa->b);
			fa=pa->a/f;
			fb=-pa->b/f;
			pa->a=1;
			pa->b=0;
			j=i+1;
			++pa;
			for(;j<n1;++j,++pa)
			{
				//d*c=(a+bi)(c+di)=(ac-bd)+i(ad+bc);
				fa1=pa->a;
				fb1=pa->b;
				pa->a=fa1*fa -fb1*fb;
				pa->b=fa1*fb+fb1*fa;
			}
			pa=b[i];
			for(j=0;j<n2;++j,++pa)
			{
				fa1=pa->a;
				fb1=pa->b;
				pa->a=fa1*fa -fb1*fb;
				pa->b=fa1*fb+fb1*fa;
			}
		}
		for(j=i+1;j<n1;++j)
		{
			fa=a[j][i].a;
			fb=a[j][i].b;
			if(fb==0)
			{
				pa=a[i]+i;
				pb=a[j]+i;
				for(k=i;k<n1;++k,++pa,++pb)
				{
					pb->a-=pa->a*fa;
					pb->b-=pa->b*fa;
				}
				pa=b[i];
				pb=b[j];
				for(k=0;k<n2;++k,++pa,++pb)
				{
					pb->a-=pa->a*fa;
					pb->b-=pa->b*fa;
				}
			}else
			{
				pa=a[i]+i;
				pb=a[j]+i;
				for(k=i;k<n1;++k,++pa,++pb)
				{
					pb->a-=pa->a*fa -pa->b*fb;
					pb->b-=pa->a*fb+pa->b*fa;
				}
				pa=b[i];
				pb=b[j];
				for(k=0;k<n2;++k,++pa,++pb)
				{
					pb->a-=pa->a*fa -pa->b*fb;
					pb->b-=pa->a*fb+pa->b*fa;
				}
			}
		}
	}
	for(i=n1-1;i>=0;--i)
	{
		for(j=0;j<i;++j)
		{
			fa=a[j][i].a;
			fb=a[j][i].b;
			if(fb==0)
			{
				pa=b[i];
				pb=b[j];
				for(k=0;k<n2;++k,++pa,++pb)
				{
					pb->a-=pa->a*fa;
					pb->b-=pa->b*fa;
				}
			}else
			{
				pa=b[i];
				pb=b[j];
				for(k=0;k<n2;++k,++pa,++pb)
				{
					pb->a-=pa->a*fa -pa->b*fb;
					pb->b-=pa->a*fb+pa->b*fa;
				}
			}
		}
	}
}


void wtk_complex_guass_elimination_raw(wtk_complex_t **a,wtk_complex_t **b,int n1,int n2)
{
	int i,j,k;
	wtk_complex_t c,d;
	double f;
	wtk_complex_t *pa,*pb;

	for(i=0;i<n1;++i)
	{
		//(a+bi)/(c+di)=(a+bi)(c-di)/(c*c+d*d)=(ac+bd)+i(-ad+bc)/(c*c+d*d)
		pa=a[i];
		f=(pa[i].a*pa[i].a+pa[i].b*pa[i].b);
		c.a=pa[i].a/f;
		c.b=-pa[i].b/f;
		for(j=i;j<n1;++j)
		{
			//d*c=(a+bi)(c+di)=(ac-bd)+i(ad+bc);
			d=pa[j];
			pa[j].a=d.a*c.a -d.b*c.b;
			pa[j].b=d.a*c.b+d.b*c.a;
		}
		pa=b[i];
		for(j=0;j<n2;++j)
		{
			d=pa[j];
			pa[j].a=d.a*c.a -d.b*c.b;
			pa[j].b=d.a*c.b+d.b*c.a;
		}
		for(j=i+1;j<n1;++j)
		{
			c=a[j][i];
			pa=a[i];
			pb=a[j];
			for(k=i;k<n1;++k)
			{
				d=pa[k];
				pb[k].a-=d.a*c.a -d.b*c.b;
				pb[k].b-=d.a*c.b+d.b*c.a;
			}
			pa=b[i];
			pb=b[j];
			for(k=0;k<n2;++k)
			{
				d=pa[k];
				pb[k].a-=d.a*c.a -d.b*c.b;
				pb[k].b-=d.a*c.b+d.b*c.a;
			}
		}
	}
	for(i=n1-1;i>=0;--i)
	{
		for(j=0;j<i;++j)
		{
			c=a[j][i];
			pa=b[i];
			pb=b[j];
			for(k=0;k<n2;++k)
			{
				d=pa[k];
				pb[k].a-=d.a*c.a -d.b*c.b;
				pb[k].b-=d.a*c.b+d.b*c.a;
			}
		}
	}
}

void wtk_complex_p2_scale_add(wtk_complex_t **dst,wtk_complex_t **src,int n1,int n2,float f1)
{
	int i,j;
	float f2;
	wtk_complex_t *p1,*p2;

	f2=1-f1;
	for(i=0;i<n1;++i)
	{
		p1=dst[i];
		p2=src[i];
		for(j=0;j<n2;++j)
		{
			p1[j].a=p1[j].a*f2+f1*p2[j].a;
			p1[j].b=p1[j].b*f2+f1*p2[j].b;
		}
	}
}


void wtk_complex_p2_cpy(wtk_complex_t **dst,wtk_complex_t **src,int n1,int n2)
{
	int i;
	int t;

	t=n2*sizeof(wtk_complex_t);
	for(i=0;i<n1;++i)
	{
		memcpy(dst[i],src[i],t);
	}
}

void wtk_complex_p2_cpy2(wtk_dcomplex_t **dst,wtk_complex_t **src,int n1,int n2)
{
	int i,j;

	for(i=0;i<n1;++i)
	{
		for(j=0;j<n2;++j)
		{
			dst[i][j].a=src[i][j].a;
			dst[i][j].b=src[i][j].b;
		}
	}
}

wtk_complex_t**** wtk_complex_new_p4(int n1,int n2,int n3,int n4)
{
	wtk_complex_t ****c;
	int i,j,k;

	c=(wtk_complex_t****)wtk_calloc(n1,sizeof(wtk_complex_t****));
	for(i=0;i<n1;++i)
	{
		c[i]=(wtk_complex_t***)wtk_calloc(n2,sizeof(wtk_complex_t**));
		for(j=0;j<n2;++j)
		{
			c[i][j]=(wtk_complex_t**)wtk_calloc(n3,sizeof(wtk_complex_t*));
			for(k=0;k<n3;++k)
			{
				c[i][j][k]=(wtk_complex_t*)wtk_calloc(n4,sizeof(wtk_complex_t));
			}
		}
	}
	return c;
}

wtk_dcomplex_t**** wtk_dcomplex_new_p4(int n1,int n2,int n3,int n4)
{
	wtk_dcomplex_t ****c;
	int i,j,k;

	c=(wtk_dcomplex_t****)wtk_calloc(n1,sizeof(wtk_dcomplex_t****));
	for(i=0;i<n1;++i)
	{
		c[i]=(wtk_dcomplex_t***)wtk_calloc(n2,sizeof(wtk_dcomplex_t**));
		for(j=0;j<n2;++j)
		{
			c[i][j]=(wtk_dcomplex_t**)wtk_calloc(n3,sizeof(wtk_dcomplex_t*));
			for(k=0;k<n3;++k)
			{
				c[i][j][k]=(wtk_dcomplex_t*)wtk_calloc(n4,sizeof(wtk_dcomplex_t));
			}
		}
	}
	return c;
}

void wtk_complex_delete_p4(wtk_complex_t ****p,int n1,int n2,int n3)
{
	int i,j,k;

	for(i=0;i<n1;++i)
	{
		for(j=0;j<n2;++j)
		{
			for(k=0;k<n3;++k)
			{
				wtk_free(p[i][j][k]);
			}
			wtk_free(p[i][j]);
		}
		wtk_free(p[i]);
	}
	wtk_free(p);
}

float wtk_complex_p2_max_abs(wtk_complex_t **p,int n1,int n2)
{
	float t,maxt;
	int i,j;

	maxt=0;
	for(i=0;i<n1;++i)
	{
		for(j=0;j<n2;++j)
		{
			t=p[i][j].a*p[i][j].a+p[i][j].b+p[i][j].b;
			if(t>maxt)
			{
				maxt=t;
			}
		}
	}
	return sqrt(maxt);
}

float wtk_complex_p2_dist(wtk_complex_t **p1,wtk_complex_t **p2,int n1,int n2)
{
	double t;
	int i,j;
	float ta,tb;
	wtk_complex_t *pp1,*pp2;

	t=0;
	for(i=0;i<n1;++i)
	{
		pp1=p1[i];
		pp2=p2[i];
		for(j=0;j<n2;++j)
		{
			ta=pp1[j].a-pp2[j].a;
			tb=pp1[j].b-pp2[j].b;
			if(isnan(ta) || isnan(tb))
			{
				wtk_debug("ta=%f tb=%f %f/%f %f/%f\n",ta,tb,pp1[j].a,pp1[j].b,pp2[j].a,pp2[j].b);
				exit(0);
			}
			t+=ta*ta+tb*tb;
		}
	}
	return sqrt(t);
}

#ifdef PI
#undef PI
#endif
#define PI 3.1415926535897932384626433832795

float wtk_complex_p2_angle(wtk_complex_t **p1,wtk_complex_t **p2,int n1,int n2)
{
	double t,ta,tb;
	int i,j;
	wtk_complex_t *pp1,*pp2;

	t=0;
	ta=tb=0;
	for(i=0;i<n1;++i)
	{
		pp1=p1[i];
		pp2=p2[i];
		for(j=0;j<n2;++j)
		{
			//pp1*pp2
			//(a+bi)(c+di)=(ac-bd)+i(ad+bd);
			ta+=pp1[j].a*pp2[j].a - pp1[j].b*pp2[j].b;
			tb+=pp1[j].a*pp2[j].b + pp1[j].b*pp2[j].a;

			t+=pp1[j].a*pp1[j].a;
			t+=pp1[j].b*pp1[j].b;
			t+=pp2[j].a*pp2[j].a;
			t+=pp2[j].b*pp2[j].b;
		}
	}
	t=sqrt(t);
	wtk_debug("ta=%f tb=%f t=%f\n",ta,tb,t);
	ta=sqrt(ta*ta+tb*tb);
	//t=ta/t;
	wtk_debug("f=%f %f\n",ta/t,acos(ta/t));
	t=acos(ta/t)*180/PI;
	//wtk_debug("ta=%f tb=%f t=%f\n",ta,tb,t);
	return t;
}


void wtk_complex_p2_mul(wtk_complex_t **p,int n1,int n2,float f)
{
	int i,j;
	wtk_complex_t *c;

	for(i=0;i<n1;++i)
	{
		c=p[i];
		for(j=0;j<n2;++j)
		{
			c[j].a*=f;
			c[j].b*=f;
		}
	}
}

void wtk_complex_identity(wtk_complex_t *a,int n)
{
	int i;

	memset(a,0,n*n*sizeof(wtk_complex_t));
	for(i=0;i<n;++i)
	{
		a[i*n+i].a=1;
	}
}

void wtk_dcmplex_p2_mul(wtk_dcomplex_t **p,int n1,int n2,float f)
{
	int i,j;
	wtk_dcomplex_t *c;

	for(i=0;i<n1;++i)
	{
		c=p[i];
		for(j=0;j<n2;++j)
		{
			c[j].a*=f;
			c[j].b*=f;
		}
	}
}

void wtk_complex_print(wtk_complex_t *c,int n)
{
	int i;
	float a,b;

	a=b=0;
	for(i=0;i<n;++i)
	{
		//wtk_debug("v[%d]=%.4f+%.4fi\n",i,c[i].a,c[i].b);
		a+=c[i].a;
		b+=c[i].b;
		wtk_debug("v[%d]=%f+%fi\n",i,c[i].a,c[i].b);
		//wtk_debug("v[%d]=%e+%ei\n",i,c[i].a,c[i].b);
	}
	wtk_debug("tot=%f+%fi\n\n",a,b);
}

wtk_complex_t wtk_complex_p2_sum(wtk_complex_t **c,int n1,int n2)
{
	int i,j;
	wtk_complex_t b;
	float ta,tb;

	ta=tb=0;
	for(i=0;i<n1;++i)
	{
		for(j=0;j<n2;++j)
		{
			ta+=c[i][j].a;
			tb+=c[i][j].b;
		}
	}
	b.a=ta;
	b.b=tb;
	return b;
}

void wtk_complex_print2(wtk_complex_t *c,int r,int col)
{
	int i,j;
	int k=0;
	wtk_dcomplex_t x;
	float f=1;//10000;
	int use_e=0;

	//f=1000;
	//f=10000;
	x.a=0;
	x.b=0;
	for(i=0;i<r;++i)
	{
		for(j=0;j<col;++j)
		{
			x.a+=c[k].a;
			x.b+=c[k].b;
			if(j>0)
			{
				printf("\t");
			}
			if(c[k].b>=0)
			{
				if(use_e)
				{
					printf("%e + %ei",c[k].a*f,c[k].b*f);
				}else
				{
					printf("%.8f + %.8fi",c[k].a*f,c[k].b*f);
				}
			}else
			{
				if(use_e)
				{
					printf("%e - %ei",c[k].a*f,-c[k].b*f);
				}else
				{
					printf("%.8f + %.8fi",c[k].a*f,c[k].b*f);
				}
			}
			++k;
		}
		printf("\n");
	}
	wtk_debug("tot: %e+i%e\n\n",x.a,x.b);
}

void wtk_dcomplex_print(wtk_dcomplex_t *c,int n)
{
	double ta,tb;
	int i;

	ta=tb=0;
	for(i=0;i<n;++i)
	{
		ta+=c[i].a;
		tb+=c[i].b;
		wtk_debug("v[%d]=%.10f+%.10fi\n",i,c[i].a,c[i].b);
	}
	wtk_debug("tot=%f+%fi\n",ta,tb);
}

void wtk_dcomplex_p2_print(wtk_dcomplex_t **c,int n1,int n2)
{
	int i,j;
	double ta,tb;

	ta=tb=0;
	for(i=0;i<n1;++i)
	{
		for(j=0;j<n2;++j)
		{
//			if(j>0)
//			{
//				printf(" ");
//			}
//			printf("%e+%e",c[i][j].a,c[i][j].b);
			wtk_debug("v[%d/%d]=%.10f+%.10fi\n",i,j,c[i][j].a,c[i][j].b);
			//wtk_debug("v[%d/%d]=%f+%fi\n",i,j,c[i][j].a,c[i][j].b);
			ta+=c[i][j].a;
			tb+=c[i][j].b;
		}
		//printf("\n");
	}
	wtk_debug("tot=%f+%fi\n",ta,tb);
}

void wtk_dcomplex_p2_print2(wtk_dcomplex_t **c,int n1,int n2)
{
	int i,j;

	for(i=0;i<n1;++i)
	{
		for(j=0;j<n2;++j)
		{
			if(j>0)
			{
				printf(" ");
			}
			//printf("%e",c[i][j].a);
			//printf("%e+%e",c[i][j].a,c[i][j].b);
			//printf("%.4f+%.4f",c[i][j].a,c[i][j].b);
			printf("%f",c[i][j].a);
			//wtk_debug("v[%d/%d]=%f+%fi\n",i,j,c[i][j].a,c[i][j].b);
			//wtk_debug("v[%d/%d]=%f+%fi\n",i,j,c[i][j].a,c[i][j].b);
		}
		printf("\n");
		//printf(";");
	}
}

void wtk_dcomplex_mult(wtk_dcomplex_t **c,int n1,int n2,float f)
{
	int i,j;
	wtk_dcomplex_t *pc;

	for(i=0;i<n1;++i)
	{
		pc=c[i];
		for(j=0;j<n2;++j)
		{
			pc[j].a*=f;
			pc[j].b*=f;
		}
	}
}

void wtk_dcomplex_print2(wtk_dcomplex_t *c,int r,int col)
{
	int i,j;
	int k=0;
	wtk_dcomplex_t x;
	float f=1;//10000;
	int b=1;

	//b=0;
	//f=1000;
	x.a=0;
	x.b=0;
	for(i=0;i<r;++i)
	{
		for(j=0;j<col;++j)
		{
			x.a+=c[k].a;
			x.b+=c[k].b;
			if(j>0)
			{
				printf(" ");
			}
			if(b)
			{
				if(c[k].b>=0)
				{
					printf("%.6f + %.6fi",c[k].a*f,c[k].b*f);
				}else
				{
					printf("%.6f - %.6fi",c[k].a*f,-c[k].b*f);
				}
			}else
			{
				if(c[k].b>=0)
				{
					printf("%.6f+%.6fi",c[k].a*f,c[k].b*f);
				}else
				{
					printf("%.6f-%.6fi",c[k].a*f,-c[k].b*f);
				}
			}
			++k;
		}
		if(b)
		{
			printf("\n");
		}else
		{
			printf(";");
		}
	}
	printf("\n");
	wtk_debug("tot: %e+i%e\n\n",x.a,x.b);
}

wtk_complex_t wtk_complex_div(wtk_complex_t *a,wtk_complex_t *b)
{
	float f;
	wtk_complex_t c;

	f=1.0/(b->a*b->a+b->b*b->b);
	c.a=(a->a*b->a+a->b*b->b)*f;//(xc*xc+xd*xd);
	c.b=(a->b*b->a-a->a*b->b)*f;//(xc*xc+xd*xd);
	return c;
}

wtk_complex_t wtk_complex_mul(wtk_complex_t *a,wtk_complex_t *b)
{
	wtk_complex_t c;

	//a+bi c+di => ac-bd +i ad+bc
	c.a=a->a*b->a - a->b*b->b;
	c.b=a->a*b->b + a->b*b->a;
	return c;
}

wtk_dcomplex_t wtk_dcomplex_mul(wtk_dcomplex_t *a,wtk_complex_t *b)
{
	wtk_dcomplex_t c;

	//a+bi c+di => ac-bd +i ad+bc
	c.a=a->a*b->a - a->b*b->b;
	c.b=a->a*b->b + a->b*b->a;
	return c;
}


wtk_dcomplex_t wtk_dcomplex_mul2(wtk_dcomplex_t *a,wtk_dcomplex_t *b)
{
	wtk_dcomplex_t c;

	//a+bi c+di => ac-bd +i ad+bc
	c.a=a->a*b->a - a->b*b->b;
	c.b=a->a*b->b + a->b*b->a;
	return c;
}


//c+=|a*b|
void wtk_complex_add_mul(wtk_complex_t *c,wtk_complex_t *a,wtk_complex_t *b)
{
	//a+bi c+di => ac-bd +i ad+bc
	c->a+=a->a*b->a - a->b*b->b;
	c->b+=a->a*b->b + a->b*b->a;
}

void wtk_complex_sub(wtk_complex_t *a,wtk_complex_t b)
{
	a->a-=b.a;
	a->b-=b.b;
}


//a=b*c  |1*4|*|4*2|
void wtk_complex_matrix_mul(wtk_complex_t *a,wtk_complex_t *b,wtk_complex_t *c,int row,int col,int col2)
{
	int i,j,k;
	wtk_complex_t *x;
	wtk_complex_t t;

	for(i=0;i<row;++i)
	{
		x=b+i*row;
		for(j=0;j<col2;++j)
		{
			t.a=t.b=0;
			for(k=0;k<col;++k)
			{
				//x[k] c[k][j]
				wtk_complex_add_mul(&(t),x+k,c+k*col2+j);
			}
			//a[i][j]=t;
			a[i*col2+j]=t;
			//wtk_debug("v[%d/%d]=%f/%f\n",i,j,t.a,t.b);
			//exit(0);
		}
	}
}

void wtk_complex_matrix_mul2(wtk_complex_t **a,wtk_complex_t **b,wtk_complex_t **c,int row,int col,int col2)
{
	int i,j,k;
	double ta,tb;
	wtk_complex_t *pa;

	for(i=0;i<row;++i)
	{
		pa=a[i];
		for(j=0;j<col2;++j)
		{
			ta=tb=0;
			for(k=0;k<col;++k)
			{
				//pa[k]*b[k][j]
				//(a+bi)*(c+di)=(ac-bd)+i(ad+bc);
				ta+=pa[k].a*b[k][j].a - pa[k].b*b[k][j].b;
				tb+=pa[k].a*b[k][j].b+pa[k].b*b[k][j].a;
			}
			c[i][j].a=ta;
			c[i][j].b=tb;
		}
	}
}

void wtk_dcomplex_matrix_mul(wtk_dcomplex_t **a,wtk_dcomplex_t **b,wtk_dcomplex_t **c,int row,int col,int col2)
{
	int i,j,k;
	double ta,tb;
	wtk_dcomplex_t *pa;

	for(i=0;i<row;++i)
	{
		pa=a[i];
		for(j=0;j<col2;++j)
		{
			ta=tb=0;
			for(k=0;k<col;++k)
			{
				//pa[k]*b[k][j]
				//(a+bi)*(c+di)=(ac-bd)+i(ad+bc);
				ta+=pa[k].a*b[k][j].a - pa[k].b*b[k][j].b;
				tb+=pa[k].a*b[k][j].b+pa[k].b*b[k][j].a;
			}
			c[i][j].a=ta;
			c[i][j].b=tb;
		}
	}
}


void print_complex(wtk_complex_t *c,int n)
{
	int i;

	for(i=0;i<n;++i)
	{
		wtk_debug("v[%d]=%f+i%f\n",i,c[i].a,c[i].b);
	}
}

//a=b*c  |1*4|*|4*2|
void wtk_complexa_matrix_mul(wtk_complexa_t *a,wtk_complexa_t *b,wtk_complexa_t *c,int row,int col,int col2)
{
	int i,j,k;
	//wtk_complex_t t;
	float xa,xb;
	int idx,idx2;
	int rowi;

	for(i=0,rowi=0;i<row;++i,rowi+=row)
	{
		//x=b+i*row;
		for(j=0;j<col2;++j)
		{
			xa=xb=0;
			for(k=0;k<col;++k)
			{
				//x[k] c[k][j]
				//wtk_complex_add_mul(&(t),x+k,c+k*col2+j);
				idx=rowi+k;
				idx2=k*col2+j;
				xa+=b->a[idx]*c->a[idx2] - b->b[idx]*c->b[idx2];
				xb+=b->a[idx]*c->b[idx2] + b->b[idx]*c->a[idx2];
			}
			//a[i][j]=t;
			idx=i*col2+j;
			a->a[idx]=xa;
			a->b[idx]=xb;
			//a[i*col2+j]=t;
		}
	}
}




wtk_complexa_t* wtk_complexa_new(int n)
{
	wtk_complexa_t *ca;

	ca=(wtk_complexa_t*)wtk_malloc(sizeof(wtk_complexa_t));
	ca->a=(float*)wtk_malloc(sizeof(float)*n);
	ca->b=(float*)wtk_malloc(sizeof(float)*n);
	ca->n=n;
	return ca;
}

void wtk_complexa_delete(wtk_complexa_t *ca)
{
	wtk_free(ca->a);
	wtk_free(ca->b);
	wtk_free(ca);
}

void wtk_complexa_zero(wtk_complexa_t *ca)
{
	memset(ca->a,0,sizeof(float)*ca->n);
	memset(ca->b,0,sizeof(float)*ca->n);
}

void wtk_complexa_print(wtk_complexa_t *ca)
{
	int i;

	for(i=0;i<ca->n;++i)
	{
		wtk_debug("v[%d]=%f+%fi\n",i,ca->a[i],ca->b[i]);
	}
}



void wtk_complex_inv_2x(wtk_complex_t *a,int r,int c,wtk_complex_t *b)
{
	int i,j,k;
	wtk_complex_t *pc,*pc2,*pc3,*pc4,*pc5,*pc6,*pc7;
	double f;
	double xa,xb;
	double ta,tb;

	//if(r!=c){return;}
	k=r*c;
	//memset(b,0,sizeof(wtk_complex_t)*k);
	for(i=0;i<k;++i)
	{
		b[i].a=b[i].b=0;
	}
	for(i=0,j=0;i<r;++i) //+=(r+1))
	{
		b[j].a=1;
		//b[j].b=0;
		j+=r+1;
	}
	pc5=a;
	pc6=b;
	for(k=0;k<r;++k,pc5+=c,pc6+=c)
	{
		pc=a;
		pc2=b;
		pc7=pc5+k;//a+k*c+k;
		//f=1.0/(pc7->a*pc7->a+pc7->b*pc7->b);
		ta=pc7->a;
		tb=pc7->b;
		//wtk_debug("ta=%f/%f\n",ta,tb);
		f=ta*ta+tb*tb;
		if(f==0)
		{
			f=1e-35;
		}
		f=1.0/f;//(ta*ta+tb*tb);
		if(isnan(f))
		{
			wtk_debug("ta=%f/%f\n",ta,tb);
			exit(0);
		}
		ta*=f;
		tb*=f;
		if(isnan(ta))
		{
			wtk_debug("ta=%f/%f\n",ta,f);
			exit(0);
		}
		//wtk_debug("%f/%f\n",pc7->a,pc7->b);
		//exit(0);
		for(i=0;i<c;++i)
		{
			if(i==k)
			{
				pc+=c;
				pc2+=c;
				continue;
			}
			pc3=a+i*c+k;
//			pc7=a+k*c+k;
//			f=1.0/(pc7->a*pc7->a+pc7->b*pc7->b);
			//xa=(pc3->a*pc7->a+pc3->b*pc7->b)*f;
			//xb=(pc3->b*pc7->a-pc3->a*pc7->b)*f;
			xa=(pc3->a*ta+pc3->b*tb);
			if(isnan(xa))
			{
        		wtk_debug("%f/%f/%f/%f\n",ta,pc3->a,tb,pc3->b);
				exit(0);
			}
			xb=(pc3->b*ta-pc3->a*tb);
			//ratio=wtk_complex_div(a+i*c+k,a+k*c+k);

			//wtk_debug("xa=%f xb=%f f=%f\n",xa,xb,f);
			//exit(0);
			//pc=a+i*c;
			//pc2=b+i*c;
			pc3=pc5;//a+k*c;
			pc4=pc6;//b+k*c;
			//pc3=pc5;//a+k*c;
			//pc4=pc6;//b+k*c;
            for(j=0;j<c;j++)
            {
            	//a=ratio b=pc3;
            	//wtk_complex_sub(a+i*c+j,wtk_complex_mul(&ratio,a+k*c+j));
            	pc->a-=xa*pc3->a - xb*pc3->b;
            	if(isnan(pc->a))
            	{
            		wtk_debug("%f %f/%f/%f/%f\n",pc->a,xa,pc3->a,xb,pc3->b);
            		exit(0);
            	}
            	(pc++)->b-=xa*pc3->b + xb*pc3->a;
            	if(isnan((pc-1)->b))
            	{
            		wtk_debug("%f\n",pc->b);
            		exit(0);
            	}
            	++pc3;

            	//wtk_complex_sub(b+i*c+j,wtk_complex_mul(&ratio,b+k*c+j));
            	pc2->a-=xa*pc4->a - xb*pc4->b;
            	if(isnan(pc2->a))
            	{
            		wtk_debug("%f\n",pc2->a);
            		exit(0);
            	}
            	(pc2++)->b-=xa*pc4->b + xb*pc4->a;
            	if(isnan((pc2-1)->b))
            	{
            		wtk_debug("%f\n",pc2->b);
            		exit(0);
            	}
            	//++pc;++pc2;
            	++pc4;
            }
		}
	}
	for(i=0;i<r;++i)
	{
		pc=a+i*c+i;
		ta=pc->a;
		tb=pc->b;
		f=ta*ta+tb*tb;
		if(f==0)
		{
			f=1e-35;
		}
		f=1.0/f;//(ta*ta+tb*tb);
		if(isnan(f))
		{
			wtk_debug("ta=%f/%f\n",ta,ta);
			exit(0);
		}
		ta*=f;
		tb*=f;
		for(j=0,pc2=b+i*c;j<c;++j)
		{
			//b[k]=wtk_complex_div(b+k,pc);
			//b[i*c+j]=wtk_complex_div(b+i*c+j,a+i*c+i);
			//b[i*c+j]/=a[i*c+i];
			//xa=(pc2->a*pc->a+pc2->b*pc->b)*f;
			//xb=(pc2->b*pc->a-pc2->a*pc->b)*f;
			xa=(pc2->a*ta + pc2->b*tb);
			xb=(pc2->b*ta - pc2->a*tb);
        	if(isnan(xa))
        	{
        		wtk_debug("%f %f/%f/%f/%f\n",xa,pc2->a,ta,pc2->b,tb);
        		exit(0);
        	}
			pc2->a=xa;
			(pc2++)->b=xb;
			//++pc2;
		}
	}
}

void wtk_complex_inv(wtk_complex_t *a,int r,int c,wtk_complex_t *b)
{
	wtk_complex_t *at;
	int n;

	n=r*c*sizeof(wtk_complex_t);
	at=(wtk_complex_t*)wtk_malloc(n);
	memcpy(at,a,n);
	wtk_complex_inv_2x(at,r,c,b);
	wtk_free(at);
}


void wtk_complex_inv_x(wtk_complex_t *a,int r,int c,wtk_complex_t *b)
{
	int i,j,k;
	wtk_complex_t *pc,*pc2,*pc3,*pc4,*pc5,*pc6,*pc7;
	float f;
	float xa,xb;
	float ta,tb;

	//if(r!=c){return;}
	k=r*c;
	//memset(b,0,sizeof(wtk_complex_t)*k);
	for(i=0;i<k;++i)
	{
		b[i].a=b[i].b=0;
	}
	for(i=0,j=0;i<r;++i) //+=(r+1))
	{
		b[j].a=1;
		//b[j].b=0;
		j+=r+1;
	}
	pc5=a;
	pc6=b;
	for(k=0;k<r;++k,pc5+=c,pc6+=c)
	{
		pc=a;
		pc2=b;
		pc7=pc5+k;//a+k*c+k;
		//f=1.0/(pc7->a*pc7->a+pc7->b*pc7->b);
		ta=pc7->a;
		tb=pc7->b;
		f=1.0/(ta*ta+tb*tb);
		ta*=f;
		tb*=f;
		//wtk_debug("%f/%f\n",pc7->a,pc7->b);
		//exit(0);
		for(i=0;i<c;++i)
		{
			if(i==k)
			{
				pc+=c;
				pc2+=c;
				continue;
			}
			pc3=a+i*c+k;
//			pc7=a+k*c+k;
//			f=1.0/(pc7->a*pc7->a+pc7->b*pc7->b);
			//xa=(pc3->a*pc7->a+pc3->b*pc7->b)*f;
			//xb=(pc3->b*pc7->a-pc3->a*pc7->b)*f;
			xa=(pc3->a*ta+pc3->b*tb);
			xb=(pc3->b*ta-pc3->a*tb);
			//ratio=wtk_complex_div(a+i*c+k,a+k*c+k);

			//wtk_debug("xa=%f xb=%f f=%f\n",xa,xb,f);
			//exit(0);
			//pc=a+i*c;
			//pc2=b+i*c;
			pc3=pc5;//a+k*c;
			pc4=pc6;//b+k*c;
			//pc3=pc5;//a+k*c;
			//pc4=pc6;//b+k*c;
            for(j=0;j<c;j++)
            {
            	//a=ratio b=pc3;
            	//wtk_complex_sub(a+i*c+j,wtk_complex_mul(&ratio,a+k*c+j));
            	pc->a-=xa*pc3->a - xb*pc3->b;
            	(pc++)->b-=xa*pc3->b + xb*pc3->a;
            	++pc3;

            	//wtk_complex_sub(b+i*c+j,wtk_complex_mul(&ratio,b+k*c+j));
            	pc2->a-=xa*pc4->a - xb*pc4->b;
            	(pc2++)->b-=xa*pc4->b + xb*pc4->a;
            	//++pc;++pc2;
            	++pc4;
            }
		}
	}
	pc2=b;
	for(i=0;i<r;++i)
	{
		pc=a+i*c+i;
		ta=pc->a;
		tb=pc->b;
		f=1.0/(ta*ta+tb*tb);//pc->a*pc->a+pc->b*pc->b);
		ta*=f;
		tb*=f;
		for(j=0;j<c;++j)
		{
			//b[k]=wtk_complex_div(b+k,pc);
			//b[i*c+j]=wtk_complex_div(b+i*c+j,a+i*c+i);
			//b[i*c+j]/=a[i*c+i];
			//xa=(pc2->a*pc->a+pc2->b*pc->b)*f;
			//xb=(pc2->b*pc->a-pc2->a*pc->b)*f;
//			xa=(pc2->a*ta + pc2->b*tb);
//			xb=(pc2->b*ta - pc2->a*tb);
//			pc2->a=xa;
//			(pc2++)->b=xb;

			xa=(pc2->a*ta + pc2->b*tb);
			pc2->b=(pc2->b*ta - pc2->a*tb);
			(pc2++)->a=xa;
			//(pc2++)->b=xb;
			//++pc2;
		}
	}
}

void wtk_complex_inv2(wtk_complex_t *a,int r,int c,wtk_complex_t *b)
{
	int i,j,k;
	wtk_complex_t ratio;
	wtk_complex_t *pc;
	float f;

	if(r!=c){return;}
	k=r*c;
	for(i=0;i<k;++i)
	{
		b[i].a=b[i].b=0;
	}
	for(i=0,j=0;i<r;++i) //+=(r+1))
	{
		//b[i].a=1;
		//b[i].b=0;
		b[j].a=1;
		b[j].b=0;
		j+=r+1;
	}
	for(k=0;k<r;++k)
	{
		for(i=0;i<c;++i)
		{
			if(i==k)
			{
				continue;
			}
			ratio=wtk_complex_div(a+i*c+k,a+k*c+k);
            for(j=0;j<c;j++)
            {
            	wtk_complex_sub(a+i*c+j,wtk_complex_mul(&ratio,a+k*c+j));
            	wtk_complex_sub(b+i*c+j,wtk_complex_mul(&ratio,b+k*c+j));
            }
		}
	}
	for(i=0;i<r;++i)
	{
		k=i*c;
		pc=a+i*c+i;
		f=1.0/(pc->a*pc->a+pc->b*pc->b);
		for(j=0;j<c;++j,++k)
		{
			//b[k]=wtk_complex_div(b+k,pc);
			//b[i*c+j]=wtk_complex_div(b+i*c+j,a+i*c+i);
			//b[i*c+j]/=a[i*c+i];
			ratio.a=(b[k].a*pc->a+b[k].b*pc->b)*f;
			ratio.b=(b[k].b*pc->a-b[k].a*pc->b)*f;
			b[k]=ratio;
		}
	}
}

//------------------------------------------------------------------
//功能: 采用部分主元的高斯消去法求方阵A的逆矩阵B
//入口参数: 输入方阵，输出方阵，方阵阶数
//返回值: true or false
//-------------------------------------------------------------------
#define N 10
int Gauss(float A[][N], float B[][N], int n)
{
    int i, j, k;
    float max, temp;
    float t[N][N];                //临时矩阵

    //将A矩阵存放在临时矩阵t[n][n]中
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            t[i][j] = A[i][j];
        }
    }
    //初始化B矩阵为单位阵
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            B[i][j] = (i == j) ? (float)1 : 0;
        }
    }
    for (i = 0; i < n; i++)
    {
        //寻找主元
        max = t[i][i];
        k = i;
        for (j = i + 1; j < n; j++)
        {
            if (fabs(t[j][i]) > fabs(max))
            {
                max = t[j][i];
                k = j;
            }
        }
        //如果主元所在行不是第i行，进行行交换
        if (k != i)
        {
            for (j = 0; j < n; j++)
            {
                temp = t[i][j];
                t[i][j] = t[k][j];
                t[k][j] = temp;
                //B伴随交换
                temp = B[i][j];
                B[i][j] = B[k][j];
                B[k][j] = temp;
            }
        }
        //判断主元是否为0, 若是, 则矩阵A不是满秩矩阵,不存在逆矩阵
        if (t[i][i] == 0)
        {
        	return -1;
        }
        //消去A的第i列除去i行以外的各行元素
        temp = t[i][i];
        for (j = 0; j < n; j++)
        {
            t[i][j] = t[i][j] / temp;        //主对角线上的元素变为1
            B[i][j] = B[i][j] / temp;        //伴随计算
        }
        for (j = 0; j < n; j++)        //第0行->第n行
        {
            if (j != i)                //不是第i行
            {
                temp = t[j][i];
                for (k = 0; k < n; k++)        //第j行元素 - i行元素*j列i行元素
                {
                    t[j][k] = t[j][k] - t[i][k] * temp;
                    B[j][k] = B[j][k] - B[i][k] * temp;
                }
            }
        }
    }
    return 0;
}

int bcinv(ar,ai,n)
int n;
double ar[],ai[];
{ int *is,*js,i,j,k,l,u,v,w;
  double p,q,s,t,d,b;
  is=malloc(n*sizeof(int));
  js=malloc(n*sizeof(int));
  for (k=0; k<=n-1; k++)
    { d=0.0;
      for (i=k; i<=n-1; i++)
      for (j=k; j<=n-1; j++)
        { u=i*n+j;
          p=ar[u]*ar[u]+ai[u]*ai[u];
          if (p>d) { d=p; is[k]=i; js[k]=j;}
        }
      if (d+1.0==1.0)
        { free(is); free(js); printf("err**not inv\n");
          return(0);
        }
      if (is[k]!=k)
        for (j=0; j<=n-1; j++)
          { u=k*n+j; v=is[k]*n+j;
            t=ar[u]; ar[u]=ar[v]; ar[v]=t;
            t=ai[u]; ai[u]=ai[v]; ai[v]=t;
          }
      if (js[k]!=k)
        for (i=0; i<=n-1; i++)
          { u=i*n+k; v=i*n+js[k];
            t=ar[u]; ar[u]=ar[v]; ar[v]=t;
            t=ai[u]; ai[u]=ai[v]; ai[v]=t;
          }
      l=k*n+k;
      ar[l]=ar[l]/d; ai[l]=-ai[l]/d;
      for (j=0; j<=n-1; j++)
        if (j!=k)
          { u=k*n+j;
            p=ar[u]*ar[l]; q=ai[u]*ai[l];
            s=(ar[u]+ai[u])*(ar[l]+ai[l]);
            ar[u]=p-q; ai[u]=s-p-q;
          }
      for (i=0; i<=n-1; i++)
        if (i!=k)
          { v=i*n+k;
            for (j=0; j<=n-1; j++)
              if (j!=k)
                { u=k*n+j;  w=i*n+j;
                  p=ar[u]*ar[v]; q=ai[u]*ai[v];
                  s=(ar[u]+ai[u])*(ar[v]+ai[v]);
                  t=p-q; b=s-p-q;
                  ar[w]=ar[w]-t;
                  ai[w]=ai[w]-b;
                }
          }
      for (i=0; i<=n-1; i++)
        if (i!=k)
          { u=i*n+k;
            p=ar[u]*ar[l]; q=ai[u]*ai[l];
            s=(ar[u]+ai[u])*(ar[l]+ai[l]);
            ar[u]=q-p; ai[u]=p+q-s;
          }
    }
  for (k=n-1; k>=0; k--)
    { if (js[k]!=k)
        for (j=0; j<=n-1; j++)
          { u=k*n+j; v=js[k]*n+j;
            t=ar[u]; ar[u]=ar[v]; ar[v]=t;
            t=ai[u]; ai[u]=ai[v]; ai[v]=t;
          }
      if (is[k]!=k)
        for (i=0; i<=n-1; i++)
          { u=i*n+k; v=i*n+is[k];
            t=ar[u]; ar[u]=ar[v]; ar[v]=t;
            t=ai[u]; ai[u]=ai[v]; ai[v]=t;
          }
    }
  free(is); free(js);
  return(1);
}

int wtk_dcomplex_invx3_x(wtk_dcomplex_t **A, int n, wtk_dcomplex_t **B)
{
	int *is, *js, i, j, k;
	double p, q, s, t, d, b;
	wtk_dcomplex_t c;

	is = malloc(n * sizeof(int));
	js = malloc(n * sizeof(int));
	for (k = 0; k < n; k++)
	{
		d = 0.0;
		for (i = k; i < n ; i++)
		{
			for (j = k; j <n ; j++)
			{
				p=A[i][j].a*A[i][j].a+A[i][j].b*A[i][j].b;
				if (p > d)
				{
					d = p;
					is[k] = i;
					js[k] = j;
				}
			}
		}
		if (d + 1.0 == 1.0)
		{
			free(is);
			free(js);
			return -1;
		}
		if (is[k] != k)
		{
			for (j = 0; j <n; j++)
			{
				c=A[k][j];
				A[k][j]=A[is[k]][j];
				A[is[k]][j]=c;
			}
		}
		if (js[k] != k)
		{
			for (i = 0; i <n; i++)
			{
				c=A[i][k];
				A[i][k]=A[i][js[k]];
				A[i][js[k]]=c;
			}
		}
		A[k][k].a/=d;
		A[k][k].b/=-d;
		for (j = 0; j < n; j++)
		{
			if (j != k)
			{
				p=A[k][j].a*A[k][k].a;
				q=A[k][j].b*A[k][k].b;
				s=(A[k][j].a+A[k][j].b)*(A[k][k].a+A[k][k].b);
				A[k][j].a=p-q;
				A[k][j].b=s-p-q;
			}
		}
		for (i = 0; i <n ; i++)
		{
			if (i != k)
			{
				for (j = 0; j < n; j++)
				{
					if (j != k)
					{
						p=A[k][j].a*A[i][k].a;
						q=A[k][j].b*A[i][k].b;
						s=(A[k][j].a+A[k][j].b)*(A[i][k].a+A[i][k].b);
						t = p - q;
						b = s - p - q;
						A[i][j].a-=t;
						A[i][j].b-=b;
					}
				}
			}
		}
		for (i = 0; i < n; i++)
		{
			if (i != k)
			{
				p=A[i][k].a*A[k][k].a;
				q=A[i][k].b*A[k][k].b;
				s=(A[i][k].a+A[i][k].b)*(A[k][k].a+A[k][k].b);
				A[i][k].a=q-p;
				A[i][k].b=p+q-s;
			}
		}
	}
	for (k = n - 1; k >= 0; k--)
	{
		if (js[k] != k)
		{
			for (j = 0; j < n; j++)
			{
				c=A[k][j];
				A[k][j]=A[js[k]][j];
				A[js[k]][j]=c;
			}
		}
		if (is[k] != k)
		{
			for (i = 0; i < n ; i++)
			{
				c=A[i][k];
				A[i][k]=A[i][is[k]];
				A[i][is[k]]=c;
			}
		}
	}
	for(i=0;i<n;++i)
	{
		for(j=0;j<n;++j)
		{
			B[i][j]=A[i][j];
		}
	}
	free(is);
	free(js);
	return 0;
}

int wtk_dcomplex_invx3(wtk_dcomplex_t **a,int n,wtk_dcomplex_t **b)
{
	int i,j,k;
	wtk_dcomplex_t *pc,*pc2,*pc3,*pc4,*pc5,*pc6;
	double f,xa,xb,ta,tb;
    //wtk_debug("============ phr ==========\n");
   // wtk_dcomplex_p2_print2(a,n,n);

	j=sizeof(wtk_dcomplex_t)*n;
	for(i=0;i<n;++i)
	{
		memset(b[i],0,j);
		b[i][i].a=1;
	}
	for(k=0;k<n;++k)
	{
		pc5=a[k];
		pc6=b[k];
		ta=pc5[k].a;
		tb=pc5[k].b;
		//wtk_debug("ta=%f/%f\n",ta,tb);
		f=(ta*ta+tb*tb);
		if(f==0)
		{
			wtk_debug("found err\n");
			return -1;
		}
		f=1.0/f;
		ta*=f;
		tb*=f;
		//wtk_debug("%f/%f\n",pc7->a,pc7->b);
		//exit(0);
		for(i=0;i<n;++i)
		{
			if(i==k)
			{
				continue;
			}
			pc=a[i];
			pc2=b[i];
			pc3=a[i]+k;
			xa=(pc3->a*ta+pc3->b*tb);
			xb=(pc3->b*ta-pc3->a*tb);
			pc3=pc5;//a+k*c;
			pc4=pc6;//b+k*c;
            for(j=0;j<n;j++)
            {
            	//a=ratio b=pc3;
            	//wtk_complex_sub(a+i*c+j,wtk_complex_mul(&ratio,a+k*c+j));
            	pc->a-=xa*pc3->a - xb*pc3->b;
            	(pc++)->b-=xa*pc3->b + xb*pc3->a;
            	++pc3;

            	//wtk_complex_sub(b+i*c+j,wtk_complex_mul(&ratio,b+k*c+j));
            	pc2->a-=xa*pc4->a - xb*pc4->b;
            	(pc2++)->b-=xa*pc4->b + xb*pc4->a;
            	//++pc;++pc2;
            	++pc4;
            }
		}
	}
	for(i=0;i<n;++i)
	{
		//pc=a+i*c+i;
		pc=a[i]+i;
		ta=pc->a;
		tb=pc->b;
		f=(ta*ta+tb*tb);
		if(f==0)
		{
			wtk_debug("found err\n");
			return -1;
		}
		f=1.0/f;
		ta*=f;
		tb*=f;
		for(j=0,pc2=b[i];j<n;++j)
		{
			xa=pc2->a;
			xb=pc2->b;
			pc2->a=xa*ta+xb*tb;
			(pc2++)->b=xb*ta-xa*tb;;
		}
	}

    //wtk_debug("============ phr2 ==========\n");
   // wtk_dcomplex_p2_print2(b,n,n);
	return 0;
}

int wtk_dcomplex_lu_invx3(wtk_dcomplex_t **a,wtk_dcomplex_t **L,wtk_dcomplex_t **U,wtk_dcomplex_t **l,wtk_dcomplex_t **u,int n,wtk_dcomplex_t **b)
{
    int i,j,k;
    wtk_dcomplex_t *c1,*c2,*c3;
    float f,ta,tb,ta1,tb1;

    c3 = (wtk_dcomplex_t *)malloc(n*sizeof(wtk_dcomplex_t));
    for(i=0;i<n;++i)
    {
        memset(U[i],0,n);
        memset(L[i],0,n);
        L[i][i].a=1;
        memset(l[i],0,n);
        l[i][i].a=1;
        memset(u[i],0,n);
    }

    c1 = U[0];
    c2 = a[0];
    for(i=0;i<n;++i)
    {
        c1[i].a = c2[i].a;
        c1[i].b = c2[i].b;
    }
    ta = c1->a;
    tb = c2->b;
    f = ta*ta+tb*tb ;
    if(f==0)
    {
        wtk_debug("found err\n");
        return -1;
    }
    f=1.0/f;
    ta*=f;
    tb*=f;
    c3[0].a=ta;
    c3[0].b=-tb;
    for(i=1;i<n;++i)
    {
        L[i]->a = a[i]->a*ta+a[i]->b*tb;
        L[i]->b = a[i]->b*ta-a[i]->a*tb;
    }

    for(i=1;i<n;++i)
    {
        for(j=i;j<n;++j)
        {
            ta=0;
            tb=0;
            for(k=0;k<i;++k)
            {
                ta += L[i][k].a*U[k][j].a - L[i][k].b*U[k][j].b;
                tb += L[i][k].b*U[k][j].a + L[i][k].a*U[k][j].b;
            }
            U[i][j].a = a[i][j].a-ta;
            U[i][j].b = a[i][j].b-tb;
        }

        ta1 = U[i][i].a;
        tb1 = U[i][i].b;
        f = ta1*ta1+tb1*tb1;
        if(f==0)
        {
            wtk_debug("found err\n");
            return -1;
        }
        f=1.0/f;
        ta1*=f;
        tb1*=f;
        c3[i].a=ta1;
        c3[i].b=-tb1;

        for(j=i+1;j<n;++j)
        {
            ta=0;
            tb=0;
            for(k=0;k<i;++k)
            {
                ta +=L[j][k].a*U[k][i].a - L[j][k].b*U[k][i].b;
                tb +=L[j][k].b*U[k][i].a + L[j][k].a*U[k][i].b;
            }
            ta = a[j][i].a-ta;
            tb = a[j][i].b-tb;
            L[j][i].a = ta*ta1 + tb*tb1;
            L[j][i].b = tb*ta1 - ta*tb1;
        }
    }
    wtk_debug("L===\n");
    wtk_dcomplex_p2_print2(L,n,n);
    wtk_debug("U===\n");
    wtk_dcomplex_p2_print2(U,n,n);


    for(i=0;i<n;++i)
    {
        for(j=i+1;j<n;++j)
        {
            ta=0;
            tb=0;
            for(k=i;k<j;++k)
            {
                ta += l[k][i].a*L[j][k].a - l[k][i].b*L[j][k].b;
                tb += l[k][i].a*L[j][k].b + l[k][i].b*L[j][k].a;
            }
            l[j][i].a = -ta;
            l[j][i].b = -tb;
        }

        u[i][i].a=c3[i].a;
        u[i][i].b=c3[i].b;

        for(j=i-1;j>=0;--j)
        {
            ta1=0;
            tb1=0;
            for(k=j;k<i+1;++k)
            {
                ta1 += U[j][k].a*u[k][i].a - U[j][k].b*u[k][i].b;
                tb1 += U[j][k].a*u[k][i].b + U[j][k].b*u[k][i].a;
            }
            u[j][i].a = -(u[j][j].a*ta1 - u[j][j].b*tb1);
            u[j][i].b = -(u[j][j].a*tb1 + u[j][j].b*ta1);
        }

    }
    //wtk_debug("%f\n",u[0][0].a);
    wtk_debug("l===\n");
    wtk_dcomplex_p2_print2(l,n,n);
    wtk_debug("u===\n");
    wtk_dcomplex_p2_print2(u,n,n);

    wtk_dcomplex_matrix_mul(u,l,b,n,n,n);
    free(c3);
    return 0;
}



int wtk_complex_invx3(wtk_complex_t **a,int r,int c,wtk_complex_t **b)
{
	int i,j,k;
	wtk_complex_t *pc,*pc2,*pc3,*pc4,*pc5,*pc6,*pc7;
	double f;
	double xa,xb;
	double ta,tb;

	for(i=0;i<r;++i)
	{
		memset(b[i],0,sizeof(wtk_complex_t)*c);
		b[i][i].a=1;
	}
	for(k=0;k<r;++k)
	{
		pc5=a[k];
		pc6=b[k];
		pc7=pc5+k;//a+k*c+k;
		//f=1.0/(pc7->a*pc7->a+pc7->b*pc7->b);
		ta=pc7->a;
		tb=pc7->b;
		//wtk_debug("ta=%f/%f\n",ta,tb);
		f=(ta*ta+tb*tb);
		if(f==0)
		{
			return -1;
		}
		f=1.0/f;
		ta*=f;
		tb*=f;
		//wtk_debug("%f/%f\n",pc7->a,pc7->b);
		//exit(0);
		for(i=0;i<c;++i)
		{
			if(i==k)
			{
				continue;
			}
			pc=a[i];
			pc2=b[i];
			pc3=a[i]+k;
//			pc7=a+k*c+k;
//			f=1.0/(pc7->a*pc7->a+pc7->b*pc7->b);
			//xa=(pc3->a*pc7->a+pc3->b*pc7->b)*f;
			//xb=(pc3->b*pc7->a-pc3->a*pc7->b)*f;
			xa=(pc3->a*ta+pc3->b*tb);
			xb=(pc3->b*ta-pc3->a*tb);
			//ratio=wtk_complex_div(a+i*c+k,a+k*c+k);

			//wtk_debug("xa=%f xb=%f f=%f\n",xa,xb,f);
			//exit(0);
			//pc=a+i*c;
			//pc2=b+i*c;
			pc3=pc5;//a+k*c;
			pc4=pc6;//b+k*c;
			//pc3=pc5;//a+k*c;
			//pc4=pc6;//b+k*c;
            for(j=0;j<c;j++)
            {
            	//a=ratio b=pc3;
            	//wtk_complex_sub(a+i*c+j,wtk_complex_mul(&ratio,a+k*c+j));
            	pc->a-=xa*pc3->a - xb*pc3->b;
            	(pc++)->b-=xa*pc3->b + xb*pc3->a;
            	++pc3;

            	//wtk_complex_sub(b+i*c+j,wtk_complex_mul(&ratio,b+k*c+j));
            	pc2->a-=xa*pc4->a - xb*pc4->b;
            	(pc2++)->b-=xa*pc4->b + xb*pc4->a;
            	//++pc;++pc2;
            	++pc4;
            }
		}
	}
	for(i=0;i<r;++i)
	{
		//pc=a+i*c+i;
		pc=a[i]+i;
		ta=pc->a;
		tb=pc->b;
		f=(ta*ta+tb*tb);
		if(f==0)
		{
			return -1;
		}
		f=1.0/f;
		ta*=f;
		tb*=f;
		for(j=0,pc2=b[i];j<c;++j)
		{
			//b[k]=wtk_complex_div(b+k,pc);
			//b[i*c+j]=wtk_complex_div(b+i*c+j,a+i*c+i);
			//b[i*c+j]/=a[i*c+i];
			//xa=(pc2->a*pc->a+pc2->b*pc->b)*f;
			//xb=(pc2->b*pc->a-pc2->a*pc->b)*f;
			xa=(pc2->a*ta + pc2->b*tb);
			xb=(pc2->b*ta - pc2->a*tb);
			pc2->a=xa;
			(pc2++)->b=xb;
			//++pc2;
		}
	}
	return 0;
}

int wtk_complex_invx2(wtk_complex_t *a,int r,int c,wtk_complex_t *b)
{
	int i,j,k;
	wtk_complex_t *pc,*pc2,*pc3,*pc4,*pc5,*pc6,*pc7;
	double f;
	double xa,xb;
	double ta,tb;

	memset(b,0,sizeof(wtk_complex_t)*r*c);
	for(i=0,j=0;i<r;++i) //+=(r+1))
	{
		b[j].a=1;
		//b[j].b=0;
		j+=r+1;
	}
	pc5=a;
	pc6=b;
	for(k=0;k<r;++k,pc5+=c,pc6+=c)
	{
		pc=a;
		pc2=b;
		pc7=pc5+k;//a+k*c+k;
		//f=1.0/(pc7->a*pc7->a+pc7->b*pc7->b);
		ta=pc7->a;
		tb=pc7->b;
		//wtk_debug("ta=%f/%f\n",ta,tb);
		f=(ta*ta+tb*tb);
		if(f==0)
		{
			return -1;
		}
		f=1.0/f;
		ta*=f;
		tb*=f;
		//wtk_debug("%f/%f\n",pc7->a,pc7->b);
		//exit(0);
		for(i=0;i<c;++i)
		{
			if(i==k)
			{
				pc+=c;
				pc2+=c;
				continue;
			}
			pc3=a+i*c+k;
//			pc7=a+k*c+k;
//			f=1.0/(pc7->a*pc7->a+pc7->b*pc7->b);
			//xa=(pc3->a*pc7->a+pc3->b*pc7->b)*f;
			//xb=(pc3->b*pc7->a-pc3->a*pc7->b)*f;
			xa=(pc3->a*ta+pc3->b*tb);
			xb=(pc3->b*ta-pc3->a*tb);
			//ratio=wtk_complex_div(a+i*c+k,a+k*c+k);

			//wtk_debug("xa=%f xb=%f f=%f\n",xa,xb,f);
			//exit(0);
			//pc=a+i*c;
			//pc2=b+i*c;
			pc3=pc5;//a+k*c;
			pc4=pc6;//b+k*c;
			//pc3=pc5;//a+k*c;
			//pc4=pc6;//b+k*c;
            for(j=0;j<c;j++)
            {
            	//a=ratio b=pc3;
            	//wtk_complex_sub(a+i*c+j,wtk_complex_mul(&ratio,a+k*c+j));
            	pc->a-=xa*pc3->a - xb*pc3->b;
            	(pc++)->b-=xa*pc3->b + xb*pc3->a;
            	++pc3;

            	//wtk_complex_sub(b+i*c+j,wtk_complex_mul(&ratio,b+k*c+j));
            	pc2->a-=xa*pc4->a - xb*pc4->b;
            	(pc2++)->b-=xa*pc4->b + xb*pc4->a;
            	//++pc;++pc2;
            	++pc4;
            }
		}
	}
	for(i=0;i<r;++i)
	{
		pc=a+i*c+i;
		ta=pc->a;
		tb=pc->b;
		f=(ta*ta+tb*tb);
		if(f==0)
		{
			return -1;
		}
		f=1.0/f;
		ta*=f;
		tb*=f;
		for(j=0,pc2=b+i*c;j<c;++j)
		{
			//b[k]=wtk_complex_div(b+k,pc);
			//b[i*c+j]=wtk_complex_div(b+i*c+j,a+i*c+i);
			//b[i*c+j]/=a[i*c+i];
			//xa=(pc2->a*pc->a+pc2->b*pc->b)*f;
			//xb=(pc2->b*pc->a-pc2->a*pc->b)*f;
			xa=(pc2->a*ta + pc2->b*tb);
			xb=(pc2->b*ta - pc2->a*tb);
			pc2->a=xa;
			(pc2++)->b=xb;
			//++pc2;
		}
	}
	return 0;
}

int wtk_complex_invx(wtk_complex_t *a,int r,int c,wtk_complex_t *b,wtk_complex_t *at)
{
	int n;

	n=r*c*sizeof(wtk_complex_t);
	memcpy(at,a,n);
	return wtk_complex_invx2(at,r,c,b);
}

int wtk_dcomplex_invx2(wtk_dcomplex_t *a,int r,int c,wtk_dcomplex_t *b)
{
	int i,j,k;
	wtk_dcomplex_t *pc,*pc2,*pc3,*pc4,*pc5,*pc6,*pc7;
	double f;
	double xa,xb;
	double ta,tb;

	memset(b,0,sizeof(wtk_dcomplex_t)*r*c);
	for(i=0,j=0;i<r;++i) //+=(r+1))
	{
		b[j].a=1;
		//b[j].b=0;
		j+=r+1;
	}
	pc5=a;
	pc6=b;
	for(k=0;k<r;++k,pc5+=c,pc6+=c)
	{
		pc=a;
		pc2=b;
		pc7=pc5+k;//a+k*c+k;
		//f=1.0/(pc7->a*pc7->a+pc7->b*pc7->b);
		ta=pc7->a;
		tb=pc7->b;
		//wtk_debug("ta=%f/%f\n",ta,tb);
		f=(ta*ta+tb*tb);
		if(f==0)
		{
			return -1;
		}
		f=1.0/f;
		ta*=f;
		tb*=f;
		//wtk_debug("%f/%f\n",pc7->a,pc7->b);
		//exit(0);
		for(i=0;i<c;++i)
		{
			if(i==k)
			{
				pc+=c;
				pc2+=c;
				continue;
			}
			pc3=a+i*c+k;
//			pc7=a+k*c+k;
//			f=1.0/(pc7->a*pc7->a+pc7->b*pc7->b);
			//xa=(pc3->a*pc7->a+pc3->b*pc7->b)*f;
			//xb=(pc3->b*pc7->a-pc3->a*pc7->b)*f;
			xa=(pc3->a*ta+pc3->b*tb);
			xb=(pc3->b*ta-pc3->a*tb);
			//ratio=wtk_complex_div(a+i*c+k,a+k*c+k);

			//wtk_debug("xa=%f xb=%f f=%f\n",xa,xb,f);
			//exit(0);
			//pc=a+i*c;
			//pc2=b+i*c;
			pc3=pc5;//a+k*c;
			pc4=pc6;//b+k*c;
			//pc3=pc5;//a+k*c;
			//pc4=pc6;//b+k*c;
            for(j=0;j<c;j++)
            {
            	//a=ratio b=pc3;
            	//wtk_complex_sub(a+i*c+j,wtk_complex_mul(&ratio,a+k*c+j));
            	pc->a-=xa*pc3->a - xb*pc3->b;
            	(pc++)->b-=xa*pc3->b + xb*pc3->a;
            	++pc3;

            	//wtk_complex_sub(b+i*c+j,wtk_complex_mul(&ratio,b+k*c+j));
            	pc2->a-=xa*pc4->a - xb*pc4->b;
            	(pc2++)->b-=xa*pc4->b + xb*pc4->a;
            	//++pc;++pc2;
            	++pc4;
            }
		}
	}
	for(i=0;i<r;++i)
	{
		pc=a+i*c+i;
		ta=pc->a;
		tb=pc->b;
		f=(ta*ta+tb*tb);
		if(f==0)
		{
			return -1;
		}
		f=1.0/f;
		ta*=f;
		tb*=f;
		for(j=0,pc2=b+i*c;j<c;++j)
		{
			//b[k]=wtk_complex_div(b+k,pc);
			//b[i*c+j]=wtk_complex_div(b+i*c+j,a+i*c+i);
			//b[i*c+j]/=a[i*c+i];
			//xa=(pc2->a*pc->a+pc2->b*pc->b)*f;
			//xb=(pc2->b*pc->a-pc2->a*pc->b)*f;
			xa=(pc2->a*ta + pc2->b*tb);
			xb=(pc2->b*ta - pc2->a*tb);
			pc2->a=xa;
			(pc2++)->b=xb;
			//++pc2;
		}
	}
	return 0;
}

int wtk_dcomplex_invx(wtk_dcomplex_t *a,int r,int c,wtk_dcomplex_t *b,wtk_dcomplex_t *at)
{
	int n;

	n=r*c*sizeof(wtk_dcomplex_t);
	memcpy(at,a,n);
	return wtk_dcomplex_invx2(at,r,c,b);
}


void wtk_complex_det(wtk_complex_t *c,int n,char *bit,wtk_dcomplex_t *a,int step)
{
	wtk_dcomplex_t t;
	wtk_complex_t x;
	int i,j;
	int sign;

	if(step==n-1)
	{
		for(i=0;i<n;++i)
		{
			if(!bit[i])
			{
				//wtk_debug("step=%d i=%d\n",step,i);
				c=c+step*n+i;
				a->a=c->a;
				a->b=c->b;
				return;
			}
		}
		wtk_debug("found err\n");
		exit(0);
		return;
	}
	sign=1;
	a->a=0;
	a->b=0;
	j=0;
	for(i=step;i<n;++i)
	{
		while(bit[j])
		{
			++j;
		}
		//wtk_debug("j=%d\n",j);
		bit[j]=1;
		wtk_complex_det(c,n,bit,&t,step+1);
		x=c[step*n+j];
		//wtk_debug("%f+%fi %f+%fi\n",t.a,t.b,x.a,x.b);
    	if(sign>=0)
    	{
    		a->a+=x.a*t.a-x.b*t.b;
    		a->b+=x.b*t.a+x.a*t.b;
    	}else
    	{
    		a->a-=x.a*t.a-x.b*t.b;
    		a->b-=x.b*t.a+x.a*t.b;
    	}
		bit[j]=0;
		sign=-sign;
		++j;
	}
}

void wtk_dcomplex_det1(wtk_dcomplex_t *c, int n, wtk_dcomplex_t *a)
{
	int i,j,k,n1,n2,n3,mi,mj,flag;
	double mv,tmp,real,imag;
	wtk_dcomplex_t ctmp,v,v1;

	a->a=1;
	a->b=0;
	n1=n-1;
	flag=1;

	for(k=0;k<n1;++k)
	{
		mv = 0.0;
		mi=mj=k;
		for(i=k;i<n;++i)
		{
			n2=i*n+k;
			for(j=k;j<n;++j,++n2)
			{
				tmp=c[n2].a*c[n2].a+c[n2].b*c[n2].b;
				if(tmp>mv)
				{
					mv=tmp;
					mi=i;
					mj=j;
				}
			}
		}
		if(mv == 0.0)
		{
			a->a=a->b=0;
			return;
		}
		if(mi != k)
		{
			n2=k*n+k;
			n3=mi*n+k;
			for(i=k;i<n;++i,++n2,++n3)
			{
				ctmp.a=c[n2].a;
				ctmp.b=c[n2].b;
				c[n2].a=c[n3].a;
				c[n2].b=c[n3].b;
				c[n3].a=ctmp.a;
				c[n3].b=ctmp.b;
			}
			flag=-flag;
		}
		if(mj != k)
		{
			for(i=k;i<n;++i)
			{
				n2=n*i+k;
				n3=n*i+mj;
				ctmp.a=c[n2].a;
				ctmp.b=c[n2].b;
				c[n2].a=c[n3].a;
				c[n2].b=c[n3].b;
				c[n3].a=ctmp.a;
				c[n3].b=ctmp.b;
			}
			flag=-flag;
		}

		n2=k*n+k;
		mv=1.0/mv;
		v.a=c[n2].a*mv;
		v.b=-c[n2].b*mv;
		for(i=k+1;i<n;++i)
		{
			n3=i*n+k;
			real=v.a*c[n3].a-v.b*c[n3].b;
			imag=v.a*c[n3].b+v.b*c[n3].a;

			n2=k*n+k+1;
			n3=i*n+k+1;
			for(j=k+1;j<n;++j,++n2,++n3)
			{
				v1.a=real*c[n2].a-imag*c[n2].b;
				v1.b=real*c[n2].b+imag*c[n2].a;
				c[n3].a-=v1.a;
				c[n3].b-=v1.b;
			}
		}
		n2=k*n+k;
		real=a->a;
		imag=a->b;
		a->a=real*c[n2].a-imag*c[n2].b;
		a->b=real*c[n2].b+imag*c[n2].a;
	}

	n1=n*n-1;
	real=a->a;
	imag=a->b;
	a->a=flag*(real*c[n1].a-imag*c[n1].b);
	a->b=flag*(real*c[n1].b+imag*c[n1].a);
}

double wtk_complex_abs_det(wtk_complex_t *c,int n,char *bit)
{
	wtk_dcomplex_t a;
	//char *bit;

	memset(bit,0,n);
	//bit=wtk_calloc(n,sizeof(char));
	wtk_complex_det(c,n,bit,&a,0);
	//wtk_debug("%e+i%e\n",a.a,a.b);
	return sqrt(a.a*a.a+a.b*a.b);
}

double wtk_complex_abs_det1(wtk_complex_t *c,wtk_dcomplex_t *c1,int n)
{
	wtk_dcomplex_t a;
	int i,k;

	k=n*n;
	for(i=0;i<k;++i)
	{
		c1[i].a=c[i].a;
		c1[i].b=c[i].b;
	}
	wtk_dcomplex_det1(c1,n, &a);
	return sqrt(a.a*a.a+a.b*a.b);
}

void wtk_dcomplex_det(wtk_dcomplex_t *c,int n,char *bit,wtk_dcomplex_t *a,int step)
{
	wtk_dcomplex_t t;
	wtk_dcomplex_t x;
	int i,j;
	int sign;

	if(step==n-1)
	{
		for(i=0;i<n;++i)
		{
			if(!bit[i])
			{
				//wtk_debug("step=%d i=%d\n",step,i);
				c=c+step*n+i;
				a->a=c->a;
				a->b=c->b;
				return;
			}
		}
		wtk_debug("found err\n");
		exit(0);
		return;
	}
	sign=1;
	a->a=0;
	a->b=0;
	j=0;
	for(i=step;i<n;++i)
	{
		while(bit[j])
		{
			++j;
		}
		//wtk_debug("j=%d\n",j);
		bit[j]=1;
		wtk_dcomplex_det(c,n,bit,&t,step+1);
		x=c[step*n+j];
		//wtk_debug("%f+%fi %f+%fi\n",t.a,t.b,x.a,x.b);
    	if(sign>=0)
    	{
    		a->a+=x.a*t.a-x.b*t.b;
    		a->b+=x.b*t.a+x.a*t.b;
    	}else
    	{
    		a->a-=x.a*t.a-x.b*t.b;
    		a->b-=x.b*t.a+x.a*t.b;
    	}
		bit[j]=0;
		sign=-sign;
		++j;
	}
}

double wtk_dcomplex_abs_det(wtk_dcomplex_t *c,int n,char *bit)
{
	wtk_dcomplex_t a;
	//char *bit;

	memset(bit,0,n);
	//bit=wtk_calloc(n,sizeof(char));
	wtk_dcomplex_det(c,n,bit,&a,0);
	//wtk_debug("%e+i%e\n",a.a,a.b);
	return sqrt(a.a*a.a+a.b*a.b);
}

void wtk_complex_p2_print(wtk_complex_t **p,int n1,int n2)
{
	int i,j;

	for(i=0;i<n1;++i)
	{
		for(j=0;j<n2;++j)
		{
			wtk_debug("v[%d/%d]=%.12f+%.12fi\n",i,j,p[i][j].a,p[i][j].b);
//			if(j>0)
//			{
//				printf(" ");
//			}
//			printf("%f+%fi",p[i][j].a,p[i][j].b);
		}
		//printf("\n");
	}
}

void wtk_complex_p2_print2(wtk_complex_t **p,int n1,int n2)
{
	int i,j;
	double ta,tb;

	wtk_debug("================ n1=%d n2=%d ===============\n",n1,n2);
	ta=tb=0;
	for(i=0;i<n1;++i)
	{
		for(j=0;j<n2;++j)
		{
			//wtk_debug("v[%d/%d]=%f+%fi\n",i,j,p[i][j].a,p[i][j].b);
			if(j>0)
			{
				printf(" ");
			}
			//printf("%e",p[i][j].a);
			printf("%.4f",p[i][j].a);
			//printf("%.4f+%.4fi",p[i][j].a,p[i][j].b);
			ta+=p[i][j].a;
			tb+=p[i][j].b;
		}
		printf("\n");
	}
	wtk_debug("======== tot=%f+%fi ===========\n\n",ta,tb);
}


void wtk_complex_p2_write(wtk_complex_t **p,int n1,int n2,char *fn)
{
	FILE *f;
	int i;

	f=fopen(fn,"wb");
	for(i=0;i<n1;++i)
	{
		fwrite(p[i],n2*sizeof(wtk_complex_t),1,f);
	}
	fclose(f);
}


void wtk_dcomplex_p2_add_scale(wtk_dcomplex_t **dst,wtk_dcomplex_t **src,float alpha1,float alpha2,int n1,int n2)
{
	int i,j;
	wtk_dcomplex_t *pdst,*psrc;

	for(i=0;i<n1;++i)
	{
		pdst=dst[i];
		psrc=src[i];
		for(j=0;j<n2;++j)
		{
			pdst[j].a=psrc[j].a*alpha1+alpha2*pdst[j].a;
			pdst[j].b=psrc[j].b*alpha1+alpha2*pdst[j].b;
		}
	}
}

void wtk_dcomplex_p2_add_scale_c(wtk_complex_t **dst,wtk_complex_t **src,float alpha,int n1,int n2)
{
	int i,j;
	float alpha2=1-alpha;
	wtk_complex_t *pdst;
	wtk_complex_t *psrc;

	for(i=0;i<n1;++i)
	{
		pdst=dst[i];
		psrc=src[i];
		for(j=0;j<n2;++j)
		{
			pdst[j].a=psrc[j].a*alpha+alpha2*pdst[j].a;
			pdst[j].b=psrc[j].b*alpha+alpha2*pdst[j].b;
		}
	}
}

void wtk_dcomplex_p2_cpy(wtk_dcomplex_t **dst,wtk_dcomplex_t **src,int n1,int n2)
{
	int i;
	int n;

	n=n2*sizeof(wtk_dcomplex_t);
	for(i=0;i<n1;++i)
	{
		memcpy(dst[i],src[i],n);
	}
}


void wtk_dcomplex_delete_p3(wtk_dcomplex_t ***p3,int n1,int n2)
{
	int i,j;

	for(i=0;i<n1;++i)
	{
		for(j=0;j<n2;++j)
		{
			wtk_free(p3[i][j]);
		}
		wtk_free(p3[i]);
	}
	wtk_free(p3);
}


wtk_dcomplex_t*** wtk_dcomplex_new_p3(int n1,int n2,int n3)
{
	wtk_dcomplex_t ***c;
	int i,j;

	c=(wtk_dcomplex_t***)wtk_calloc(n1,sizeof(wtk_dcomplex_t***));
	for(i=0;i<n1;++i)
	{
		c[i]=(wtk_dcomplex_t**)wtk_calloc(n2,sizeof(wtk_dcomplex_t*));
		for(j=0;j<n2;++j)
		{
			c[i][j]=(wtk_dcomplex_t*)wtk_calloc(n3,sizeof(wtk_dcomplex_t));
		}
	}
	return c;
}

