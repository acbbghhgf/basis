#include "wtk_pitch.h" 
#include <stdlib.h>

wtk_pitch_t* wtk_pitch_new(wtk_pitch_cfg_t *cfg)
{
	wtk_pitch_t *p;

	p=(wtk_pitch_t*)wtk_malloc(sizeof(wtk_pitch_t));
	p->cfg=cfg;
	p->in=wtk_flta_new(cfg->fft_frame_size);
	p->out=wtk_flta_new(cfg->fft_frame_size);
	p->in_fifo=wtk_flta_new(cfg->fft_frame_size);
	p->out_fifo=wtk_flta_new(cfg->fft_frame_size);
	p->fft_worksp=wtk_flta_new(cfg->fft_frame_size*2);
	p->last_phase=wtk_flta_new(cfg->fft_frame_size/2+1);
	p->sum_phase=wtk_flta_new(cfg->fft_frame_size/2+1);
	p->output_accum=wtk_flta_new(cfg->fft_frame_size*2);
	p->ana_freq=wtk_flta_new(cfg->fft_frame_size);
	p->ana_magn=wtk_flta_new(cfg->fft_frame_size);
	p->syn_freq=wtk_flta_new(cfg->fft_frame_size);
	p->syn_magn=wtk_flta_new(cfg->fft_frame_size);
	p->buf=wtk_strbuf_new(1024*100,1);

	p->f1=(2.0*M_PI)/p->cfg->over_sampling;
	p->f2=2.0/(p->cfg->fft_frame_size*p->cfg->over_sampling);
	p->f3=(2.0*M_PI)/p->cfg->fft_frame_size;
	p->f4=p->cfg->over_sampling*1.0/(2.0*M_PI);

	p->notify=NULL;
	p->notify_ths=NULL;
	wtk_pitch_reset(p);
	return p;
}

void wtk_pitch_delete(wtk_pitch_t *p)
{
	wtk_flta_delete(p->in);
	wtk_flta_delete(p->out);
	wtk_flta_delete(p->in_fifo);
	wtk_flta_delete(p->out_fifo);
	wtk_flta_delete(p->fft_worksp);
	wtk_flta_delete(p->last_phase);
	wtk_flta_delete(p->sum_phase);
	wtk_flta_delete(p->output_accum);
	wtk_flta_delete(p->ana_freq);
	wtk_flta_delete(p->ana_magn);
	wtk_flta_delete(p->syn_freq);
	wtk_flta_delete(p->syn_magn);
	wtk_strbuf_delete(p->buf);
	wtk_free(p);
}

void wtk_pitch_set(wtk_pitch_t *p,void *ths,wtk_pitch_noityf_f notify)
{
	p->notify_ths=ths;
	p->notify=notify;
}

void wtk_pitch_reset(wtk_pitch_t *p)
{
	wtk_flta_reset(p->in);
	wtk_flta_reset(p->out);
	wtk_flta_reset(p->in_fifo);
	wtk_flta_reset(p->out_fifo);
	wtk_flta_reset(p->fft_worksp);
	wtk_flta_reset(p->last_phase);
	wtk_flta_reset(p->sum_phase);
	wtk_flta_reset(p->output_accum);
	wtk_flta_reset(p->ana_freq);
	wtk_flta_reset(p->ana_magn);
	wtk_flta_reset(p->syn_freq);
	wtk_flta_reset(p->syn_magn);

	wtk_flta_zero(p->in_fifo);
	wtk_flta_zero(p->out_fifo);
	wtk_flta_zero(p->in);
	wtk_flta_zero(p->out);

	wtk_flta_zero(p->last_phase);
	wtk_flta_zero(p->sum_phase);
	wtk_flta_zero(p->output_accum);

	wtk_strbuf_reset(p->buf);
	p->rover=p->cfg->latency;
}



