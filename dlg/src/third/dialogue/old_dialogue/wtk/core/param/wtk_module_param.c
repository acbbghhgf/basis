#include "wtk_module_param.h"
#include "wtk_param.h"
#include "param_serialize.h"

wtk_param_t* wtk_param_new_ret_array()
{
	wtk_param_t *p;

	p=wtk_param_new_array(2);//,0);
	p->value.array.params[0]=wtk_param_new_number(0);//,0);
	p->value.array.params[1]=0;
	return p;
}

int wtk_param_delete_ret_array(wtk_param_t *ret_array)
{
	wtk_param_delete(ret_array);
	return 0;
}

void wtk_param_set_ret_array(wtk_param_t *ret_array,int no,wtk_param_t *info)
{
	ret_array->value.array.params[0]->type=WTK_NUMBER;
	ret_array->value.array.params[0]->value.number=no;
	ret_array->value.array.params[1]=info;
}

int wtk_func_param_update(wtk_func_param_t *p,wtk_func_arg_t *arg,wtk_module_param_read_f read,void *data)
{
	wtk_param_t *param;
	int i,ret=-1;

	p->valid=0;
	for(i=0;i<arg->len;++i)
	{
		param=read(data,&(arg->args[i]));
		if(!param)
		{
			if(arg->args[i].can_be_nil)
			{
				p->params[i]=0;
				++p->valid;
				continue;
			}else
			{
				//wtk_debug("%s not exist.\n",arg->args[i].name);
				goto end;
			}
		}
		++p->valid;
		p->params[i]=param;
	}
	ret=0;
end:
	return ret;
}


int wtk_func_param_init(wtk_func_param_t *param,wtk_func_arg_t *arg)
{
	param->len=arg->len;
	if(param->len>0)
	{
		param->params=(wtk_param_t**)wtk_calloc(param->len,sizeof(wtk_param_t*));
	}else
	{
		param->params=0;
	}
	return 0;
}

int wtk_func_param_reset(wtk_func_param_t *param)
{
	if(param->params)
	{
		memset(param->params,0,sizeof(wtk_param_t*)*param->len);
	}
	param->valid=0;
	return 0;
}

int wtk_func_param_clean(wtk_func_param_t *param)
{
	if(param->params)
	{
		free(param->params);
        param->params = NULL;
	}
	return 0;
}

int wtk_module_param_init(wtk_module_param_t *param,wtk_module_arg_t *arg)
{
	wtk_func_param_init(&(param->start),&(arg->start));
	wtk_func_param_init(&(param->append),&(arg->append));
	wtk_func_param_init(&(param->end),&(arg->end));
	param->write=0;
	param->write_usr_data=0;
	return 0;
}

int wtk_module_param_reset(wtk_module_param_t *param)
{
	wtk_func_param_reset(&(param->start));
	wtk_func_param_reset(&(param->append));
	wtk_func_param_reset(&(param->end));
	param->write=0;
	param->write_usr_data=0;
	return 0;
}

int wtk_module_param_clean(wtk_module_param_t *param)
{
	wtk_func_param_clean(&(param->start));
	wtk_func_param_clean(&(param->append));
	wtk_func_param_clean(&(param->end));
	return 0;
}

int wtk_func_param_release_param(wtk_func_param_t *param)
{
	int i;

	for(i=0;i<param->len;++i)
	{
		if(param->params[i])
		{
			wtk_param_delete(param->params[i]);
			param->params[i]=0;
		}
	}
	return 0;
}

int wtk_module_param_release_param(wtk_module_param_t *param)
{
	wtk_func_param_release_param(&(param->start));
	wtk_func_param_release_param(&(param->append));
	wtk_func_param_release_param(&(param->end));
	return 0;
}

int wtk_module_param_fill(wtk_module_param_t *mp,wtk_module_arg_t *ma,
		int state,void *read_data,wtk_module_param_read_f read)
{
	int ret=-1;

	wtk_module_param_reset(mp);
	if(state&WTK_SPEECH_START)
	{
		//attach start param.
		ret=wtk_func_param_update(&(mp->start),&(ma->start),read,read_data);
		if(ret!=0){goto end;}
	}
	if(state&WTK_SPEECH_APPEND)
	{
		//attach append param.
		ret=wtk_func_param_update(&(mp->append),&(ma->append),read,read_data);
		if(ret!=0){goto end;}
	}
	if((state&WTK_SPEECH_END))//||(i->e->cfg->feedback))
	{
		//attach end param.
		ret=wtk_func_param_update(&(mp->end),&(ma->end),read,read_data);
		if(ret!=0){goto end;}
	}
	mp->state=state;
	ret=0;
end:
	return ret;
}

