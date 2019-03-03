#include <math.h>
#include <limits.h>
#include <stdlib.h>
#include <float.h>
#include "wtk_json.h"

wtk_json_t* wtk_json_new()
{
	wtk_json_t *json;

	json=(wtk_json_t*)wtk_malloc(sizeof(wtk_json_t));
	json->heap=wtk_heap_new(4096);
	wtk_json_reset(json);
	return json;
}

void wtk_json_delete(wtk_json_t *json)
{
	wtk_heap_delete(json->heap);
	wtk_free(json);
}

void wtk_json_reset(wtk_json_t *json)
{
	wtk_heap_reset(json->heap);
	json->main=0;
}

wtk_json_item_t* wtk_json_new_item2(wtk_heap_t *heap,wtk_json_item_type_t type)
{
	wtk_json_item_t *item;

	item=(wtk_json_item_t*)wtk_heap_malloc(heap,sizeof(wtk_json_item_t));
	item->type=type;
	return item;
}

wtk_json_item_t* wtk_json_new_number2(wtk_heap_t *heap,double v)
{
	wtk_json_item_t *item;

	item=(wtk_json_item_t*)wtk_heap_malloc(heap,sizeof(wtk_json_item_t));
	item->type=WTK_JSON_NUMBER;
	item->v.number=v;
	return item;
}

wtk_json_item_t* wtk_json_new_string2(wtk_heap_t *heap,char *data,int len)
{
	wtk_json_item_t *item;

	item=(wtk_json_item_t*)wtk_heap_malloc(heap,sizeof(wtk_json_item_t));
	item->type=WTK_JSON_STRING;
	item->v.str=wtk_heap_dup_string(heap,data,len);
	return item;
}

wtk_json_item_t* wtk_json_new_array2(wtk_heap_t *heap)
{
	wtk_json_item_t *item;

	item=(wtk_json_item_t*)wtk_heap_malloc(heap,sizeof(wtk_json_item_t));
	item->type=WTK_JSON_ARRAY;
	item->v.object=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
	wtk_queue_init((item->v.array));
	return item;
}

wtk_json_item_t* wtk_json_new_object2(wtk_heap_t *heap)
{
	wtk_json_item_t *item;

	item=(wtk_json_item_t*)wtk_heap_malloc(heap,sizeof(wtk_json_item_t));
	item->type=WTK_JSON_OBJECT;
	item->v.object=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
	wtk_queue_init((item->v.object));
	return item;
}

wtk_json_item_t* wtk_json_new_item(wtk_json_t *json,wtk_json_item_type_t type)
{
	wtk_json_item_t *item;

	item=(wtk_json_item_t*)wtk_heap_malloc(json->heap,sizeof(wtk_json_item_t));
	item->type=type;
	return item;
}

wtk_json_item_t* wtk_json_new_number(wtk_json_t *json,double v)
{
	wtk_heap_t *heap=json->heap;
	wtk_json_item_t *item;

	item=(wtk_json_item_t*)wtk_heap_malloc(heap,sizeof(wtk_json_item_t));
	item->type=WTK_JSON_NUMBER;
	item->v.number=v;
	return item;
}

wtk_json_item_t* wtk_json_new_string(wtk_json_t *json,char *data,int len)
{
	wtk_heap_t *heap=json->heap;
	wtk_json_item_t *item;

	item=(wtk_json_item_t*)wtk_heap_malloc(heap,sizeof(wtk_json_item_t));
	item->type=WTK_JSON_STRING;
	item->v.str=wtk_heap_dup_string(heap,data,len);
	return item;
}

wtk_json_item_t* wtk_json_new_array(wtk_json_t *json)
{
	wtk_heap_t *heap=json->heap;
	wtk_json_item_t *item;

	item=(wtk_json_item_t*)wtk_heap_malloc(heap,sizeof(wtk_json_item_t));
	item->type=WTK_JSON_ARRAY;
	item->v.object=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
	wtk_queue_init((item->v.array));
	return item;
}