static void smbFft(float *fftBuffer, long fftFrameSize, long sign)
/*
	FFT routine, (C)1996 S.M.Bernsee. Sign = -1 is FFT, 1 is iFFT (inverse)
	Fills fftBuffer[0...2*fftFrameSize-1] with the Fourier transform of the
	time domain data in fftBuffer[0...2*fftFrameSize-1]. The FFT array takes
	and returns the cosine and sine parts in an interleaved manner, ie.
	fftBuffer[0] = cosPart[0], fftBuffer[1] = sinPart[0], asf. fftFrameSize
	must be a power of 2. It expects a complex input signal (see footnote 2),
	ie. when working with 'common' audio signals our input signal has to be
	passed as {in[0],0.,in[1],0.,in[2],0.,...} asf. In that case, the transform
	of the frequencies of interest is in fftBuffer[0...fftFrameSize].
*/
{
	float wr, wi, arg, *p1, *p2, temp;
	float tr, ti, ur, ui, *p1r, *p1i, *p2r, *p2i;
	long i, bitm, j, le, le2, k;
	int fftFrameSize2;

	fftFrameSize2=2*fftFrameSize;
	for (i = 2; i < fftFrameSize2-2; i += 2) {
		for (bitm = 2, j = 0; bitm < fftFrameSize2; bitm <<= 1) {
			if (i & bitm) j++;
			j <<= 1;
		}
		if (i < j) {
			p1 = fftBuffer+i; p2 = fftBuffer+j;
			temp = *p1; *(p1++) = *p2;
			*(p2++) = temp; temp = *p1;
			*p1 = *p2; *p2 = temp;
		}
	}
	for (k = 0, le = 2; k < (long)(log(fftFrameSize)/log(2.)+.5); k++) {
		le <<= 1;
		le2 = le>>1;
		ur = 1.0;
		ui = 0.0;
		arg = M_PI / (le2>>1);
		wr = cos(arg);
		wi = sign*sin(arg);
		for (j = 0; j < le2; j += 2) {
			p1r = fftBuffer+j; p1i = p1r+1;
			p2r = p1r+le2; p2i = p2r+1;
			for (i = j; i < fftFrameSize2; i += le) {
				tr = *p2r * ur - *p2i * ui;
				ti = *p2r * ui + *p2i * ur;
				*p2r = *p1r - tr; *p2i = *p1i - ti;
				*p1r += tr; *p1i += ti;
				p1r += le; p1i += le;
				p2r += le; p2i += le;
			}
			tr = ur*wr - ui*wi;
			ui = ur*wi + ui*wr;
			ur = tr;
		}
	}
}


void wtk_pitch_feed_frame(wtk_pitch_t *p,float pitch_shift)
{
	int latency=p->cfg->latency;
	int fft_frame_size=p->cfg->fft_frame_size;
	int fft_frame_size2;
	int i,k,qpd,index;
	double magn, phase, tmp, window, real, imag,freq_per_bin;
	double expct;
	short v;
	float thresh;

	fft_frame_size2=fft_frame_size>>1;
	freq_per_bin=p->cfg->sample_rate*1.0/fft_frame_size;
	expct=p->cfg->expct;
	for(i=0;i<p->in->len;++i)
	{
		//wtk_debug("v[%d]=%f\n",i,p->in->p[i]);
		p->in_fifo->p[p->rover]=p->in->p[i];
		p->out->p[i]=p->out_fifo->p[p->rover-latency];
		//wtk_debug("v[%d]=%f\n",p->rover,p->in_fifo->p[p->rover]);
		//wtk_debug("v[%d]=%f\n",i,p->out->p[i]);
		++p->rover;
		if(p->rover>=fft_frame_size)
		{
			//wtk_debug("rover=%d/%d\n",p->rover,fft_frame_size);
			//exit(0);
			p->rover=latency;
			for(k=0;k<fft_frame_size;++k)
			{
				window=-0.5*cos(p->f3*k)+0.5;
				p->fft_worksp->p[2*k]=p->in_fifo->p[k]*window;
				p->fft_worksp->p[2*k+1]=0;
				//wtk_debug("v[%d]=%f/%f/%f\n",k,window,p->fft_worksp->p[2*k],p->in_fifo->p[k]);
			}
			/* do transform */
			smbFft(p->fft_worksp->p, fft_frame_size, -1);

			for(k=0;k<=fft_frame_size2;++k)
			{
				real=p->fft_worksp->p[2*k];
				imag=p->fft_worksp->p[2*k+1];

				//wtk_debug("v[%d]=%f/%f\n",k,real,imag);

				magn=2.0*sqrt(real*real+imag*imag);
				phase=atan2(imag,real);

				tmp=phase-p->last_phase->p[k];
				p->last_phase->p[k]=phase;
				tmp-=(k*expct);
				qpd=tmp/M_PI;
				if(qpd>=0)
				{
					qpd+=qpd&1;
				}else
				{
					qpd-=qpd&1;
				}
				tmp-=M_PI*qpd;
				tmp=tmp*p->f4;//p->cfg->over_sampling*tmp/(2.0*M_PI);
				tmp=(tmp+k)*freq_per_bin;
				p->ana_magn->p[k]=magn;
				p->ana_freq->p[k]=tmp;

				//wtk_debug("v[%d]=%f/%f\n",k,magn,tmp);
			}
			//exit(0);
			wtk_flta_zero(p->syn_magn);
			wtk_flta_zero(p->syn_freq);
			for(k=0;k<=fft_frame_size2;++k)
			{
				index=k*pitch_shift;
				if(index<=fft_frame_size)
				{
					p->syn_magn->p[index]+=p->ana_magn->p[k];
					p->syn_freq->p[index]=p->ana_freq->p[k]*pitch_shift;
				}
			}

			for(k=0;k<=fft_frame_size2;++k)
			{
				magn=p->syn_magn->p[k];
				tmp=((p->syn_freq->p[k]-k*freq_per_bin)/freq_per_bin)*p->f1+k*expct;
				p->sum_phase->p[k]+=tmp;
				phase=p->sum_phase->p[k];
				p->fft_worksp->p[2*k]=magn*cos(phase);
				p->fft_worksp->p[2*k+1]=magn*sin(phase);
			}
			//wtk_debug("tts pitch memdo i=%d\n",i);
			memset(p->fft_worksp->p+fft_frame_size+2,0,(fft_frame_size*2-fft_frame_size-2)*sizeof(float));
			smbFft(p->fft_worksp->p,fft_frame_size,1);
			for(k=0;k<fft_frame_size;++k)
			{
				window=-0.5*cos(p->f3*k)+0.5;
				p->output_accum->p[k]+=p->f2*window*p->fft_worksp->p[2*k];
			}
			memcpy(p->out_fifo->p,p->output_accum->p,p->cfg->step_size*sizeof(float));
			memmove(p->output_accum->p, p->output_accum->p+p->cfg->step_size, fft_frame_size*sizeof(float));
			memmove(p->in_fifo->p,p->in_fifo->p+p->cfg->step_size,sizeof(float)*latency);
		}
	}
	//wtk_strbuf_push(p->buf,p->out->p,p->out->pos*sizeof(float));
	//wtk_debug("out=%d\n",p->out->len);
	//wtk_debug("tts pitch save\n");
	thresh=p->cfg->thresh;
	for(i=0;i<p->out->len;++i)
	{
		tmp=p->out->p[i];
		if(tmp>thresh)
		{
			tmp=thresh;
		}else if(tmp<-thresh)
		{
			tmp=-thresh;
		}
		v=(short)(p->cfg->max_v*tmp);
		wtk_strbuf_push(p->buf,(char*)&(v),2);
	}
	//wtk_debug("tts pitch notify\n");
	if(p->notify && p->buf->pos>0)
	{
		p->notify(p->notify_ths,p->buf->data,p->buf->pos);
		wtk_strbuf_reset(p->buf);
	}
	//wtk_debug("tts pitch notify2\n");
	//wtk_debug("pos=%d\n",p->buf->pos);
	//exit(0);

}


