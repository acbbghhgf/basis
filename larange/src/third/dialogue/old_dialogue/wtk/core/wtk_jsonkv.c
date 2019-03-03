#include <ctype.h>
#include "wtk_jsonkv.h" 
#include "wtk/core/wtk_os.h"

wtk_jsonkv_t* wtk_jsonkv_new(char *dn)
{
	wtk_jsonkv_t *kv;

	kv=(wtk_jsonkv_t*)wtk_malloc(sizeof(wtk_jsonkv_t));
	kv->json_parser=wtk_json_parser_new();
	kv->dn=wtk_strbuf_new(256,1);
	wtk_strbuf_push(kv->dn,dn,strlen(dn));
	kv->buf=wtk_strbuf_new(256,1);
	kv->tmp=wtk_strbuf_new(1024,1);
	kv->get_map=NULL;
	kv->get_map_ths=NULL;
	wtk_mkdir_p(dn,'/',1);
	//wtk_debug("%s\n",dn);
	return kv;
}

void wtk_jsonkv_delete(wtk_jsonkv_t *kv)
{
	wtk_strbuf_delete(kv->tmp);
	wtk_strbuf_delete(kv->buf);
	wtk_strbuf_delete(kv->dn);
	wtk_json_parser_delete(kv->json_parser);
	wtk_free(kv);
}

void wtk_jsonkv_set_dn(wtk_jsonkv_t *kv,char *dn)
{
	wtk_strbuf_reset(kv->dn);
	wtk_strbuf_push(kv->dn,dn,strlen(dn));
	wtk_mkdir_p(dn,'/',1);
}

void wtk_jsonkv_reset(wtk_jsonkv_t *kv)
{
	wtk_json_parser_reset(kv->json_parser);
}

typedef enum
{
	WTK_JSONKV_ATTR_INIT,
	WTK_JSONKV_ATTR_K,
}wtk_jsonkv_attr_state_t;

char* wtk_jsonkv_get_fn(wtk_jsonkv_t *kv,char *k,int k_bytes)
{
	wtk_strbuf_t *buf=kv->buf;
	wtk_string_t v;

	wtk_strbuf_reset(buf);
	wtk_strbuf_push(buf,kv->dn->data,kv->dn->pos);
	wtk_strbuf_push_c(buf,'/');
	if(kv->get_map)
	{
		v=kv->get_map(kv->get_map_ths,k,k_bytes);
		if(v.len>0)
		{
			wtk_strbuf_push(buf,v.data,v.len);
		}else
		{
			wtk_strbuf_push(buf,k,k_bytes);
		}
	}else
	{
		wtk_strbuf_push(buf,k,k_bytes);
	}
	wtk_strbuf_push_s(buf,".json");
	wtk_strbuf_push_c(buf,0);
	return buf->data;
}

wtk_string_t wtk_jsonkv_get_dat(wtk_jsonkv_t *kv,char *k,int k_bytes)
{
	wtk_strbuf_t *buf=kv->buf;
	char *fn;
	int ret;
	wtk_string_t v;
	char *data=NULL;
	int len;

	wtk_string_set(&(v),0,0);
	fn=wtk_jsonkv_get_fn(kv,k,k_bytes);
	ret=wtk_file_exist(fn);
	if(ret!=0){goto end;}
	data=file_read_buf(buf->data,&len);
	if(!data){goto end;}
	wtk_strbuf_reset(buf);
	wtk_strbuf_push(buf,data,len);
	wtk_string_set(&(v),data,len);
end:
	if(data)
	{
		wtk_free(data);
	}
	return v;
}

void wtk_jsonkv_set_dat(wtk_jsonkv_t *kv,char *k,int k_bytes,char *v,int v_bytes)
{
	char *fn;

	fn=wtk_jsonkv_get_fn(kv,k,k_bytes);
	file_write_buf(fn,v,v_bytes);
}

void wtk_jsonkv_save_json(wtk_jsonkv_t *kv,char *k,int k_bytes,wtk_json_t *json)
{
	char *fn;
	wtk_strbuf_t *buf2;

	fn=wtk_jsonkv_get_fn(kv,k,k_bytes);
	buf2=wtk_strbuf_new(1024,1);
	wtk_json_item_print(json->main,buf2);
//	wtk_debug("[%.*s]=%s\n",k_bytes,k,fn);
//	wtk_debug("%.*s\n",buf2->pos,buf2->data);
//	exit(0);
	file_write_buf(fn,buf2->data,buf2->pos);
	wtk_strbuf_delete(buf2);
}

