#include "wtk_riff.h"
#include "wtk/core/wtk_wavfile.h"

int wtk_riff_hdr_check(wtk_riff_hdr_t *hdr)
{
	int ret=-1;

	if(strncmp(hdr->id,"RIFF",4)!=0)
	{
		goto end;
	}
	if(strncmp(hdr->type,"WAVE",4)!=0)
	{
		goto end;
	}
	ret=0;
end:
	return ret;
}

int wtk_riff_fmt_check(wtk_riff_fmt_t *fmt)
{
	int ret=-1;

	if(strncmp(fmt->id,"fmt ",4)!=0)
	{
		goto end;
	}
	ret=0;
end:
	return ret;
}

int wtk_riff_wavhdr_check(wtk_riff_wavhdr_t *hdr)
{
	int ret=-1;

	//print_data(hdr->id,4);
	if(strncmp(hdr->id,"data",4)!=0)
	{
		goto end;
	}
	ret=0;
end:
	return ret;
}

wtk_riff_t* wtk_riff_new()
{
	wtk_riff_t *w;

	w=(wtk_riff_t*)wtk_malloc(sizeof(wtk_riff_t));
	w->file=NULL;
	w->data_pos=-1;
	return w;
}

void wtk_riff_delete(wtk_riff_t *f)
{
	if(f->file)
	{
		wtk_riff_close(f);
	}
	wtk_free(f);
}

int wtk_riff_open(wtk_riff_t *f,char *fn)
{
	int extra;
	int ret=-1;

	wtk_riff_close(f);
	f->file=fopen(fn,"rb");
	if(!f->file)
	{
		wtk_debug("file open failed:%s\n", fn);
		goto end;
	}
	ret=fread(&(f->riff_hdr),sizeof(wtk_riff_hdr_t),1,f->file);
	if(ret!=1)
	{
		wtk_debug("riff header read failed\n");
		ret=-1;goto end;
	}
	ret=wtk_riff_hdr_check(&(f->riff_hdr));
	if(ret!=0)
	{
		wtk_debug("riff header format is wrong.\n");
		goto end;
	}
	ret=fread(&(f->fmt),sizeof(wtk_riff_fmt_t),1,f->file);
	if(ret!=1)
	{
		wtk_debug("riff fmt read failed.\n");
		ret=-1;goto end;
	}
	ret=wtk_riff_fmt_check(&(f->fmt));
	if(ret!=0)
	{
		wtk_debug("riff fmt format is wrong.\n");
		goto end;
	}
	extra=f->fmt.data_size-16;
	if(extra>0)
	{
		//wtk_debug("extra=%d\n",extra);
		fseek(f->file,extra,SEEK_CUR);
	}
	ret=fread(&(f->wavhdr),sizeof(wtk_riff_wavhdr_t),1,f->file);
	if(ret!=1)
	{
		wtk_debug("wavhdr read failed.\n");
		ret=-1;goto end;
	}
	ret=wtk_riff_wavhdr_check(&(f->wavhdr));
	if(ret!=0)
	{
		wtk_debug("wavhdr format is wrong %4s.\n",f->wavhdr.id);
		goto end;
	}
	f->data_pos=ftell(f->file);
	ret=0;
end:
	return ret;
}

void wtk_riff_rewind(wtk_riff_t *f)
{
	if(f->file && f->data_pos>=0)
	{
		fseek(f->file,f->data_pos,SEEK_SET);
	}
}

int wtk_riff_read(wtk_riff_t *f,char *buf,int size)
{
	int ret;

	ret=fread(buf,1,size,f->file);
	return ret;
}

int wtk_riff_close(wtk_riff_t *f)
{
	if(f->file)
	{
		fclose(f->file);
		f->file=NULL;
	}
	return 0;
}

int wtk_riff_copy_wav(wtk_riff_t *r,char *dst,char sep,int want_len)
{
#define FILE_BUF_SIZE 1024
	char buf[FILE_BUF_SIZE];
	int ret=-1;
	int len;
	int cnt=0;
	int step;
	FILE *fin=r->file;
	wtk_wavfile_t *wavfile = NULL;

	//wtk_debug("want_len=%d\n",want_len);
	ret=wtk_mkdir_p(dst,sep,0);
	if(ret!=0){goto end;}
	wavfile=wtk_wavfile_new(r->fmt.sample_rate);
	ret=wtk_wavfile_open(wavfile,dst);
	if(ret!=0){goto end;}
	while(cnt<want_len)
	{
		step=min(want_len-cnt,FILE_BUF_SIZE);
		//wtk_debug("step=%d\n",step);
		ret=fread(buf,1,step,fin);
		if(ret<0){goto end;}
		if(ret==0){break;}
		len=ret;
		cnt+=len;
		ret=wtk_wavfile_write(wavfile,buf,len);
		if(ret!=0){goto end;}
	}
	ret=0;
end:
	if(wavfile)
	{
		wtk_wavfile_delete(wavfile);
	}
	return ret;
}

int wtk_riff_cpy(char *fn,int s,int e,char *dst)
{
	wtk_riff_t *r;
	int ret;

	wtk_debug("copy [%d-%d]\n",s,e);
	r=wtk_riff_new();
	ret=wtk_riff_open(r,fn);
	if(ret!=0){goto end;}
	ret=fseek(r->file,s,SEEK_CUR);
	if(ret<0){goto end;}
	ret=wtk_riff_copy_wav(r,dst,'/',e-s);
end:
	if(r)
	{
		wtk_riff_delete(r);
	}
	return ret;
}

void wtk_riff_print(wtk_riff_t *f)
{
	printf("========== riff =========\n");
	printf("%.*s\n",4,f->riff_hdr.id);
	printf("%.*s\n",4,f->fmt.id);
	printf("%.*s\n",4,f->wavhdr.id);
	printf("rate: %d\n",f->fmt.sample_rate);
	printf("channel: %d\n",f->fmt.channels);
	printf("compres: %d\n",f->fmt.compression_code);
	printf("bitpersample: %d\n",f->fmt.bit_per_sample);
	printf("size: %d\n",f->wavhdr.data_size);
}

wtk_strbuf_t** wtk_riff_read_channel(char *fn,int *n)
{
	wtk_strbuf_t **bufs=NULL;
	wtk_riff_t *riff;
	int ret=-1;
	short *pv;
	int chan;
	int i,len;

	riff=wtk_riff_new();
	ret=wtk_riff_open(riff,fn);
	if(ret!=0){goto end;}
	chan=riff->fmt.channels;
	if(n)
	{
		*n=chan;
	}
	bufs=(wtk_strbuf_t**)wtk_malloc(sizeof(wtk_strbuf_t*)*chan);
	for(i=0;i<chan;++i)
	{
		bufs[i]=wtk_strbuf_new(10240,1);
	}
	len=sizeof(short)*chan;
	pv=(short*)wtk_malloc(len);
	while(1)
	{
		ret=wtk_riff_read(riff,(char*)pv,len);
		if(ret!=len)
		{
			ret=0;
			break;
		}
		for(i=0;i<chan;++i)
		{
			wtk_strbuf_push(bufs[i],(char*)(pv+i),2);
		}
	}
	wtk_free(pv);
	ret=0;
end:
	wtk_riff_delete(riff);
	return bufs;
}