void wtk_pitch_process(wtk_pitch_t *p,float pitch_shift,char *data,int bytes)
{
	wtk_flta_t *in=p->in;
	float scale=p->cfg->scale;
	short *pdata;
	int len;
	int i;

	len=bytes>>1;
	pdata=(short*)data;
	for(i=0;i<len;++i)
	{
		wtk_flta_push(in,pdata[i]*scale);
		//wtk_debug("v[%d] pos=%d/%d\n",i,in->pos,in->len);
		if(in->pos==in->len)
		{
			//wtk_debug("tts pitch v[%d] pos=%d/%d\n",i,in->pos,in->len);
			wtk_pitch_feed_frame(p,pitch_shift);
			//wtk_debug("tts pitch2 v[%d] pos=%d/%d\n",i,in->pos,in->len);
			wtk_flta_reset(in);
		}
	}
}

#include "wtk/core/wtk_riff.h"
#include "wtk/core/wavehdr.h"

int wtk_pitch_convert(wtk_pitch_t *p,float shift,char *in,char *out)
{
#define BUF_SIZE 1024
	char buf[BUF_SIZE];
	wtk_riff_t *riff;
	int ret;

	wtk_pitch_reset(p);
	//wtk_debug("rover=%d\n",p->rover);
	riff=wtk_riff_new();
	ret=wtk_riff_open(riff,in);
	if (ret < 0){
		ret=-1;goto end;
	}
	while(1)
	{
		ret=wtk_riff_read(riff,buf,BUF_SIZE);
		//wtk_debug("ret=%d\n",ret);
		wtk_pitch_process(p,shift,buf,ret);
		if(ret<BUF_SIZE)
		{
			break;
		}
	}
	if (out){
		wave_write_file(out,riff->fmt.sample_rate,p->buf->data,p->buf->pos);
		//wtk_debug("write %s [shift=%f, rate=%d]\n",out,shift,riff->fmt.sample_rate);
	}
	wtk_riff_close(riff);
	wtk_riff_delete(riff);
	ret=0;
end:
	return ret;
}