wtk_json_item_t* wtk_json_new_object(wtk_json_t *json)
{
	wtk_heap_t *heap=json->heap;
	wtk_json_item_t *item;

	item=(wtk_json_item_t*)wtk_heap_malloc(heap,sizeof(wtk_json_item_t));
	item->type=WTK_JSON_OBJECT;
	item->v.object=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
	wtk_queue_init((item->v.object));
	return item;
}

wtk_json_item_t* wtk_json_obj_get(wtk_json_item_t *obj,char *key,int key_bytes)
{
	wtk_queue_node_t *qn;
	wtk_json_obj_item_t *item;

	if(obj->type!=WTK_JSON_OBJECT){return NULL;}
	for(qn=obj->v.object->pop;qn;qn=qn->next)
	{
		item=data_offset(qn,wtk_json_obj_item_t,q_n);
		//wtk_debug("item=%p [%.*s] [%.*s] %p\n",item,item->k.len,item->k.data,key_bytes,key,item->item);
		if(wtk_string_cmp(&(item->k),key,key_bytes)==0)
		{
			return item->item;
		}
	}
	return NULL;
}


wtk_json_item_t* wtk_json_obj_get_first(wtk_json_item_t *obj)
{
	wtk_queue_node_t *qn;
	wtk_json_obj_item_t *item;

	if(obj->type!=WTK_JSON_OBJECT || !obj->v.object->pop){return NULL;}
	qn=obj->v.object->pop;
	item=data_offset(qn,wtk_json_obj_item_t,q_n);
	return item?item->item:NULL;
}


wtk_json_obj_item_t* wtk_json_new_obj_item(wtk_json_t *json,char *key,int key_bytes,wtk_json_item_t *item)
{
	wtk_json_obj_item_t *x;

	x=(wtk_json_obj_item_t*)wtk_heap_malloc(json->heap,sizeof(wtk_json_obj_item_t));
	wtk_heap_fill_string(json->heap,&(x->k),key,key_bytes);
	x->item=item;
	return x;
}

void wtk_json_obj_add_item(wtk_json_t *json,wtk_json_item_t *obj,wtk_json_obj_item_t *item)
{
	if(obj->type==WTK_JSON_OBJECT)
	{
		wtk_queue_push(obj->v.object,&(item->q_n));
	}
}

int wtk_json_item_len(wtk_json_item_t *item)
{
	switch(item->type)
	{
	case WTK_JSON_FALSE:
		return 1;
		break;
	case WTK_JSON_TRUE:
		return 1;
		break;
	case WTK_JSON_NULL:
		return 1;
		break;
	case WTK_JSON_STRING:
		return 1;
		break;
	case WTK_JSON_NUMBER:
		return 1;
		break;
	case WTK_JSON_ARRAY:
		return item->v.array->length;
		break;
	case WTK_JSON_OBJECT:
		return item->v.object->length;
		break;
	}
	return 0;
}

wtk_json_obj_item_t* wtk_json_obj_get_valid_item(wtk_json_item_t *item)
{
	wtk_queue_node_t *qn;
	wtk_json_obj_item_t *vi;
	int n;

	if(item->type!=WTK_JSON_OBJECT){goto end;}
	for(qn=item->v.object->pop;qn;qn=qn->next)
	{
		vi=data_offset2(qn,wtk_json_obj_item_t,q_n);
		if(vi->k.data[vi->k.len-1]=='^')
		{
			continue;
		}
		n=wtk_json_item_len(vi->item);
		if(n>0)
		{
			return vi;
		}
	}
end:
	return NULL;
}

wtk_json_item_t* wtk_json_obj_remove(wtk_json_item_t *item,char *k,int k_len)
{
	wtk_queue_node_t *qn;
	wtk_json_obj_item_t *vi;

	for(qn=item->v.object->pop;qn;qn=qn->next)
	{
		vi=data_offset2(qn,wtk_json_obj_item_t,q_n);
		if(wtk_string_cmp(&(vi->k),k,k_len)==0)
		{
			wtk_queue_remove(item->v.object,qn);
			return vi->item;
		}
	}
	return NULL;
}

