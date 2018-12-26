#include "wtk_fixmath.h"

wtk_fixi_t* wtk_fixi_new(int row,int col)
{
	wtk_fixi_t *fix;

	fix=(wtk_fixi_t*)wtk_malloc(sizeof(wtk_fixi_t));
	fix->row=row;
	fix->col=col;
	fix->p=(int*)wtk_malloc(row*col*sizeof(int));
	fix->scale=0;
	return fix;
}

void wtk_fixi_delete(wtk_fixi_t *fixi)
{
	wtk_free(fixi->p);
	wtk_free(fixi);
}

void wtk_fixi_scale(wtk_fixi_t *fix,wtk_vector_t *v,float scale)
{
	int i,n;

	n=wtk_vector_size(v);
	for(i=0;i<n;++i)
	{
		fix->p[i]=wtk_float_round(v[i]*scale);
	}
}


void wtk_fixi_print(wtk_fixi_t *fix)
{
	int i,j;

	for(i=0;i<fix->row;++i)
	{
		for(j=0;j<fix->col;++j)
		{
			wtk_debug("v[%d][%d]=%d\n",i,j,fix->p[i*fix->col+j]);
		}
	}
}

/*
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
}*/

/**
 *	1*b *b*c =1*c;
 */
void wtk_fixi_mult_mv(wtk_fixi_t *fix,wtk_matrix_t *m,wtk_vector_t *src,float scale)
{
	int rows,cols,i;
	float vi;
	float f;
	register float *px,*py,*pxe;

	rows=wtk_matrix_rows(m);
	cols=wtk_matrix_cols(m);
	fix->row=1;
	fix->col=rows;
	fix->scale=scale;
	//wtk_debug("mul=%d/%d\n",rows,cols);
	for(i=1;i<=rows;++i)
	{
		vi=0;
		px=m[i];
		py=src;
		pxe=px+cols;
		while(px<pxe)
		{
			vi+=(*(++py))*(*(++px));
		}
		f=vi*scale;
		//wtk_debug("vi[%d]=%f f=%f\n",i,vi,f);
		fix->p[i-1]=wtk_float_round(f);
	}
}

wtk_fixc_t* wtk_fixc_new(wtk_matrix_t *mat)
{
	wtk_fixc_t *fix;
	int row,col;
	float max,scale;
	char *px,*ppx;
	float *fp;
	int i,j;
	float f;

	fix=(wtk_fixc_t*)wtk_malloc(sizeof(wtk_fixc_t));
	row=wtk_matrix_rows(mat);
	col=wtk_matrix_cols(mat);
	fix->row=row;
	fix->col=col;
	fix->p=(char*)wtk_malloc(row*col);
	max=wtk_matrix_max_abs(mat);
	scale=127.0/max;
	fix->scale=scale;
	for(px=fix->p,i=0;i<row;++i,px+=col)
	{
		for(ppx=px,fp=mat[i+1],j=0;j<col;++j)
		{
			f=(*(++fp))*scale;
			*(ppx++)=(char)(wtk_float_round(f));
		}
	}
	return fix;
}

void wtk_fixc_delete(wtk_fixc_t *fixc)
{
	wtk_free(fixc->p);
	wtk_free(fixc);
}
