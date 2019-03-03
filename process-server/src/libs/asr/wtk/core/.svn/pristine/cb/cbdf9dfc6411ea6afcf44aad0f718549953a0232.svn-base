#include "wtk_wavfile.h"
void wtk_wavfile_close_fd(wtk_wavfile_t *f);

wtk_wavfile_t* wtk_wavfile_new(int sample_rate)
{
	wtk_wavfile_t *f;

	f=(wtk_wavfile_t*)wtk_calloc(1,sizeof(*f));
	wtk_wavfile_init(f,sample_rate);
	return f;
}

int wtk_wavfile_delete(wtk_wavfile_t *f)
{
	wtk_wavfile_clean(f);
	wtk_free(f);
	return 0;
}

void wtk_wavfile_init(wtk_wavfile_t *f,int sample_rate)
{
	f->file=0;
	f->max_pend=-1;
	f->pending=0;
	f->writed=0;
	wavehdr_init(&(f->hdr));
	wavehdr_set_fmt(&(f->hdr),1,sample_rate,2);
}

void wtk_wavfile_set_channel(wtk_wavfile_t *f,int c)
{
	wtk_wavfile_set_channel2(f,c,2);
}

void wtk_wavfile_set_channel2(wtk_wavfile_t *f,int c,int bytes_per_sample)
{
	wavehdr_set_fmt(&(f->hdr),c,f->hdr.fmt_sample_rate,bytes_per_sample);
}

void wtk_wavfile_clean(wtk_wavfile_t *f)
{
	wtk_wavfile_close(f);
	//wtk_wavfile_close_fd(f);
}

void wtk_wavfile_close_fd(wtk_wavfile_t *f)
{
	if(f->file)
	{
		fclose(f->file);
		f->file=0;
	}
}

int wtk_wavfile_open(wtk_wavfile_t *f,char *fn)
{
	int ret=-1;

	f->pending=f->writed=0;
	wtk_wavfile_close_fd(f);
	wtk_mkdir_p(fn,'/',0);
	f->file=fopen(fn,"wb");
	if(!f->file){goto end;}
	ret=fwrite(&(f->hdr),sizeof(f->hdr),1,f->file);
	ret=ret==1?0:-1;
end:
	return ret;
}

int wtk_wavfile_open2(wtk_wavfile_t *f,char *prev)
{
	char buf[1024];
	static int ki=0;

	++ki;
	//sprintf(buf,"%s.%f.wav",prev,time_get_ms());
	sprintf(buf,"%s.%d.wav",prev,ki);
	wtk_debug(" ==========> open %s\n",buf);
	return wtk_wavfile_open(f,buf);
}

int wtk_wavfile_write(wtk_wavfile_t *f,const char *data,int bytes)
{
	int ret;

	ret=fwrite(data,bytes,1,f->file);
	if(ret!=1){ret=-1;goto end;}
	f->writed+=bytes;
	f->pending+=bytes;
	if(f->max_pend>=0 && f->pending>f->max_pend)
	{
		ret=wtk_wavfile_flush(f);
	}else
	{
		ret=0;
	}
end:
	return ret;
}

int wtk_wavfile_writef(wtk_wavfile_t *f,float *data,int len)
{
	short *v;
	int i,ret;

	v=(short*)wtk_malloc(len*sizeof(short));
	for(i=0;i<len;++i)
	{
		v[i]=data[i]*30000;
	}
	ret=wtk_wavfile_write(f,(char*)v,len*2);
	wtk_free(v);
	return ret;
}

void wtk_wavfile_write_float(wtk_wavfile_t *f,float **data,int len)
{
	int channel,i,j;
	short *pv;
	short *p;

//	print_float(data[0],min(len,10));
//	print_float(data[1],min(len,10));
	channel=f->hdr.fmt_channels;
	pv=(short*)wtk_malloc(len*channel*sizeof(short));
	p=pv;
	for(i=0;i<len;++i)
	{
		for(j=0;j<channel;++j)
		{
			*(p++)=data[j][i]*30000;
		}
	}
	wtk_wavfile_write(f,(char*)pv,len*channel*sizeof(short));
//	wtk_free(pv);
//	exit(0);
}

int wtk_wavfile_flush(wtk_wavfile_t *f)
{
	int ret;

	ret=fseek(f->file,0,SEEK_SET);
	if(ret==-1){goto end;}
	wavehdr_set_size(&(f->hdr),f->writed);
	ret=fwrite(&(f->hdr),sizeof(f->hdr),1,f->file);
	if(ret!=1){ret=-1;goto end;}
	fflush(f->file);
	//fseek(f->file,0,SEEK_END);
	fseek(f->file,f->writed+sizeof(f->hdr),SEEK_SET);
	f->pending=0;
	ret=0;
end:
	return ret;
}

int wtk_wavfile_cancel(wtk_wavfile_t *f,int n)
{
	int ret;

	if(f->writed<=0)
	{
		ret=0;
		goto end;
	}
	f->writed-=n;
	if(f->writed<0)
	{
		f->writed=0;
	}
	ret=wtk_wavfile_flush(f);
	if(ret!=0){goto end;}
end:
	return ret;
}

int wtk_wavfile_close(wtk_wavfile_t *f)
{
	int ret;

	if(!f->file){ret=0;goto end;}
	ret=wtk_wavfile_flush(f);
	if(ret!=0){goto end;}
	wtk_wavfile_close_fd(f);
end:
	return ret;
}


void wtk_wavfile_write_mc(wtk_wavfile_t *f,short **mic,int len)
{
	int i,j;
	int chanel=f->hdr.fmt_channels;

	//wtk_debug("len=%d\n",len);
	for(i=0;i<len;++i)
	{
		for(j=0;j<chanel;++j)
		{
			wtk_wavfile_write(f,(char*)(mic[j]+i),2);
		}
	}
}

void wtk_wavfile_write_int(wtk_wavfile_t *f,int **mic,int len)
{
	int i,j;
	int chanel=f->hdr.fmt_channels;

	//wtk_debug("len=%d\n",len);
	for(i=0;i<len;++i)
	{
		for(j=0;j<chanel;++j)
		{
			wtk_wavfile_write(f,(char*)(mic[j]+i),4);
		}
	}
}