void wtk_json_obj_add_item2(wtk_json_t *json,wtk_json_item_t *obj,char *key,int key_bytes,wtk_json_item_t *item)
{
	wtk_json_obj_item_t *x;

	x=wtk_json_new_obj_item(json,key,key_bytes,item);
	wtk_json_obj_add_item(json,obj,x);
}

wtk_json_item_t* wtk_json_obj_add_str2(wtk_json_t *json,wtk_json_item_t *obj,char *key,int key_len,char *v,int v_len)
{
	wtk_json_item_t *item;
	wtk_json_obj_item_t *x;

	item=wtk_json_new_item(json,WTK_JSON_STRING);
	item->v.str=wtk_heap_dup_string(json->heap,v,v_len);
	x=(wtk_json_obj_item_t*)wtk_heap_malloc(json->heap,sizeof(wtk_json_obj_item_t));
	x->item=item;
	wtk_heap_fill_string(json->heap,&(x->k),key,key_len);
	wtk_json_obj_add_item(json,obj,x);
	return item;
}

wtk_json_item_t* wtk_json_obj_add_ref_str(wtk_json_t *json,wtk_json_item_t *obj,char *key,int key_len,wtk_string_t *v)
{
	wtk_json_item_t *item;
	wtk_json_obj_item_t *x;

	item=wtk_json_new_item(json,WTK_JSON_STRING);
	item->v.str=v;
	x=(wtk_json_obj_item_t*)wtk_heap_malloc(json->heap,sizeof(wtk_json_obj_item_t));
	x->item=item;
	wtk_string_set(&(x->k),key,key_len);
	wtk_json_obj_add_item(json,obj,x);
	return item;
}

wtk_json_item_t* wtk_json_obj_add_str(wtk_json_t *json,wtk_json_item_t *obj,char *key,int key_len,wtk_string_t *v)
{
	v=wtk_heap_dup_string(json->heap,v->data,v->len);
	return  wtk_json_obj_add_ref_str(json,obj,key,key_len,v);
}

wtk_json_item_t* wtk_json_obj_add_ref_number(wtk_json_t *json,wtk_json_item_t *obj,char *key,int key_len,double number)
{
	wtk_json_item_t *item;
	wtk_json_obj_item_t *x;

	item=wtk_json_new_item(json,WTK_JSON_NUMBER);
	item->v.number=number;
	x=(wtk_json_obj_item_t*)wtk_heap_malloc(json->heap,sizeof(wtk_json_obj_item_t));
	x->item=item;
	wtk_string_set(&(x->k),key,key_len);
	wtk_json_obj_add_item(json,obj,x);
	return item;
}


void wtk_json_obj_set_last_item_value(wtk_json_t *json,wtk_json_item_t *obj,wtk_json_item_t *item)
{
	wtk_json_obj_item_t *x;

	x=data_offset(obj->v.object->push,wtk_json_obj_item_t,q_n);
	x->item=item;
}

void wtk_json_array_add_item(wtk_json_t *json,wtk_json_item_t *array,wtk_json_item_t *item)
{
	wtk_json_array_add_item2(json,array,item,0);
}

void wtk_json_array_add_item2(wtk_json_t *json,wtk_json_item_t *array,wtk_json_item_t *item,int push_front)
{
	wtk_json_array_item_t *x;

	x=(wtk_json_array_item_t*)wtk_heap_malloc(json->heap,sizeof(wtk_json_array_item_t));
	x->item=item;
	if(push_front)
	{
		wtk_queue_push_front(array->v.array,&(x->q_n));
	}else
	{
		wtk_queue_push(array->v.array,&(x->q_n));
	}
}

wtk_json_item_t* wtk_json_array_add_ref_str(wtk_json_t *json,wtk_json_item_t *array,wtk_string_t *v)
{
	wtk_json_item_t *item;
	wtk_json_array_item_t *x;

	item=wtk_json_new_item(json,WTK_JSON_STRING);
	item->v.str=v;
	x=(wtk_json_array_item_t*)wtk_heap_malloc(json->heap,sizeof(wtk_json_array_item_t));
	x->item=item;
	wtk_queue_push(array->v.array,&(x->q_n));
	return item;
}

