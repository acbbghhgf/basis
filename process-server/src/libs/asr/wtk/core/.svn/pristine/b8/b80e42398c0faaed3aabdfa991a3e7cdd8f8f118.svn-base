#include "wtk_qstft.h" 


wtk_qstft_t* wtk_qstft_new(wtk_qstft_cfg_t *cfg,wtk_stft_cfg_t *stft_cfg)
{
	wtk_qstft_t *qf;

	qf=(wtk_qstft_t*)wtk_malloc(sizeof(wtk_qstft_t));
	qf->cfg=cfg;
	qf->stft_cfg=stft_cfg;
	qf->robin=wtk_robin_new(cfg->lt*2+1);
	qf->winf=wtk_math_create_hanning_window(2*cfg->lf+1);
	qf->wint=wtk_math_create_hanning_window(2*cfg->lt+1);
	//print_float(qf->winf,2*cfg->lf+1);
	//print_float(qf->wint,2*cfg->lt+1);
	qf->XX=wtk_complex_new_p3(stft_cfg->channel,stft_cfg->channel,(stft_cfg->win>>1)+1);
	qf->max_ind=(2*cfg->lf+1)*(2*cfg->lt+1);
	qf->xx=wtk_complex_new_p2(stft_cfg->channel,qf->max_ind);
	qf->wei=(float*)wtk_malloc(qf->max_ind*sizeof(float));

	qf->notify_ths=NULL;
	qf->notify=NULL;
	qf->delete_msg_ths=NULL;
	qf->delete_msg=NULL;
	wtk_qstft_reset(qf);
	return qf;
}

void wtk_qstft_delete(wtk_qstft_t *qf)
{
	wtk_complex_delete_p2(qf->xx,qf->stft_cfg->channel);
	wtk_free(qf->wei);
	wtk_complex_delete_p3(qf->XX,qf->stft_cfg->channel,qf->stft_cfg->channel);
	wtk_free(qf->winf);
	wtk_free(qf->wint);
	wtk_robin_delete(qf->robin);
	wtk_free(qf);
}

void wtk_qstft_reset(wtk_qstft_t *qf)
{
	qf->state=WTK_QSTFT_INIT;
	wtk_robin_reset(qf->robin);
}

void wtk_qstft_set_notify(wtk_qstft_t *qf,void *ths,wtk_qstft_notify_f notify)
{
	qf->notify_ths=ths;
	qf->notify=notify;
}

void wtk_qstft_set_delete_msg(wtk_qstft_t *qf,void *ths,wtk_qstft_delete_msg_f delete_msg)
{
	qf->delete_msg_ths=ths;
	qf->delete_msg=delete_msg;
}

void wtk_qstft_update_raw(wtk_qstft_t *qf,int t,int is_end)
{
	wtk_robin_t *rb=qf->robin;
	int nbin=(qf->stft_cfg->win>>1)+1;
	int i,c,j,jfs,jfe,jts,jte,k,ki;
	int lf=qf->cfg->lf;
	int lt=qf->cfg->lt;
	int chan=qf->stft_cfg->channel;
	wtk_complex_t **xx;
	float *wei;
	int nind;
	wtk_stft_msg_t *msg;
	int nt,nf;
	float *winf=qf->winf;
	float *wint=qf->wint;
	float *pf,*pt;
	double f,ta,tb;
	wtk_complex_t *c1,*c2,*a,*b;
	//wtk_complex_t **local_cx;
	wtk_complex_t ***XX;

	//wtk_debug("used=%d nbin=%d\n",rb->used,nbin);
	XX=wtk_complex_new_p3(chan,chan,nbin);
	jts=max(0,t-lt);
	jte=min(rb->used-1,t+lt);
	jts+=lt-t;
	jte+=lt-t;
//	wtk_debug("jts=%d jte=%d\n",jts,jte);
	nt=jte-jts+1;
	pt=wint+jts;
	for(i=0;i<nbin;++i)
	{
		jfs=max(0,i-lf);
		jfe=min(nbin-1,i+lf);
		nf=jfe-jfs+1;
		nind=rb->used*nf;
		//wtk_debug("i=%d js=%d,je=%d nind=%d\n",i,jfs,jfe,nind);
		xx=wtk_complex_new_p2(chan,nind);
		k=nf*sizeof(wtk_complex_t);
		for(j=0;j<rb->used;++j)
		{
			msg=wtk_robin_at(rb,j);
			//wtk_complex_p2_print(msg->fft,1,8);
			for(c=0;c<chan;++c)
			{
				memcpy(xx[c]+j*nf,msg->fft[c]+jfs,k);
			}
		}
		if(i<qf->cfg->lf)
		{
			pf=winf+qf->cfg->lf*2+1-nf;
		}else
		{
			pf=winf;
			//exit(0);
		}
		wei=(float*)wtk_malloc(nind*sizeof(float));
		f=0;
		//wtk_debug("nt=%d nf=%d\n",nt,nf);
		for(j=0,ki=0;j<nt;++j)
		{
			ta=pt[j];
			for(k=0;k<nf;++k)
			{
				f+=wei[ki++]=pf[k]*ta;
			}
		}
		f=1.0/f;
		for(j=0;j<chan;++j)
		{
			c1=xx[j];
			for(k=0;k<chan;++k)
			{
				c2=xx[k];
				ta=tb=0;
				for(ki=0;ki<nind;++ki)
				{
					//j*wei*k;
					a=c1+ki;
					b=c2+ki;
					//(a+bi)*(c-di)=(ac+bd)+i(-ad+bc);
					ta+=(a->a*b->a+a->b*b->b)*wei[ki];
					tb+=(-a->a*b->b+a->b*b->a)*wei[ki];
					//wtk_debug("v[%d]=%f+%f %f+%f %f+%f\n",ki,ta,tb,a->a,a->b,b->a,b->b);
				}
				XX[j][k][i].a=ta*f;///f;
				XX[j][k][i].b=tb*f;///f;
			}
		}
		//wtk_complex_p2_print(XX[i],chan,chan);
	}
	if(qf->notify)
	{
		qf->notify(qf->notify_ths,XX,is_end);
	}
}

