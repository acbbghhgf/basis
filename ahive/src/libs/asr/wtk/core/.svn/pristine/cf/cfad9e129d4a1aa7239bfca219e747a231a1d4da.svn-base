#include "wtk_rfft.h"
#include "math.h"

#ifdef PI
#undef PI
#endif
#define PI 3.1415926535897932384626433832795
#define SQRT2 1.41421356237309514547462185873883


int wtk_rfft_next_pow (long x)
{
	--x;

	int p = 0;
	while ((x & ~0xFFFFL) != 0)
	{
		p += 16;
		x >>= 16;
	}
	while ((x & ~0xFL) != 0)
	{
		p += 4;
		x >>= 4;
	}
	while (x > 0)
	{
		++p;
		x >>= 1;
	}

	return (p);
}

void wtk_rfft_init_br_lut(wtk_rfft_t *f)
{
	int len=1<<f->nbr_bits;
	int i,bit,idx;

	f->br_lut=(int*)wtk_malloc(len*sizeof(int));
	f->br_lut[0]=0;
	idx=0;
	for(i=1;i<len;++i)
	{
		bit=len>>1;
		while (((idx ^= bit) & bit) == 0)
		{
			bit >>= 1;
		}
		f->br_lut[i] = idx;
	}
}

int  wtk_rfft_get_trigo_level_index(int level)
{
	return ((1L << (level - 1)) - 4);
}


void wtk_rfft_init_trigo_lut(wtk_rfft_t *fft)
{
	int len;
	int level,i;
	float *fp;
	double f;

	if(fft->nbr_bits<=3)
	{
		return;
	}
	len=(1<<(fft->nbr_bits-1))-4;
	//wtk_debug("len=%d\n",len);
	fft->trigo_lut=(float*)wtk_malloc(len*sizeof(float));
	//wtk_debug("fp=%p/%p\n",fft->trigo_lut,fft->trigo_lut+len);
	for(level=3;level<fft->nbr_bits;++level)
	{
		len=1L << (level - 1);
		i=wtk_rfft_get_trigo_level_index(level);
		fp=fft->trigo_lut+i;
		//wtk_debug("i=%d fp=%p\n",i,fp);
		f=PI/(len<<1);
		for(i=0;i<len;++i)
		{
			fp[i]=cos(i*f);
		}
	}
}

void wtk_oscsincos_init(wtk_oscsincos_t *t)
{
	t->pos_cos=1;
	t->pos_sin=0;
	t->step_cos=1;
	t->step_sin=0;
}

void wtk_oscsincos_set_step(wtk_oscsincos_t *t,double angle_rad)
{
	t->step_cos=cos(angle_rad);
	t->step_sin=sin(angle_rad);
}

void wtk_oscsincos_step(wtk_oscsincos_t *t)
{
	float c,s;

	c=t->pos_cos;
	s=t->pos_sin;
	t->pos_cos=c*t->step_cos-s*t->step_sin;
	t->pos_sin=c*t->step_sin+s*t->step_cos;
}

void wtk_oscsincos_clean_buffers(wtk_oscsincos_t *t)
{
	t->pos_cos=1;
	t->pos_sin=0;
}


void wtk_rfft_init_trigo_osc(wtk_rfft_t *f)
{
	int nbr_osc=f->nbr_bits-TRIGO_BD_LIMIT;
	int i;
	wtk_oscsincos_t *t;
	int len;
	double mul;

	if(nbr_osc<=0){return;}
	f->trigo_osc=(wtk_oscsincos_t*)wtk_malloc(nbr_osc *sizeof(wtk_oscsincos_t));
	for(i=0;i<nbr_osc;++i)
	{
		t=f->trigo_osc+i;
		wtk_oscsincos_init(t);
		len = 1L << (TRIGO_BD_LIMIT + i);
		mul = (0.5 * PI) / len;
		wtk_oscsincos_set_step(t,mul);
	}
}



wtk_rfft_t* wtk_rfft_new(int len)
{
	wtk_rfft_t *f;

	f=(wtk_rfft_t*)wtk_malloc(sizeof(wtk_rfft_t));
	f->win=pow(2.0,(int)ceil(log(len)/log(2.0)));
	//f->win=pow(2.0,(int)(log(len)/log(2.0)+0.5));
	len=f->len=f->win*2;
	f->nbr_bits=wtk_rfft_next_pow(len);
	//wtk_debug("win=%d/%d bits=%d\n",f->win,f->len,f->nbr_bits);
	f->br_lut=NULL;
	f->trigo_lut=NULL;
	f->buffer=(float*)wtk_malloc(sizeof(float)*len);
	f->trigo_osc=NULL;
	wtk_rfft_init_br_lut(f);
	wtk_rfft_init_trigo_lut(f);
	wtk_rfft_init_trigo_osc(f);
	return f;
}

void wtk_rfft_delete(wtk_rfft_t *rf)
{
	if(rf->trigo_osc)
	{
		wtk_free(rf->trigo_osc);
	}
	if(rf->trigo_lut)
	{
		wtk_free(rf->trigo_lut);
	}
	if(rf->br_lut)
	{
		wtk_free(rf->br_lut);
	}
	wtk_free(rf->buffer);
	wtk_free(rf);
}

void wtk_rfft_compute_direct_pass_1_2(wtk_rfft_t *r,float *df,float *x)
{
	int *bit_rev_lut_ptr=r->br_lut;
	int coef_index=0;
	int rev_index_0,rev_index_1,rev_index_2,rev_index_3;
	float *df2;
	float sf_0,sf_2;

	do
	{
		rev_index_0 = bit_rev_lut_ptr [coef_index];
		rev_index_1 = bit_rev_lut_ptr [coef_index + 1];
		rev_index_2 = bit_rev_lut_ptr [coef_index + 2];
		rev_index_3 = bit_rev_lut_ptr [coef_index + 3];
		df2 = df + coef_index;
		df2 [1] = x [rev_index_0] - x [rev_index_1];
		df2 [3] = x [rev_index_2] - x [rev_index_3];

		sf_0 = x [rev_index_0] + x [rev_index_1];
		sf_2 = x [rev_index_2] + x [rev_index_3];

		df2 [0] = sf_0 + sf_2;
		df2 [2] = sf_0 - sf_2;

		coef_index += 4;
	}while(coef_index<r->len);

}

void wtk_rfft_compute_direct_pass_3(wtk_rfft_t *r,float *df,float *sf)
{
	double sqrt2_2=SQRT2 * 0.5;
	long				coef_index = 0;
	float v;
	do
	{
		df [coef_index] = sf [coef_index] + sf [coef_index + 4];
		df [coef_index + 4] = sf [coef_index] - sf [coef_index + 4];
		df [coef_index + 2] = sf [coef_index + 2];
		df [coef_index + 6] = sf [coef_index + 6];

		v = (sf [coef_index + 5] - sf [coef_index + 7]) * sqrt2_2;
		df [coef_index + 1] = sf [coef_index + 1] + v;
		df [coef_index + 3] = sf [coef_index + 1] - v;

		v = (sf [coef_index + 5] + sf [coef_index + 7]) * sqrt2_2;
		df [coef_index + 5] = v + sf [coef_index + 3];
		df [coef_index + 7] = v - sf [coef_index + 3];

		coef_index += 8;
	}
	while (coef_index < r->len);
}

