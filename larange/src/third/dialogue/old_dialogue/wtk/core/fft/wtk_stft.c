#include "wtk_stft.h" 

wtk_stft_msg_t* wtk_stft_msg_new(wtk_stft_t *stft)
{
	wtk_stft_msg_t *msg;

	msg=(wtk_stft_msg_t*)wtk_malloc(sizeof(wtk_stft_msg_t));
	msg->hook=NULL;
	msg->fft=wtk_complex_new_p2(stft->cfg->channel,stft->nbin);
	return msg;
}

void wtk_stft_msg_delete(wtk_stft_t *stft,wtk_stft_msg_t *msg)
{
	wtk_complex_delete_p2(msg->fft,stft->cfg->channel);
	wtk_free(msg);
}

wtk_stft_msg_t* wtk_bssmvdr2_pop_msg(wtk_stft_t *stft)
{
	return  (wtk_stft_msg_t*)wtk_hoard_pop(&(stft->msg_hoard));
}

void wtk_stft_push_msg(wtk_stft_t *stft,wtk_stft_msg_t *msg)
{
	wtk_hoard_push(&(stft->msg_hoard),msg);
}

wtk_stft_t* wtk_stft_new(wtk_stft_cfg_t *cfg)
{
	wtk_stft_t *stft;
	int i;

	stft=(wtk_stft_t*)wtk_malloc(sizeof(wtk_stft_t));
	stft->cfg=cfg;
	wtk_hoard_init2(&(stft->msg_hoard),offsetof(wtk_stft_msg_t,hoard_n),100,
			(wtk_new_handler_t)wtk_stft_msg_new,
			(wtk_delete_handler2_t)wtk_stft_msg_delete,
			stft);

	stft->input=wtk_float_new_p2(cfg->channel,cfg->win);
	//stft->output=(short*)wtk_calloc(cfg->win,sizeof(short));
	stft->output=(float**)wtk_calloc(cfg->channel,sizeof(float*));
	for(i=0;i<cfg->channel;++i)
	{
		stft->output[i]=(float*)wtk_calloc(cfg->win,sizeof(float));
	}
	stft->rfft=wtk_rfft_new(cfg->win/2);

	stft->pad=wtk_float_new_p2(cfg->channel,cfg->win);

	if(cfg->use_sine)
	{
		stft->win=wtk_math_create_sine_window(cfg->win);
		stft->fft_scale=1.0;
	}else if(cfg->use_hamming)
	{
		stft->win=wtk_math_create_float_ham_window(cfg->win);
		stft->fft_scale=1.0/sqrt(cfg->win);
	}else if(cfg->use_hann)
	{
		stft->win=wtk_math_create_hanning_window2(cfg->win);
		stft->fft_scale=1.0/sqrt(cfg->win);
	}
	//print_float(stft->win,10);//cfg->win);
	//exit(0);
	stft->input_win=(float*)wtk_calloc(cfg->win,sizeof(float));
	stft->input_fft=(float*)wtk_calloc(cfg->win,sizeof(float));
	//stft->fft_scale*=20;

	stft->nbin=cfg->win/2+1;

	stft->notify=NULL;
	stft->notify_ths=NULL;

	stft->notify_float=NULL;
	stft->notify_float_ths=NULL;

	wtk_stft_reset(stft);
	return stft;
}

void wtk_stft_delete(wtk_stft_t *stft)
{
	int i;

	for(i=0;i<stft->cfg->channel;++i)
	{
		wtk_free(stft->output[i]);
	}
	wtk_free(stft->output);
	wtk_hoard_clean(&(stft->msg_hoard));
	wtk_float_delete_p2(stft->input,stft->cfg->channel);
	//wtk_free(stft->output);
	wtk_rfft_delete(stft->rfft);
	wtk_float_delete_p2(stft->pad,stft->cfg->channel);
	wtk_free(stft->win);
	wtk_free(stft->input_win);
	wtk_free(stft->input_fft);
	wtk_free(stft);
}

void wtk_stft_reset(wtk_stft_t *stft)
{
	//wtk_debug("hoard use=%d free=%d\n",stft->msg_hoard.use_length,stft->msg_hoard.cur_free);
	stft->pos=0;
	wtk_float_p2_zero(stft->pad,stft->cfg->channel,stft->cfg->win);
}

void wtk_stft_set_notify(wtk_stft_t *stft,void *ths,wtk_stft_notify_f notify)
{
	stft->notify_ths=ths;
	stft->notify=notify;
}