wtk_json_t* wtk_jsonkv_get_json(wtk_jsonkv_t *kv,char *k,int k_bytes)
{
	wtk_json_parser_t *json=kv->json_parser;
	char *fn;
	int ret;
	wtk_string_t v;
	wtk_json_t *xj=NULL;

	wtk_string_set(&(v),0,0);
	wtk_json_parser_reset(json);
	fn=wtk_jsonkv_get_fn(kv,k,k_bytes);
	ret=wtk_file_exist(fn);
	//wtk_debug("%s=%d\n",fn,ret);
	///exit(0);
	if(ret==0)
	{
		wtk_json_parser_parse_file(json,fn);
	}else
	{
		//wtk_debug("[%.*s] not found.\n",buf->pos,buf->data);
		//goto end;
		json->json->main=wtk_json_new_object(json->json);
	}
	if(!json->json->main)
	{
		wtk_debug("[%s] null.\n",fn);
		goto end;
	}
	xj=json->json;
end:
	return xj;
}

wtk_string_t wtk_jsonkv_get(wtk_jsonkv_t *kv,char *k,int k_bytes,char *attr,int attr_bytes)
{
	wtk_json_parser_t *json=kv->json_parser;
	char *fn;
	int ret;
	char *s,*e;
	wtk_string_t nm;
	wtk_string_t last_nm;
	int cnt;
	wtk_jsonkv_attr_state_t state;
	wtk_json_item_t *item,*parent;
	wtk_string_t v;
	int find;

	wtk_string_set(&(v),0,0);
	wtk_json_parser_reset(json);
	fn=wtk_jsonkv_get_fn(kv,k,k_bytes);
	ret=wtk_file_exist(fn);
	if(ret==0)
	{
		wtk_json_parser_parse_file(json,fn);
	}else
	{
		//wtk_debug("[%.*s] not found.\n",buf->pos,buf->data);
		goto end;
	}
	if(!json->json->main)
	{
		wtk_debug("[%s] null.\n",fn);
		goto end;
	}
	s=attr;e=s+attr_bytes;
	state=WTK_JSONKV_ATTR_INIT;
	parent=json->json->main;
	item=NULL;
	wtk_string_set(&(last_nm),0,0);
	wtk_string_set(&(nm),0,0);
	while(s<e)
	{
		cnt=wtk_utf8_bytes(*s);
		find=0;
		switch(state)
		{
		case WTK_JSONKV_ATTR_INIT:
			if(cnt==1 && isspace(*s))
			{

			}else
			{
				nm.data=s;
				state=WTK_JSONKV_ATTR_K;
				if(s+cnt>=e)
				{
					find=1;
					nm.len=cnt;
				}
			}
			break;
		case WTK_JSONKV_ATTR_K:
			if(cnt==1 &&(isspace(*s) || *s==':'))
			{
				find=1;
				nm.len=s-nm.data;
			}else if(s+cnt>=e)
			{
				find=1;
				nm.len=s-nm.data+cnt;
			}
			break;
		}
		if(find)
		{
			if(last_nm.len>0)
			{
				//wtk_debug("[%.*s]\n",last_nm.len,last_nm.data)
				item=wtk_json_obj_get(parent,last_nm.data,last_nm.len);
				if(!item)
				{
					wtk_debug("[%.*s] not found\n",last_nm.len,last_nm.data)
					goto end;
				}
				parent=item;
			}
			last_nm=nm;
			state=WTK_JSONKV_ATTR_INIT;
		}
		s+=cnt;
	}
	//wtk_json_item_print3(parent);
	if(nm.len>0)
	{
		//wtk_debug("[%.*s]\n",nm.len,nm.data);
		item=wtk_json_obj_get(parent,nm.data,nm.len);
		if(!item)
		{
			if(nm.len==1 && nm.data[0]=='*')
			{
				item=parent;
			}else
			{
				goto end;
			}
		}
	}else
	{
		item=parent;
	}
	//wtk_json_item_print3(item);
	if(item->type==WTK_JSON_STRING)
	{
		v=*(item->v.str);
	}else
	{
		wtk_json_item_print(item,kv->tmp);
		wtk_string_set(&(v),kv->tmp->data,kv->tmp->pos);
	}
end:
	return v;
}

