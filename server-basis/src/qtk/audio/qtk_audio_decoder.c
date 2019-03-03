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
#include "qtk_audio_decoder.h"
#include "qtk_decode.h"
int qtk_audio_decoder_read(qtk_audio_decoder_t *d,uint8_t *buf,int buf_size);
int qtk_audio_decoder_write(qtk_audio_decoder_t *d,char *buf,int buf_size);
int qtk_audio_decoder_done(qtk_audio_decoder_t *d);
qtk_adevent_t* qtk_audio_decoder_new_adevent(qtk_audio_decoder_t *d);
int qtk_audio_decoder_done_resample(qtk_audio_decoder_t *d,char *buf,int buf_size);
int qtk_audio_decoder_run(qtk_audio_decoder_t *d,wtk_thread_t *thread);

qtk_audio_decoder_t* qtk_audio_decoder_new(int dst_samplerate)
{
	qtk_audio_decoder_t *d;
	int size=1024*1024;
	float rate=1;
	int ret;

	d=(qtk_audio_decoder_t*)wtk_calloc(1,sizeof(*d));
	wtk_lock_init(&(d->input_hoard_lock));
	wtk_hoard_init(&(d->input_hoard),offsetof(qtk_adevent_t,hoard_n),10,(wtk_new_handler_t)qtk_audio_decoder_new_adevent,(wtk_delete_handler_t)qtk_adevent_delete,d);
	d->parser=qtk_audio_parser_new(d,(qtk_audio_parser_read_handler_t)qtk_audio_decoder_read,
			(qtk_audio_parser_write_handler_t)qtk_audio_decoder_write,
			(qtk_audio_parser_done_handler_t)qtk_audio_decoder_done);
#ifdef USE_LIBRESAMPLE
	d->resampler=wtk_resample2_new(8192);
#else
	d->resampler=qtk_audio_resampler_new(dst_samplerate,d,(qtk_audio_resampler_write_handler_t)qtk_audio_decoder_done_resample);
#endif
#ifdef DEBUG_RESAMPLE_WAV
	d->output_wav_buf=wtk_strbuf_new(size,rate);
#endif
	d->decode_audio=wtk_strbuf_new(size,rate);
	d->resample_audio=wtk_strbuf_new(size,rate);
	d->input_audio=wtk_strbuf_new(size,rate);
	d->temp_buf=wtk_strbuf_new(size,rate);
	ret=wtk_sem_init(&(d->start_sem),0);
	if(ret!=0){goto end;}
	ret=wtk_sem_init(&(d->output_sem),0);
	if(ret!=0){goto end;}
	ret=wtk_blockqueue_init(&(d->input_queue));
	if(ret!=0){goto end;}
	ret=wtk_thread_init(&(d->decoder_thread),(thread_route_handler)qtk_audio_decoder_run,d);
	if(ret!=0){goto end;}
	ret=wtk_thread_start(&(d->decoder_thread));
end:
	if(ret!=0)
	{
		qtk_audio_decoder_delete(d);
		d=0;
	}
	return d;
}

int qtk_audio_decoder_stop_thread(qtk_audio_decoder_t *d)
{
	d->run=0;
	wtk_sem_release(&(d->start_sem),1);
    wtk_blockqueue_wake(&d->input_queue);
	return wtk_thread_join(&(d->decoder_thread));
}

int qtk_audio_decoder_delete(qtk_audio_decoder_t *d)
{
	qtk_audio_decoder_stop_thread(d);
	wtk_thread_clean(&(d->decoder_thread));
#ifdef DEBUG_RESAMPLE_WAV
	wtk_strbuf_delete(d->output_wav_buf);
#endif
	wtk_strbuf_delete(d->input_audio);
	wtk_strbuf_delete(d->decode_audio);
	wtk_strbuf_delete(d->resample_audio);
	wtk_strbuf_delete(d->temp_buf);
	qtk_audio_parser_delete(d->parser);
#ifdef USE_LIBRESAMPLE
	wtk_resample2_delete(d->resampler);
#else
	qtk_audio_resampler_delete(d->resampler);
#endif
	wtk_sem_clean(&(d->start_sem));
	wtk_sem_clean(&(d->output_sem));
	wtk_blockqueue_clean(&(d->input_queue));
	wtk_lock_clean(&(d->input_hoard_lock));
	wtk_hoard_clean(&(d->input_hoard));
	wtk_free(d);
	return 0;
}