void wtk_stft_set_float_notify(wtk_stft_t *stft,void *ths,wtk_stft_notify_float_f notify)
{
	stft->notify_float_ths=ths;
	stft->notify_float=notify;
}


void wtk_stft_update(wtk_stft_t *stft,int is_end)
{
	wtk_stft_cfg_t *cfg=stft->cfg;
	int win=cfg->win;
	int channel=cfg->channel;
	float *input;
	float *hwin=stft->win;
	float *input_win=stft->input_win;
	float *fft=stft->input_fft;
	int i,j;
	float scale=stft->fft_scale;
	wtk_stft_msg_t *msg;
	wtk_complex_t *c;
	int step=win>>1;

	msg=wtk_bssmvdr2_pop_msg(stft);
	for(i=0;i<channel;++i)
	{
		//wtk_debug("i=%d\n",i);
		input=stft->input[i];
		//input *win;
		for(j=0;j<win;++j)
		{
			input_win[j]=input[j]*hwin[j];
		}
		//input=> fft;
		//print_float(input_win,10);
		wtk_rfft_process_fft(stft->rfft,fft,input_win);
		//wtk_rfft_print_fft(fft,win);
		c=msg->fft[i];
		if(scale==1)
		{
			c[0].a=fft[0];
			c[0].b=0;
			for(j=1;j<step;++j)
			{
				c[j].a=fft[j];
				c[j].b=-fft[j+step];
			}
			c[step].a=fft[step];
			c[step].b=0;
		}else
		{
			c[0].a=fft[0]*scale;
			c[0].b=0;
			for(j=1;j<step;++j)
			{
				c[j].a=fft[j]*scale;
				c[j].b=-fft[j+step]*scale;
			}
			c[step].a=fft[step]*scale;
			c[step].b=0;
		}
		//wtk_complex_print(c,step+1);
		//exit(0);
	}
	if(stft->notify)
	{
		stft->notify(stft->notify_ths,msg,stft->pos,is_end);
	}
}


void wtk_stft_feed(wtk_stft_t *stft,short **pv,int n,int is_end)
{
	static float scale=1.0/32768.0;
	wtk_stft_cfg_t *cfg=stft->cfg;
	int channel=cfg->channel;
	int win=cfg->win;
	float **input=stft->input;
	int len,step;
	int i,j;
	int pos=stft->pos;
	float *pf;
	short *sf;

	len=0;
	while(len<n)
	{
		step=min(n-len,win-pos);
		for(i=0;i<channel;++i)
		{
			pf=input[i]+pos;
			sf=pv[i]+len;
			for(j=0;j<step;++j)
			{
				pf[j]=sf[j]*scale;
			}
		}
		pos+=step;
		if(pos>=win)
		{
			//calc fft;
			stft->pos=pos;
			wtk_stft_update(stft,0);
			pos-=cfg->step;
			for(i=0;i<channel;++i)
			{
				memmove(input[i],input[i]+cfg->step,pos*sizeof(float));
			}
		}
		len+=step;
	}
	if(is_end)
	{
		if(pos>0)
		{
			for(i=0;i<channel;++i)
			{
				memset(input[i]+pos,0,(win-pos)*sizeof(float));
			}
			stft->pos=pos;
			//update;
			wtk_stft_update(stft,1);
		}
		pos=0;
	}
	stft->pos=pos;
}

void wtk_stft_feed2(wtk_stft_t *stft,short *pv,int n,int is_end)
{
	static float scale=1.0/32768.0;
	wtk_stft_cfg_t *cfg=stft->cfg;
	int channel=cfg->channel;
	int win=cfg->win;
	float **input=stft->input;
	int len,step;
	int i,j;
	int pos=stft->pos;

	len=0;
	while(len<n)
	{
		step=min(n-len,win-pos);
		for(i=0;i<step;++i)
		{
			for(j=0;j<channel;++j)
			{
				input[j][pos+i]=*(pv++)*scale;
			}
		}
		pos+=step;
		if(pos>=win)
		{
			//calc fft;
			stft->pos=pos;
			wtk_stft_update(stft,0);
			pos-=cfg->step;
			for(i=0;i<channel;++i)
			{
				memmove(input[i],input[i]+cfg->step,pos*sizeof(float));
			}
		}
		len+=step;
	}
	if(is_end)
	{
		if(pos>0)
		{
			for(i=0;i<channel;++i)
			{
				memset(input[i]+pos,0,(win-pos)*sizeof(float));
			}
			stft->pos=pos;
			//wtk_debug("pos=%d\n",stft->pos);
			//update;
			wtk_stft_update(stft,1);
		}
		pos=0;
	}
	stft->pos=pos;
}
//
//void wtk_stft_notify_sample(wtk_stft_t *stft,float *y,int n,int channel)
//{
//	short *pv=stft->output;
//	int win=stft->cfg->win;
//	float *ps,*pe;
//	float scale=32767.0/2;//32768.0;
//	int step,i;
//
//	if(stft->notify_float)
//	{
//		stft->notify_float(stft->notify_float_ths,y,n,channel);
//		return;
//	}
//	ps=y;
//	pe=y+n;
//	while(ps<pe)
//	{
//		step=min(pe-ps,win);
//		for(i=0;i<step;++i)
//		{
//			pv[i]=(short)(ps[i]*scale);
//		}
//		//print_short(pv,step);
//		if(stft->notify_sample)
//		{
//			stft->notify_sample(stft->notify_sample_ths,(char*)pv,step<<1,channel);
//		}
//		ps+=step;
//	}
//}

