#include "qtk_ifly_tts_module.h" 
#include "qtk_ifly_tts.h"
#include "wtk/core/cfg/wtk_main_cfg.h"
#include "wtk/core/param/wtk_module_param.h"
#include "qtk/audio/qtk_mp3_encoder.h"
#define MODULE qtk_ifly_tts_module
#define WTK_MODULE_FUNC(name) CAT(CAT(MODULE,_),name)

qtk_service_t ifly_tts;
static qtk_mp3encoder_t mp3enc;

static wtk_param_arg_t start_args[]=
{
	wtk_param_arg(WTK_STRING,"text",1),
	wtk_param_arg(WTK_NUMBER,"useStream",1),
};

static wtk_param_arg_t append_args[]=
{
};

static wtk_param_arg_t end_args[]={
};

static wtk_param_t failed_param;
static wtk_param_t append_param;
static wtk_param_t end_param;

static int WTK_MODULE_FUNC(init)(wtk_local_cfg_t *lc,void **module_data)
{
	qtk_service_t *l=&(ifly_tts);
	wtk_module_arg_t *module_arg=&(l->arg);

	module_arg->start.args=start_args;
	module_arg->start.len=sizeof(start_args)/sizeof(wtk_param_arg_t);
	module_arg->append.args=append_args;
	module_arg->append.len=sizeof(append_args)/sizeof(wtk_param_arg_t);
	module_arg->end.args=end_args;
	module_arg->end.len=sizeof(end_args)/sizeof(wtk_param_arg_t);

	wtk_param_set_ref_number(&failed_param,WTK_MODULE_STREAM_ERR);
	wtk_param_set_ref_number(&append_param,WTK_MODULE_STREAM_APPENDED);
	wtk_param_set_ref_number(&end_param,WTK_MODULE_STREAM_END);

    qtk_mp3enc_init(&mp3enc, 4096);

	return 0;
}

static int WTK_MODULE_FUNC(release)(void *module_data)
{
    qtk_mp3enc_clean(&mp3enc);
	return 0;
}

static void* WTK_MODULE_FUNC(new_engine)(void *module_data,wtk_local_cfg_t *lc)
{
	wtk_main_cfg_t *cfg = NULL;
	wtk_string_t *v;

	v=wtk_local_cfg_find_string_s(lc,"cfg");
	//wtk_debug("v=%p\n",v);
	if(!v){goto end;}
	cfg=wtk_main_cfg_new_type(qtk_ifly_cfg,v->data);
end:
	return cfg;
}

static int WTK_MODULE_FUNC(delete_engine)(void *cfg)
{
	if(cfg)
	{
		wtk_main_cfg_delete((wtk_main_cfg_t*)cfg);
	}
	return 0;
}

void* WTK_MODULE_FUNC(new_handler)(void *cfg)
{
	wtk_main_cfg_t *xcfg;
	qtk_ifly_tts_t *ifly;

	xcfg=(wtk_main_cfg_t*)cfg;
	ifly=qtk_ifly_tts_new((qtk_ifly_cfg_t*)(xcfg->cfg));
	return ifly;
}

static int WTK_MODULE_FUNC(delete_handler)(void *cfg,void *h)
{
	qtk_ifly_tts_delete((qtk_ifly_tts_t*)h);
	return 0;
}


static int qtk_ifly_tts_fill_buf(wtk_strbuf_t *sbuf, char *buf, int len, int is_end) {
    (void)is_end;
    wtk_strbuf_push(sbuf, buf, len);
    return 0;
}


static int qtk_ifly_tts_data_ready(wtk_param_t *ret_array, char *buf, int len, int is_end) {
    wtk_param_t *param;
    wtk_param_t *result;
    result = wtk_param_new_bin(buf, len);
	param = ret_array->value.array.params[0];
	ret_array->value.array.params[1] = result;
	ret_array->value.array.len = result?2:1;
    if (is_end) {
        param->value.number = WTK_MODULE_STREAM_END;
    } else {
        param->value.number = WTK_MODULE_STREAM_APPENDED;
        ifly_tts.responser(ifly_tts.instance);
    }
    return 0;
}


static int WTK_MODULE_FUNC(calc)(void *ins,
                                 wtk_module_param_t *module_param,
                                 wtk_param_t *ret_array)
{
    qtk_ifly_tts_t *ifly;
    wtk_param_t *param;
    wtk_func_param_t *func_param;
    int use_stream = 1;
    int ret;
    wtk_strbuf_t *buf = NULL;

    ifly=(qtk_ifly_tts_t*)ins;
    func_param=&(module_param->start);
    //wtk_module_param_print(module_param);
    param=func_param->params[1];
    if (param) {
        use_stream = (int)param->value.number;
    }
    param=func_param->params[0];
    if (use_stream) {
        qtk_mp3enc_set_notifier(&mp3enc, (qtk_mp3enc_ready_f)qtk_ifly_tts_data_ready, ret_array);
    } else {
        buf = wtk_strbuf_new(4096, 1);
        qtk_mp3enc_set_notifier(&mp3enc, (qtk_mp3enc_ready_f)qtk_ifly_tts_fill_buf, buf);
    }
    qtk_ifly_tts_set_notify(ifly, &mp3enc, (qtk_ifly_tts_notify_f)qtk_mp3enc_feed);
    qtk_ifly_tts_start(ifly);
    ret=qtk_ifly_tts_process(ifly, param->value.str.data, param->value.str.len);
    if (ret) {
        param = ret_array->value.array.params[0];
        param->value.number = WTK_MODULE_STREAM_ERR;
        ret_array->value.array.params[1] = NULL;
        ret_array->value.array.len = 1;
    } else if (!use_stream) {
        wtk_param_t *result=wtk_param_new_bin(buf->data,buf->pos);
        param = ret_array->value.array.params[0];
        ret_array->value.array.params[1] = result;
        ret_array->value.array.len = result ? 2 : 1;
        param->value.number = WTK_MODULE_STREAM_END;
    }
    if (!use_stream) {
        wtk_strbuf_delete(buf);
    }
    qtk_mp3enc_reset(&mp3enc);

    return ret;
}


qtk_service_t* qtk_service_ifly_tts_init(void)
{
	qtk_service_t *l=&(ifly_tts);

	wtk_string_set_s(&(l->name),"tts.en");
	l->init_module=WTK_MODULE_FUNC(init);
	l->clean_module=WTK_MODULE_FUNC(release);
	l->create_engine=WTK_MODULE_FUNC(new_engine);
	l->dispose_engine=WTK_MODULE_FUNC(delete_engine);
	l->create_instance=WTK_MODULE_FUNC(new_handler);
	l->dispose_instance=WTK_MODULE_FUNC(delete_handler);
	l->calc=WTK_MODULE_FUNC(calc);

    return l;
}
