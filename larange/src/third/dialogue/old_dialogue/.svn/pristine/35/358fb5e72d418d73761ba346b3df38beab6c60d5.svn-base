#include "wtk_math.h"


void ifft(float x[], float y[], int n) //数组x存储时域序列的实部，数组y存储时域序列的虚部,n代表N点FFT.
{

	int i, j, k, l, m, n1, n2;
	float c, c1, e, s, s1, t, tr, ti;

	for (j = 1, i = 1; i < 16; i++) //计算 i = log2N；变量j进行每次自乘2，当j=n，即算出i的值.
	{
		m = i;
		j = 2 * j;
		if ((j == n)) //此时判断j是否与n相等时，j与2乘，所以n输入时应该选为2的次方。
			break;

	}
	n1 = n - 1; //计算蝶形图的输入下标（码位倒读）
	for (j = 0, i = 0; i < n1; i++) {
		if (i < j) {

			tr = x[j];
			ti = y[j];
			x[j] = x[i];
			y[j] = y[i];
			x[i] = tr;
			y[i] = ti;

		}
		k = n / 2;
		while (k < (j + 1)) {

			j = j - k;
			k = k / 2;

		}
		j = j + k;

	}
	n1 = 1; //计算每一级的输出，l为某一级，i为同一级的不同群，使用同一内存（即位运算）
	for (l = 1; l <= m; l++) {

		n1 = 2 * n1;
		n2 = n1 / 2;
		e = PI / n2;
		c = 1.0;
		s = 0.0;
		c1 = cos(e);
		s1 = sin(e);
		for (j = 0; j < n2; j++) {
			for (i = j; i < n; i += n1) {
				k = i + n2;
				tr = c * x[k] - s * y[k];
				ti = c * y[k] + s * x[k];
				x[k] = x[i] - tr;
				y[k] = y[i] - ti;
				x[i] = x[i] + tr;
				y[i] = y[i] + ti;

			}
			t = c;
			c = c * c1 - s * s1;
			s = t * s1 + s * c1;
		}
	}
	for (i = 0; i < n; i++) {
		x[i] /= n;
		y[i] /= n;

	}
}


void wtk_vector_zero_mean_frame(wtk_vector_t* v)
{
	float sum,off;

	sum=0;
	wtk_vector_do_p(v,sum+=,);
	off=sum/wtk_vector_size(v);
	wtk_vector_do_p(v,,-=off);
}

void wtk_vector_pre_emphasise(wtk_vector_t* v,float k)
{
	int i;

	for(i=wtk_vector_size(v);i>=2;--i)
	{
		v[i]-=v[i-1]*k;
	}
	v[1]*=1.0-k;
}

wtk_vector_t* wtk_math_create_ham_window_h(wtk_heap_t *h,int frame_size)
{
	wtk_vector_t *v;
	float a;

	v=wtk_vector_new_h(h,frame_size);
	a=TPI/(frame_size-1);
	wtk_vector_do_i(v,,=0.54-0.46*cos(a*(i-1)));
	return v;
}


void wtk_math_init_hanning_window(float *f,int n)
{
	int i;
	float t1,t2;

	t1=2*PI/(n+1);
	//wtk_debug("n=%d\n",n);
	for(i=0,t2=t1;i<n;++i,t2+=t1)
	{
		//f[i]=0.5*(1-cos(2*PI*(i+1)/(n+1)));
		f[i]=0.5*(1-cos(t2));
		//wtk_debug("v[%d]=%.12f\n",i,sqrt(f[i]));
	}
	//exit(0);
}

float* wtk_math_create_hanning_window(int n)
{
	float *f;

	f=(float*)wtk_malloc(sizeof(float)*n);
	wtk_math_init_hanning_window(f,n);
	return f;
}

float* wtk_math_create_hanning_window2(int n)
{
	float *f;
	int i;

	f=(float*)wtk_malloc(sizeof(float)*n);
	//wtk_debug("n=%d\n",n);
	for(i=0;i<n;++i)
	{
		f[i]=0.5*(1-cos(2*PI*(i)/(n-1)));
		//wtk_debug("v[%d]=%.12f\n",i,sqrt(f[i]));
	}
	return f;
}