int wtk_stft_output_ifft(wtk_stft_t *stft,wtk_complex_t *c,int channel,int pos,int is_end,float *y)
{
	int nbin=stft->nbin;
	int k,i;
	float *fft;
	float *t;
	float fx;
	int win=nbin-1;//mvdr->cfg->win;
	float *pad=stft->pad[channel];
	int n;

	fft=stft->input_fft;
	t=stft->input_win;
	//exit(0);
	fft[0]=c[0].a;
	for(i=1;i<win;++i)
	{
		fft[i]=c[i].a;
		fft[i+win]=-c[i].b;
	}
	fft[win]=c[i].a;
	//wtk_rfft_print_fft(fft,win*2);
	wtk_rfft_process_ifft(stft->rfft,fft,t);
	fx=1.0/stft->fft_scale;
	//y=fft;
	win=stft->cfg->win;
	n=(win-stft->cfg->step)*sizeof(float);
	memcpy(y,pad,win*sizeof(float));
	//wtk_debug("========== ki=%d =========\n",ki);
	if(is_end)
	{
		//wtk_stft_notify_sample(stft,y,pos,channel);
		//wtk_debug("pos=%d win=%d nbin=%d\n",pos,win,stft->nbin);
		if(pos<win)
		{
			//不足窗设置为0;
			//wtk_debug("========= 123 ============\n");
			//memset(y,0,pos*sizeof(float));
			memcpy(y,pad,win*sizeof(float));
			return pos;
		}
		for(k=0;k<win;++k)
		{
			y[k]+=t[k]*fx;
		}
		if(stft->cfg->keep_win)
		{
			return win;
		}else
		{
			return pos;
		}
	}else
	{
		for(k=0;k<win;++k)
		{
			y[k]+=t[k]*fx;
		}
		memcpy(pad,y+stft->cfg->step,n);
		return stft->cfg->step;
		//wtk_stft_notify_sample(stft,y,stft->cfg->step,channel);
	}
}

void wtk_stft_process_ifft(wtk_stft_t *stft,wtk_complex_t ***fft,int n,int last_pos,int is_end)
{
	int channel=stft->cfg->channel;
	int i,j;
	float **output=stft->output;
	int len=0;

	if(n>0)
	{
		if(is_end)
		{
			for(i=0;i<n;++i)
			{
				len=0;
				if(i==n-1)
				{
					for(j=0;j<channel;++j)
					{
						len=wtk_stft_output_ifft(stft,fft[i][j],j,last_pos,is_end,output[j]);
					}
				}else
				{
					for(j=0;j<channel;++j)
					{
						len=wtk_stft_output_ifft(stft,fft[i][j],j,0,0,output[j]);
					}
				}
				if(len>0 && stft->notify_float)
				{
					stft->notify_float(stft->notify_float_ths,output,len);
				}
			}
		}else
		{
			for(i=0;i<n;++i)
			{
				len=0;
				for(j=0;j<channel;++j)
				{
					len=wtk_stft_output_ifft(stft,fft[i][j],j,0,0,output[j]);
				}
				if(len>0 && stft->notify_float)
				{
					stft->notify_float(stft->notify_float_ths,output,len);
				}
			}
		}
	}
//	if(is_end)
//	{
//		j=stft->cfg->win-stft->cfg->step;
//		for(i=0;i<channel;++i)
//		{
//			wtk_stft_notify_sample(stft,stft->pad[i],j,i);
//		}
//	}
}

