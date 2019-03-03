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
#include "qtk_decode.h"

void ffmpeg_log(void * data, int level, const char * fmt, va_list list)
{
	return;
}

wtk_audio_decode_t* wtk_audio_decode_new()
{
	wtk_audio_decode_t *d;
    ByteIOContext *ctx;

    av_register_all();
    av_log_set_callback(ffmpeg_log);
    d=(wtk_audio_decode_t*)wtk_calloc(1,sizeof(wtk_audio_decode_t));
    //this may not be safe.
    ctx=(ByteIOContext*)wtk_calloc(1,sizeof(ByteIOContext)*2);
    ctx->is_streamed=1;
    d->io_ctx=ctx;
    d->input_size=4096+FF_INPUT_BUFFER_PADDING_SIZE;
    d->input=(char*)wtk_calloc(1,d->input_size);
    d->output_size=AVCODEC_MAX_AUDIO_FRAME_SIZE;
    d->output_alloc=(char*)wtk_calloc(1,(d->output_size + 0xF)*2);
    d->output=(short*) (( (long) (d->output_alloc + 0xF) )  & (~0xF));
    d->stack=wtk_stack_new(4906*20,4096000,1);
    wtk_lock_init(&d->l);
    return d;
}

int wtk_audio_decode_delete(wtk_audio_decode_t *d)
{
	wtk_lock_clean(&(d->l));
	wtk_stack_delete(d->stack);
	wtk_free(d->io_ctx);
	wtk_free(d->input);
	wtk_free(d->output_alloc);
	wtk_free(d);
	return 0;
}

static int x=0;

static int read_audio_packet(void* data,uint8_t *buf,int buf_size)
{
	void **magic=(void**)data;
	char **p=(char**)magic[0];
	int *left=(int*)magic[1];
	int nleft;

	//if(x){return 0;}
	nleft=*left;
	if(nleft<buf_size)
	{
		buf_size=nleft;
	}
	if(buf_size==0){return 0;}
	memcpy(buf,*p,buf_size);
	nleft-=buf_size;
	*p+=buf_size;
	*left=nleft;
	wtk_debug("%d\n",buf_size);
	return buf_size;
}

AVInputFormat* get_fmt(char* buf, int len)
{
	AVProbeData probe_data;
	probe_data.filename = "";
	probe_data.buf_size = len;

	probe_data.buf =(unsigned char*)buf;
	AVInputFormat *ret = av_probe_input_format(&probe_data, 1);
	return ret;
}