wtk_json_item_t* wtk_json_array_add_str(wtk_json_t *json,wtk_json_item_t *array,char *data,int bytes)
{
	wtk_string_t *v;

	v=wtk_heap_dup_string(json->heap,data,bytes);
	return wtk_json_array_add_ref_str(json,array,v);
}

void wtk_json_array_add_ref_number(wtk_json_t *json,wtk_json_item_t *array,double v)
{
	wtk_json_item_t *item;
	wtk_json_array_item_t *x;

	item=wtk_json_new_item(json,WTK_JSON_NUMBER);
	item->v.number=v;
	x=(wtk_json_array_item_t*)wtk_heap_malloc(json->heap,sizeof(wtk_json_array_item_t));
	x->item=item;
	wtk_queue_push(array->v.array,&(x->q_n));
}

wtk_json_item_t* wtk_json_array_get(wtk_json_item_t* item,int idx)
{
	wtk_queue_node_t *qn;
	wtk_json_array_item_t *ai;

	qn=wtk_queue_peek(item->v.array,idx);
	//wtk_debug("v[%d]=%p\n",idx,qn);
	if(!qn){return NULL;}
	ai=data_offset2(qn,wtk_json_array_item_t,q_n);
	return ai->item;
}

int wtk_json_array_has_string_value(wtk_json_item_t *item,char *v,int v_bytes)
{
	wtk_queue_node_t *qn;
	wtk_json_array_item_t *ai;

	for(qn=item->v.array->pop;qn;qn=qn->next)
	{
		ai=data_offset2(qn,wtk_json_array_item_t,q_n);
		if(ai->item->type==WTK_JSON_STRING && wtk_string_cmp(ai->item->v.str,v,v_bytes)==0)
		{
			return 1;
		}
	}
	return 0;
}

void wtk_json_array_remove_string_value(wtk_json_item_t *item,char *v,int v_bytes)
{
	wtk_queue_node_t *qn;
	wtk_json_array_item_t *ai;

	for(qn=item->v.array->pop;qn;qn=qn->next)
	{
		ai=data_offset2(qn,wtk_json_array_item_t,q_n);
		if(ai->item->type==WTK_JSON_STRING && wtk_string_cmp(ai->item->v.str,v,v_bytes)==0)
		{
			wtk_queue_remove(item->v.array,qn);
			return;
		}
	}
}

void wtk_json_array_add_unq_str(wtk_json_t* json,wtk_json_item_t *item,char *v,int v_bytes)
{
	wtk_json_item_t *ti;

	ti=wtk_json_array_get_string_value(item,v,v_bytes);
	//wtk_debug("ti=%p\n",ti);
	if(!ti)
	{
		wtk_json_array_add_str(json,item,v,v_bytes);
	}
}

wtk_json_item_t* wtk_json_array_get_string_value(wtk_json_item_t *item,char *v,int v_bytes)
{
	wtk_queue_node_t *qn;
	wtk_json_array_item_t *ai;

	for(qn=item->v.array->pop;qn;qn=qn->next)
	{
		ai=data_offset2(qn,wtk_json_array_item_t,q_n);
		if(ai->item->type==WTK_JSON_STRING && wtk_string_cmp(ai->item->v.str,v,v_bytes)==0)
		{
			return ai->item;
		}
	}
	return NULL;
}