qtk_adevent_t* qtk_audio_decoder_new_adevent(qtk_audio_decoder_t *d)
{
	return qtk_adevent_new(d->parser->input_size);
}

qtk_adevent_t* qtk_audio_decoder_pop_adevent(qtk_audio_decoder_t *d)
{
	wtk_lock_t *l=&(d->input_hoard_lock);
	qtk_adevent_t *evt=0;
	int ret;

	ret=wtk_lock_lock(l);
	if(ret!=0){goto end;}
	evt=(qtk_adevent_t*)wtk_hoard_pop(&(d->input_hoard));
	wtk_lock_unlock(l);
end:
	return evt;
}

int qtk_audio_decoder_push_adevent(qtk_audio_decoder_t *d,qtk_adevent_t *e)
{
	wtk_lock_t *l=&(d->input_hoard_lock);
	int ret;

	qtk_adevent_reset(e);
	ret=wtk_lock_lock(l);
	if(ret!=0){goto end;}
	ret=wtk_hoard_push(&(d->input_hoard),&(e->hoard_n));
	wtk_lock_unlock(l);
end:
	return ret;
}

int qtk_audio_decoder_read(qtk_audio_decoder_t *d,uint8_t *buf,int buf_size)
{
	qtk_adevent_t *event;
	wtk_queue_node_t *n;
	int cpy=0;

	if(d->is_end){return 0;}
	n=wtk_blockqueue_pop(&(d->input_queue),-1,0);
	event=data_offset(n,qtk_adevent_t,q_n);
	switch(event->state)
	{
	case WTK_ADBLOCK_APPEND:
		cpy=event->buf->pos;
		memcpy(buf,event->buf->data,cpy);
		qtk_audio_decoder_push_adevent(d,event);
		break;
	case WTK_ADBLOCK_END:
		qtk_audio_decoder_push_adevent(d,event);
		wtk_sem_release(&(d->output_sem),1);
		cpy=qtk_audio_decoder_read(d,buf,buf_size);
		break;
	case WTK_ADSTREAM_END:
		cpy=event->buf->pos;
		if(cpy>0)
		{
			memcpy(buf,event->buf->data,cpy);
		}
		qtk_audio_decoder_push_adevent(d,event);
		d->is_end=1;
		break;
	}
	//wtk_debug("############### cpy=%d#################\n",cpy);
	//print_data((char*)buf,cpy);
	return cpy;
}

int qtk_audio_decoder_write(qtk_audio_decoder_t *d,char *buf,int buf_size)
{
	//wtk_debug("buf_size=%d\n",buf_size);
	wtk_strbuf_push(d->decode_audio,buf,buf_size);
	return 0;
}

int qtk_audio_decoder_done(qtk_audio_decoder_t *d)
{
	wtk_sem_release(&(d->output_sem),1);
	return 0;
}

int qtk_audio_decoder_done_resample(qtk_audio_decoder_t *d,char *buf,int buf_size)
{
	wtk_strbuf_push(d->resample_audio,buf,buf_size);
	return 0;
}

int qtk_audio_decoder_run(qtk_audio_decoder_t *d,wtk_thread_t *thread)
{
	int ret=0;

	d->run=1;
	while(d->run)
	{
		ret=wtk_sem_acquire(&(d->start_sem),-1);
		if(!d->run){break;}
		if(ret!=0){goto end;}
		d->is_decode=1;
		//wtk_debug("parser start ...\n");
		ret=qtk_audio_parser_run(d->parser);
		//wtk_debug("parser end ...\n");
		d->is_decode=0;
		if(ret!=0)
		{
			wtk_debug("decode failed.\n");
		}
	}
end:
	return ret;
}