void wtk_rfft_compute_direct_pass_n_lut2(wtk_rfft_t *r,float *df,float *sf,int pass)
{
	int nbr_coef = 1 << pass;
	int h_nbr_coef = nbr_coef >> 1;
	int d_nbr_coef = nbr_coef << 1;
	//int coef_index = 0;
	float *cos_ptr=r->trigo_lut+wtk_rfft_get_trigo_level_index(pass);
	float *sf1r,*sf2r,*dfr,*dfi;
	float *sf1i,*sf2i;
	float c,s,v1,v2,v3;
	int i;
	//int ki=0;
	float *sf1e;

	sf1r=sf;
	sf1e=sf1r+r->len;
	dfr=df;
	do
	{
		//++ki;
		sf2r=sf1r+nbr_coef;
		dfi=dfr+nbr_coef;

		// Extreme coefficients are always real
		dfr[0]=sf1r[0]+sf2r[0];
		dfr[h_nbr_coef]=sf1r[h_nbr_coef];
		dfi[0]=sf1r[0]-sf2r[0];
		dfi[h_nbr_coef]=sf2r[h_nbr_coef];

		sf1i = sf1r + h_nbr_coef;
		sf2i = sf1i + nbr_coef;
		for (i = 1; i < h_nbr_coef; ++ i)
		{
			c = cos_ptr [i];					// cos (i*PI/nbr_coef);
			s = cos_ptr [h_nbr_coef - i];	// sin (i*PI/nbr_coef);
			v1 = sf2r [i] * c - sf2i [i] * s;
			v2 = sf2r [i] * s + sf2i [i] * c;
			v3=sf1r[i];
			dfr [i] =v3+v1;// sf1r [i] + v1;
			dfi [-i] =v3-v1;// sf1r [i] - v1;	// dfr [nbr_coef - i] =

			v3=sf1i[i];
			dfi [i] =v2+v3;// v2 + sf1i [i];
			dfi [nbr_coef - i] =v2-v3;// v2 - sf1i [i];
		}
		sf1r+=d_nbr_coef;
		dfr+=d_nbr_coef;
	}while(sf1r<sf1e);
	//wtk_debug("ki=%d\n",ki);
	//print_float(df,256);
	//exit(0);
}

void wtk_rfft_compute_direct_pass_n_lut(wtk_rfft_t *r,float *df,float *sf,int pass)
{
	int nbr_coef = 1 << pass;
	int h_nbr_coef = nbr_coef >> 1;
	int d_nbr_coef = nbr_coef << 1;
	int coef_index = 0;
	float *cos_ptr=r->trigo_lut+wtk_rfft_get_trigo_level_index(pass);
	register float *sf1r,*sf2r,*dfr,*dfi;
	register float *sf1i,*sf2i;
	register float c,s,v1;//,v2;
	int i,j,k;

	do
	{
		sf1r = sf + coef_index;
		sf2r = sf1r + nbr_coef;
		dfr = df + coef_index;
		dfi = dfr + nbr_coef;

		// Extreme coefficients are always real
		dfr [0] = sf1r [0] + sf2r [0];
		dfr [h_nbr_coef] = sf1r [h_nbr_coef];
		dfi [0] = sf1r [0] - sf2r [0];	// dfr [nbr_coef] =
		dfi [h_nbr_coef] = sf2r [h_nbr_coef];

		// Others are conjugate complex numbers
		sf1i = sf1r + h_nbr_coef;
		sf2i = sf1i + nbr_coef;
		for (i = 1,j=h_nbr_coef-i,k=nbr_coef - i; i < h_nbr_coef; ++i,--j,--k)
		{
			c = cos_ptr [i];					// cos (i*PI/nbr_coef);
			s = cos_ptr [j];	// sin (i*PI/nbr_coef);
			v1 = sf2r [i] * c - sf2i [i] * s;

			dfr [i] = sf1r [i] + v1;
			dfi [-i] = sf1r [i] - v1;	// dfr [nbr_coef - i] =

			v1 = sf2r [i] * s + sf2i [i] * c;

			dfi [i] = v1 + sf1i [i];
			dfi [k] = v1 - sf1i [i];
		}
		coef_index += d_nbr_coef;
		//wtk_debug("d_nbr_coef=%d h_nbr_coef=%d nbr_coef=%d\n",d_nbr_coef,h_nbr_coef,nbr_coef);
		//exit(0);
	}
	while (coef_index < r->len);
}


void wtk_rfft_compute_direct_pass_n_lut3(wtk_rfft_t *r,float *df,float *sf,int pass)
{
	int nbr_coef = 1 << pass;
	int h_nbr_coef = nbr_coef >> 1;
	int d_nbr_coef = nbr_coef << 1;
	int coef_index = 0;
	float *cos_ptr=r->trigo_lut+wtk_rfft_get_trigo_level_index(pass);
	float *sf1r,*sf2r,*dfr,*dfi;
	float *sf1i,*sf2i;
	float c,s,v;
	int i;

	do
	{
		sf1r = sf + coef_index;
		sf2r = sf1r + nbr_coef;
		dfr = df + coef_index;
		dfi = dfr + nbr_coef;

		// Extreme coefficients are always real
		dfr [0] = sf1r [0] + sf2r [0];
		dfr [h_nbr_coef] = sf1r [h_nbr_coef];
		dfi [0] = sf1r [0] - sf2r [0];	// dfr [nbr_coef] =
		dfi [h_nbr_coef] = sf2r [h_nbr_coef];

		// Others are conjugate complex numbers
		sf1i = sf1r + h_nbr_coef;
		sf2i = sf1i + nbr_coef;
		for (i = 1; i < h_nbr_coef; ++ i)
		{
			c = cos_ptr [i];					// cos (i*PI/nbr_coef);
			s = cos_ptr [h_nbr_coef - i];	// sin (i*PI/nbr_coef);
			v = sf2r [i] * c - sf2i [i] * s;
			dfr [i] = sf1r [i] + v;
			dfi [-i] = sf1r [i] - v;	// dfr [nbr_coef - i] =

			v = sf2r [i] * s + sf2i [i] * c;
			dfi [i] = v + sf1i [i];
			dfi [nbr_coef - i] = v - sf1i [i];
		}

		coef_index += d_nbr_coef;
		//wtk_debug("d_nbr_coef=%d h_nbr_coef=%d nbr_coef=%d\n",d_nbr_coef,h_nbr_coef,nbr_coef);
		//exit(0);
	}
	while (coef_index < r->len);
}

void wtk_rfft_compute_direct_pass_n_osc(wtk_rfft_t *r,float *df,float *sf,int pass)
{
	const long		nbr_coef = 1 << pass;
	const long		h_nbr_coef = nbr_coef >> 1;
	const long		d_nbr_coef = nbr_coef << 1;
	long				coef_index = 0;
	wtk_oscsincos_t *osc=r->trigo_osc+pass - (TRIGO_BD_LIMIT + 1);
	float *sf1r,*sf2r,*dfr,*dfi;
	float *sf1i,*sf2i;
	float c,s,v;
	int i;

	do
	{
		sf1r = sf + coef_index;
		sf2r = sf1r + nbr_coef;
		dfr = df + coef_index;
		dfi = dfr + nbr_coef;

		wtk_oscsincos_clean_buffers(osc);

		// Extreme coefficients are always real
		dfr [0] = sf1r [0] + sf2r [0];
		dfi [0] = sf1r [0] - sf2r [0];	// dfr [nbr_coef] =
		dfr [h_nbr_coef] = sf1r [h_nbr_coef];
		dfi [h_nbr_coef] = sf2r [h_nbr_coef];

		// Others are conjugate complex numbers
		sf1i = sf1r + h_nbr_coef;
		sf2i = sf1i + nbr_coef;
		for (i = 1; i < h_nbr_coef; ++ i)
		{
			wtk_oscsincos_step(osc);
			c=osc->pos_cos;
			s=osc->pos_sin;

			v = sf2r [i] * c - sf2i [i] * s;
			dfr [i] = sf1r [i] + v;
			dfi [-i] = sf1r [i] - v;	// dfr [nbr_coef - i] =

			v = sf2r [i] * s + sf2i [i] * c;
			dfi [i] = v + sf1i [i];
			dfi [nbr_coef - i] = v - sf1i [i];
		}

		coef_index += d_nbr_coef;
	}
	while (coef_index < r->len);
}