void wtk_qstft_check_triangle(wtk_complex_t ***XX,int chan,int ibin)
{
	int i,j;
	float ta,tb;

	for(i=0;i<chan;++i)
	{
		for(j=i;j<chan;++j)
		{
			//wtk_debug("i=%d j=%d\n",i,j);
			ta=fabs(XX[i][j][ibin].a-XX[j][i][ibin].a);
			tb=fabs(XX[i][j][ibin].b+XX[j][i][ibin].b);
			if(ta>1e-5 || tb>1e-5)
			{
				wtk_debug("found bug [%d/%d]=%f+%f %f+%f\n",i,j,XX[i][j][ibin].a,XX[i][j][ibin].b,XX[j][i][ibin].a,XX[j][i][ibin].b);
				exit(0);
			}
		}
	}
}


void wtk_qstft_print_xx(wtk_complex_t ***XX,int chan,int ibin)
{
	int i,j;

	for(i=0;i<chan;++i)
	{
		for(j=0;j<chan;++j)
		{
			if(j>0)
			{
				printf(" ");
			}
			printf("%f+%f",XX[i][j][ibin].a,XX[i][j][ibin].b);
		}
		printf("\n");
	}
}

void wtk_qstft_update(wtk_qstft_t *qf,int t,int is_end)
{
	wtk_robin_t *rb=qf->robin;
	int nbin=(qf->stft_cfg->win>>1)+1;
	int i,c,j,jfs,jfe,jts,jte,k,ki;
	int lf=qf->cfg->lf;
	int lt=qf->cfg->lt;
	int chan=qf->stft_cfg->channel;
	wtk_complex_t **xx;
	float *wei;
	int nind;
	wtk_stft_msg_t *msg;
	int nt,nf;
	float *winf=qf->winf;
	float *wint=qf->wint;
	float *pf,*pt;
	double f,ta,tb;
	wtk_complex_t *c1,*c2,*a,*b;
	//wtk_complex_t **local_cx;
	wtk_complex_t ***XX;
	wtk_complex_t **xx2,*xx1;
	wtk_complex_t *xa,*xb;

	//wtk_debug("used=%d nbin=%d\n",rb->used,nbin);
	//XX=wtk_complex_new_p3(chan,chan,nbin);
	XX=qf->XX;
	jts=max(0,t-lt);
	jte=min(rb->used-1,t+lt);
	jts+=lt-t;
	jte+=lt-t;
//	wtk_debug("jts=%d jte=%d\n",jts,jte);
	nt=jte-jts+1;
	pt=wint+jts;
	xx=qf->xx;
	wei=qf->wei;
	for(i=0;i<nbin;++i)
	{
		jfs=max(0,i-lf);
		jfe=min(nbin-1,i+lf);
		nf=jfe-jfs+1;
		nind=rb->used*nf;
		//wtk_debug("=============== i=%d ============\n",i);
		//xx=wtk_complex_new_p2(chan,nind);
		k=nf*sizeof(wtk_complex_t);
		for(j=0;j<rb->used;++j)
		{
			msg=wtk_robin_at(rb,j);
			//wtk_complex_p2_print(msg->fft,1,8);
			for(c=0;c<chan;++c)
			{
				memcpy(xx[c]+j*nf,msg->fft[c]+jfs,k);
			}
		}
		//wtk_complex_print3(xx,4,16);
		if(i<qf->cfg->lf)
		{
			pf=winf+qf->cfg->lf*2+1-nf;
		}else
		{
			pf=winf;
			//exit(0);
		}
		//wei=(float*)wtk_malloc(nind*sizeof(float));
		f=0;
		//wtk_debug("nt=%d nf=%d\n",nt,nf);
		for(j=0,ki=0;j<nt;++j)
		{
			ta=pt[j];
			for(k=0;k<nf;++k)
			{
				f+=wei[ki++]=pf[k]*ta;
			}
		}
		//print_float(wei,nt*nf);
		f=1.0/f;
		for(j=0;j<chan;++j)
		{
			c1=xx[j];
			xx2=XX[j];
			for(k=j;k<chan;++k)
			{
				c2=xx[k];
				ta=tb=0;
				xx1=xx2[k];
				for(ki=0;ki<nind;++ki)
				{
					a=c1+ki;
					b=c2+ki;
					//(a+bi)*(c-di)=(ac+bd)+i(-ad+bc);
					ta+=(a->a*b->a+a->b*b->b)*wei[ki];
					tb+=(-a->a*b->b+a->b*b->a)*wei[ki];
				}
				xx1[i].a=ta*f;
				xx1[i].b=tb*f;
				//wtk_debug("v[%d,%d]=%f+%f\n",j,k,xx1[i].a,xx1[i].b);
			}
			for(k=0;k<j;++k)
			{
				xa=xx2[k]+i;
				xb=XX[k][j]+i;
				//xx2[k][i]=XX[k][j][i];
				xa->a=xb->a;
				xa->b=-xb->b;
			}
		}
		//wtk_qstft_print_xx(XX,chan,i);
		//wtk_qstft_check_triangle(XX,chan,i);
		//wtk_complex_p2_print(XX[i],chan,chan);
	}
//	wtk_complex_print(XX[0][0],10);
//	wtk_complex_print(XX[0][1],10);
//	wtk_complex_print(XX[1][0],10);
//	exit(0);
	if(qf->notify)
	{
		qf->notify(qf->notify_ths,XX,is_end);
	}
}


