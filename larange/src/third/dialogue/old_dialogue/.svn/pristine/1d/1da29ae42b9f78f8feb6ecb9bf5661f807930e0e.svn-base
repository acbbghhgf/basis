#include <ctype.h>
#include "wtk_json_parse.h"
void wtk_json_parser_set_state(wtk_json_parser_t *p,wtk_json_parse_state_t state);
int wtk_json_parser_feed(wtk_json_parser_t *p,char c);

wtk_json_parser_t* wtk_json_parser_new()
{
	wtk_json_parser_t *p;

	p=(wtk_json_parser_t*)wtk_malloc(sizeof(wtk_json_parser_t));
	p->json=wtk_json_new();
	p->heap=wtk_heap_new(4096);
	p->buf=wtk_strbuf_new(256,1);
	wtk_json_parser_reset(p);
	return p;
}

void wtk_json_parser_delete(wtk_json_parser_t *p)
{
	wtk_heap_delete(p->heap);
	wtk_strbuf_delete(p->buf);
	wtk_json_delete(p->json);
	wtk_free(p);
}

void wtk_json_parser_reset(wtk_json_parser_t *p)
{
	wtk_queue_init(&(p->stack_q));
	wtk_heap_reset(p->heap);
	p->cur_json=NULL;
	wtk_json_reset(p->json);
	wtk_str_parse_init(&(p->str_parse),p->buf);
	p->cur=0;
	wtk_queue_init(&(p->stack_q));
	wtk_json_parser_set_state(p,WTK_JSON_PARSE_MAIN_WAIT);
}

void wtk_json_parser_set_state(wtk_json_parser_t *p,wtk_json_parse_state_t state)
{
	p->state=state;
	p->sub_state=-1;
}

void wtk_json_parse_push_stack(wtk_json_parser_t *p)
{
	wtk_json_stack_item_t *item;

	item=(wtk_json_stack_item_t*)wtk_heap_malloc(p->heap,sizeof(wtk_json_stack_item_t));
	item->main_state=p->state;
	item->sub_state=p->sub_state;
	item->item=p->cur;
	//wtk_debug("push %d\n",p->state);
	wtk_queue_push_front(&(p->stack_q),&(item->q_n));
	p->cur=NULL;
	p->value=NULL;
}

void wtk_json_parse_pop_stack(wtk_json_parser_t *p)
{
	wtk_json_stack_item_t *item;
	wtk_queue_node_t *n;

	n=wtk_queue_pop(&(p->stack_q));
	item=data_offset(n,wtk_json_stack_item_t,q_n);
	p->state=item->main_state;
	p->sub_state=item->sub_state;
	p->value=p->cur;
	p->cur=item->item;
	//wtk_debug("pop %d\n",p->state);
}

wtk_json_t* wtk_json_parser_get_json(wtk_json_parser_t *p)
{
	return p->cur_json?p->cur_json:p->json;
}

int wtk_json_parser_feed_main_wait(wtk_json_parser_t *p,char c)
{
	wtk_json_t *json;

	json=wtk_json_parser_get_json(p);
	if(c=='[')
	{
		p->cur=wtk_json_new_array(json);
		json->main=p->cur;
		wtk_json_parser_set_state(p,WTK_JSON_PARSE_ARRAY);
	}else if(c=='{')
	{
		p->cur=wtk_json_new_object(json);
		json->main=p->cur;
		wtk_json_parser_set_state(p,WTK_JSON_PARSE_OBJECT);
	}else if(!isspace(c))
	{
		return -1;
	}
	return 0;
}