void wtk_rfft_compute_direct_pass_n(wtk_rfft_t *rfft,float *df,float *sf,int pass)
{
	//wtk_debug("i=%d/%d\n",pass,TRIGO_BD_LIMIT);
	if (pass <= TRIGO_BD_LIMIT)
	{
		wtk_rfft_compute_direct_pass_n_lut (rfft,df, sf, pass);
	}
	else
	{
		//wtk_debug("...................\n");
		wtk_rfft_compute_direct_pass_n_osc (rfft,df, sf, pass);
	}
}


void wtk_rfft_compute_fft_general(wtk_rfft_t *rfft,float *f,float *x)
{
	float *sf,*df;
	int i;
	float *tf;

	if((rfft->nbr_bits&1)!=0)
	{
		//wtk_debug("-------------- 1 -----------\n");
		df=rfft->buffer;
		sf=f;
	}else
	{
		//wtk_debug("-------------- 2 -----------\n");
		df=f;
		sf=rfft->buffer;
	}
	//exit(0);
	wtk_rfft_compute_direct_pass_1_2(rfft,df,x);
	//wtk_debug("%f/%f/%f\n",df[0],df[16],df[31]);
	wtk_rfft_compute_direct_pass_3(rfft,sf,df);
	//wtk_debug("%f/%f/%f\n",sf[0],sf[16],sf[31]);

	for(i=3;i<rfft->nbr_bits;++i)
	{
		wtk_rfft_compute_direct_pass_n(rfft,df,sf,i);
		//wtk_debug("v[%d]=%f/%f/%f\n",i,f[0],f[rfft->len/2],f[rfft->len-1]);
//		if(i==4)
//		{
//			exit(0);
//		}
		tf=df;
		df=sf;
		sf=tf;
	}
	//exit(0);
}


/*
==============================================================================
Name: do_fft
Description:
	Compute the FFT of the array.
Input parameters:
	- x: pointer on the source array (time).
Output parameters:
	- f: pointer on the destination array (frequencies).
		f [0...length(x)/2] = real values,
		f [length(x)/2+1...length(x)-1] = negative imaginary values of
		coefficents 1...length(x)/2-1.
Throws: Nothing

for comples value(win):
0: v[0] 0
1: v[1] v[1+win]
2: v[2] v[2+win]
..
win: v[win] 0
win+1 v[win-1] -v[win-1]
==============================================================================
*/
void wtk_rfft_process_fft(wtk_rfft_t *rf,float *f,float *x)
{
	float b_0,b_2;

	//wtk_debug("nbits=%d len=%d\n",rf->nbr_bits,rf->len);
	if(rf->nbr_bits>2)
	{
		wtk_rfft_compute_fft_general(rf,f,x);
	}else if(rf->nbr_bits==2)
	{
		f [1] = x [0] - x [2];
		f [3] = x [1] - x [3];

		b_0 = x [0] + x [2];
		b_2 = x [1] + x [3];

		f [0] = b_0 + b_2;
		f [2] = b_0 - b_2;
	}else if(rf->nbr_bits==1)
	{
		f [0] = x [0] + x [1];
		f [1] = x [0] - x [1];
	}else
	{
		f[0]=x[0];
	}
}

void wtk_rfft_compute_inverse_pass_n_lut(wtk_rfft_t *rfft,float *df,float *sf,int pass)
{
	const long		nbr_coef = 1 << pass;
	const long		h_nbr_coef = nbr_coef >> 1;
	const long		d_nbr_coef = nbr_coef << 1;
	long				coef_index = 0;
	float *cos_ptr=rfft->trigo_lut+wtk_rfft_get_trigo_level_index(pass);
	float *sfr,*sfi,*df1r,*df2r;
	float *df1i,*df2i;
	float c,s,vr,vi;
	int i;

	do
	{
		sfr = sf + coef_index;
		sfi = sfr + nbr_coef;
		df1r = df + coef_index;
		df2r = df1r + nbr_coef;

		// Extreme coefficients are always real
		df1r [0] = sfr [0] + sfi [0];		// + sfr [nbr_coef]
		df2r [0] = sfr [0] - sfi [0];		// - sfr [nbr_coef]
		df1r [h_nbr_coef] = sfr [h_nbr_coef] * 2;
		df2r [h_nbr_coef] = sfi [h_nbr_coef] * 2;

		// Others are conjugate complex numbers
		df1i = df1r + h_nbr_coef;
		df2i = df1i + nbr_coef;
		for (i = 1; i < h_nbr_coef; ++ i)
		{
			df1r [i] = sfr [i] + sfi [-i];		// + sfr [nbr_coef - i]
			df1i [i] = sfi [i] - sfi [nbr_coef - i];

			c = cos_ptr [i];					// cos (i*PI/nbr_coef);
			s = cos_ptr [h_nbr_coef - i];	// sin (i*PI/nbr_coef);
			vr = sfr [i] - sfi [-i];		// - sfr [nbr_coef - i]
			vi = sfi [i] + sfi [nbr_coef - i];

			df2r [i] = vr * c + vi * s;
			df2i [i] = vi * c - vr * s;
		}

		coef_index += d_nbr_coef;
	}
	while (coef_index < rfft->len);
}

void wtk_rfft_compute_inverse_pass_n_osc(wtk_rfft_t *rfft,float *df,float *sf,int pass)
{
	int nbr_coef = 1 << pass;
	int h_nbr_coef = nbr_coef >> 1;
	int d_nbr_coef = nbr_coef << 1;
	int coef_index = 0;
	wtk_oscsincos_t *osc=rfft->trigo_osc+pass - (TRIGO_BD_LIMIT + 1);
	float *sfr,*sfi,*df1r,*df2r;
	float *df1i,*df2i;
	float c,s,vr,vi;
	int i;

	do
	{
		sfr = sf + coef_index;
		sfi = sfr + nbr_coef;
		df1r = df + coef_index;
		df2r = df1r + nbr_coef;

		wtk_oscsincos_clean_buffers(osc);

		// Extreme coefficients are always real
		df1r [0] = sfr [0] + sfi [0];		// + sfr [nbr_coef]
		df2r [0] = sfr [0] - sfi [0];		// - sfr [nbr_coef]
		df1r [h_nbr_coef] = sfr [h_nbr_coef] * 2;
		df2r [h_nbr_coef] = sfi [h_nbr_coef] * 2;

		// Others are conjugate complex numbers
		df1i = df1r + h_nbr_coef;
		df2i = df1i + nbr_coef;
		for (i = 1; i < h_nbr_coef; ++ i)
		{
			df1r [i] = sfr [i] + sfi [-i];		// + sfr [nbr_coef - i]
			df1i [i] = sfi [i] - sfi [nbr_coef - i];

			wtk_oscsincos_step(osc);
			c=osc->pos_cos;
			s=osc->pos_sin;
			vr = sfr [i] - sfi [-i];		// - sfr [nbr_coef - i]
			vi = sfi [i] + sfi [nbr_coef - i];

			df2r [i] = vr * c + vi * s;
			df2i [i] = vi * c - vr * s;
		}

		coef_index += d_nbr_coef;
	}
	while (coef_index < rfft->len);
}