int qtk_audio_decoder_prepare(qtk_audio_decoder_t *d,int type,int channel,int samplerate,int samplebytes)
{
	qtk_audio_parser_t *p=d->parser;
	int ret=0;

	d->channel=channel;
	d->sampelrate=samplerate;
	d->samplebytes=samplebytes;
	d->need_resample=1;
	p->use_ogg=0;
	switch(type)
	{
	case AUDIO_MP3:
		p->audio_type="mp3";
		p->code_id=CODEC_ID_MP3;
		d->need_decode=1;
		break;
	case AUDIO_OGG:
		p->audio_type="ogg";
		p->use_ogg=1;
		d->need_decode=1;
		break;
	case AUDIO_FLV:
		p->audio_type="flv";
		//p->code_id=CODEC_ID_SPEEX;
		p->code_id=CODEC_ID_NELLYMOSER;
		d->need_decode=1;
		break;
	case AUDIO_WAV:
		d->need_decode=0;
		break;
	default:
		ret=-1;
		break;
	}
	//wtk_debug("type=%d, %s\n",type,p->audio_type);
	if(ret==0)
	{
		int dst_rate=d->resampler->dst_samplerate;

		if(d->sampelrate==dst_rate)
		{
			d->need_resample=0;
		}else
		{
			d->need_resample=1;
#ifdef USE_LIBRESAMPLE
			ret=wtk_resample2_start(d->resampler,d->sampelrate,dst_rate);
#else
			ret=qtk_audio_resampler_start(d->resampler,d->channel,d->sampelrate,d->samplebytes);
#endif
		}
	}
	return ret;
}


int qtk_audio_decoder_start(qtk_audio_decoder_t *d,int type,int channel,int samplerate,int samplebytes)
{
	int ret;

	//wtk_debug("type=%d,channel=%d,samplerate=%d,samplebytes=%d\n",type,channel,samplerate,samplebytes);
	if(d->is_decode)
	{
		qtk_audio_decoder_feed(d,0,0,1);
	}
#ifdef DEBUG_RESAMPLE_WAV
	wtk_strbuf_reset(d->output_wav_buf);
#endif
	wtk_strbuf_reset(d->decode_audio);
	wtk_strbuf_reset(d->resample_audio);
	wtk_strbuf_reset(d->input_audio);
	wtk_strbuf_reset(d->temp_buf);
	d->is_end=0;
	ret=qtk_audio_decoder_prepare(d,type,channel,samplerate,samplebytes);
	if(ret!=0){goto end;}
	if(d->need_decode)
	{
		ret=wtk_sem_release(&(d->start_sem),1);
	}
end:
	return ret;
}

int qtk_audio_decoder_feed_event(qtk_audio_decoder_t *d,qtk_adevent_t *e)
{
	return wtk_blockqueue_push(&(d->input_queue),&(e->q_n));;
}

int qtk_audio_decoder_push(qtk_audio_decoder_t *d,char *c,int bytes,int is_end)
{
	wtk_strbuf_t *buf;
	qtk_adevent_t *event;
	int block_size;

	if(bytes<=0)
	{
		if(is_end)
		{
			goto raise;
		}else
		{
			goto end;
		}
	}
	buf=d->input_audio;
	wtk_strbuf_push(buf,c,bytes);
	if(d->parser->use_ogg)
	{
		if(buf->pos>0)
		{
			event=qtk_audio_decoder_pop_adevent(d);
			qtk_adevent_push(event,buf->data,buf->pos);
			event->state=WTK_ADBLOCK_APPEND;
			qtk_audio_decoder_feed_event(d,event);
			wtk_strbuf_reset(buf);
		}
	}else
	{
		block_size=d->parser->input_size;
		while(buf->pos>=block_size)
		{
			event=qtk_audio_decoder_pop_adevent(d);
			qtk_adevent_push(event,buf->data,block_size);
			event->state=WTK_ADBLOCK_APPEND;
			qtk_audio_decoder_feed_event(d,event);
			wtk_strbuf_pop(buf,0,block_size);
		}
	}
raise:
	event=qtk_audio_decoder_pop_adevent(d);
	if(is_end)
	{
		qtk_adevent_push(event,d->input_audio->data,d->input_audio->pos);
		event->state=WTK_ADSTREAM_END;
		wtk_strbuf_reset(d->input_audio);
	}else
	{
		event->state=WTK_ADBLOCK_END;
	}
	qtk_audio_decoder_feed_event(d,event);
end:
	return 0;
}

