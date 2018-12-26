#include "wtk_flog.h"
#include "wtk/core/wtk_os.h"
void wtk_flog_reset(wtk_flog_t *f);

wtk_flog_t* wtk_flog_new(int max_pend)
{
	wtk_flog_t *f;

	f=(wtk_flog_t*)wtk_calloc(1,sizeof(*f));
	f->max_pend=max_pend;
	wtk_flog_reset(f);
	return f;
}

int wtk_flog_delete(wtk_flog_t *f)
{
	wtk_flog_reset(f);
	wtk_free(f);
	return 0;
}

void wtk_flog_reset(wtk_flog_t *f)
{
	f->writed=0;
	f->pending=0;
	f->write_cb=0;
	f->app_data=0;
	wtk_flog_close(f);
}

void wtk_flog_set_cb(wtk_flog_t *f,void *app_data,wtk_flog_write_cb_f cb)
{
	f->app_data=app_data;
	f->write_cb=cb;
}

int wtk_flog_open(wtk_flog_t *f,char *fn,void *cb_hook,wtk_flog_write_cb_f cb)
{
	int ret;

	wtk_flog_reset(f);
	wtk_mkdir_p(fn,'/',0);
	f->file=fopen(fn,"wb");
	ret=f->file?0:-1;
	if(ret!=0){goto end;}
	wtk_flog_set_cb(f,cb_hook,cb);
	if(f->write_cb)
	{
		ret=f->write_cb(f->app_data,WTK_FLOG_OPEN,f);
		if(ret<0){goto end;}
	}
	ret=0;
end:
	return ret;
}

int wtk_flog_close(wtk_flog_t *f)
{
	if(f->file)
	{
		wtk_flog_flush(f);
		if(f->write_cb)
		{
			f->write_cb(f->app_data,WTK_FLOG_CLOSE,f);
		}
		fclose(f->file);
		f->file=0;
	}
	return 0;
}

int wtk_flog_write(wtk_flog_t *f,const char *data,int bytes)
{
	int ret;

	ret=fwrite(data,bytes,1,f->file);
	if(ret!=1){ret=-1;goto end;}
	f->writed+=bytes;
	f->pending+=bytes;
	if(f->max_pend>=0 && f->pending>f->max_pend)
	{
		ret=wtk_flog_flush(f);
	}else
	{
		ret=0;
	}
end:
	return ret;
}

int wtk_flog_flush(wtk_flog_t *f)
{
	int ret;

	if(f->pending<=0){return 0;}
	if(f->write_cb)
	{
		ret=f->write_cb(f->app_data,WTK_FLOG_FLUSH,f);
		if(ret!=0){goto end;}
	}
	ret=fflush(f->file);
	if(ret!=0){goto end;}
	f->pending=0;
end:
	return ret;
}