int wtk_audio_decode(wtk_audio_decode_t* d,char *buf,int size,char* audio_type,wtk_stack_t *s,wtk_log_t *log)
{
	AVFormatContext* fmt_ctx;
	AVInputFormat* input_fmt;
	AVCodecContext* codec_ctx;
	AVPacket pkt;
	AVCodec* codec;
	int ret,stream_index;
	int output;
	WaveHeader hdr,*phdr;
	char *p;
	void *magic[2];
	int left;
#ifdef USE_X
    int i;
#endif
    ByteIOContext *ctx;

  //  wtk_debug("input: %d\n",size);
    ctx=(ByteIOContext*)d->io_ctx;
	codec_ctx=0;fmt_ctx=0;
	//input_fmt=get_fmt(buf,size);
	input_fmt = av_find_input_format(audio_type);
	ret=-1;
	if(!input_fmt)
    {
        wtk_log_log(log,"%s format not found.",audio_type);
		goto end;
	}
    input_fmt->flags |= AVFMT_NOFILE;
	left=size;p=buf;
	magic[0]=&p;
	magic[1]=&left;
	ret=init_put_byte(ctx,(unsigned char*)d->input,d->input_size,0,
			magic,read_audio_packet,0,0);
	//wtk_debug("%d\n",ret);
	if(ret!=0)
	{
		wtk_log_log(log,"%s","init io context failed.");
		goto end;
	}
	x=1;
	ret= av_open_input_stream(&(fmt_ctx), ctx, "", input_fmt,NULL);
	//wtk_debug("%d\n",ret);
	if (ret != 0)
	{
		wtk_log_log(log,"%s","open stream failed");
		goto end;
	}
	//wtk_debug(" .open... \n");
	x=0;
#ifdef USE_X
	ret = av_find_stream_info(fmt_ctx);
	wtk_debug("%d\n",ret);
	if (ret < 0)
	{
		wtk_log_log(log,"stream information not found (%s,%p,%d)",audio_type,fmt_ctx,ret);
		goto end;
	}
	wtk_debug(" found stream information ...\n");
	for (i = 0; i < fmt_ctx->nb_streams; ++i)
	{
		//type=fmt_ctx->streams[i]->codec->codec_type;
		if (fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO
				|| fmt_ctx->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO)
		{
			stream_index = i;
			codec_ctx = fmt_ctx->streams[i]->codec;
			wtk_debug("%#x,%#x\n",codec_ctx->codec_id,CODEC_ID_NELLYMOSER);
			codec = avcodec_find_decoder(codec_ctx->codec_id);
			break;
		}
	}
#else
	//i=0;
    stream_index=1;
	codec_ctx=avcodec_alloc_context();
	//avcodec_get_context_defaults();
	//codec_ctx=avcodec_alloc_context2(AVMEDIA_TYPE_AUDIO);
	codec = avcodec_find_decoder(CODEC_ID_NELLYMOSER);
#endif
	//wtk_debug("stream index: %d\n",stream_index);
	if(!codec_ctx)
	{
		wtk_log_log(log,"%s","code context not found.");
		goto end;
	}
	ret = avcodec_open(codec_ctx, codec);
	if(ret!=0)
	{
		wtk_log_log(log,"codec %d open failed.",codec_ctx->codec_id);
		goto end;
	}
	wavehdr_init(&hdr);
	wavehdr_set_fmt(&hdr, codec_ctx->channels,
			codec_ctx->sample_rate, codec_ctx->bits_per_coded_sample > 0 ? (codec_ctx->bits_per_coded_sample>>3):2);
	wavehdr_set_fmt(&hdr,1,
			22050, 2);
	phdr=(WaveHeader*)s->push->push;
	wtk_stack_push(s,(char*)&hdr,sizeof(hdr));
	while (1)
	{
		ret = av_read_frame(fmt_ctx, &pkt);
		if (pkt.size <= 0 || ret != 0)
		{
			break;
		}
		if (pkt.stream_index == stream_index)
		{
			output=AVCODEC_MAX_AUDIO_FRAME_SIZE;
			ret=avcodec_decode_audio3(codec_ctx,d->output,&(output),&pkt);
			if(ret>0)
			{
				wtk_stack_push(s,(char*)d->output,output);
			}
		}
		av_free_packet(&pkt);
	}
	wavehdr_set_size(phdr,s->len-sizeof(hdr));
	wavehdr_print(phdr);
	ret=0;
end:
	if(codec_ctx){avcodec_close(codec_ctx);}
	if(fmt_ctx){av_close_input_stream(fmt_ctx);}
	//wtk_debug("%d\n",ret);
	{
		char *p;
		int de_len=s->len;

		p=(char*)wtk_malloc(de_len);
		wtk_stack_merge(s,p);
		file_write_buf("decode.wav",p,de_len);
	}
	exit(0);
	return ret;
}