void wtk_json_item_print2(wtk_json_item_t *item,wtk_strbuf_t *buf)
{
	wtk_queue_node_t *n;

	switch(item->type)
	{
	case WTK_JSON_FALSE:
		wtk_strbuf_push_s(buf,"false");
		break;
	case WTK_JSON_TRUE:
		wtk_strbuf_push_s(buf,"true");
		break;
	case WTK_JSON_NULL:
		wtk_strbuf_push_s(buf,"null");
		break;
	case WTK_JSON_STRING:
		wtk_strbuf_push_c(buf,'\"');
		if(item->v.str)
		{
			wtk_strbuf_push_add_escape_str(buf,item->v.str->data,item->v.str->len);
		}
		wtk_strbuf_push_c(buf,'\"');
		break;
	case WTK_JSON_NUMBER:
		{
			double d=item->v.number;

			//wtk_debug("%f\n",d-(int)d);
			if(fabs(d-(int)d)<=DBL_EPSILON && d<=INT_MAX && d>=INT_MIN)
			{
				wtk_strbuf_push_f(buf,"%d",(int)d);
			}
			else
			{
				if (fabs(floor(d)-d)<=DBL_EPSILON)
				{
					wtk_strbuf_push_f(buf,"%.0f",d);
				}/*else if (fabs(d)<1.0e-6 || fabs(d)>1.0e9)
				{
					wtk_strbuf_push_f(buf,"%e",d);
				}*/
				else
				{
					wtk_strbuf_push_f(buf,"%f",d);
				}
			}
			//wtk_strbuf_push_f(buf,"(n)");
		}
		break;
	case WTK_JSON_ARRAY:
		{
			wtk_json_array_item_t *aitem;
			int i;

			wtk_strbuf_push_c(buf,'[');
			for(i=0,n=item->v.array->pop;n;n=n->next,++i)
			{
				if(i>0)
				{
					wtk_strbuf_push_c(buf,',');
				}
				aitem=data_offset(n,wtk_json_array_item_t,q_n);
				//printf("[%d]=",i);
				wtk_json_item_print2(aitem->item,buf);
				//printf("\n");
			}
			wtk_strbuf_push_c(buf,']');
		}
		break;
	case WTK_JSON_OBJECT:
		{
			wtk_json_obj_item_t *oitem;

			wtk_strbuf_push_c(buf,'{');
			for(n=item->v.object->pop;n;n=n->next)
			{
				if(n!=item->v.object->pop)
				{
					wtk_strbuf_push_c(buf,',');
				}
				oitem=data_offset(n,wtk_json_obj_item_t,q_n);
				//wtk_debug("[%.*s]\n",oitem->k.len,oitem->k.data);
				wtk_strbuf_push_c(buf,'\"');
				wtk_strbuf_push_add_escape_str(buf,oitem->k.data,oitem->k.len);
				//wtk_strbuf_push_f(buf,"(%p)",oitem->item);
				wtk_strbuf_push_s(buf,"\":");
				wtk_json_item_print2(oitem->item,buf);
				//printf("\n");
			}
			wtk_strbuf_push_c(buf,'}');
		}
		break;
	}
	//printf("\n");
}

void wtk_json_item_print(wtk_json_item_t *item,wtk_strbuf_t *buf)
{
	wtk_strbuf_reset(buf);
	wtk_json_item_print2(item,buf);
}

void wtk_json_item_print4(wtk_json_item_t *item)
{
	wtk_strbuf_t *buf;

	buf=wtk_strbuf_new(1024,1);
	wtk_json_item_print(item,buf);
	//wtk_debug("[%.*s]\n",buf->pos,buf->data);
	printf("[%.*s]",buf->pos,buf->data);
	wtk_strbuf_delete(buf);
}

void wtk_json_item_print3(wtk_json_item_t *item)
{
	wtk_strbuf_t *buf;

	buf=wtk_strbuf_new(1024,1);
	wtk_json_item_print(item,buf);
	wtk_debug("[%.*s]\n",buf->pos,buf->data);
	wtk_strbuf_delete(buf);
}

void wtk_json_print(wtk_json_t *json,wtk_strbuf_t *buf)
{
	wtk_strbuf_reset(buf);
	wtk_json_item_print2(json->main,buf);
	//wtk_debug("%.*s\n",buf->pos,buf->data);
}

