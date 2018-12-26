#include "wtk_dnnsvrd.h" 
#include "wtk/os/wtk_proc.h"
#include "wtk_dnnsvrd.h"
int wtk_dnnsvrd_start(wtk_dnnsvrd_t *s);
int wtk_dnnsvrd_stop(wtk_dnnsvrd_t *s);
int wtk_dnnsvrd_join(wtk_dnnsvrd_t *s);

wtk_dnnsvrd_t* wtk_dnnsvrd_new(wtk_dnnsvrd_cfg_t *cfg)
{
	wtk_dnnsvrd_t *d;
	int ret=-1;

	d=(wtk_dnnsvrd_t*)wtk_calloc(1,sizeof(wtk_dnnsvrd_t));
	d->cfg=cfg;
	d->dnnsvr=wtk_dnnsvr_new(&(cfg->dnnsvr));
	if(!d->dnnsvr){goto end;}
	d->daemon=wtk_daemon_new(&(cfg->daemon),d->dnnsvr->nk->log,d,
			(wtk_daemon_start_f)wtk_dnnsvrd_start,
			(wtk_daemon_stop_f)wtk_dnnsvrd_stop,
			(wtk_daemon_join_f)wtk_dnnsvrd_join);
	ret=0;
end:
	if(ret!=0)
	{
		wtk_dnnsvrd_delete(d);
		d=0;
	}
	return d;
}

void wtk_dnnsvrd_delete(wtk_dnnsvrd_t *s)
{
	if(s->dnnsvr)
	{
		wtk_dnnsvr_delete(s->dnnsvr);
	}
	if(s->daemon)
	{
		wtk_daemon_delete(s->daemon);
	}
	wtk_free(s);
}

int wtk_dnnsvrd_start(wtk_dnnsvrd_t *s)
{
    wtk_dnnupsvr_cfg_t *upsvr = &s->cfg->dnnsvr.upsvr;
    if(!upsvr->dnn.gpu) {
      printf("wtk_cudnn_cfg_update_cuda\r\n");
      wtk_cudnn_cfg_update_cuda(&upsvr->dnn);
    }
    wtk_dnnupsvr_init(s->dnnsvr->upsvr);
	return wtk_dnnsvr_start(s->dnnsvr);
}

int wtk_dnnsvrd_stop(wtk_dnnsvrd_t *s)
{
	return wtk_dnnsvr_stop(s->dnnsvr);
}

int wtk_dnnsvrd_join(wtk_dnnsvrd_t *s)
{
	return wtk_dnnsvr_join(s->dnnsvr);
}

int wtk_dnnsvrd_run(wtk_dnnsvrd_t *s)
{
	return wtk_daemon_run(s->daemon);
}