int wtk_audio_resample(wtk_audio_decode_t *d,char *buf,int len,wtk_stack_t *s)
{
	struct AVResampleContext * resample_ctx;
	WaveHeader *phdr,*cphdr;
	int ret,consumed,input_resample,output_sample;
	char *output,*input;
	int channels,count,i,n,t1,t2;
	int resample_thresh=4;

	len-=44;
	if(len<=0){return -1;}
	output = 0;
	phdr = (WaveHeader*)buf;
	if(phdr->fmt_sample_rate<100 || (phdr->fmt_channels!=2 && phdr->fmt_channels!=1))
	{
		return -1;
	}
	resample_ctx = av_resample_init(16000, phdr->fmt_sample_rate, 16, 10, 0,
			0.8);
	wavehdr_set_size(phdr,len);
	cphdr=(WaveHeader*)s->push->push;
	wtk_stack_push(s,(char*)phdr,sizeof(*phdr));
	input=buf+44;
	output=(char*)d->output;
	input_resample=len/2;output_sample=d->output_size/2;
	channels=phdr->fmt_channels;
	count=0;
	while(1)
	{
		ret = av_resample(resample_ctx,(short*)output, (short*)input, &consumed,input_resample, output_sample, 0);
		//printf("%d-%d-%d-%d-%d\n",ret,consumed,input_resample,len,output_sample);
		if(ret<=0 || consumed<resample_thresh){ret=0;break;}
		input += (consumed<<1);
		if (channels == 2)
		{
			count += ret;
			n=ret/2;
			for (i=0;i <n; ++i)
			{
				t1=i<<1;t2=t1<<1;
				output[t1]=output[t2];
				output[t1+1]=output[t2+1];
			}
			wtk_stack_push(s,output,n*2);
			//wtk_stack_push(s,output,ret*2);
		}
		else
		{
			count += ret << 1;
			wtk_stack_push(s, output, ret << 1);
		}
		if (consumed == 0)
		{
			ret=0;
			break;
		}
		input_resample = input_resample - consumed;
	}
	wavehdr_init(cphdr);
	wavehdr_set_fmt(cphdr, 1, 16000, 2);
	wavehdr_set_size(cphdr, count);
	av_resample_close(resample_ctx);
	return ret;
}

int wtk_audio_translate(wtk_audio_decode_t *d,char* buf,int len,int audo_type,char** output,int *output_len,wtk_log_t *log)
{
	int ret;
	WaveHeader *phdr;
	char *p;
	wtk_stack_t *s;
	int de_len;
	char *fmt;

	switch(audo_type)
	{
	case AUDIO_WAV:
		fmt=0;
		break;
	case AUDIO_MP3:
		fmt="mp3";
		break;
	case AUDIO_FLV:
		fmt="flv";
		break;
	default:
		return -1;
	}
	if(len<=0){return -1;}
	ret=wtk_lock_lock(&(d->l));
	s=d->stack;
	if(ret!=0)
	{
		wtk_debug("lock failed.\n");
		goto end;
	}
	wtk_stack_reset(s);
	if(fmt)
	{
		ret=wtk_audio_decode(d,buf,len,fmt,s,log);
		if(ret!=0)
		{
			//wtk_debug0("decode failed.\n");
			goto unlock;
		}
		phdr=(WaveHeader*)s->pop->pop;
		//wavehdr_print(phdr);
		de_len=s->len;
		p=(char*)wtk_malloc(de_len);
		wtk_stack_merge(s,p);
		//file_write_buf("decode.wav",p,de_len);
		if(phdr->fmt_sample_rate==16000 && phdr->fmt_bit_per_sample == 16)
		{
			*output=p; *output_len=de_len;
			ret=0;
			goto unlock;
		}
		wtk_stack_reset(s);
	}else
	{
		p=buf;
		de_len=len;
	}
	if(de_len>44)
	{
		ret=wtk_audio_resample(d,p,de_len,s);
		if(ret==0)
		{
			if(s->len>de_len || !fmt)
			{
				if(fmt){free(p);}
				p=(char*)wtk_malloc(s->len);
			}
			wtk_stack_merge(s,p);
			//file_write_buf("resample.wav",p,s->len);
			*output=p;
			*output_len=s->len;
		}else
		{
			wtk_debug("resample failed.\n");
			if(fmt){free(p);}
			ret=-1;
		}
	}else
	{
		if(fmt){free(p);}
		ret=-1;
	}
unlock:
	wtk_lock_unlock(&(d->l));
end:
	return ret;
}
