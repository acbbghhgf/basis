#include "wtk_lms.h" 

wtk_lms_t* wtk_lms_new(int frame_size,float rate)
{
	wtk_lms_t *l;

	l=(wtk_lms_t*)wtk_malloc(sizeof(wtk_lms_t));
	l->frame_size=frame_size;
	l->rate=rate;
	l->x=(short*)wtk_calloc(frame_size,sizeof(short));
	l->win=(float*)wtk_calloc(frame_size,sizeof(float));
	return l;
}

void wtk_lms_delete(wtk_lms_t *lms)
{
	wtk_free(lms);
}



void wtk_lms_process(wtk_lms_t *lms,short *mic,int mic_len,short *sp,int sp_len)
{
	short *me,*se;
	int i;
	short *x=lms->x;
	float f,e;
	float *win=lms->win;
	int ki=0;
	int nx=2;

	wtk_debug("mic_len=%d sp_len=%d\n",mic_len,sp_len);
	me=mic+mic_len;
	se=sp+sp_len;
	while(mic<me && sp<se)
	{
		++ki;
		for(i=lms->frame_size-1;i>0;--i)
		{
			x[i]=x[i-1];
		}
		x[0]=*sp;
		f=0;
		for(i=0;i<lms->frame_size;++i)
		{
			f+=x[i]*win[i];
			//wtk_debug("v[%d]=%f x=%d win=%f\n",i,f,x[i],win[i]);
		}
		e=*mic-f;
		//wtk_debug("v[%d]=e=%f %d f=%f\n",ki,e,*mic,f);
		//wtk_debug("v[%d]=%f\n",ki,e);
//		if(ki==1500)
//		{
//			exit(0);
//		}
		for(i=0;i<lms->frame_size;++i)
		{
			win[i]=win[i]+2*lms->rate*e*x[i];
			//wtk_debug("update win[%d]=%f e=%f xi=%d\n",i,win[i],e,x[i]);
			if(win[i]>nx)
			{
				win[i]=nx;
			}else if(win[i]<-nx)
			{
				win[i]=-nx;
			}
		}
		*mic=e;//*mic-f*0.5;
		++sp;
		++mic;
	}

}