wtk_json_item_t* wtk_json_item_dup(wtk_json_item_t *src,wtk_heap_t *heap)
{
	wtk_json_item_t *dst,*vi;
	wtk_json_t json;
	wtk_queue_node_t *qn;
	wtk_json_array_item_t *ai;
	wtk_json_obj_item_t *oi;

	json.heap=heap;
	switch(src->type)
	{
	case WTK_JSON_FALSE:
	case WTK_JSON_TRUE:
	case WTK_JSON_NULL:
		dst=wtk_json_new_item(&(json),src->type);
		break;
	case WTK_JSON_NUMBER:
		dst=wtk_json_new_number(&(json),src->v.number);
		break;
	case WTK_JSON_STRING:
		dst=wtk_json_new_string(&(json),src->v.str->data,src->v.str->len);
		break;
	case WTK_JSON_ARRAY:
		dst=wtk_json_new_array(&(json));
		for(qn=src->v.array->pop;qn;qn=qn->next)
		{
			ai=data_offset2(qn,wtk_json_array_item_t,q_n);
			vi=wtk_json_item_dup(ai->item,heap);
			wtk_json_array_add_item(&(json),dst,vi);
		}
		break;
	case WTK_JSON_OBJECT:
		dst=wtk_json_new_object(&(json));
		for(qn=src->v.object->pop;qn;qn=qn->next)
		{
			oi=data_offset2(qn,wtk_json_obj_item_t,q_n);
			vi=wtk_json_item_dup(oi->item,heap);
			wtk_json_obj_add_item2(&(json),dst,oi->k.data,oi->k.len,vi);
		}
		break;
	default:
		dst=NULL;
		break;
	}
	return dst;
}

int wtk_json_copy_obj_dict(wtk_json_t *json,wtk_json_item_t *dst,wtk_json_item_t *src)
{
	wtk_queue_node_t *qn;
	wtk_json_obj_item_t *oi;
	wtk_json_item_t *vi;

	if(src->type!=WTK_JSON_OBJECT && dst->type!=WTK_JSON_OBJECT)
	{
		return -1;
	}
	for(qn=src->v.object->pop;qn;qn=qn->next)
	{
		oi=data_offset2(qn,wtk_json_obj_item_t,q_n);
		vi=wtk_json_item_dup(oi->item,json->heap);
		wtk_json_obj_add_item2(json,dst,oi->k.data,oi->k.len,vi);
	}
	return 0;
}

int wtk_json_item_cmp(wtk_json_item_t *src,wtk_json_item_t *dst)
{
	wtk_queue_node_t *qn,*qn1;
	wtk_json_array_item_t *ai,*ai2;
	wtk_json_obj_item_t *oi;
	wtk_json_item_t *vi;
	int ret;

	if(src->type!=dst->type)
	{
		//wtk_debug("type wrong[%d/%d]\n",src->type,dst->type);
		return -1;
	}
	switch(src->type)
	{
	case WTK_JSON_FALSE:
	case WTK_JSON_TRUE:
	case WTK_JSON_NULL:
		return 0;
		break;
	case WTK_JSON_STRING:
		return wtk_string_cmp(src->v.str,dst->v.str->data,dst->v.str->len);
		break;
	case WTK_JSON_NUMBER:
		//wtk_debug("number wrong\n");
		return src->v.number-dst->v.number;
		break;
	case WTK_JSON_ARRAY:
		if(src->v.array->length!=dst->v.array->length){return -1;}
		for(qn=src->v.array->pop,qn1=dst->v.array->pop;qn;qn=qn->next,qn1=qn1->next)
		{
			ai=data_offset2(qn,wtk_json_array_item_t,q_n);
			ai2=data_offset2(qn1,wtk_json_array_item_t,q_n);
			ret=wtk_json_item_cmp(ai->item,ai2->item);
			if(ret!=0){return ret;}
		}
		return 0;
		break;
	case WTK_JSON_OBJECT:
		if(src->v.object->length!=dst->v.object->length){return -1;}
		for(qn=src->v.object->pop;qn;qn=qn->next)
		{
			oi=data_offset2(qn,wtk_json_obj_item_t,q_n);
			vi=wtk_json_obj_get(dst,oi->k.data,oi->k.len);
			if(vi && wtk_json_item_cmp(vi,oi->item)==0)
			{

			}else
			{
				//wtk_debug("obj wrong\n");
				return -1;
			}
		}
		return 0;
		break;
	}
	//wtk_debug("wrong\n");
//	if(ret!=0)
//	{
//		wtk_json_item_print3(src);
//		wtk_json_item_print3(dst);
//	}
	return -1;
}