wtk_vector_t* wtk_math_create_ham_window(int frame_size)
{
	wtk_vector_t *v;
	float a;

	v=wtk_vector_new(frame_size);
	a=TPI/(frame_size-1);
	wtk_vector_do_i(v,,=0.54-0.46*cos(a*(i-1)));
	return v;
}

/*
 * win=sin((.5:wlen-.5)/wlen*pi).';
 */
float * wtk_math_create_sine_window(int win)
{
	float *f;
	int i;

	f=(float*)wtk_malloc(win*sizeof(float));
	for(i=0;i<win;++i)
	{
		f[i]=sin((0.5+i)*PI/(win));
	}
	//print_float(f,10);
	return f;
}

float* wtk_math_create_float_ham_window(int frame_size)
{
	float *v;
	float a;
	int i;

	v=(float*)wtk_malloc(frame_size*sizeof(float));
	a=2*PI/(frame_size-1);
	for(i=0;i<frame_size;++i)
	{
		//v[i]=0.54-0.46*cos(a*i);
		//v[i]=0.54-0.46*cos(2*PI*(i)/(frame_size));
		//v[i]=0.54-0.46*cos(2*PI*(i)/(frame_size-1));
		v[i]=0.54-0.46*cos(a*i);
	}
	return v;
}

void wtk_fft(wtk_vector_t* s, int invert)
{
   int ii,jj,n,nn,limit,m,j,inc,i;
   double wx,wr,wpr,wpi,wi,theta;
   double xre,xri,x;

   n=wtk_vector_size(s);
   nn=n / 2; j = 1;
   for (ii=1;ii<=nn;ii++) {
      i = 2 * ii - 1;
      if (j>i) {
         xre = s[j]; xri = s[j + 1];
         s[j] = s[i];  s[j + 1] = s[i + 1];
         s[i] = xre; s[i + 1] = xri;
      }
      m = n / 2;
      while (m >= 2  && j > m) {
         j -= m; m /= 2;
      }
      j += m;
   };
   limit = 2;
   while (limit < n) {
      inc = 2 * limit; theta = TPI / limit;
      if (invert) theta = -theta;
      x = sin(0.5 * theta);
      wpr = -2.0 * x * x; wpi = sin(theta);
      wr = 1.0; wi = 0.0;
      for (ii=1; ii<=limit/2; ii++) {
         m = 2 * ii - 1;
         for (jj = 0; jj<=(n - m) / inc;jj++) {
            i = m + jj * inc;
            j = i + limit;
            xre = wr * s[j] - wi * s[j + 1];
            xri = wr * s[j + 1] + wi * s[j];
            s[j] = s[i] - xre; s[j + 1] = s[i + 1] - xri;
            s[i] = s[i] + xre; s[i + 1] = s[i + 1] + xri;
         }
         wx = wr;
         wr = wr * wpr - wi * wpi + wr;
         wi = wi * wpr + wx * wpi + wi;
      }
      limit = inc;
   }
   if (invert)
      for (i = 1;i<=n;i++)
         s[i] = s[i] / nn;

}