void wtk_jsonkv_set(wtk_jsonkv_t *kv,char *k,int k_bytes,char *attr,int attr_bytes,char *v,int v_bytes)
{
	wtk_json_parser_t *json=kv->json_parser;
	char *fn;
	int ret;
	char *s,*e;
	wtk_string_t nm = {0}, vx = {0};
	wtk_string_t last_nm;
	int cnt;
	wtk_jsonkv_attr_state_t state;
	wtk_json_item_t *item,*parent;
	int find;

	wtk_json_parser_reset(json);
	fn=wtk_jsonkv_get_fn(kv,k,k_bytes);
	ret=wtk_file_exist(fn);
	if(ret==0)
	{
		wtk_json_parser_parse_file(json,fn);
	}
	s=attr;e=s+attr_bytes;
	state=WTK_JSONKV_ATTR_INIT;
	if(!json->json->main)
	{
		json->json->main=wtk_json_new_object(json->json);
	}
	parent=json->json->main;
	item=NULL;
	wtk_string_set(&(last_nm),0,0);
	while(s<e)
	{
		cnt=wtk_utf8_bytes(*s);
		find=0;
		switch(state)
		{
		case WTK_JSONKV_ATTR_INIT:
			if(cnt==1 && isspace(*s))
			{

			}else
			{
				nm.data=s;
				state=WTK_JSONKV_ATTR_K;
				if(s+cnt>=e)
				{
					find=1;
					nm.len=cnt;
				}
			}
			break;
		case WTK_JSONKV_ATTR_K:
			if(cnt==1 &&(isspace(*s) || *s==':'))
			{
				find=1;
				nm.len=s-nm.data;
			}else if(s+cnt>=e)
			{
				find=1;
				nm.len=s-nm.data+cnt;
			}
			break;
		}
		if(find)
		{
			if(last_nm.len>0)
			{
				//wtk_debug("[%.*s]\n",last_nm.len,last_nm.data)
				item=wtk_json_obj_get(parent,last_nm.data,last_nm.len);
				if(!item)
				{
					item=wtk_json_new_object(json->json);
					wtk_json_obj_add_item2(json->json,parent,last_nm.data,last_nm.len,item);
				}
				parent=item;
			}
			last_nm=nm;
			state=WTK_JSONKV_ATTR_INIT;
		}
		s+=cnt;
	}
//	if(parent)
//	{
//		wtk_debug("type=%d\n",parent->type);
//	}
//	wtk_debug("nm=%.*s\n",nm.len,nm.data);
	item=wtk_json_obj_get(parent,nm.data,nm.len);
	if(item)
	{
		wtk_string_set(&(vx),v,v_bytes);
		item->v.str=&(vx);
		item->type=WTK_JSON_STRING;
	}else
	{
		wtk_string_set(&(vx),v,v_bytes);
		wtk_json_obj_add_ref_str(json->json,parent,nm.data,nm.len,&(vx));
	}
	//wtk_json_item_print3(json->json->main);
	wtk_json_print(json->json,kv->tmp);
	file_write_buf(fn,kv->tmp->data,kv->tmp->pos);
}


void wtk_jsonkv_test()
{
	wtk_jsonkv_t *kv;
	wtk_string_t v;

	kv=wtk_jsonkv_new("jkv");
	wtk_jsonkv_set_sss(kv,"lz","name:sex:你好:hello:我","boy");
	wtk_jsonkv_set_sss(kv,"lz","name:sex:我","girl");

	v=wtk_jsonkv_get_ss(kv,"lz","name");
	wtk_debug("[%.*s]\n",v.len,v.data);

	v=wtk_jsonkv_get_ss(kv,"lz","name:sex:你好:hello:我");
	wtk_debug("[%.*s]\n",v.len,v.data);

	v=wtk_jsonkv_get_ss(kv,"lz","name:sex:我");
	wtk_debug("[%.*s]\n",v.len,v.data);

	v=wtk_jsonkv_get_ss(kv,"lz","");
	wtk_debug("[%.*s]\n",v.len,v.data);

	wtk_jsonkv_delete(kv);
}