void  wtk_rfft_compute_inverse_pass_3(wtk_rfft_t *rfft,float *df,float *sf)
{
	float sqrt2_2=SQRT2 * 0.5;
	int	 coef_index = 0;
	float vr,vi;
	do
	{
		df [coef_index] = sf [coef_index] + sf [coef_index + 4];
		df [coef_index + 4] = sf [coef_index] - sf [coef_index + 4];
		df [coef_index + 2] = sf [coef_index + 2] * 2;
		df [coef_index + 6] = sf [coef_index + 6] * 2;

		df [coef_index + 1] = sf [coef_index + 1] + sf [coef_index + 3];
		df [coef_index + 3] = sf [coef_index + 5] - sf [coef_index + 7];

		vr = sf [coef_index + 1] - sf [coef_index + 3];
		vi = sf [coef_index + 5] + sf [coef_index + 7];

		df [coef_index + 5] = (vr + vi) * sqrt2_2;
		df [coef_index + 7] = (vi - vr) * sqrt2_2;

		coef_index += 8;
	}
	while (coef_index < rfft->len);
}

void wtk_rfft_compute_inverse_pass_1_2(wtk_rfft_t *rfft,float *x,float *sf)
{
	int *	bit_rev_lut_ptr =rfft->br_lut;
	float *	sf2 = sf;
	int	coef_index = 0;
	float b_0,b_2,b_1,b_3;

	do
	{
		{
			b_0 = sf2 [0] + sf2 [2];
			b_2 = sf2 [0] - sf2 [2];
			b_1 = sf2 [1] * 2;
			b_3 = sf2 [3] * 2;

			x [bit_rev_lut_ptr [0]] = b_0 + b_1;
			x [bit_rev_lut_ptr [1]] = b_0 - b_1;
			x [bit_rev_lut_ptr [2]] = b_2 + b_3;
			x [bit_rev_lut_ptr [3]] = b_2 - b_3;
		}
		{
			b_0 = sf2 [4] + sf2 [6];
			b_2 = sf2 [4] - sf2 [6];
			b_1 = sf2 [5] * 2;
			b_3 = sf2 [7] * 2;

			x [bit_rev_lut_ptr [4]] = b_0 + b_1;
			x [bit_rev_lut_ptr [5]] = b_0 - b_1;
			x [bit_rev_lut_ptr [6]] = b_2 + b_3;
			x [bit_rev_lut_ptr [7]] = b_2 - b_3;
		}

		sf2 += 8;
		coef_index += 8;
		bit_rev_lut_ptr += 8;
	}
	while (coef_index < rfft->len);
}




void wtk_rfft_compute_inverse_pass_n(wtk_rfft_t *rfft,float *df,float *sf,int pass)
{
	if (pass <= TRIGO_BD_LIMIT)
	{
		wtk_rfft_compute_inverse_pass_n_lut (rfft,df, sf, pass);
	}
	else
	{
		wtk_rfft_compute_inverse_pass_n_osc (rfft,df, sf, pass);
	}
}


void wtk_rfft_compute_ifft_general(wtk_rfft_t *rfft,float *f,float *x)
{
	float *sf=f;
	float *df;
	float *df_temp;
	int pass;
	float *temp_ptr;

	if (rfft->nbr_bits & 1)
	{
		df = rfft->buffer;
		df_temp = x;
	}else
	{
		df = x;
		df_temp = rfft->buffer;
	}
	for (pass = rfft->nbr_bits - 1; pass >= 3; -- pass)
	{
		wtk_rfft_compute_inverse_pass_n (rfft,df, sf, pass);

		if (pass < rfft->nbr_bits - 1)
		{
			temp_ptr = df;
			df = sf;
			sf = temp_ptr;
		}
		else
		{
			sf = df;
			df = df_temp;
		}
	}
	wtk_rfft_compute_inverse_pass_3 (rfft,df, sf);
	wtk_rfft_compute_inverse_pass_1_2 (rfft,x, df);
}


/*
==============================================================================
Name: do_ifft
Description:
	Compute the inverse FFT of the array. Note that data must be post-scaled:
	IFFT (FFT (x)) = x * length (x).
Input parameters:
	- f: pointer on the source array (frequencies).
		f [0...length(x)/2] = real values
		f [length(x)/2+1...length(x)-1] = negative imaginary values of
		coefficents 1...length(x)/2-1.
Output parameters:
	- x: pointer on the destination array (time).
Throws: Nothing
==============================================================================
*/
void wtk_rfft_process_ifft(wtk_rfft_t *rf,float *f,float *x)
{
#define USE_X
	float b_0,b_2;
#ifdef USE_X
	register float t;
	register float *p1;
	float *pe,*pe2;
#else
	int i;
#endif

	// General case
	if (rf->nbr_bits > 2)
	{
		wtk_rfft_compute_ifft_general(rf,f, x);
#ifdef USE_X
		t=1.0/rf->len;
		p1=x;
		pe=p1+rf->len;
		pe2=p1+((rf->len>>3)<<3);
		while(p1<pe2)
		{
			p1[0]*=t;
			p1[1]*=t;
			p1[2]*=t;
			p1[3]*=t;

			p1[4]*=t;
			p1[5]*=t;
			p1[6]*=t;
			p1[7]*=t;

			p1+=8;
		}
		while(p1<pe)
		{
			*(p1++)*=t;
		}
#else
		for(i=0;i<rf->len;++i)
		{
			x[i]/=rf->len;
		}
#endif
	}
	// 4-point IFFT
	else if (rf->nbr_bits == 2)
	{
		b_0 = f [0] + f [2];
		b_2 = f [0] - f [2];

		x [0] = (b_0 + f [1] * 2)/4;
		x [2] = (b_0 - f [1] * 2)/4;
		x [1] = (b_2 + f [3] * 2)/4;
		x [3] = (b_2 - f [3] * 2)/4;
	}

	// 2-point IFFT
	else if (rf->nbr_bits == 1)
	{
		x [0] = (f [0] + f [1])/2;
		x [1] = (f [0] - f [1])/2;
	}

	// 1-point IFFT
	else
	{
		x [0] = f [0];
	}

}



//fft -ref_fft
void wtk_xcorr_freq_mult(float *fft1,float *fft2,float *xy,int fft_size)
{
	int i;
	float t;
	int nx;
	float x1,x2;

	nx=fft_size<<1;
	x1=fft1[0]*fft2[0];
	t=abs(x1);
	if(t==0){t=1;}
	xy[0]=x1/(t*nx);
	for(i=1;i<fft_size;i++)
	{
		//do the multiplication: (a+jb)(c+jd)* = (a+jb)*(c-jd)=(ac+bd)+j(bc-ad);
		x1=fft1[i]*fft2[i]+fft1[i+fft_size]*fft2[i+fft_size];
		x2=fft1[i+fft_size]*fft2[i]-fft1[i]*fft2[i+fft_size];
		t=sqrt(x1*x1+x2*x2);
		if(t==0){t=1;}
		t=1.0/(t*nx);
		xy[i]=x1*t;///(t*nx);
		xy[i+fft_size]=x2*t;///(t*nx);
	}
	x1=fft1[fft_size]*fft2[fft_size];
	t=abs(x1);
	if(t==0){t=1;}
	xy[fft_size]=x1/(t*nx);
}