void wtk_realft (wtk_vector_t* s)
{
   int n, n2, i, i1, i2, i3, i4;
   double xr1, xi1, xr2, xi2, wrs, wis;
   double yr, yi, yr2, yi2, yr0, theta, x;

   n=wtk_vector_size(s) / 2; n2 = n/2;
   theta = PI / n;
   wtk_fft(s, 0);
   //print_float(s+1,10);
   //exit(0);
   x = sin(0.5 * theta);
   yr2 = -2.0 * x * x;
   yi2 = sin(theta); yr = 1.0 + yr2; yi = yi2;
   for (i=2; i<=n2; i++) {
      i1 = i + i - 1;      i2 = i1 + 1;
      i3 = n + n + 3 - i2; i4 = i3 + 1;
      wrs = yr; wis = yi;
      xr1 = (s[i1] + s[i3])/2.0; xi1 = (s[i2] - s[i4])/2.0;
      xr2 = (s[i2] + s[i4])/2.0; xi2 = (s[i3] - s[i1])/2.0;
      s[i1] = xr1 + wrs * xr2 - wis * xi2;
      s[i2] = xi1 + wrs * xi2 + wis * xr2;
      s[i3] = xr1 - wrs * xr2 + wis * xi2;
      s[i4] = -xi1 + wrs * xi2 + wis * xr2;
      yr0 = yr;
      yr = yr * yr2 - yi  * yi2 + yr;
      yi = yi * yr2 + yr0 * yi2 + yi;
   }
   xr1 = s[1];
   s[1] = xr1 + s[2];
   s[2] = 0.0;
}

void wtk_math_do_diff(wtk_vector_t** pv,int window_size,double sigma,int start_pos,int step)
{
	int i,j,k,end=start_pos+step;
	wtk_vector_t *v=pv[window_size];
	wtk_vector_t *p,*n;
	int vs=start_pos-step;
	/*
	double sigma;

	sigma=0;
	for(i=1;i<=window_size;++i)
	{
		sigma+=i*i;
	}
	sigma*=2;
	*/
	for(i=1;i<=window_size;++i)
	{
		p=pv[window_size-i];
		n=pv[window_size+i];
		for(j=start_pos,k=vs;j<end;++j,++k)
		{
			if(i==1)
			{
				v[j]=(n[k]-p[k]);
			}else
			{
				v[j]+=i*(n[k]-p[k]);
			}
			if(i==window_size)
			{
				v[j]/=sigma;
			}
		}
	}
}

/**
 *	dt=(C{t+w}-C{t-w))/2w
 */
void wtk_math_do_simple_diff(wtk_vector_t** pv,int window_size,int start_pos,int step)
{
	int j,k,end=start_pos+step;
	wtk_vector_t *v=pv[window_size];
	wtk_vector_t *p,*n;
	int vs=start_pos-step;
	int dw;

	dw=window_size<<1;
	p=pv[0];
	n=pv[dw];
	for(j=start_pos,k=vs;j<end;++j,++k)
	{
		v[j]=(n[k]-p[k])/dw;
	}
}


int wtk_source_read_vector(wtk_source_t* s,wtk_vector_t* v,int bin)
{
	return wtk_source_read_float(s,v+1,wtk_vector_size(v),bin);
}

int wtk_source_read_matrix(wtk_source_t *s,wtk_matrix_t *m,int bin)
{
	int i,nrows,ret=0;

	//wtk_debug("row=%d col=%d\n",wtk_matrix_rows(m),wtk_matrix_cols(m));
	nrows=wtk_matrix_rows(m);
	for(i=1;i<=nrows;++i)
	{
		//wtk_debug("col=%d\n",wtk_vector_size(m[i]));
		ret=wtk_source_read_vector(s,m[i],bin);
		if(ret!=0)
		{
			wtk_debug("%d,sm=%d\n",ret,wtk_vector_size(m[i]));
			goto end;
		}
	}
end:
	return ret;
}

int wtk_source_read_hlda_bin(wtk_matrix_t **pm,wtk_source_t *s)
{
	wtk_matrix_t *m;
	int r_c[2];
	int ret;
	int i;

	//wtk_debug("read bin\n");
	s->swap=0;
	ret=wtk_source_read_int(s,r_c,2,1);
	if(ret!=0){goto end;}
	//wtk_debug("[%d/%d]\n",r_c[0],r_c[1]);
	//m=wtk_matrix_new(r_c[0],r_c[1]);
	m=wtk_matrix_new2(r_c[0],r_c[1]);
	//wtk_debug("[%d,%d]\n",wtk_matrix_rows(m),wtk_matrix_cols(m));
	for(i=1;i<=r_c[0];++i)
	{
		ret=wtk_source_read_vector(s,m[i],1);
		if(ret!=0){goto end;}
	}
	*pm=m;
	//wtk_matrix_print(m);
	//exit(0);
	ret=0;
end:
	//wtk_debug("ret=%d\n",ret);
	//exit(0);
	return ret;
}