wtk_string_t* wtk_json_item_get_str_value(wtk_json_item_t *item)
{
	switch(item->type)
	{
	case WTK_JSON_STRING:
		return item->v.str;
		break;
	case WTK_JSON_ARRAY:
		item=wtk_json_array_get(item,0);
		if(item)
		{
			return wtk_json_item_get_str_value(item);
		}
		break;
	case WTK_JSON_OBJECT:
		item=wtk_json_obj_get_s(item,"_v");
		if(item)
		{
			return wtk_json_item_get_str_value(item);
		}
		break;
	default:
		break;
	}
	return NULL;
}

wtk_string_t* wtk_json_item_get_str_value2(wtk_json_item_t *item)
{
	char tmp[64];
	wtk_json_item_t *ti;

	switch(item->type)
	{
	case WTK_JSON_FALSE:
		return wtk_string_dup("false");
		break;
	case WTK_JSON_TRUE:
		return wtk_string_dup("true");
		break;
	case WTK_JSON_NULL:
		return wtk_string_dup("nil");
		break;
	case WTK_JSON_STRING:
		return wtk_string_dup_data(item->v.str->data,item->v.str->len);
		break;
	case WTK_JSON_NUMBER:
		{
			double d=item->v.number;

			//wtk_debug("%f\n",d-(int)d);
			if(fabs(d-(int)d)<=DBL_EPSILON && d<=INT_MAX && d>=INT_MIN)
			{
				sprintf(tmp,"%d",(int)d);
			}
			else
			{
				if (fabs(floor(d)-d)<=DBL_EPSILON)
				{
					sprintf(tmp,"%.0f",d);
				}/*else if (fabs(d)<1.0e-6 || fabs(d)>1.0e9)
				{
					wtk_strbuf_push_f(buf,"%e",d);
				}*/
				else
				{
					sprintf(tmp,"%f",d);
				}
			}
			return wtk_string_dup_data(tmp,strlen(tmp));
			//wtk_strbuf_push_f(buf,"(n)");
		}
		break;
	case WTK_JSON_ARRAY:
		item=wtk_json_array_get(item,0);
		if(item)
		{
			return wtk_json_item_get_str_value2(item);
		}
		break;
	case WTK_JSON_OBJECT:
		ti=wtk_json_obj_get_s(item,"_v");
		if(ti)
		{
			return wtk_json_item_get_str_value2(ti);
		}else
		{
			ti=wtk_json_obj_get_first(item);
			if(ti)
			{
				return wtk_json_item_get_str_value2(ti);
			}
		}
		break;
	}
	return NULL;
}

wtk_json_item_t* wtk_json_item_add_path_item(wtk_json_t *json,wtk_json_item_t *item,char *k,int k_len,wtk_json_item_type_t type)
{
	char *s,*e;
	int n;
	wtk_string_t x;
	wtk_json_item_t *ji,*ti=NULL;

	if(!item || item->type!=WTK_JSON_OBJECT){goto end;}
	s=k;e=s+k_len;
	x.data=NULL;
	x.len=0;
	while(s<e)
	{
		n=wtk_utf8_bytes(*s);
		if(x.data==NULL)
		{
			x.data=s;
		}else if(*s=='.')
		{
			x.len=s-x.data;
			//wtk_debug("[%.*s]\n",x.len,x.data);
			ji=wtk_json_obj_get(item,x.data,x.len);
			if(!ji)
			{
				ji=wtk_json_new_object(json);
				wtk_json_obj_add_item2(json,item,x.data,x.len,ji);
			}else if(ji->type!=WTK_JSON_OBJECT)
			{
				wtk_json_obj_remove(item,x.data,x.len);
				ji=wtk_json_new_object(json);
				wtk_json_obj_add_item2(json,item,x.data,x.len,ji);
			}
			item=ji;
			x.data=NULL;
		}
		s+=n;
	}
	if(x.data)
	{
		x.len=s-x.data;
		ti=wtk_json_obj_get(item,x.data,x.len);
		if(ti && ti->type==type)
		{

		}else
		{
			if(ti)
			{
				wtk_json_obj_remove(item,x.data,x.len);
			}
			switch(type)
			{
			case WTK_JSON_ARRAY:
				ti=wtk_json_new_array(json);
				break;
			case WTK_JSON_OBJECT:
				ti=wtk_json_new_object(json);
				break;
			default:
				ti=wtk_json_new_item(json,type);
				break;
			}
			wtk_json_obj_add_item2(json,item,x.data,x.len,ti);
		}
	}
end:
	return ti;
}