int  wtk_xcorr_find_best_value(float *xcorr_value,int nx,int margin)
{
	int i,j;
	float f;
	float max=-1e10;
	int idx=0;

	//vs=wtk_larray_new(n/4,sizeof(float));
	//ps=wtk_larray_new(n/4,sizeof(int));
	//finding the maximums
	//I don't consider the delay=0 a maximum, as it has a discontinuity always.
	f=xcorr_value[nx-1];
	if(f>xcorr_value[nx-2] && f>xcorr_value[0])
	{
		idx=margin;
		//wtk_debug("v[%d]=%f\n",x,f);
		max=f;
	}
	for(i=nx-margin-1,j=0;i<(nx-1);++i,++j)
	{
		f=xcorr_value[i];
		//wtk_debug("v[%d]=%f\n",i,f);
		if(f>xcorr_value[i-1] && f>xcorr_value[i+1] && f>max)
		{
			//wtk_debug("v[%d]=%f\n",j,f);
			idx=j;
			max=f;
		}
	}
	f=xcorr_value[0];
	if(f>xcorr_value[1] && f>xcorr_value[nx-1] && f>max)
	{
		idx=margin+1;
		max=f;
		//wtk_debug("v[%d]=%f\n",x,f);
	}
	for(i=1;i<margin;++i)
	{
		f=xcorr_value[i];
		if(f>xcorr_value[i-1] && f>xcorr_value[i+1] && f>max)
		{
			idx=i+margin+1;
			max=f;
		}
	}
	idx-=(margin+1);
	//wtk_debug("idx=%d max=%f\n",idx,max);
	//exit(0);
	//wtk_debug("max=%f idx=%d\n",max,idx);
	return idx;
}

void wtk_xcorr_find_nbest_values(wtk_larray_t *vs,wtk_larray_t *ps,float *xcorr_value,int nx,int margin,int *delays,float *values,int nbest)
{
	int win=5;
	int i,j,k=-1;
	int n;
	float f;
	int v=-1;
	int b;
	//wtk_larray_t *vs;
	//wtk_larray_t *ps;
	float *pvs;
	int *pps;
	int x;

	n=2*margin-1;
	wtk_larray_reset(vs);
	wtk_larray_reset(ps);
	//vs=wtk_larray_new(n/4,sizeof(float));
	//ps=wtk_larray_new(n/4,sizeof(int));
	//finding the maximums
	//I don't consider the delay=0 a maximum, as it has a discontinuity always.
	f=xcorr_value[nx-1];
	if(f>xcorr_value[nx-2] && f>xcorr_value[0])
	{
		wtk_larray_push2(vs,&(f));
		x=margin;
		wtk_larray_push2(ps,&(x));
		//wtk_debug("v[%d]=%f\n",0,f);
	}
	for(i=nx-margin-1,j=0;i<(nx-1);++i,++j)
	{
		f=xcorr_value[i];
		//wtk_debug("v[%d]=%f\n",i,f);
		if(f>xcorr_value[i-1] && f>xcorr_value[i+1])
		{
			wtk_larray_push2(vs,&(f));
			wtk_larray_push2(ps,&(j));
			//wtk_debug("v[%d]=%f\n",j-(margin+1),f);
		}
	}
	f=xcorr_value[0];
	if(f>xcorr_value[1] && f>xcorr_value[nx-1])
	{
		wtk_larray_push2(vs,&(f));
		x=margin+1;
		wtk_larray_push2(ps,&(x));
		//wtk_debug("v[%d]=%f\n",0,f);
	}
	for(i=1;i<margin;++i)
	{
		f=xcorr_value[i];
		if(f>xcorr_value[i-1] && f>xcorr_value[i+1])
		{
			wtk_larray_push2(vs,&(f));
			x=i+margin+1;
			wtk_larray_push2(ps,&(x));
			//wtk_debug("v[%d]=%f\n",i,f);
		}
	}
	//wtk_debug("n=%d/%d\n",n,n/4);
	n=vs->nslot;
	pvs=(float*)vs->slot;
	pps=(int*)ps->slot;
	//wtk_debug("max=%d n=%d\n",max,n);
	delays[0]=margin+1;
	if(values)
	{
		values[0]=0;
	}
	for(i=0;i<nbest;++i)
	{
		f=-1;
		for(j=0;j<n;++j)
		{
			if(pvs[j]>f)
			{
				f=pvs[j];
				v=pps[j];
				k=j;
			}
		}
		//wtk_debug("f=%f v=%d\n",f,v);
		if(f!=-1)
		{
			x=1;
			for(j=0;j<i;++j)
			{
				b=v-delays[j];
				if(b<win && b>-win)
				{
					x=0;
					pvs[k]=-1;
				}
			}
			//wtk_debug("x=%d\n",x);
			if(x==0)
			{
				--i;
			}else
			{
				delays[i]=v;
				if(values)
				{
					values[i]=f;
				}
				pvs[k]=-1;
			}
		}else
		{
			  //we didn't find any more maximums, we repeat the first delay and value (it's a way to put null)
			  delays[i]=delays[0];
			  if(values)
			  {
				  values[i]=values[0];
			  }
		}
	}
	for(j=0;j<nbest;++j)
	{
		//wtk_debug("v[%d]=%d\n",i,delays[j]);
		delays[j]-=margin+1;
		//wtk_debug("v[%d/%d]=%d/%f\n",j,max,delays[j],values[j]);
	}
	//wtk_larray_delete(vs);
	//wtk_larray_delete(ps);
}

int wtk_rfft_xcorr2(char *mic,int mic_len,char *sp,int sp_len)
{
	float f;

	f=wtk_rfft_xcorr3(mic,mic_len,sp,sp_len,800,NULL);
	if(f>0)
	{
		return (int)(f+0.5);
	}else
	{
		return (int)(f-0.5);
	}
}

void wtk_rfft_find_max_value2(float x1,float y1,float x2,float y2,float x3,float y3)
{
	float a,b,c;
	float idx,v;

	a=(y1-y2-(x1-x2)*(y1-y3)/(x1-x3))/(x1*x1-x2*x2-(x1-x2)*(x1+x3));
	b=((y1-y3)-a*(x1*x1-x3*x3))/(x1-x3);
	c=y1-a*x1*x1-b*x1;
	wtk_debug("a=%f b=%f c=%f\n",a,b,c);
	wtk_debug("%f/%f/%f\n",a*x1*x1+b*x1+c,a*x2*x2+b*x2+c,a*x3*x3+b*x3+c);
	idx=-b/(2*a);
	v=c-b*b/(4*a);
	wtk_debug("v[%f]=%f/%f\n",idx,v,a*idx*idx+b*idx+c);
	exit(0);
}

float wtk_rfft_find_max_value(float x1,float y1,float x2,float y2,float x3,float y3,float *max_v)
{
	float a,b,c;
	float idx;

	a=(y1-y2-(x1-x2)*(y1-y3)/(x1-x3))/(x1*x1-x2*x2-(x1-x2)*(x1+x3));
	b=((y1-y3)-a*(x1*x1-x3*x3))/(x1-x3);
	c=y1-a*x1*x1-b*x1;
	idx=-b/(2*a);
	if(max_v)
	{
		*max_v=c-b*b/(4*a);
	}
	//wtk_debug("v[%f]=%f\n",idx,v);
	return idx;
}