int wtk_json_parser_feed_array(wtk_json_parser_t *p,char c)
{
	enum wtk_json_array_state_t
	{
		WTK_JSON_ARRAY_WAIT=0,
		WTK_JSON_ARRAY_VALUE,
	};//wtk_json_array_state_t;
	wtk_json_t *json;

	json=wtk_json_parser_get_json(p);
	if(p->sub_state==-1)
	{
		p->sub_state=WTK_JSON_ARRAY_WAIT;
	}
	switch(p->sub_state)
	{
	case WTK_JSON_ARRAY_WAIT:
		if(isspace(c) || c==',')
		{
		}else if(c==']')
		{
			if(p->stack_q.length>0)
			{
				wtk_json_parse_pop_stack(p);
			}
		}else
		{
			p->sub_state=WTK_JSON_ARRAY_VALUE;
			wtk_json_parse_push_stack(p);
			p->state=WTK_JSON_PARSE_WAIT_VALUE;
			return wtk_json_parser_feed(p,c);
		}
		break;
	case WTK_JSON_ARRAY_VALUE:
		wtk_json_array_add_item(json,p->cur,p->value);
		p->sub_state=WTK_JSON_ARRAY_WAIT;
		return wtk_json_parser_feed(p,c);
		break;
	default:
		wtk_debug("found bug\n");
		exit(0);
		break;
	}
	return 0;
}

int wtk_json_parser_feed_object(wtk_json_parser_t *p,char c)
{
enum wtk_json_obj_state_t
{
	WTK_JSON_OBJ_WAIT=0,
	WTK_JSON_KEY,
	WTK_JSON_WAIT_COLON,
	WTK_JSON_WAIT_VALUE,
};
	int ret=0;
	wtk_strbuf_t *buf=p->buf;
	wtk_json_t *json;

	json=wtk_json_parser_get_json(p);
	if(p->sub_state==-1)
	{
		p->sub_state=WTK_JSON_OBJ_WAIT;
	}
	//wtk_debug("[%c]\n",c);
	switch(p->sub_state)
	{
	case WTK_JSON_OBJ_WAIT:
		if(c=='\"')
		{
			p->sub_state=WTK_JSON_KEY;
			wtk_str_parse_init(&(p->str_parse),buf);
			wtk_strbuf_reset(buf);
		}else if(c=='}')
		{
			if(p->stack_q.length>0)
			{
				wtk_json_parse_pop_stack(p);
			}
			//wtk_json_item_print(p->cur);
		}else if(!isspace(c))
		{
			ret=-1;
		}
		break;
	case WTK_JSON_KEY:
		ret=wtk_str_parse_feed(&(p->str_parse),c);
		if(ret==-1)
		{
			return ret;
		}else if(ret==1)
		{
			wtk_json_obj_add_item2(json,p->cur,p->str_parse.buf->data,p->str_parse.buf->pos,0);
			//wtk_debug("[%.*s]\n",p->str_parse.buf->pos,p->str_parse.buf->data);
			p->sub_state=WTK_JSON_WAIT_COLON;
		}
		break;
	case WTK_JSON_WAIT_COLON:
		if(c==':')
		{
			p->sub_state=WTK_JSON_WAIT_VALUE;
			wtk_json_parse_push_stack(p);
			p->state=WTK_JSON_PARSE_WAIT_VALUE;
		}
		break;
	case WTK_JSON_WAIT_VALUE:
		wtk_json_obj_set_last_item_value(json,p->cur,p->value);
		//wtk_json_item_print(p->cur);
		/*
		wtk_json_item_print(p->value);
		wtk_json_item_print(p->cur);
		wtk_debug("found value\n");
		exit(0);
		*/
		p->sub_state=WTK_JSON_OBJ_WAIT;
		return wtk_json_parser_feed(p,c);
		break;
	}
	return 0;
}

int wtk_json_parser_feed_value(wtk_json_parser_t *p,char c)
{
	wtk_json_t *json;

	json=wtk_json_parser_get_json(p);
	switch(c)
	{
	case '\"':
		wtk_json_parser_set_state(p,WTK_JSON_PARSE_STRING);
		wtk_str_parse_init(&(p->str_parse),p->buf);
		break;
	case '[':
		p->cur=wtk_json_new_array(json);
		wtk_json_parser_set_state(p,WTK_JSON_PARSE_ARRAY);
		break;
	case '{':
		p->cur=wtk_json_new_object(json);
		wtk_json_parser_set_state(p,WTK_JSON_PARSE_OBJECT);
		break;
	case 'n':
		wtk_json_parser_set_state(p,WTK_JSON_PARSE_NULL);
		break;
	case 't':
		wtk_json_parser_set_state(p,WTK_JSON_PARSE_TRUE);
		break;
	case 'f':
		wtk_json_parser_set_state(p,WTK_JSON_PARSE_FALSE);
		break;
	default:
		if(wtk_number_parse_can_parse(c))
		{
			wtk_number_parse_init(&(p->num_parse));
			wtk_number_parse_feed(&(p->num_parse),c);
			wtk_json_parser_set_state(p,WTK_JSON_PARSE_NUMBER);
		}else if(!isspace(c))
		{
			wtk_debug("un processed[%c]\n",c);
			exit(0);
		}
		break;
	}
	return 0;
}