int wtk_source_read_hlda(wtk_source_t *s,wtk_matrix_t **pm)
{
	wtk_strbuf_t *buf;
	int ret;
	int row_col[2];
	wtk_matrix_t *m=0;

	buf=wtk_strbuf_new(64,1);
	while(1)
	{
		ret=wtk_source_read_string(s,buf);
		if(ret!=0){goto end;}
		if(wtk_str_equal_s(buf->data,buf->pos,"<XFORM>"))
		{
			break;
		}
	}
	ret=wtk_source_read_int(s,row_col,2,0);
	if(ret!=0){goto end;}
	m=wtk_matrix_new(row_col[0],row_col[1]);
	ret=wtk_source_read_matrix(s,m,0);
	//wtk_matrix_print(m);
	//exit(0);
end:
	if(ret==0)
	{
		*pm=m;
	}else
	{
		if(m)
		{
			wtk_matrix_delete(m);
		}
	}
	wtk_strbuf_delete(buf);
	return ret;
}

int wtk_hlda_read(wtk_matrix_t **pm,wtk_source_t *s)
{
	return wtk_source_read_hlda(s,pm);
}


void wtk_matrix_multiply_vector(wtk_vector_t *dst,wtk_matrix_t *m,wtk_vector_t *src)
{
	int rows,cols,i,j;
	float vi;
	float *mi;

	rows=wtk_matrix_rows(m);
	cols=wtk_matrix_cols(m);
	for(i=1;i<=rows;++i)
	{
		vi=0;mi=m[i];
		for(j=1;j<=cols;++j)
		{
			vi+=mi[j]*src[j];
		}
		dst[i]=vi;
	}
}


int wtk_floatfix_q(float max,float min,int n)
{
	float f;
	int i;

	n-=1;
	for(i=0;i<n;++i)
	{
		f=pow(2,i);
		//wtk_debug("v[%d]=%f(%f/%f)\n",i,f,cfg->min,cfg->max);
		if(f>max && f>-min)
		{
			//wtk_debug("v[%d]=%f(%f/%f)\n",i,f,cfg->min,cfg->max);
			break;
		}
	}
	return n-i;
}

float wtk_float_sum(float *a,int n)
{
	int i;
	float f;

	f=0;
	for(i=0;i<n;++i)
	{
		f+=a[i];
	}
	return f;
}

void wtk_float_mul(float *a,int n,float f)
{
	int i;

	for(i=0;i<n;++i)
	{
		a[i]*=f;
	}
}

float wtk_short_sum(short *pv,int len)
{
	float t=0;
	int i;

	for(i=0;i<len;++i)
	{
		t+=pv[i];
	}
	return t;
}

float wtk_float_energy(float *a,int n)
{
	float f;
	int i;

	f=0;
	for(i=0;i<n;++i)
	{
		f+=a[i]*a[i];
	}
	return f/n;
}

float wtk_float_abs_max(float *a,int n)
{
	int i;
	float f,t;

	//f=fabs(a[0]);
	f=a[0]>0?a[0]:-a[0];
	for(i=1;i<n;++i)
	{
		t=a[i]>0?a[i]:-a[i];
		if(t>f)
		{
			f=t;
		}
	}
	return f;
}

void wtk_float_scale(float *a,register float f,float *c,int n)
{
	float *e,*e2;

	e=c+n;
	e2=c+((n>>3)<<3);
	while(c<e2)
	{
		c[0]=a[0]*f;
		c[1]=a[1]*f;
		c[2]=a[2]*f;
		c[3]=a[3]*f;
		c[4]=a[4]*f;
		c[5]=a[5]*f;
		c[6]=a[6]*f;
		c[7]=a[7]*f;
		c+=8;
		a+=8;
	}
	while(c<e)
	{
		*(c++)=(*(a++))*f;
	}
}