float wtk_rfft_find_best_value(float *x1,int len,int margin,float *max_v)
{
	int idx;
	float ft;
	int i;
	int win=len/2;
	float fidx;
	int j=0;

	if(margin>win)
	{
		margin=win;
	}
	//wtk_debug("margin=%d\n",margin);
	idx=0;
	ft=x1[0];
	for(i=1;i<margin;++i)
	{
		if(x1[i]>ft)
		{
			idx=i;
			ft=x1[i];
		}
	}
	//print_float(x1,20);
	//print_float(x1+len-20,20);
	//wtk_debug("idx=%d %e\n", idx,ft);
	for(i=len-margin,j=-margin;i<len;++i,++j)
	{
		if(x1[i]>ft)
		{
			idx=j;
			ft=x1[i];
		}
	}
	//wtk_debug("idx=%d %e\n", idx,ft);
//	wtk_debug("idx=%d %e\n",idx,ft);
//	wtk_debug("margin=%d\n",margin);
//	print_float(x1,10);
//	print_float(x1+rfft->len-10,10);
	//wtk_debug("idx[%d]=%f\n",idx,ft);
	//wtk_debug("idx=%d f=%f len=%d/%d\n",idx,ft,rfft->win,rfft->len);
//	if(len>3)
//	{
//		//寻找中间值
//		if(idx>win)
//		{
//			v[0]=idx-len;
//			v[1]=x1[idx];
//			v[2]=idx-1-len;
//			v[3]=x1[idx-1];
//			if((idx+1)<len)
//			{
//				v[4]=idx+1-len;
//				v[5]=x1[idx+1];
//			}else
//			{
//				v[4]=idx-2-len;
//				v[5]=x1[idx-2];
//			}
//			fidx=wtk_rfft_find_max_value(v[0],v[1],v[2],v[3],v[4],v[5],max_v);
//			//fidx-=rfft->len;
//		}else
//		{
//			v[0]=idx;
//			v[1]=x1[idx];
//			wtk_debug("%f=%e\n",v[0],v[1]);
//			if(idx<=0)
//			{
//				v[2]=idx+2;
//				v[3]=x1[idx+2];
//			}else
//			{
//				v[2]=idx-1;
//				v[3]=x1[idx-1];
//			}
//			v[4]=idx+1;
//			v[5]=x1[idx+1];
//			fidx=wtk_rfft_find_max_value(v[0],v[1],v[2],v[3],v[4],v[5],max_v);
//		}
//		wtk_debug("fidx=%f\n",fidx);
//	}else
	{
		if(max_v)
		{
			*max_v=ft;//x1[idx];
		}
		fidx=idx;
	}
	return fidx;
}

float wtk_rfft_xcorr3(char *mic,int mic_len,char *sp,int sp_len,int margin,float *max_v)
{
	int len;
	wtk_rfft_t *rfft;
	float *x1;
	float *x2;
	float *x3;
	int i;
	short *smc,*ssp;
	float ft;
	double x;
	float fidx;

	//wtk_debug("mic=%d sp=%d\n",mic_len,sp_len);
	len=min(mic_len/2,sp_len/2);
	rfft=wtk_rfft_new(len);
	smc=(short*)mic;
	ssp=(short*)sp;
	x1=(float*)wtk_calloc(rfft->len,sizeof(float));
	x2=(float*)wtk_calloc(rfft->len,sizeof(float));
	x3=(float*)wtk_calloc(rfft->len,sizeof(float));
	x=6.283185307/(len-1);
	for(i=0;i<len;++i)
	{
		ft=0.54 - 0.46*cos(x*i);
		//ft=1;
		x1[i]=smc[i]*ft;
		x2[i]=ssp[i]*ft;
	}
	wtk_rfft_process_fft(rfft,x3,x1);//x3 mic fft;
	wtk_rfft_process_fft(rfft,x1,x2);//x1 sp fft;
	wtk_xcorr_freq_mult(x3,x1,x2,rfft->win);
	wtk_rfft_process_ifft(rfft,x2,x1);

	fidx=wtk_rfft_find_best_value(x1,rfft->len,margin,max_v);
	//wtk_debug("idx=%f f=%f len=%d/%d\n",fidx,ft,rfft->win,rfft->len);
	wtk_free(x1);
	wtk_free(x2);
	wtk_free(x3);
	wtk_rfft_delete(rfft);
	return fidx;
}

int wtk_rfft_xcorr(char *mic,int mic_len,char *sp,int sp_len)
{
#define NBEST 4
	int idx;
	int delays[NBEST];
	int v;
	int i;

	wtk_rfft_nbest_xcorr(mic,mic_len,sp,sp_len,min(mic_len,sp_len)/2,delays,NBEST);
	wtk_debug("----------------------------\n");
	for(i=0;i<NBEST;++i)
	{
		wtk_debug("v[%d]=%d\n",i,delays[i]);
	}
	idx=delays[0];
	for(i=0;i<NBEST-1;++i)
	{
		//wtk_debug("v[%d]=%d\n",i,delays[i]);
		v=delays[i]-delays[i+1];
		v=abs(v);
		if(v<80)
		{
			idx=delays[i];
			break;
		}
	}
	return idx;
}

void wtk_rfft_nbest_xcorr2(char *mic,int mic_len,char *sp,int sp_len,int *delays,int nbest)
{
	wtk_rfft_nbest_xcorr(mic,mic_len,sp,sp_len,min(mic_len,sp_len)/2,delays,nbest);
}

//#define DUEBG_XV

void wtk_rfft_nbest_xcorr_int(int *mic,int mic_len,int *sp,int sp_len,int margin,int *delays,int nbest)
{
	float *p1,*p2;
	int i;

	p1=(float*)wtk_calloc(mic_len,sizeof(float));
	p2=(float*)wtk_calloc(sp_len,sizeof(float));
	for(i=0;i<mic_len;++i)
	{
		p1[i]=mic[i];
	}
	for(i=0;i<sp_len;++i)
	{
		p2[i]=sp[i];
	}
	wtk_rfft_nbest_xcorr_float(p1,mic_len,p2,sp_len,margin,delays,nbest);

	wtk_free(p1);
	wtk_free(p2);
}

void wtk_rfft_nbest_xcorr_float(float *mic,int mic_len,float *sp,int sp_len,int margin,int *delays,int nbest)
{
	int len;
	wtk_rfft_t *rfft;
	float *x1;
	float *x2;
	float *x3;
	int i;
	short *smc,*ssp;
	wtk_larray_t *vs,*ps;
	float ft;
	double x;
#ifdef DUEBG_XV
	float values[10];
#else
	float *values=NULL;
#endif

	//wtk_debug("mic=%d sp=%d\n",mic_len,sp_len);
	len=min(mic_len/2,sp_len/2);
	rfft=wtk_rfft_new(len);
	smc=(short*)mic;
	ssp=(short*)sp;
	x1=(float*)wtk_calloc(rfft->len,sizeof(float));
	x2=(float*)wtk_calloc(rfft->len,sizeof(float));
	x3=(float*)wtk_calloc(rfft->len,sizeof(float));
	x=6.283185307/(len-1);
	for(i=0;i<len;++i)
	{
		ft=0.54 - 0.46*cos(x*i);
		x1[i]=smc[i]*ft;
		x2[i]=ssp[i]*ft;
	}
	wtk_rfft_process_fft(rfft,x3,x1);//x3 mic fft;
	wtk_rfft_process_fft(rfft,x1,x2);//x1 sp fft;
	wtk_xcorr_freq_mult(x3,x1,x2,rfft->win);
	wtk_rfft_process_ifft(rfft,x2,x1);

	vs=wtk_larray_new(rfft->win/3,sizeof(float));
	ps=wtk_larray_new(vs->nslot,sizeof(int));
	wtk_xcorr_find_nbest_values(vs,ps,x1,rfft->len,margin,delays,values,nbest);

//	i=wtk_xcorr_find_best_value(x1,rfft->len,margin);
//	wtk_debug("i=%d\n",i);
#ifdef DUEBG_XV
	for(i=0;i<nbest;++i)
	{
		wtk_debug("v[%d]=%d/%f\n",i,delays[i],values[i]);
	}
#endif
	wtk_larray_delete(vs);
	wtk_larray_delete(ps);
	wtk_free(x1);
	wtk_free(x2);
	wtk_free(x3);
	wtk_rfft_delete(rfft);
}

