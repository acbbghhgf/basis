#include "param_serialize.h"
#include "wtk/core/wtk_stack.h"

int wtk_param_write_bin(wtk_param_t* param,wtk_stack_t* chars)
{
	int len;
	int ret;

	len=param->value.bin.len;
	ret=wtk_stack_push(chars,(char*)&len,sizeof(len));
	if(ret!=0){goto end;}
	if(len>0)
	{
		ret=wtk_stack_push(chars,(char*)param->value.bin.data,len);
	}
end:
	return ret;
}

wtk_param_t* wtk_stack_read_bin_param(wtk_stack_t* chars)//,wtk_heap_t *heap)
{
	wtk_param_t* result;
	int len,ret;

	result=0;
	ret=wtk_stack_pop(chars,(char*)&len,sizeof(len));
	if(ret!=0){goto end;}
	result=wtk_param_new_bin(0,len);//,heap);
	ret=wtk_stack_pop(chars,(char*)result->value.bin.data,len);
	if(ret!=0)
	{
		wtk_param_delete(result);
		result=0;
	}
end:
	return result;
}

wtk_param_t* wtk_stack_read_oct_param(wtk_stack_t* chars)//,wtk_heap_t *heap)
{
	wtk_param_t* result;
	int len,ret;

	result=0;
	ret=wtk_stack_pop(chars,(char*)&len,sizeof(len));
	if(ret!=0){goto end;}
	result=wtk_param_new_oct(0,len);//,heap);
	ret=wtk_stack_pop(chars,(char*)result->value.bin.data,len);
	if(ret!=0)
	{
		wtk_param_delete(result);
		result=0;
	}
end:
	return result;
}

int wtk_param_write_double(wtk_param_t* param,wtk_stack_t* chars)
{
	return wtk_stack_push(chars,(char*)&(param->value.number),sizeof(param->value.number));
}

wtk_param_t* wtk_stack_read_double_param(wtk_stack_t* chars)//,wtk_heap_t *heap)
{
	wtk_param_t* result;
	double number;
	int ret;

	result=0;
	ret=wtk_stack_pop(chars,(char*)&number,sizeof(number));
	if(ret!=0){goto end;}
	result=wtk_param_new_number(number);//,heap);
end:
	return result;
}

int wtk_param_write_str(wtk_param_t* param,wtk_stack_t* chars)
{
	int len;
	int ret;

	len=param->value.str.len;//strlen(param->value.str.data)+1;
	ret=wtk_stack_push(chars,(char*)&len,sizeof(len));
	if(ret!=0){goto end;}
	ret=wtk_stack_push(chars,(char*)param->value.str.data,len);
end:
	return ret;
}

wtk_param_t* wtk_stack_read_str_param(wtk_stack_t* chars) //,wtk_heap_t *heap)
{
	wtk_param_t* result;
	int len,ret;

	result=0;
	ret=wtk_stack_pop(chars,(char*)&len,sizeof(len));
	if(ret!=0){goto end;}
	result=wtk_param_new_str(0,len);//,heap);
	ret=wtk_stack_pop(chars,result->value.str.data,len);
	if(ret!=0)
	{
		wtk_param_delete(result);
		result=0;
	}
end:
	return result;
}

wtk_param_t* wtk_stack_read_array_param(wtk_stack_t* chars) //,wtk_heap_t *heap)
{
	wtk_param_t	*result,*p;
	int len,ret,i;

	result=0;
	ret=wtk_stack_pop(chars,(char*)&len,sizeof(len));
	if(ret!=0){goto end;}
	result=wtk_param_new_array(len);//,heap);
	for(i=0;i<len;++i)
	{
		p=wtk_stack_read_param(chars);//,heap);
		if(p)
		{
			result->value.array.params[i]=p;
		}else
		{
			wtk_param_delete(result);
			result=0;
			goto end;
		}
	}
end:
	return result;
}

int wtk_param_write_array(wtk_param_t *param,wtk_stack_t *chars)
{
	int ret=0;
	int i,len;

	len=param->value.array.len;
	ret=wtk_stack_push(chars,(char*)&len,sizeof(len));
	if(ret!=0){goto end;}
	for(i=0;i<len;++i)
	{
		ret=wtk_param_write(param->value.array.params[i],chars);
		if(ret!=0){goto end;}
	}
end:
	return ret;
}

int wtk_stack_write_number_param(struct wtk_stack* s,double x)
{
	wtk_param_t v;

	v.type=WTK_NUMBER;
	v.value.number=x;
	return wtk_param_write(&v,s);
}

int wtk_stack_read_number_param(wtk_stack_t* s,double *x)
{
	wtk_param_t *p;
	int ret;

	p=wtk_stack_read_param(s);//,0);
	if(!p){ret=-1;goto end;}
	*x=p->value.number;
	wtk_param_delete(p);
	ret=0;
end:
	return ret;
}

int wtk_param_write(wtk_param_t* param,wtk_stack_t* chars)
{
	int ret;
	char type;

	if(param)
	{
		type=param->type;
	}else
	{
		type=WTK_NIL;
	}
	ret=wtk_stack_push(chars,&type,sizeof(type));
	if(ret!=0){goto end;}
	if(!param){goto end;}
	switch(param->type)
	{
	case WTK_NIL:
		ret=0;
		break;
	case WTK_OCT:
	case WTK_BIN:
		ret=wtk_param_write_bin(param,chars);
		break;
	case WTK_NUMBER:
		ret=wtk_param_write_double(param,chars);
		break;
	case WTK_STRING:
		ret=wtk_param_write_str(param,chars);
		break;
	case WTK_ARRAY:
		ret=wtk_param_write_array(param,chars);
		break;
	}
end:
	return ret;
}