void qtk_audio_decoder_save_file(qtk_audio_decoder_t *d,wtk_strbuf_t *buf,int rate)
{
	WaveHeader hdr,*phdr=&hdr;
	static int i=0;
	char tmp[32];
	FILE* f;

	sprintf(tmp,"wtk.%d.wav",++i);
	f=fopen(tmp,"wb");
	wavehdr_init(phdr);
	wavehdr_set_fmt(phdr,1,rate,2);
	wavehdr_set_size(phdr,buf->pos);
	fwrite(phdr,sizeof(hdr),1,f);
	fwrite(buf->data,buf->pos,1,f);
	fclose(f);
}

int qtk_audio_decoder_feed(qtk_audio_decoder_t *d,char *c,int bytes,int is_end)
{
	int ret=0;
	short *data;
	int len,consumed;

	//wtk_debug("bytes: %d\n",bytes);
	//wtk_debug("is_end: %d\n",is_end);
	wtk_strbuf_reset(d->resample_audio);
	wtk_strbuf_reset(d->decode_audio);
	if(d->need_decode)
	{
		qtk_audio_decoder_push(d,c,bytes,is_end);
		if(bytes>0 || is_end)
		{
			ret=wtk_sem_acquire(&(d->output_sem),-1);
			if(ret!=0)
			{
				wtk_debug("audio sem acquire failed...\n");
				goto end;
			}
		}
		c=d->decode_audio->data;
		bytes=d->decode_audio->pos;
	}
	//wtk_debug("bytes: %d\n",bytes);
	if(d->need_resample)
	{
		wtk_strbuf_push(d->temp_buf,c,bytes);
		consumed=0;
		data=(short*)d->temp_buf->data;
		len=d->temp_buf->pos/2;
		//wtk_debug("bytes=%d\n",len);
		if(len>=d->resampler->input_size || is_end)
		{
			//wtk_debug("len: %d\n",len);
#ifdef USE_LIBRESAMPLE
			ret=wtk_resample2_process(d->resampler,is_end,data,len,&consumed,d->resample_audio);
#else
			ret=qtk_audio_resampler_run2(d->resampler,is_end,data,len,&consumed,d->resample_audio);
#endif
			if(consumed>0)
			{
				wtk_strbuf_pop(d->temp_buf,0,consumed*2);
			}
			if(ret!=0){goto end;}
		}
	}else
	{
		wtk_strbuf_push(d->resample_audio,c,bytes);
	}
	//wtk_debug("bytes: %d\n",bytes);
#ifdef DEBUG_RESAMPLE_WAV
	wtk_strbuf_push(d->output_wav_buf,d->resample_audio->data,d->resample_audio->pos);
	//wtk_strbuf_push(d->output_wav_buf,d->decode_audio->data,d->decode_audio->pos);
	if(is_end)
	{
		qtk_audio_decoder_save_file(d,d->output_wav_buf,16000);
		//qtk_audio_decoder_save_file(d,d->output_wav_buf,22050);
	}
#endif
	ret=0;
	//wtk_debug("resample bytes: %d\n",d->resample_audio->pos);
end:
	//wtk_debug("#######################end %d...\n",d->resample_audio->pos);
	return ret;
}

wtk_strbuf_t* qtk_audio_decoder_get_audio(qtk_audio_decoder_t *d)
{
	if(d->resampler->dst_samplerate==d->sampelrate)
	{
		return d->decode_audio;
	}else
	{
		return d->resample_audio;
	}
}