void wtk_rfft_nbest_xcorr(char *mic,int mic_len,char *sp,int sp_len,int margin,int *delays,int nbest)
{
#define DUEBG_XV
	int len;
	wtk_rfft_t *rfft;
	float *x1;
	float *x2;
	float *x3;
	int i;
	short *smc,*ssp;
	wtk_larray_t *vs,*ps;
	float ft;
	double x;
#ifdef DUEBG_XV
	float values[64];
#else
	float *values=NULL;
#endif

	//wtk_debug("mic=%d sp=%d\n",mic_len,sp_len);
	len=min(mic_len/2,sp_len/2);
	rfft=wtk_rfft_new(len);
	smc=(short*)mic;
	ssp=(short*)sp;
	x1=(float*)wtk_calloc(rfft->len,sizeof(float));
	x2=(float*)wtk_calloc(rfft->len,sizeof(float));
	x3=(float*)wtk_calloc(rfft->len,sizeof(float));
	x=6.283185307/(len-1);
	for(i=0;i<len;++i)
	{
		ft=0.54 - 0.46*cos(x*i);
		x1[i]=smc[i]*ft;
		x2[i]=ssp[i]*ft;
	}
	wtk_rfft_process_fft(rfft,x3,x1);//x3 mic fft;
	wtk_rfft_process_fft(rfft,x1,x2);//x1 sp fft;
	wtk_xcorr_freq_mult(x3,x1,x2,rfft->win);
	wtk_rfft_process_ifft(rfft,x2,x1);

	vs=wtk_larray_new(rfft->win/3,sizeof(float));
	ps=wtk_larray_new(vs->nslot,sizeof(int));
	wtk_xcorr_find_nbest_values(vs,ps,x1,rfft->len,margin,delays,values,nbest);

//	i=wtk_xcorr_find_best_value(x1,rfft->len,margin);
//	wtk_debug("i=%d\n",i);
#ifdef DUEBG_XV
	for(i=0;i<nbest;++i)
	{
		wtk_debug("v[%d]=%d/%e\n",i,delays[i],values[i]);
	}
#endif
	wtk_larray_delete(vs);
	wtk_larray_delete(ps);
	wtk_free(x1);
	wtk_free(x2);
	wtk_free(x3);
	wtk_rfft_delete(rfft);
}

int wtk_int_max(int *delay,int n)
{
	int v;
	int i;

	v=delay[0];
	for(i=1;i<n;++i)
	{
		if(delay[i]>v)
		{
			v=delay[i];
		}
	}
	return v;
}

int wtk_int_min(int *delay,int n)
{
	int v;
	int i;

	v=delay[0];
	for(i=1;i<n;++i)
	{
		if(delay[i]<v)
		{
			v=delay[i];
		}
	}
	return v;
}

void wtk_rfft_nbest_xcorr3(int channel,char **mic,int mic_len,char *sp,int sp_len,int *delays)
{
	int len;
	wtk_rfft_t *rfft;
	float **x1;
	float *x2;
	float *x3;
	int i,j;
	short **smc,*ssp;
	float ft;
	double x;
	int margin;

	//wtk_debug("mic=%d sp=%d\n",mic_len,sp_len);
	len=min(mic_len/2,sp_len/2);
	margin=min(len,600);
	rfft=wtk_rfft_new(len);
	smc=(short**)mic;
	ssp=(short*)sp;
	x1=(float**)wtk_malloc(sizeof(float*)*channel);
	for(i=0;i<channel;++i)
	{
		x1[i]=(float*)wtk_calloc(rfft->len,sizeof(float));
	}
	x2=(float*)wtk_calloc(rfft->len,sizeof(float));
	x3=(float*)wtk_calloc(rfft->len,sizeof(float));
	x=6.283185307/(len-1);
	for(i=0;i<len;++i)
	{
		ft=0.54 - 0.46*cos(x*i);
		for(j=0;j<channel;++j)
		{
			x1[j][i]=smc[j][i]*ft;
		}
		x2[i]=ssp[i]*ft;
	}
	wtk_rfft_process_fft(rfft,x3,x2);//x1 sp fft;

	for(i=0;i<channel;++i)
	{
		wtk_rfft_process_fft(rfft,x2,x1[i]);//mic fft
		wtk_xcorr_freq_mult(x2,x3,x1[i],rfft->win); //sp fft
		wtk_rfft_process_ifft(rfft,x1[i],x2);
		//wtk_xcorr_find_nbest_values();
		ft=wtk_rfft_find_best_value(x2,rfft->len,margin,NULL);
		delays[i]=ft>0?((int)(ft+0.5)):((int)(ft-0.5));
		//wtk_debug("ft=%f/%d\n",ft,delays[i]);
		wtk_free(x1[i]);
	}
	wtk_free(x1);
	wtk_free(x2);
	wtk_free(x3);
	wtk_rfft_delete(rfft);
}

void wtk_rfft_nbest_xcorr4(int channel,char **mic,int mic_len,char *sp,int sp_len,int nbest,int *nbest_delays)
{
	int len;
	wtk_rfft_t *rfft;
	float **x1;
	float *x2;
	float *x3;
	int i,j;
	short **smc,*ssp;
	float ft;
	double x;
	int margin;
	wtk_larray_t *ps;
	wtk_larray_t *vs;

	//wtk_debug("mic=%d sp=%d\n",mic_len,sp_len);
	len=min(mic_len/2,sp_len/2);
	margin=min(len,3200);
	rfft=wtk_rfft_new(len);
	ps=wtk_larray_new(rfft->win/3,sizeof(int));
	vs=wtk_larray_new(rfft->win/3,sizeof(float));
	smc=(short**)mic;
	ssp=(short*)sp;
	x1=(float**)wtk_malloc(sizeof(float*)*channel);
	for(i=0;i<channel;++i)
	{
		x1[i]=(float*)wtk_calloc(rfft->len,sizeof(float));
	}
	x2=(float*)wtk_calloc(rfft->len,sizeof(float));
	x3=(float*)wtk_calloc(rfft->len,sizeof(float));
	x=6.283185307/(len-1);
	for(i=0;i<len;++i)
	{
		ft=0.54 - 0.46*cos(x*i);
		for(j=0;j<channel;++j)
		{
			x1[j][i]=smc[j][i]*ft;
		}
		x2[i]=ssp[i]*ft;
	}
	wtk_rfft_process_fft(rfft,x3,x2);//x1 sp fft;
	for(i=0;i<channel;++i)
	{
		wtk_rfft_process_fft(rfft,x2,x1[i]);//mic fft
		wtk_xcorr_freq_mult(x2,x3,x1[i],rfft->win); //sp fft
		wtk_rfft_process_ifft(rfft,x1[i],x2);
		//wtk_xcorr_find_nbest_values();
		//delays[i]=wtk_xcorr_find_best_value(x2,rfft->len,margin);
		//wtk_debug("v[%d]=%d\n",i,delays[i]);
		//exit(0);
		wtk_xcorr_find_nbest_values(vs,ps,x2,rfft->len,margin,nbest_delays+i*nbest,NULL,nbest);
		//print_float(vx,nbest);
		wtk_free(x1[i]);
	}
	wtk_larray_delete(ps);
	wtk_larray_delete(vs);
	wtk_free(x1);
	wtk_free(x2);
	wtk_free(x3);
	wtk_rfft_delete(rfft);
}

