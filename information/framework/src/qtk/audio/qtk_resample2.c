#include "qtk_resample2.h"
#include "libresample.h"
#include "wtk/core/wavehdr.h"
#include "wtk/core/wtk_os.h"

wtk_resample2_t* wtk_resample2_new(int cache)
{
	wtk_resample2_t *r;

	r=(wtk_resample2_t*)wtk_calloc(1,sizeof(*r));
	r->input_size=r->output_size=cache;
	r->input=(float*)wtk_calloc(sizeof(float),r->input_size);
	r->output=(float*)wtk_calloc(sizeof(float),r->output_size);
	r->handle=0;
	return r;
}

int wtk_resample2_delete(wtk_resample2_t *r)
{
	wtk_resample2_close(r);
	wtk_free(r->input);
	wtk_free(r->output);
	wtk_free(r);
	return 0;
}

int wtk_resample2_close(wtk_resample2_t *r)
{
	if(r->handle)
	{
		resample_close(r->handle);
		r->handle=0;
	}
	return 0;
}

//static int x=0;

int wtk_resample2_start(wtk_resample2_t *r,int src_rate,int dst_rate)
{
	double factor;

	//wtk_debug("================ start ==================\n");
	wtk_resample2_close(r);
	factor=dst_rate*1.0/src_rate;
	r->handle=resample_open(1,factor,factor);
	r->factor=factor;
	//x=y=0;
	return r->handle?0:-1;
}



int wtk_resample2_process(wtk_resample2_t *r,int is_last,short *data,int len,int *consumed,wtk_strbuf_t *buf)
{
	int i,n;
	short *s,*e,*p;
	int ret,used;
	int last;
	short d;

	//test_resample2(r);
	s=data;
	e=data+len;
	last=0;
	while(1)
	{
		if(!is_last && (e-s)<r->input_size)
		{
			//wtk_debug("break: %d,input=%d,len=%d,left=%d\n",is_last,r->input_size,len,(e-s));
			break;
		}
		p=s;
		for(i=0,n=0;i<r->input_size &&(p<e);++i)
		{
			r->input[i]=*p;
			++p;++n;
		}
		if(is_last && n<r->input_size)
		{
			last=1;
		}
		//wtk_debug("last=%d\n",last);
		used=0;
		ret=resample_process(r->handle,r->factor,r->input,n,last,&used,r->output,r->output_size);
		//wtk_debug("handler=%p,ret=%d,f=%f,%d,last=%d\n",r->handle,ret,r->factor,n,last);
		s+=used;
		for(i=0;i<ret;++i)
		{
			d=(short)(r->output[i]);
			wtk_strbuf_push(buf,(char*)&d,sizeof(short));
		}
		if(ret<=0){break;}
	}
	*consumed=s-data;
	//x+=*consumed;
	//wtk_debug("x=%d\n",x);
	return 0;
}
