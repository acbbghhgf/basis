#include "wtk_mbin_cfg.h"
#include "wtk/core/cfg/wtk_source.h"

wtk_mbin_cfg_t* wtk_mbin_cfg_new(int cfg_bytes,wtk_main_cfg_init_f init,
		wtk_main_cfg_clean_f clean,wtk_main_cfg_update_local_f update_lc,
		wtk_main_cfg_update2_f update,char *bin_fn,char *cfg_fn,wtk_local_cfg_t *custom)
{
	wtk_mbin_cfg_t *cfg;
	wtk_rbin2_item_t *item;
	int ret;
	wtk_source_loader_t sl;

	cfg=(wtk_mbin_cfg_t*)wtk_malloc(sizeof(wtk_mbin_cfg_t));
	cfg->cfg=(void*)wtk_calloc(1,cfg_bytes);
	cfg->init=init;
	cfg->clean=clean;
	cfg->update_lc=update_lc;
	cfg->update=update;
	cfg->rbin=wtk_rbin2_new();
	ret=wtk_rbin2_read(cfg->rbin,bin_fn);
	if(ret!=0)
	{
		wtk_debug("read [%s:%s] failed.",bin_fn,cfg_fn);
		goto end;
	}
	//wtk_debug("f=%p\n",cfg->rbin->f);
	item=wtk_rbin2_get2(cfg->rbin,cfg_fn,strlen(cfg_fn));
	if(!item)
	{
		wtk_debug("read [%s:%s] failed.",bin_fn,cfg_fn);
		ret=-1;goto end;
	}
	cfg->cfile=wtk_cfg_file_new();
	wtk_cfg_file_set_rbin(cfg->cfile,cfg->rbin);
	//wtk_debug("f=%p\n",cfg->rbin->f);
	wtk_cfg_file_add_var_ks(cfg->cfile,"pwd",".",1);
	ret=wtk_cfg_file_feed(cfg->cfile,item->data->data,item->data->len);
	if(ret!=0){goto end;}
	ret=cfg->init(cfg->cfg);
	if(ret!=0){goto end;}
	//wtk_debug("update lc\n");
	if(custom)
	{
		wtk_local_cfg_update(cfg->cfile->main,custom);
		//wtk_local_cfg_print(cfg->cfile->main);
	}
	//exit(0);
	ret=cfg->update_lc(cfg->cfg,cfg->cfile->main);
	if(ret!=0){goto end;}
	sl.hook=cfg->rbin;
	sl.vf=(wtk_source_loader_v_t)wtk_rbin2_load_file;
	cfg->sl=sl;
	ret=cfg->update(cfg->cfg,&sl);
end:
	//wtk_debug("ret=%d\n",ret);
	//wtk_debug("f=%p\n",cfg->rbin->f);
	if(ret!=0)
	{
		wtk_mbin_cfg_delete(cfg);
		cfg=NULL;
		ret=0;
	}
	return cfg;
}