void wtk_rfft_nbest_xcorr_mult_channel(int channel,char **mic,int mic_len,char *sp,int sp_len,int *delays)
{
	wtk_rfft_nbest_xcorr4(channel,mic,mic_len,sp,sp_len,1,delays);
}

void wtk_rfft_nbest_xcorr_mult_channel2(int channel,char **mic,int mic_len,char *sp,int sp_len,int nbest,int *delays)
{
	wtk_heap_t *heap;
	wtk_kcls_t *cls;
	wtk_kcls_value_t *vx;
	float max_sse=3;
	int *nbest_delay;
	int i;

	nbest_delay=(int*)wtk_malloc(sizeof(int)*channel*nbest);
	wtk_rfft_nbest_xcorr4(channel,mic,mic_len,sp,sp_len,nbest,nbest_delay);
	heap=wtk_heap_new(4096);
	cls=wtk_kcls_new(heap);
	for(i=0;i<channel;++i)
	{
		print_int(nbest_delay+i*nbest,nbest);
		vx=wtk_kcls_value_new(heap,i,nbest_delay[i*nbest]);
		wtk_queue_push(&(cls->item_q),&(vx->q_n));
	}
	cls=wtk_kcls_cluster(cls,heap,max_sse);
	wtk_kcls_print(cls);
	wtk_heap_delete(heap);
	exit(0);
	wtk_free(nbest_delay);
}

void wtk_rfft_mult(register float *fft1,register float *fft2,register float *xy,int win)
{
	int i;
	register float *fp1,*fp2,*fpx;

	//do the multiplication: (a+jb)(c+jd)=(ac-bd)+j(bc+ad);
	xy[0]=fft1[0]*fft2[0];
	fp1=fft1+win;
	fp2=fft2+win;
	fpx=xy+win;
	//wtk_debug("v[0]=%f\n",xy[0]);
	for(i=1;i<win;++i)
	{
		//a=fft1[i] b=fp1[i]
		//c=fft2[i] d=fp2[i]
		xy[i]=fft1[i]*fft2[i]-fp1[i]*fp2[i];
		fpx[i]=fp1[i]*fft2[i]+fft1[i]*fp2[i];
		//wtk_debug("%f+%f %f+%f\n",fft1[i],fp1[i],fft2[i],fp2[i]);
		//wtk_debug("v[%d]=%f+%f\n",i,xy[i],fpx[i]);
		//exit(0);
	}
	xy[win]=fft1[win]*fft2[win];
}

//do the multiplication: (a+jb)'(c+jd) =(a-jb)(c+jd)=(ac+bd)+j(ad-bc);
void wtk_rfft_mult2(register float *fft1,register float *fft2,float *xy,int win)
{
	int i;
	register float *fp1,*fp2,*fpx;

	xy[0]=fft1[0]*fft2[0];
	fp1=fft1+win;
	fp2=fft2+win;
	fpx=xy+win;
	for(i=1;i<win;++i)
	{
		//a=fft1[i] b=fp1[i]
		//c=fft2[i] d=fp2[i]
		xy[i]=fft1[i]*fft2[i]+fp1[i]*fp2[i];
		fpx[i]=fft1[i]*fp2[i]-fp1[i]*fft2[i];
	}
	xy[win]=fft1[win]*fft2[win];
}

//do the multiplication: (a+jb).conj(c+jd) =(a+jb)(c-jd)=(ac+bd)+j(-ad+bc);
void wtk_rfft_mult3(register float *fft1,register float *fft2,float *xy,int win)
{
	int i;
	register float *fp1,*fp2,*fpx;

	xy[0]=fft1[0]*fft2[0];
	fp1=fft1+win;
	fp2=fft2+win;
	fpx=xy+win;
	for(i=1;i<win;++i)
	{
		//a=fft1[i] b=fp1[i]
		//c=fft2[i] d=fp2[i]
		xy[i]=fft1[i]*fft2[i]+fp1[i]*fp2[i];
		fpx[i]=-fft1[i]*fp2[i]+fp1[i]*fft2[i];
	}
	xy[win]=fft1[win]*fft2[win];
}


void wtk_rfft_set_value(float *f,int n,int idx,wtk_complex_t c)
{
	int win;

	if(idx==0)
	{
		f[0]=c.a;
	}else
	{
		win=n>>1;
		if(idx<win)
		{
			f[idx]=c.a;
			f[idx+win]=-c.b;
		}else if(idx==win)
		{
			f[idx]=c.a;
		}else
		{
			f[n-idx]=c.a;
			f[n-idx+win]=c.b;
		}
	}
}

void wtk_rfft_set_value2(float *f,int n,int idx,float a,float b)
{
	int win;

	if(idx==0)
	{
		f[0]=a;
	}else
	{
		win=n>>1;
		if(idx<win)
		{
			f[idx]=a;
			f[idx+win]=-b;
		}else if(idx==win)
		{
			f[idx]=a;
		}else
		{
			f[n-idx]=a;
			f[n-idx+win]=b;
		}
	}
}



wtk_complex_t wtk_rfft_get_value(float *f,int n,int idx)
{
	wtk_complex_t c;
	int win;

	if(idx==0)
	{
		c.a=f[0];
		c.b=0;
	}else
	{
		win=n>>1;
		if(idx<win)
		{
			c.a=f[idx];
			c.b=-f[idx+win];
		}else if(idx==win)
		{
			c.a=f[idx];
			c.b=0;
		}else
		{
			c.a=f[n-idx];
			c.b=f[n-idx+win];
		}
	}
	return c;
}


void wtk_rfft_print_fft(float *f,int n)
{
	int i;
	int win=n/2;
	double ta,tb;

	ta=tb=0;
	printf("----------------- fft --------------------------\n");
	ta+=f[0];
	wtk_debug("v[%d]=%.6f+%.6fi\n",0,f[0],0.0);
	for(i=1;i<win;++i)
	{
		ta+=f[i];
		tb+=-f[i+win];
		wtk_debug("v[%d]=%.6f+%.6fi\n",i,f[i],-f[i+win]);
	}
	wtk_debug("v[%d]=%.6f+%.6fi\n",win,f[win],0.0);
	ta+=f[win];
	for(i=win+1;i<n;++i)
	{
		ta+=f[n-i];
		tb+=f[n-i+win];
		wtk_debug("v[%d]=%.6f+%.6fi\n",i,f[n-i],f[n-i+win]);
		//exit(0);
	}
	wtk_debug("tot=%f+%f\n",ta,tb);
	//exit(0);
}