void wtk_short_scale(short *a,register float f,float *c,int n)
{
	float *e,*e2;

	e=c+n;
	e2=c+((n>>3)<<3);
	while(c<e2)
	{
		c[0]=a[0]*f;
		c[1]=a[1]*f;
		c[2]=a[2]*f;
		c[3]=a[3]*f;
		c[4]=a[4]*f;
		c[5]=a[5]*f;
		c[6]=a[6]*f;
		c[7]=a[7]*f;
		c+=8;
		a+=8;
	}
	while(c<e)
	{
		*(c++)=(*(a++))*f;
	}
}

void wtk_short_scale_add(short *a,register float f,float *c,int n)
{
	float *e,*e2;

	e=c+n;
	e2=c+((n>>3)<<3);
	while(c<e2)
	{
		c[0]+=a[0]*f;
		c[1]+=a[1]*f;
		c[2]+=a[2]*f;
		c[3]+=a[3]*f;
		c[4]+=a[4]*f;
		c[5]+=a[5]*f;
		c[6]+=a[6]*f;
		c[7]+=a[7]*f;
		c+=8;
		a+=8;
	}
	while(c<e)
	{
		*(c++)+=(*(a++))*f;
	}
}

void wtk_float_scale_add(float *a,register float f,float *c,int n)
{
	float *e,*e2;

	e=c+n;
	e2=c+((n>>3)<<3);
	while(c<e2)
	{
		c[0]+=a[0]*f;
		c[1]+=a[1]*f;
		c[2]+=a[2]*f;
		c[3]+=a[3]*f;
		c[4]+=a[4]*f;
		c[5]+=a[5]*f;
		c[6]+=a[6]*f;
		c[7]+=a[7]*f;
		c+=8;
		a+=8;
	}
	while(c<e)
	{
		*(c++)+=(*(a++))*f;
	}
}

//a*b=c;
void wtk_float_mult(float *a,float *b,float *c,int n)
{
	float *e,*e2;

	e=c+n;
	e2=c+((n>>3)<<3);
	while(c<e2)
	{
		c[0]=a[0]*b[0];
		c[1]=a[1]*b[1];
		c[2]=a[2]*b[2];
		c[3]=a[3]*b[3];
		c[4]=a[4]*b[4];
		c[5]=a[5]*b[5];
		c[6]=a[6]*b[6];
		c[7]=a[7]*b[7];
		c+=8;
		a+=8;
		b+=8;
	}
	while(c<e)
	{
		*(c++)=(*(a++))*(*(b++));
	}
}

//c+=a*b;
void wtk_float_mult_add(float *a,float *b,float *c,int n)
{
	float *e,*e2;

	e=c+n;
	e2=c+((n>>3)<<3);
	while(c<e2)
	{
		c[0]+=a[0]*b[0];
		c[1]+=a[1]*b[1];
		c[2]+=a[2]*b[2];
		c[3]+=a[3]*b[3];
		c[4]+=a[4]*b[4];
		c[5]+=a[5]*b[5];
		c[6]+=a[6]*b[6];
		c[7]+=a[7]*b[7];
		c+=8;
		a+=8;
		b+=8;
	}
	while(c<e)
	{
		*(c++)+=(*(a++))*(*(b++));
	}
}


void wtk_softmax(float* a,int len)
{
	float max,sum;
	float *p,*e;

	max=wtk_math_max(a,len);
	sum=0;
	p=a;e=p+len;
	while(p<e)
	{
		*p=expf(*p-max);
		sum+=*p;
		++p;
	}
	sum=1.0f/sum;
	p=a;e=p+len;
	while(p<e)
	{
		*p*=sum;
		++p;
	}
}

void wtk_sigmoid(float *f,int len)
{
	float *p;
	float *e;

	p=f;e=p+len;
	while(p<e)
	{
		*p=1.0/(1.0+expf(-*p));
		++p;
	}
}