int wtk_json_parser_feed_string_value(wtk_json_parser_t *p,char c)
{
	wtk_json_t *json;
	int ret;

	json=wtk_json_parser_get_json(p);
	ret=wtk_str_parse_feed(&(p->str_parse),c);
	if(ret==-1)
	{
		return ret;
	}else if(ret==1)
	{
		p->cur=wtk_json_new_string(json,p->str_parse.buf->data,p->str_parse.buf->pos);
		wtk_json_parse_pop_stack(p);
	}
	return 0;
}

int wtk_json_parser_feed_number_value(wtk_json_parser_t *p,char c)
{
	double v;
	int ret;
	wtk_json_t *json;

	json=wtk_json_parser_get_json(p);
	ret=wtk_number_parse_feed(&(p->num_parse),c);
	if(ret==-1)
	{
		v=wtk_number_parse_to_value(&(p->num_parse));
		//wtk_debug("v=%f\n",v);
		p->cur=wtk_json_new_number(json,v);
		wtk_json_parse_pop_stack(p);
		//wtk_debug("state=%d\n",p->state);
		return wtk_json_parser_feed(p,c);
	}
	return 0;
}

int wtk_json_parser_feed_null_value(wtk_json_parser_t *p,char c)
{
enum wtk_json_parser_null_state_t
{
	WTK_JSON_NULL_0,
	WTK_JSON_NULL_1,
	WTK_JSON_NULL_2,
};
	wtk_json_t *json;

	json=wtk_json_parser_get_json(p);
	if(p->sub_state==-1)
	{
		p->sub_state=WTK_JSON_NULL_0;
	}
	switch(p->sub_state)
	{
	case WTK_JSON_NULL_0:
		if(c!='u')
		{
			return -1;
		}
		p->sub_state=WTK_JSON_NULL_1;
		break;
	case WTK_JSON_NULL_1:
		if(c!='l')
		{
			return -1;
		}
		p->sub_state=WTK_JSON_NULL_2;
		break;
	case WTK_JSON_NULL_2:
		if(c!='l')
		{
			return -1;
		}
		p->cur=wtk_json_new_item(json,WTK_JSON_NULL);
		wtk_json_parse_pop_stack(p);
		break;
	}
	return 0;
}

int wtk_json_parser_feed_true_value(wtk_json_parser_t *p,char c)
{
enum wtk_json_parser_true_state_t
{
	WTK_JSON_TRUE_0,
	WTK_JSON_TRUE_1,
	WTK_JSON_TRUE_2,
	WTK_JSON_TRUE_3,
};
	wtk_json_t *json;

	json=wtk_json_parser_get_json(p);
	if(p->sub_state==-1)
	{
		p->sub_state=WTK_JSON_TRUE_0;
	}
	switch(p->sub_state)
	{
	case WTK_JSON_TRUE_0:
		if(c!='r')
		{
			return -1;
		}
		p->sub_state=WTK_JSON_TRUE_1;
		break;
	case WTK_JSON_TRUE_1:
		if(c!='u')
		{
			return -1;
		}
		p->sub_state=WTK_JSON_TRUE_2;
		break;
	case WTK_JSON_TRUE_2:
		if(c!='e')
		{
			return -1;
		}
		p->cur=wtk_json_new_item(json,WTK_JSON_TRUE);
		wtk_json_parse_pop_stack(p);
		break;
	}
	return 0;
}