void wtk_param_arg_from_lc(wtk_param_arg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_x(lc,cfg,type,v,(wtk_paramtype_t)atoi);
	wtk_local_cfg_update_cfg_i(lc,cfg,can_be_nil,v);
}

int wtk_func_arg_from_lc(wtk_func_arg_t *arg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc=main;
	wtk_array_t *a;
	wtk_string_t **s;
	int ret=-1;
	int i;

	a=wtk_local_cfg_find_array_s(lc,"arg");
	if(!a){goto end;}
	arg->len=a->nslot;
	arg->args=(wtk_param_arg_t*)wtk_calloc(arg->len,sizeof(wtk_param_arg_t));
	s=(wtk_string_t**)a->slot;
	for(i=0;i<a->nslot;++i)
	{
		lc=wtk_local_cfg_find_lc(main,s[i]->data,s[i]->len);
		if(!lc){ret=-1;goto end;}
		wtk_param_arg_from_lc(arg->args+i,lc);
		arg->args[i].name=s[i]->data;
		arg->args[i].name_len=s[i]->len;
	}
	ret=0;
end:
	return ret;
}

void wtk_func_arg_clean(wtk_func_arg_t *arg)
{
	if(arg->len>0 && arg->args)
	{
		wtk_free(arg->args);
		arg->args=0;
		arg->len=0;
	}
}

void wtk_param_arg_print(wtk_param_arg_t *arg)
{
	printf("name:\t%*.*s\n",arg->name_len,arg->name_len,arg->name);
	//printf("type:\t%d\n",arg->type);
	//printf("canil:\t%d\n",arg->can_be_nil);
}

void wtk_func_arg_print(wtk_func_arg_t *arg)
{
	int i;

	for(i=0;i<arg->len;++i)
	{
		wtk_param_arg_print(&(arg->args[i]));
	}
}

void wtk_module_arg_print(wtk_module_arg_t* arg)
{
	printf("------- start ------------\n");
	wtk_func_arg_print(&(arg->start));
	printf("------- append ------------\n");
	wtk_func_arg_print(&(arg->append));
	printf("------- end ------------\n");
	wtk_func_arg_print(&(arg->end));
}

void wtk_func_param_print(wtk_func_param_t *p)
{
	int i;

	for(i=0;i<p->valid;++i)
	{
		wtk_debug("%d,%p\n",i,p->params[i]);
		if(p->params[i])
		{
			wtk_param_print(p->params[i]);
		}
	}
}

void wtk_module_param_print(wtk_module_param_t *p)
{
	printf("############### module param ###############\n");
	printf("state:\t%d\n",p->state);
	printf("start %d param...\n",p->start.valid);
	wtk_func_param_print(&(p->start));
	printf("append %d param...\n",p->append.valid);
	wtk_func_param_print(&(p->append));
	printf("end %d param ...\n",p->end.valid);
	wtk_func_param_print(&(p->end));
}

void wtk_audio_tag_reset(wtk_audio_tag_t *tag)
{
	tag->type=AUDIO_UNKNOWN;
	tag->samplesize=-1;//2;
	tag->samplerate=-1;//16000;
	tag->channel=-1;//1;
}

int wtk_audio_tag_need_pre(wtk_audio_tag_t *tag,int dst_rate)
{
	int b;

	if((tag->type!=AUDIO_WAV) || (tag->channel!=1) || (tag->samplerate!=dst_rate) || (tag->samplesize!=2))
	{
		b=1;
	}else
	{
		b=0;
	}
	return b;
}

void wtk_audio_tag_update_local(wtk_audio_tag_t *at,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_i2(lc,at,channel,Channel,v);
	wtk_local_cfg_update_cfg_i2(lc,at,samplerate,SampleRate,v);
	wtk_local_cfg_update_cfg_i2(lc,at,samplesize,SampleBytes,v);
	v=wtk_local_cfg_find_string_s(lc,"AudioType");
	if(v)
	{
		at->type=wtk_audio_type_from_string(v);
	}
}

void wtk_audio_tag_print(wtk_audio_tag_t *tag)
{
	/*
	static char *at[]={
			"AUDIO_WAV","AUDIO_MP3","AUDIO_FLV","AUDIO_TEXT","AUDIO_UNKNOWN","AUDIO_NONE"
	};

	printf("type:        %s\n",at[tag->type]);
	*/
	printf("type:        %s\n",wtk_audio_type_to_string(tag->type)->data);
	printf("channel:     %d\n",tag->channel);
	printf("samplerate:  %d\n",tag->samplerate);
	printf("samplesize:  %d\n",tag->samplesize);
}
