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
#include "qtk_audio_parser.h"
#include "qtk_ogg_dec.h"
#include "qtk/audio/qtk_audio_decoder.h"

static void ffmpeg_log(void * data, int level, const char * fmt, va_list list)
{
	return;
}

qtk_audio_parser_t* qtk_audio_parser_new(void *hook,qtk_audio_parser_read_handler_t read,qtk_audio_parser_write_handler_t write,qtk_audio_parser_done_handler_t done)
{
	qtk_audio_parser_t *p;
	ByteIOContext *ctx;

    av_register_all();
    av_log_set_callback(ffmpeg_log);
    p=(qtk_audio_parser_t*)wtk_calloc(1,sizeof(*p));
	ctx=(ByteIOContext*)wtk_calloc(1,sizeof(ByteIOContext)*2);
	ctx->is_streamed=1;
	p->io_ctx=ctx;
    p->input_size=512+FF_INPUT_BUFFER_PADDING_SIZE;
    p->input=(char*)wtk_calloc(1,p->input_size);
    p->output_size=AVCODEC_MAX_AUDIO_FRAME_SIZE;
    p->output_alloc=(char*)wtk_calloc(1,(p->output_size + 0xF)*2);
    p->output=(short*) (( (long) (p->output_alloc + 0xF) )  & (~0xF));
    p->hook=hook;
    p->read=read;
    p->write=write;
    p->done=done;
    p->use_ogg=0;
    return p;
}

int qtk_audio_parser_delete(qtk_audio_parser_t *p)
{
	wtk_free(p->io_ctx);
	wtk_free(p->input);
	wtk_free(p->output_alloc);
	wtk_free(p);
	return 0;
}

int qtk_audio_parser_run_ffmpeg(qtk_audio_parser_t* d)
{
	AVFormatContext* fmt_ctx;
	AVInputFormat* input_fmt;
	AVCodecContext* codec_ctx;
	AVPacket pkt;
	AVCodec* codec;
	int ret;
	int output;
    ByteIOContext *ctx;
    char *audio_type;

    audio_type=d->audio_type;
    ctx=(ByteIOContext*)d->io_ctx;
	codec_ctx=0;fmt_ctx=0;
	input_fmt = av_find_input_format(audio_type);
	ret=-1;
	if(!input_fmt)
    {
        wtk_debug("%s format not found.\n",audio_type);
		goto end;
	}
    input_fmt->flags |= AVFMT_NOFILE;
	ret=init_put_byte(ctx,(unsigned char*)d->input,d->input_size,0,
			d->hook,d->read,0,0);
	if(ret!=0)
	{
		wtk_debug("init io context failed.\n");
		goto end;
	}
	ret= av_open_input_stream(&(fmt_ctx), ctx, "", input_fmt,NULL);
	if(ret!=0)
	{
		wtk_debug("open stream failed.\n");
		goto end;
	}
	ret=-1;
	codec=avcodec_find_decoder(d->code_id);
	codec_ctx=avcodec_alloc_context2(AVMEDIA_TYPE_AUDIO);
	if(!codec_ctx)
	{
		wtk_debug("code context not found.\n");
		goto end;
	}
	ret = avcodec_open(codec_ctx, codec);
	if(ret!=0)
	{
		wtk_debug("codec %d open failed.",codec_ctx->codec_id);
		goto end;
	}
	while (1)
	{
		ret = av_read_frame(fmt_ctx, &pkt);
		if (pkt.size <= 0 || ret != 0)
		{
			break;
		}
		//wtk_debug("%d,%d,%d\n",codec_ctx->channels,codec_ctx->sample_rate, codec_ctx->bits_per_coded_sample);
		//wtk_debug("si=%d,%d\n",pkt.stream_index,stream_index);
		//there is only one channel in stream.
		//if (pkt.stream_index == stream_index)
		{
			output=AVCODEC_MAX_AUDIO_FRAME_SIZE;
			ret=avcodec_decode_audio3(codec_ctx,d->output,&(output),&pkt);
			//wtk_debug("ret=%d,ouput=%d\n",ret,output);
			if(ret>0)
			{
				d->write(d->hook,(char*)d->output,output);
			}
		}
		av_free_packet(&pkt);
	}
	ret=0;
end:
	d->done(d->hook);
	if(codec_ctx)
	{
		avcodec_close(codec_ctx);
		av_free(codec_ctx);
	}
	if(fmt_ctx){av_close_input_stream(fmt_ctx);}
	return ret;
}



int qtk_audio_parser_run_ogg(qtk_audio_parser_t *p)
{
	qtk_adevent_t *event;
	wtk_queue_node_t *n;
	wtk_ogg_dec_t *dec;
	qtk_audio_decoder_t *d=(qtk_audio_decoder_t*)p->hook;
	int ret;
	int run=1;

	dec=wtk_ogg_dec_new();
	wtk_ogg_dec_start(dec,(wtk_ogg_dec_write_f)p->write,p->hook);
	while(run)
	{
		n=wtk_blockqueue_pop(&(d->input_queue),-1,0);
		event=data_offset(n,qtk_adevent_t,q_n);
		switch(event->state)
		{
		case WTK_ADBLOCK_APPEND:
			if(event->buf->pos>0)
			{
				ret=wtk_ogg_dec_feed(dec,event->buf->data,event->buf->pos);
				if(ret!=0)
				{
					run=0;
				}
			}
			qtk_audio_decoder_push_adevent(d,event);
			break;
		case WTK_ADBLOCK_END:
			qtk_audio_decoder_push_adevent(d,event);
			wtk_sem_release(&(d->output_sem),1);
			break;
		case WTK_ADSTREAM_END:
			if(event->buf->pos>0)
			{
				ret=wtk_ogg_dec_feed(dec,event->buf->data,event->buf->pos);
			}
			qtk_audio_decoder_push_adevent(d,event);
			run=0;
			break;
		}
	}
	wtk_ogg_dec_stop(dec);
	wtk_ogg_dec_delete(dec);
	p->done(p->hook);
	return 0;
}


int qtk_audio_parser_run_ogg2(qtk_audio_parser_t *p)
{
	wtk_ogg_dec_t *dec;
	uint8_t buf[1024];
	int ret;

	dec=wtk_ogg_dec_new();
	wtk_ogg_dec_start(dec,(wtk_ogg_dec_write_f)p->write,p->hook);
	while(1)
	{
		ret=p->read(p->hook,buf,sizeof(buf));
		if(ret<=0){break;}
		ret=wtk_ogg_dec_feed(dec,(char*)buf,ret);
		//wtk_debug("ret=%d\n",ret);
		if(ret!=0){break;}
	}
	wtk_ogg_dec_stop(dec);
	wtk_ogg_dec_delete(dec);
	p->done(p->hook);
	return 0;
}

int qtk_audio_parser_run(qtk_audio_parser_t* p)
{
	int ret;

	if(p->use_ogg)
	{
		ret=qtk_audio_parser_run_ogg(p);
	}
    else
    {
        ret=qtk_audio_parser_run_ffmpeg(p);
    }

	return ret;
}