int wtk_json_parser_feed_false_value(wtk_json_parser_t *p,char c)
{
enum wtk_json_parser_false_state_t
{
	WTK_JSON_FALSE_0,
	WTK_JSON_FALSE_1,
	WTK_JSON_FALSE_2,
	WTK_JSON_FALSE_3,
};
	wtk_json_t *json;

	json=wtk_json_parser_get_json(p);
	if(p->sub_state==-1)
	{
		p->sub_state=WTK_JSON_FALSE_0;
	}
	switch(p->sub_state)
	{
	case WTK_JSON_FALSE_0:
		if(c!='a')
		{
			return -1;
		}
		p->sub_state=WTK_JSON_FALSE_1;
		break;
	case WTK_JSON_FALSE_1:
		if(c!='l')
		{
			return -1;
		}
		p->sub_state=WTK_JSON_FALSE_2;
		break;
	case WTK_JSON_FALSE_2:
		if(c!='s')
		{
			return -1;
		}
		p->sub_state=WTK_JSON_FALSE_3;
		break;
	case WTK_JSON_FALSE_3:
		if(c!='e')
		{
			return -1;
		}
		p->cur=wtk_json_new_item(json,WTK_JSON_FALSE);
		wtk_json_parse_pop_stack(p);
		break;
	}
	return 0;
}


int wtk_json_parser_feed(wtk_json_parser_t *p,char c)
{
	int ret;

	//wtk_debug("[%c:%d]\n",c,p->state);
	switch(p->state)
	{
	case WTK_JSON_PARSE_MAIN_WAIT:
		ret=wtk_json_parser_feed_main_wait(p,c);
		break;
	case WTK_JSON_PARSE_OBJECT:
		ret=wtk_json_parser_feed_object(p,c);
		break;
	case WTK_JSON_PARSE_ARRAY:
		ret=wtk_json_parser_feed_array(p,c);
		break;
	case WTK_JSON_PARSE_WAIT_VALUE:
		ret=wtk_json_parser_feed_value(p,c);
		break;
	case WTK_JSON_PARSE_STRING:
		ret=wtk_json_parser_feed_string_value(p,c);
		break;
	case WTK_JSON_PARSE_NUMBER:
		ret=wtk_json_parser_feed_number_value(p,c);
		break;
	case WTK_JSON_PARSE_NULL:
		ret=wtk_json_parser_feed_null_value(p,c);
		break;
	case WTK_JSON_PARSE_TRUE:
		ret=wtk_json_parser_feed_true_value(p,c);
		break;
	case WTK_JSON_PARSE_FALSE:
		ret=wtk_json_parser_feed_false_value(p,c);
		break;
	default:
		ret=-1;
		wtk_debug("not found[state=%d]\n",p->state);
		exit(0);
		break;
	}
	return ret;
}


int wtk_json_parser_parse(wtk_json_parser_t *parser,char *data,int len)
{
	return wtk_json_parser_parse2(parser,NULL,data,len);
}

int wtk_json_parser_parse2(wtk_json_parser_t *parser,wtk_json_t *json,char *data,int len)
{
	int ret;
	char *p,*e;

	parser->cur_json=json;
	p=data;e=data+len;
	while(p<e)
	{
		ret=wtk_json_parser_feed(parser,*p);
		if(ret!=0)
		{
			wtk_debug("process [%c:%d] failed\n",*p,parser->state);
			wtk_debug("input=[%.*s]\n",len,data);
			//exit(0);
			goto end;
		}
		++p;
	}
	ret=0;
end:
	//wtk_json_print(parser->json,parser->buf);
	return ret;
}

#include "wtk/core/wtk_os.h"

int wtk_json_parser_parse_file(wtk_json_parser_t *p,char *fn)
{
	int len;
	char *data;
	int ret=-1;

	data=file_read_buf(fn,&len);
	if(!data){goto end;}
	ret=wtk_json_parser_parse(p,data,len);
end:
	if(data)
	{
		wtk_free(data);
	}
	return ret;
}

