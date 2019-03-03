#ifdef __cplusplus
extern "C"
{
#endif
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/mem.h"
#ifdef __cplusplus
};
#endif
#define HAVE_TYPE
#include "qtk_audio_resampler.h"
int qtk_audio_resampler_run(qtk_audio_resampler_t *r,char *buf,int bytes);


qtk_audio_resampler_t* qtk_audio_resampler_new(int dst_samplerate,void *hook,qtk_audio_resampler_write_handler_t write)
{
	qtk_audio_resampler_t *r;

	r=(qtk_audio_resampler_t*)wtk_malloc(sizeof(*r));
	r->ctx=0;
    r->output_size=AVCODEC_MAX_AUDIO_FRAME_SIZE;
    r->output_alloc=(char*)wtk_calloc(1,(r->output_size + 0xF)*2);
	r->output=(short*) (( (long) (r->output_alloc + 0xF) )  & (~0xF));
	//r->buf=wtk_strbuf_new(1024*32,1);
	r->hook=hook;
	r->write=write;
	r->dst_channel=1;
	r->dst_samplerate=dst_samplerate;//16000
	r->dst_samplesize=2;
	r->input_size=4096;
	return r;
}

int qtk_audio_resampler_delete(qtk_audio_resampler_t *r)
{
	qtk_audio_resampler_stop(r);
	wtk_free(r->output_alloc);
	wtk_free(r);
	return 0;
}

int qtk_audio_resampler_start(qtk_audio_resampler_t *r,int channel,int samplerate,int samplesize)
{
	struct AVResampleContext * ctx;

	qtk_audio_resampler_stop(r);
	if((channel!=r->dst_channel) || (samplerate!=r->dst_samplerate) || (samplesize!=r->dst_samplesize))
	{
		r->need_resample=1;
		r->src_channel=channel;
		r->src_samplerate=samplerate;
		r->src_samplesize=samplesize;
		ctx=av_resample_init(r->dst_samplerate, samplerate, 16, 10, 0,0.8);
		r->ctx=ctx;
	}else
	{
		r->need_resample=0;
		r->ctx=0;
	}
	return 0;
}

int qtk_audio_resampler_stop(qtk_audio_resampler_t *r)
{
	struct AVResampleContext *ctx=(struct AVResampleContext *)r->ctx;

	if(ctx)
	{
		av_resample_close(ctx);
		r->ctx=0;
	}
	return 0;
}

int qtk_audio_resampler_feed(qtk_audio_resampler_t *r,char *buf,int bytes)
{
	if(r->need_resample)
	{
		qtk_audio_resampler_run(r,buf,bytes);
	}else
	{
		r->write(r->hook,buf,bytes);
	}
	return 0;
}

int qtk_audio_resampler_run(qtk_audio_resampler_t *r,char *buf,int bytes)
{
	struct AVResampleContext *ctx=(struct AVResampleContext *)r->ctx;
	short *output=(short*)r->output;//r->buf->data;
	int output_sample=r->output_size;//r->buf->length/2;
	short *input=(short*)buf;
	int input_resample=bytes/2;
	int consumed=0,last_consume,ret=0;
	int resample_thresh=4;
	int n,t1,t2,i;
	//short *end=input+input_resample;

	last_consume=input_resample;
	while(1)
	{
		ret = av_resample(ctx,(short*)output, (short*)input, &consumed,input_resample, output_sample, 1);
		//wtk_debug("ret=%d,bytes=%d,c=%d\n",ret,bytes,consumed);
		if(ret<=0 || (consumed<resample_thresh&&last_consume<resample_thresh))
		{
			//wtk_debug("break ..\n");
			ret=0;break;
		}
		last_consume=consumed;
		input += (consumed<<1);
		if (r->src_channel == 2)
		{
			n=ret/2;
			for (i=0;i <n; ++i)
			{
				t1=i<<1;t2=t1<<1;
				output[t1]=output[t2];
				output[t1+1]=output[t2+1];
			}
		}
		else
		{
			ret=ret<<1;
		}
		r->write(r->hook,(char*)output,ret);
		if (consumed == 0)
		{
			wtk_debug("breakdd ..\n");
			ret=0;
			break;
		}
		input_resample -= consumed;
	}
	return ret;
}

int qtk_audio_resampler_run2(qtk_audio_resampler_t *r,int is_last,short *data,int len,int *pconsumed,wtk_strbuf_t *buf)
{
	struct AVResampleContext *ctx=(struct AVResampleContext *)r->ctx;
	short *output=(short*)r->output;//r->buf->data;
	int output_sample=r->output_size;//r->buf->length/2;
	short *input=(short*)data;
	int input_resample=len;
	int consumed=0,last_consume,ret=0;
	int resample_thresh=4;
	//short *end=input+input_resample;
	int tot_consumed;

	last_consume=input_resample;
	tot_consumed=0;
	while(1)
	{
		if(!is_last && (input_resample)<r->input_size)
		{
			//wtk_debug("break: %d,input=%d,len=%d,left=%d\n",is_last,r->input_size,len,(e-s));
			ret=0;
			break;
		}
		if(input_resample==0)
		{
			ret=0;
			break;
		}
		ret = av_resample(ctx,(short*)output, (short*)input, &consumed,input_resample, output_sample, 1);
		if(ret<=0 || (consumed<resample_thresh&&last_consume<resample_thresh))
		{
			//wtk_debug("break ..\n");
			ret=0;break;
		}
		tot_consumed+=consumed;
		last_consume=consumed;
		input += (consumed<<1);
		ret=ret<<1;
		wtk_strbuf_push(buf,(char*)output,ret);
		if (consumed == 0)
		{
			//wtk_debug("breakdd ..\n");
			ret=0;
			break;
		}
		input_resample -= consumed;
	}
	*pconsumed=tot_consumed;
	return ret;
}

int qtk_audio_resampler_run3(qtk_audio_resampler_t *r,int is_last,short *data,int len,int *pconsumed,wtk_strbuf_t *buf)
{
	/*
	short *s,*e;
	int consumed;
	int ret,t;
	int step=

	s=data;e=data+len;
	consumed=0;
	while(s<e)
	{
		//ret=qtk_audio_resampler_run2(r,is_last,s,)
	}
end:
	*pconsumed=consumed;
	return ret;
*/
	return 0;
}

