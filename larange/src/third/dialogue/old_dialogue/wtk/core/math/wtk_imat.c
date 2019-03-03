#include <math.h>
#include "wtk_imat.h"

wtk_imat_t* wtk_imat_new(int row,int col)
{
	int **i=0;
	int *ip;
	int n;
	int col_bytes;
	char *pdata;

	ip=wtk_calloc(1,wtk_imat_bytes(row,col));
	//set row col;
	*(int*)ip=row;
	++ip;
	*(int*)ip=col;
	i=(int**)(ip);
	//set index;
	col_bytes=col*sizeof(int);
	pdata=(char*)i+sizeof(int*)*(row+1);
	for(n=1;n<=row;++n,pdata+=col_bytes)
	{
		i[n]=(int*)(pdata-sizeof(int));
	}
	return i;
}

void wtk_imat_delete(wtk_imat_t *m)
{
	wtk_free((char*)m-sizeof(int));
}

void wtk_imat_reset(wtk_imat_t *m)
{
	char *data;

	data=wtk_imat_data(m);
	memset(data,0,sizeof(int)*wtk_imat_row(m)*wtk_imat_col(m));
}

double wtk_imat_euclid_distance2(wtk_imat_t *m1,wtk_imat_t *m2)
{
	int i,j;
	int N,M;
	int t;
	double d=0;

	N=wtk_imat_row(m1);
	M=wtk_imat_col(m1);
	for(i=1;i<=N;++i)
	{
		for(j=1;j<=M;++j)
		{
			t=m1[i][j]-m2[i][j];
			d+=t*t;
		}
	}
	return sqrt(d);
}

double wtk_imat_euclid_distance(wtk_imat_t *m1,wtk_imat_t *m2)
{
	int N,M;
	int t;
	double d;
	int *p1,*p2,*p1e;

	N=wtk_imat_row(m1);
	M=wtk_imat_col(m1);
	p1=(int*)wtk_imat_data(m1);
	p2=(int*)wtk_imat_data(m2);
	N*=M;
	d=0;
	p1e=p1+N;
	while(p1<p1e)
	{
		t=*p1++-*p2++;
		d+=t*t;
	}
	return sqrt(d);
}

double wtk_imat_cos2(wtk_imat_t *m1,wtk_imat_t *m2)
{
	int i,j;
	int N,M;
	int t;
	double d=0,a=0,b=0;

	N=wtk_imat_row(m1);
	M=wtk_imat_col(m1);
	for(i=1;i<=N;++i)
	{
		for(j=1;j<=M;++j)
		{
			t=m1[i][j]*m2[i][j];
			d+=t*t;
			t=m1[i][j];
			a+=t*t;
			t=m2[i][j];
			b+=t*t;
		}
	}
	wtk_debug("d=%f,a=%f,b=%f\n",d,a,b);
	d=d/(sqrt(a)*sqrt(b));
	return d;
}

double wtk_imat_cos(wtk_imat_t *m1,wtk_imat_t *m2)
{
	int N,M;
	int *p1,*p2,*p1e;
	double d,a,b;
	int t1,t2;

	N=wtk_imat_row(m1);
	M=wtk_imat_col(m1);
	p1=(int*)wtk_imat_data(m1);
	p2=(int*)wtk_imat_data(m2);
	N*=M;
	d=a=b=0;
	p1e=p1+N;
	while(p1<p1e)
	{
		t1=*p1++;
		t2=*p2++;
		if(t1!=0 && t2!=0)
		{
			//wtk_debug("t1=%d,t2=%d\n",t1,t2);
			//wtk_debug("d=%f,a=%f,b=%f\n",d,a,b);
			d+=t1*t2;
			//wtk_debug("d=%f\n",d);
		}
		if(t1!=0)
		{
			a+=t1*t1;
			//wtk_debug("a=%f\n",a);
		}
		if(t2!=0)
		{
			b+=t2*t2;
			//wtk_debug("b=%f\n",a);
		}
	}
	//wtk_debug("d=%f,a=%f,b=%f\n",d,a,b);
	d=d/(sqrt(a)*sqrt(b));
	return d;
}


void wtk_imat_print(wtk_imat_t *m)
{
	int i,j;
	int N,M;

	N=wtk_imat_row(m);
	M=wtk_imat_col(m);
	for(i=1;i<=N;++i)
	{
		printf("%d:\t",i);
		for(j=1;j<=M;++j)
		{
			if(j>1)
			{
				printf(" ");
			}
			printf("[%d]",m[i][j]);
		}
		printf("\n");
	}
}

//---------------- float matrix section -----------------
wtk_fmat_t* wtk_fmat_new(int row,int col)
{
	float **i=0,*pf;
	int *ip;
	int n;
	int col_bytes;
	char *pdata;
	int j;

	ip=wtk_calloc(1,wtk_fmat_bytes(row,col));
	//set row col;
	*(int*)ip=row;
	++ip;
	*(int*)ip=col;
	i=(float**)(ip);
	//set index;
	col_bytes=col*sizeof(float);
	pdata=(char*)i+sizeof(float*)*(row+1);
	for(n=1;n<=row;++n,pdata+=col_bytes)
	{
		pf=i[n]=(float*)(pdata-sizeof(float));
		for(j=1;j<col;++j)
		{
			pf[j]=0;
		}
	}
	return i;
}

void wtk_fmat_delete(wtk_fmat_t *m)
{
	wtk_free((char*)m-sizeof(int));
}

void wtk_fmat_print(wtk_fmat_t *m)
{
	int i,j;
	int N,M;

	N=wtk_imat_row(m);
	M=wtk_imat_col(m);
	for(i=1;i<=N;++i)
	{
		printf("%d:\t",i);
		for(j=1;j<=M;++j)
		{
			if(j>1)
			{
				printf(" ");
			}
			printf("[%f]",m[i][j]);
		}
		printf("\n");
	}
}