void wtk_qstft_feed(wtk_qstft_t *qf,wtk_stft_msg_t *msg,int len,int is_end)
{
	wtk_robin_t *rb=qf->robin;
	int i;

	//wtk_complex_print(msg->fft[0],10);
	wtk_robin_push(rb,msg);
	switch(qf->state)
	{
	case WTK_QSTFT_INIT:
		//wtk_debug("win=%d\n",qf->cfg->lt);
		if(rb->used==qf->cfg->lt+1)
		{
			wtk_qstft_update(qf,0,0);
			qf->state=WTK_QSTFT_UPDATE;
		}
		break;
	case WTK_QSTFT_UPDATE:
		//wtk_debug("used=%d/%d\n",rb->used,rb->nslot);
		if(rb->used<rb->nslot)
		{
			wtk_qstft_update(qf,rb->used-qf->cfg->lt-1,0);
		}else
		{
			wtk_qstft_update(qf,rb->used-qf->cfg->lt-1,0);
			msg=(wtk_stft_msg_t*)wtk_robin_pop(rb);
			if(qf->delete_msg)
			{
				qf->delete_msg(qf->delete_msg_ths,msg);
			}
		}
		break;
	}
	if(is_end)
	{
		//wtk_debug("used=%d/%d\n",rb->used,rb->nslot);
		switch(qf->state)
		{
		case WTK_QSTFT_INIT:
			for(i=0;i<rb->used;++i)
			{
				wtk_qstft_update(qf,i,i==rb->used-1);
			}
			break;
		case WTK_QSTFT_UPDATE:
			for(i=qf->cfg->lt;i<rb->used;++i)
			{
				wtk_qstft_update(qf,i,i==rb->used-1);
			}
			break;
		}
		for(i=0;i<rb->used;++i)
		{
			msg=(wtk_stft_msg_t*)wtk_robin_at(rb,i);
			if(qf->delete_msg)
			{
				qf->delete_msg(qf->delete_msg_ths,msg);
			}
		}
		wtk_qstft_reset(qf);
	}
}