wtk_mbin_cfg_t* wtk_mbin_cfg_new2(int seek_pos,int cfg_bytes,wtk_main_cfg_init_f init,
		wtk_main_cfg_clean_f clean,wtk_main_cfg_update_local_f update_lc,
		wtk_main_cfg_update2_f update,char *bin_fn,char *cfg_fn)
{
	wtk_mbin_cfg_t *cfg;
	wtk_rbin2_item_t *item;
	int ret;
	wtk_source_loader_t sl;

	cfg=(wtk_mbin_cfg_t*)wtk_malloc(sizeof(wtk_mbin_cfg_t));
	cfg->cfg=(void*)wtk_calloc(1,cfg_bytes);
	cfg->init=init;
	cfg->clean=clean;
	cfg->update_lc=update_lc;
	cfg->update=update;
	cfg->rbin=wtk_rbin2_new();
	ret=wtk_rbin2_read2(cfg->rbin,bin_fn,seek_pos);
	if(ret!=0)
	{
		//wtk_debug("read failed\n");
		goto end;
	}
	//wtk_debug("f=%p\n",cfg->rbin->f);
	item=wtk_rbin2_get2(cfg->rbin,cfg_fn,strlen(cfg_fn));
	if(!item){ret=-1;goto end;}
	cfg->cfile=wtk_cfg_file_new();
	//wtk_debug("f=%p\n",cfg->rbin->f);
	wtk_cfg_file_add_var_ks(cfg->cfile,"pwd",".",1);
	ret=wtk_cfg_file_feed(cfg->cfile,item->data->data,item->data->len);
	if(ret!=0){goto end;}
	ret=cfg->init(cfg->cfg);
	if(ret!=0){goto end;}
	//wtk_debug("update lc\n");
	ret=cfg->update_lc(cfg->cfg,cfg->cfile->main);
	if(ret!=0){goto end;}
	sl.hook=cfg->rbin;
	sl.vf=(wtk_source_loader_v_t)wtk_rbin2_load_file;
	cfg->sl=sl;
	ret=cfg->update(cfg->cfg,&sl);
end:
	//wtk_debug("ret=%d\n",ret);
	//wtk_debug("f=%p\n",cfg->rbin->f);
	if(ret!=0)
	{
		wtk_mbin_cfg_delete(cfg);
		cfg=NULL;
		ret=0;
	}
	return cfg;
}

void wtk_mbin_cfg_delete(wtk_mbin_cfg_t *cfg)
{
	cfg->clean(cfg->cfg);
	wtk_free(cfg->cfg);
	if (cfg->cfile){
		wtk_cfg_file_delete(cfg->cfile);
	}
	wtk_rbin2_delete(cfg->rbin);
	wtk_free(cfg);
}

#include "wtk/core/rbin/wtk_rbin.h"

wtk_main_cfg_t *wtk_main_cfg_new_bin(int cfg_bytes,wtk_main_cfg_init_f init,
		wtk_main_cfg_clean_f clean,wtk_main_cfg_update_local_f update_lc,
		wtk_main_cfg_update2_f update,char *bin_fn,char *cfg_fn)
{
	wtk_main_cfg_t *cfg;
	wtk_rbin_t *rbin;
	wtk_ritem_t *item;
	wtk_cfg_file_t *cf=0;
	void *xcfg;
	int ret=-1;
	wtk_source_loader_t sl;

	cfg=(wtk_main_cfg_t*)wtk_calloc(1,sizeof(*cfg));
	cfg->init=init;
	cfg->clean=clean;
	cfg->update_lc=update_lc;
	cfg->update=0;
	cfg->update2=update;
	rbin=wtk_rbin_new();
	ret=wtk_rbin_read(rbin,bin_fn);
	if(ret!=0){goto end;}
	item=wtk_rbin_find(rbin,cfg_fn,strlen(cfg_fn));
	if(!item){goto end;}
	cf=wtk_cfg_file_new();
	wtk_cfg_file_add_var_ks(cf,"pwd",".",1);
	ret=wtk_cfg_file_feed(cf,item->data.data,item->data.len);
	if(ret!=0){goto end;}
	cfg->cfile=cf;
	xcfg=(void*)calloc(1,cfg_bytes);
	ret=cfg->init(xcfg);
	if(ret!=0){goto end;}
	ret=cfg->update_lc(xcfg,cfg->cfile->main);
	if(ret!=0){goto end;}
	sl.hook=rbin;
	sl.vf=(wtk_source_loader_v_t)wtk_rbin_load_file;
	ret=cfg->update2(xcfg,&sl);
	if(ret!=0){goto end;}
	cfg->cfg_bytes=cfg_bytes;
	cfg->cfg=xcfg;
	cf=0;
	ret=0;
end:
	if(cf)
	{
		wtk_cfg_file_delete(cf);
	}
	wtk_rbin_delete(rbin);
	if(ret!=0)
	{
		wtk_free(cfg);
		cfg=0;
	}
	return cfg;
}