int wtk_stack_write_params(wtk_stack_t* s,wtk_param_t** params,int count)
{
	int i,ret;

	ret=0;
	for(i=0;i<count;++i)
	{
		ret=wtk_param_write(params[i],s);
		if(ret!=0){goto end;}
	}
end:
	return ret;
}

wtk_param_t* wtk_stack_read_param(wtk_stack_t* chars)//,wtk_heap_t *heap)
{
	wtk_param_t* result;
	int ret;
	char type;

	result=0;
	//wtk_debug("len=%d\n",chars->len);
	ret=wtk_stack_pop(chars,(char*)&type,sizeof(type));
	//wtk_debug("ret=%d type=%d len=%d\n",ret,type,chars->len);
	if(ret!=0){goto end;}
	switch(type)
	{
	case WTK_NIL:
		result=wtk_param_new(WTK_NIL);
		break;
	case WTK_OCT:
		result=wtk_stack_read_oct_param(chars);//,heap);
		break;
	case WTK_BIN:
		result=wtk_stack_read_bin_param(chars);//,heap);
		break;
	case WTK_NUMBER:
		result=wtk_stack_read_double_param(chars);//,heap);
		break;
	case WTK_STRING:
		result=wtk_stack_read_str_param(chars);//,heap);
		break;
	case WTK_ARRAY:
		result=wtk_stack_read_array_param(chars);//,heap);
		break;
	}
end:
	return result;
}


void wtk_func_param_write(wtk_func_param_t *param,wtk_stack_t *s)
{
	wtk_param_t v;
	int i;

	v.type=WTK_NUMBER;
	v.value.number=param->valid;
	wtk_param_write(&v,s);
	for(i=0;i<param->valid;++i)
	{
		wtk_param_write(param->params[i],s);
	}
}

int wtk_func_param_read(wtk_func_param_t *param,wtk_stack_t *s)
{
	double v;
	int ret=-1,i;

	ret=wtk_stack_read_number_param(s,&(v));
	if(ret!=0){goto end;}
	//wtk_debug("%d\n",(int)v);
	param->valid=(int)v;
	for(i=0;i<param->valid;++i)
	{
		param->params[i]=wtk_stack_read_param(s);//,0);
		//wtk_debug("%p\n",param->params[i]);
		//wtk_param_print(param->params[i]);
		if(!param->params[i]){goto end;}
		if(param->params[i]->type==WTK_NIL)
		{
			wtk_param_delete(param->params[i]);
			param->params[i]=0;
		}
	}
	ret=0;
end:
	return ret;
}

void wtk_module_param_write(wtk_module_param_t *param,wtk_stack_t *s)
{
	wtk_stack_write_number_param(s,param->state);
	if(param->state & WTK_SPEECH_START)
	{
		wtk_stack_write_number_param(s,param->audio_tag.type);
		wtk_stack_write_number_param(s,param->audio_tag.channel);
		wtk_stack_write_number_param(s,param->audio_tag.samplerate);
		wtk_stack_write_number_param(s,param->audio_tag.samplesize);
		wtk_func_param_write(&(param->start),s);
	}
	if(param->state & WTK_SPEECH_APPEND)
	{
		wtk_func_param_write(&(param->append),s);
	}
	if((param->state & WTK_SPEECH_END))
	{
		wtk_func_param_write(&(param->end),s);
	}
}

int wtk_module_param_read(wtk_module_param_t *param,wtk_stack_t *s)
{
	double v;
	int ret;

	wtk_module_param_reset(param);
	ret=wtk_stack_read_number_param(s,&(v));
	if(ret!=0){goto end;}
	param->state=(int)v;
	if(param->state & WTK_SPEECH_START)
	{
		ret=wtk_stack_read_number_param(s,&v);
		if(ret!=0){goto end;}
		param->audio_tag.type=(int)v;
		ret=wtk_stack_read_number_param(s,&v);
		if(ret!=0){goto end;}
		param->audio_tag.channel=(int)v;
		ret=wtk_stack_read_number_param(s,&v);
		if(ret!=0){goto end;}
		param->audio_tag.samplerate=(int)v;
		ret=wtk_stack_read_number_param(s,&v);
		if(ret!=0){goto end;}
		param->audio_tag.samplesize=(int)v;
		ret=wtk_func_param_read(&(param->start),s);
		if(ret!=0){goto end;}
	}else
	{
		param->start.valid=0;
	}
	if(param->state & WTK_SPEECH_APPEND)
	{
		ret=wtk_func_param_read(&(param->append),s);
		if(ret!=0){goto end;}
	}else
	{
		param->append.valid=0;
	}
	if(param->state & WTK_SPEECH_END)
	{
		ret=wtk_func_param_read(&(param->end),s);
	}else
	{
		param->end.valid=0;
	}
end:
	//wtk_debug("%d\n",ret);
	if(ret!=0)
	{
		wtk_debug("release param...\n");
		wtk_module_param_release_param(param);
	}
	return ret;
}