wtk_json_item_t* wtk_json_item_get_path_item(wtk_json_item_t *item,char *k,int k_len,wtk_string_t *last_k)
{
	char *s,*e;
	int n;
	wtk_string_t x;
	wtk_json_item_t *ji,*ti=NULL;

	if(last_k)
	{
		wtk_string_set(last_k,0,0);
	}
	if(!item || item->type!=WTK_JSON_OBJECT){goto end;}
	s=k;e=s+k_len;
	x.data=NULL;
	x.len=0;
	while(s<e)
	{
		n=wtk_utf8_bytes(*s);
		if(x.data==NULL)
		{
			x.data=s;
		}else if(*s=='.')
		{
			x.len=s-x.data;
			//wtk_debug("[%.*s]\n",x.len,x.data);
			ji=wtk_json_obj_get(item,x.data,x.len);
			if(!ji || ji->type!=WTK_JSON_OBJECT)
			{
				goto end;
			}
			item=ji;
			x.data=NULL;
		}
		s+=n;
	}
	if(x.data)
	{
		x.len=s-x.data;
		ti=wtk_json_obj_get(item,x.data,x.len);
		if(last_k  && ti)
		{
			wtk_string_set(last_k,x.data,x.len);
		}
	}
end:
	return ti;
}

wtk_string_t* wtk_json_item_get_path_string(wtk_json_item_t *item,char *k,int k_len)
{
	wtk_string_t *v=NULL;
	wtk_json_item_t *vi;

	vi=wtk_json_item_get_path_item(item,k,k_len,NULL);
	if(!vi){goto end;}
	v=wtk_json_item_get_str_value(vi);
end:
	return v;
}

int wtk_json_item_set_path_str(wtk_json_t *json,char *k,int k_len,char *v,int v_len)
{
	char *s,*e;
	int n;
	wtk_string_t x;
	wtk_json_item_t *item,*ji;
	int ret=-1;

	item=json->main;
	if(!item || item->type!=WTK_JSON_OBJECT){goto end;}
	s=k;e=s+k_len;
	x.data=NULL;
	x.len=0;
	while(s<e)
	{
		n=wtk_utf8_bytes(*s);
		if(x.data==NULL)
		{
			x.data=s;
		}else if(*s=='.')
		{
			x.len=s-x.data;
			//wtk_debug("[%.*s]\n",x.len,x.data);
			ji=wtk_json_obj_get(item,x.data,x.len);
			if(!ji)
			{
				ji=wtk_json_new_object(json);
				wtk_json_obj_add_item2(json,item,x.data,x.len,ji);
			}else if(ji->type!=WTK_JSON_OBJECT)
			{
				goto end;
			}
			item=ji;
			x.data=NULL;
		}
		s+=n;
	}
	if(x.data)
	{
		x.len=s-x.data;
		//wtk_debug("[%.*s]\n",x.len,x.data);
		//exit(0);
		wtk_json_obj_remove(item,x.data,x.len);
		//wtk_debug("[%.*s]=[%.*s]\n",x.len,x.data,v_len,v);
		wtk_json_obj_add_str2(json,item,x.data,x.len,v,v_len);
		ret=0;
	}
end:
	return ret;
}
