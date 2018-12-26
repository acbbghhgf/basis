#include "wtk_act_lua.h"
#include "wtk/core/json/wtk_json.h"

void wtk_act_init_hash(wtk_act_t *act,wtk_str_hash_t *hash)
{
	act->v.hash=hash;
	act->type=WTK_ACT_HASH;
}

void wtk_act_init_json(wtk_act_t *act,wtk_heap_t *heap)
{
	act->type=WTK_ACT_JSON;
	act->v.json.heap=heap;
	wtk_act_reset(act);
}

void wtk_act_init_slot(wtk_act_t *act,wtk_semslot_t *slot)
{
	act->type=WTK_ACT_SLOT;
	act->v.slot=slot;
}

void wtk_act_reset(wtk_act_t *act)
{
	switch(act->type)
	{
	case WTK_ACT_JSON:
		act->v.json.json=NULL;
		act->v.json.req=NULL;
		break;
	case WTK_ACT_SLOT:
		wtk_semslot_reset(act->v.slot);
		break;
	case WTK_ACT_HASH:
		wtk_str_hash_reset(act->v.hash);
		break;
	}
}

int wtk_act_nslot(wtk_act_t *act)
{
	switch(act->type)
	{
	case WTK_ACT_JSON:
//		if(act->v.json.req)
//		{
//			wtk_json_item_print4(act->v.json.req);
//		}
		return act->v.json.req?act->v.json.req->v.object->length:0;
		break;
	case WTK_ACT_SLOT:
		return act->v.slot->slot_q.length;
		break;
	case WTK_ACT_HASH:
		return wtk_str_hash_elems(act->v.hash);
		break;
	}
	return 0;
}

void wtk_act_update(wtk_act_t *act)
{
	wtk_json_act_t *j;

	if(act->type==WTK_ACT_JSON)
	{
		j=&(act->v.json);
		if(j->json)
		{
			j->req=wtk_json_obj_get_s(j->json,"request");
		}else
		{
			j->req=NULL;
		}
	}
}

void wtk_act_print(wtk_act_t *act)
{
	switch(act->type)
	{
	case WTK_ACT_JSON:
		if(act->v.json.json)
		{
			wtk_json_item_print3(act->v.json.json);
		}
		break;
	case WTK_ACT_SLOT:
		wtk_semslot_print(act->v.slot);
		break;
	case WTK_ACT_HASH:
		{
			wtk_str_hash_it_t it;
			hash_str_node_t *node;
			wtk_string_t *v;

			it=wtk_str_hash_iterator(act->v.hash);
			while(1)
			{
				node=wtk_str_hash_it_next(&(it));
				//wtk_debug("node=%p\n",node);
				if(!node){break;}
				v=(wtk_string_t*)(node->value);
				printf("%.*s=%.*s\n",node->key.len,node->key.data,v->len,v->data);
			}
		}
		break;
	}
}

wtk_json_item_t* wtk_json_act_get_value(wtk_json_act_t *act,char *k,int k_len)
{
	wtk_json_item_t *vi=NULL;

	if(!act->req){goto end;}
	vi=wtk_json_obj_get(act->req,k,k_len);
end:
	return vi;
}

int wtk_act_is_str_value(wtk_act_t *a,char *k,int k_len)
{
	wtk_json_item_t *vi;
	wtk_json_act_t *act;
	int ret=0;

	if(a->type!=WTK_ACT_JSON)
	{
		ret=1;goto end;
	}
	act=&(a->v.json);
	if(!act->req)
	{
		//wtk_debug("miss req\n");
		goto end;
	}
	vi=wtk_json_obj_get(act->req,k,k_len);
	if(!vi)
	{
		//wtk_debug("[%.*s] miss\n",k_len,k);
		goto end;
	}
	if(vi->type==WTK_JSON_ARRAY)
	{
		vi=wtk_json_array_get(vi,0);
		if(!vi)
		{
			//wtk_debug("miss array\n");
			goto end;
		}
	}
	if(vi->type==WTK_JSON_OBJECT)
	{
		vi=wtk_json_obj_get_s(vi,"_v");
		if(!vi)
		{
			goto end;
		}
	}
	ret=vi->type==WTK_JSON_STRING?1:0;
end:
	return ret;
}

wtk_string_t* wtk_act_json_get_str_value(wtk_json_item_t *req)
{
	wtk_json_item_t *vi,*ti;
	wtk_string_t *v=NULL;

	vi=req;
	if(!vi){goto end;}
	while(vi->type==WTK_JSON_ARRAY)
	{
		vi=wtk_json_array_get(vi,0);
		if(!vi)
		{
			//wtk_debug("miss array\n");
			goto end;
		}
	}
	//wtk_json_item_print3(vi);
	while(vi->type==WTK_JSON_OBJECT)
	{
		ti=wtk_json_obj_get_s(vi,"_v");
		if(ti)
		{
			vi=ti;
			break;
		}else
		{
			vi=wtk_json_obj_get_first(vi);
			if(!vi){goto end;}
			if(vi->type==WTK_JSON_STRING)
			{
				break;
			}else if(vi->type!=WTK_JSON_OBJECT)
			{
				break;
			}
		}
	}
	v=vi->type==WTK_JSON_STRING?vi->v.str:NULL;
end:
	return v;
}

int wtk_act_has_key(wtk_act_t *a,char *k,int k_len)
{
	wtk_json_item_t *vi;
	wtk_string_t *v=NULL;
	wtk_json_act_t *act;

	if(a->type!=WTK_ACT_JSON)
	{
		switch(a->type)
		{
		case WTK_ACT_SLOT:
			v=wtk_semslot_get2(a->v.slot,k,k_len);
			return v?1:0;
			break;
		case WTK_ACT_HASH:
			v=(wtk_string_t*)wtk_str_hash_find(a->v.hash,k,k_len);
			return v?1:0;
			break;
		default:
			break;
		}
	}
	act=&(a->v.json);
	if(!act->req)
	{
		//wtk_debug("miss req\n");
		return 0;
		goto end;
	}
	vi=wtk_json_obj_get(act->req,k,k_len);
	return vi?1:0;
end:
	return 0;
}


wtk_string_t* wtk_act_get_str_value(wtk_act_t *a,char *k,int k_len)
{
	wtk_json_item_t *vi,*ti;
	wtk_string_t *v=NULL;
	wtk_json_act_t *act;

	if(a->type!=WTK_ACT_JSON)
	{
		switch(a->type)
		{
		case WTK_ACT_SLOT:
			return wtk_semslot_get2(a->v.slot,k,k_len);
			break;
		case WTK_ACT_HASH:
			return (wtk_string_t*)wtk_str_hash_find(a->v.hash,k,k_len);
			break;
		default:
			break;
		}
	}
	act=&(a->v.json);
	if(!act->req)
	{
		//wtk_debug("miss req\n");
		goto end;
	}
	vi=wtk_json_obj_get(act->req,k,k_len);
	if(!vi)
	{
		//wtk_debug("[%.*s] miss\n",k_len,k);
		goto end;
	}
	//if array get first element;
	while(vi->type==WTK_JSON_ARRAY)
	{
		vi=wtk_json_array_get(vi,vi->v.array->length-1);
		if(!vi)
		{
			//wtk_debug("miss array\n");
			goto end;
		}
	}
	//wtk_json_item_print3(vi);
	while(vi->type==WTK_JSON_OBJECT)
	{
		ti=wtk_json_obj_get_s(vi,"_v");
		if(ti)
		{
			vi=ti;
			break;
		}else
		{
			vi=wtk_json_obj_get_first(vi);
			if(!vi){goto end;}
			if(vi->type==WTK_JSON_STRING)
			{
				break;
			}else if(vi->type!=WTK_JSON_OBJECT)
			{
				break;
			}
		}
	}
	v=vi->type==WTK_JSON_STRING?vi->v.str:NULL;
end:
	return v;
}

int wtk_lua_act_print(lua_State *l)
{
	wtk_act_t *act;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	act=(wtk_act_t*)lua_touserdata(l,i);
	wtk_act_print(act);
end:
	return 0;
}

int wtk_lua_act_set(lua_State *l)
{
#define DEF_REQ "request"
	wtk_act_t *a;
	wtk_json_act_t *act;
	wtk_json_item_t *vi;
	char *s;
	size_t slen;
	char* k;
	size_t klen;
	char *v;
	size_t vlen;
	int i;
	int cnt;
	wtk_json_t json;

	cnt=lua_gettop(l);
	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	a=(wtk_act_t*)lua_touserdata(l,i);
	if(!a){goto end;}
	if(cnt==4)
	{
		++i;
		if(!lua_isstring(l,i)){goto end;}
		s=(char*)lua_tolstring(l,i,&slen);
	}else
	{
		s=DEF_REQ;
		slen=sizeof(DEF_REQ)-1;
	}
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k=(char*)lua_tolstring(l,i,&klen);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v=(char*)lua_tolstring(l,i,&vlen);
	//wtk_debug("[%.*s]\n",(int)slen,s);
	//wtk_debug("[%.*s]=[%.*s]\n",(int)klen,k,(int)vlen,v);
	switch(a->type)
	{
	case WTK_ACT_JSON:
		act=&(a->v.json);
		json.heap=act->heap;
		vi=wtk_json_obj_get(act->json,s,slen);
		if(!vi)
		{

			vi=wtk_json_new_object(&(json));
			wtk_json_obj_add_item2(&(json),act->json,s,slen,vi);
		}else
		{
			wtk_json_obj_remove(vi,k,klen);
		}
		wtk_json_obj_add_str2(&(json),vi,k,klen,v,vlen);
		break;
	case WTK_ACT_SLOT:
		wtk_semslot_set(a->v.slot,k,klen,v,vlen);
		break;
	case WTK_ACT_HASH:
		{
			wtk_string_t *pv;

			pv=wtk_heap_dup_string(a->v.hash->heap,v,vlen);
			wtk_str_hash_add2(a->v.hash,k,klen,pv);
		}
		break;
	}
	wtk_act_update(a);
end:
	//exit(0);
	return 0;
}

void wtk_lua_push_json(lua_State *l,wtk_json_item_t *item)
{
	wtk_queue_node_t *qn;
	wtk_json_array_item_t *ai;
	wtk_json_obj_item_t *oi;
	int i;

	//wtk_json_item_print3(item);
	switch(item->type)
	{
	case WTK_JSON_FALSE:
		lua_pushboolean(l,0);
		break;
	case WTK_JSON_TRUE:
		lua_pushboolean(l,1);
		break;
	case WTK_JSON_NULL:
		lua_pushnil(l);
		break;
	case WTK_JSON_STRING:
		lua_pushlstring(l,item->v.str->data,item->v.str->len);
		break;
	case WTK_JSON_NUMBER:
		lua_pushnumber(l,item->v.number);
		break;
	case WTK_JSON_ARRAY:
		lua_newtable(l);
		for(i=1,qn=item->v.array->pop;qn;qn=qn->next,++i)
		{
			ai=data_offset2(qn,wtk_json_array_item_t,q_n);
			lua_pushnumber(l,i);
			wtk_lua_push_json(l,ai->item);
			lua_settable(l,-3);
		}
		//lua_settable(l,-3);
		break;
	case WTK_JSON_OBJECT:
		lua_newtable(l);
		for(qn=item->v.object->pop;qn;qn=qn->next)
		{
			oi=data_offset2(qn,wtk_json_obj_item_t,q_n);
			lua_pushlstring(l,oi->k.data,oi->k.len);
			wtk_lua_push_json(l,oi->item);
			lua_settable(l,-3);
		}
		//lua_settable(l,-3);
		break;
	}
//	wtk_debug("------------------\n");
//	wtk_json_item_print3(item);
}

int wtk_lua_json_set(lua_State *l)
{
	wtk_json_t *json;
	wtk_json_item_t *vi;
	char* k;
	size_t klen;
	char *v;
	size_t vlen;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	json=(wtk_json_t*)lua_touserdata(l,i);
	if(!json){goto end;}
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k=(char*)lua_tolstring(l,i,&klen);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v=(char*)lua_tolstring(l,i,&vlen);
	vi=wtk_json_obj_get(json->main,k,klen);
	if(vi)
	{
		wtk_json_obj_remove(json->main,k,klen);
	}
	//wtk_debug("[%.*s]=[%.*s]\n",(int)klen,k,(int)vlen,v);
	wtk_json_obj_add_str2(json,json->main,k,klen,v,vlen);
end:
	//exit(0);
	return 0;
}

int wtk_lua_json_get(lua_State *l)
{
	wtk_json_t *json;
	wtk_json_item_t *vi;
	char* k;
	size_t klen;
	int i;
	int vx=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	json=(wtk_json_t*)lua_touserdata(l,i);
	if(!json){goto end;}
	vi=json->main;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k=(char*)lua_tolstring(l,i,&klen);
	//wtk_debug("[%.*s]=[%.*s]\n",(int)slen,s,(int)klen,k);4
	vi=wtk_json_obj_get(vi,k,klen);
	if(!vi){goto end;}
	//wtk_json_item_print3(vi);
	wtk_lua_push_json(l,vi);
	vx=1;
end:
	return vx;
}


int wtk_lua_act_get(lua_State *l)
{
#define DEF_REQ "request"
	wtk_act_t *a;
	wtk_json_act_t *act;
	wtk_json_item_t *vi;
	char *s;
	size_t slen;
	char* k;
	size_t klen;
	int i;
	int cnt;
	int vx=0;

	cnt=lua_gettop(l);
	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	a=(wtk_act_t*)lua_touserdata(l,i);
	if(!a){goto end;}
	if(cnt==3)
	{
		++i;
		if(!lua_isstring(l,i)){goto end;}
		s=(char*)lua_tolstring(l,i,&slen);
	}else
	{
		s=DEF_REQ;
		slen=sizeof(DEF_REQ)-1;
	}
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k=(char*)lua_tolstring(l,i,&klen);
	//wtk_debug("[%.*s]=[%.*s]\n",(int)slen,s,(int)klen,k);4

	switch(a->type)
	{
	case WTK_ACT_JSON:
		act=&(a->v.json);
		vi=wtk_json_obj_get(act->json,s,slen);
		if(!vi){goto end;}
		vi=wtk_json_obj_get(vi,k,klen);
		if(!vi){goto end;}
		//wtk_json_item_print3(vi);
		wtk_lua_push_json(l,vi);
		//wtk_json_item_print3(vi);
		break;
	case WTK_ACT_SLOT:
		{
			wtk_string_t *v;

			v=wtk_semslot_get2(a->v.slot,k,klen);
			if(!v){goto end;}
			lua_pushlstring(l,v->data,v->len);
		}
		break;
	case WTK_ACT_HASH:
		{
			wtk_string_t *v;

			v=(wtk_string_t*)wtk_str_hash_find(a->v.hash,k,klen);
			if(!v){goto end;}
			lua_pushlstring(l,v->data,v->len);
		}
		break;
	}
	vx=1;
end:
	return vx;
}


int wtk_lua_chn2number(lua_State *l)
{
	const char *p;
	size_t len;
	int cnt;
	int i;
	int v;
	int left;

	cnt=0;
	i=1;
	if(!lua_isstring(l,i)){goto end;}
	p=lua_tolstring(l,i,&len);
	if(p[0]>='0' && p[0]<='9')
	{
		v=wtk_str_atoi((char*)p,(int)len);
	}else
	{
		v=wtk_chnstr_atoi((char*)p,len,&left);
		//wtk_debug("v=%d,left=%d\n",v,left);
		if(left>0){goto end;}
	}
	lua_pushnumber(l,v);
	//wtk_debug("n=%d\n",n);
	cnt=1;
end:
	return cnt;
}

int wtk_lua_itochn(lua_State *l)
{
	int i,v,cnt;
	wtk_strbuf_t *buf;

	cnt=0;
	i=1;
	if(!lua_isnumber(l,i)){goto end;}
	v=lua_tonumber(l,i);
	buf=wtk_strbuf_new(256,1);
	wtk_itochn(buf,v);
	lua_pushlstring(l,buf->data,buf->pos);
	wtk_strbuf_delete(buf);
	cnt=1;
end:
	return cnt;
}

int wtk_lua_chnmap(lua_State *l)
{
	const char *p;
	size_t len;
	int cnt;
	int i;
	int n;
	char *s,*e;
	wtk_strbuf_t *buf;

	buf=wtk_strbuf_new(256,1);
	cnt=0;
	i=1;
	if(!lua_isstring(l,i)){goto end;}
	p=lua_tolstring(l,i,&len);
	//wtk_debug("%s\n",p);
	s=(char*)p;e=s+len;
	while(s<e)
	{
		n=wtk_utf8_bytes(*s);
		i=wtk_chnstr_atoi4(s,n);
		//wtk_debug("i=%d\n",i);
		if(i>=0)
		{
			wtk_strbuf_push_f(buf,"%d",i);
		}else
		{
			//wtk_debug("[%.*s]=%d\n",n,s,i);
			cnt=0;
			goto end;
			//wtk_strbuf_push(buf,s,n);
		}
		s+=n;
	}
	lua_pushlstring(l,buf->data,buf->pos);
	cnt=1;
end:
	wtk_strbuf_delete(buf);
	return cnt;
}


int wtk_lua_chnmap2(lua_State *l)
{
	const char *p;
	size_t len;
	int cnt;
	int i;
	double v;
	int n;
	char *s,*e;

	cnt=0;
	i=1;
	if(!lua_isstring(l,i)){goto end;}
	p=lua_tolstring(l,i,&len);
	s=(char*)p;e=s+len;
	v=0;
	while(s<e)
	{
		n=wtk_utf8_bytes(*s);
		i=wtk_chnstr_atoi3(s,n);
		wtk_debug("[%.*s]=%d\n",n,s,i);
		if(v<0){goto end;}
		v=v*10+i;
		s+=n;
	}
	lua_pushnumber(l,v);
	//wtk_debug("n=%d\n",cnt);
	cnt=1;
end:
	return cnt;
}

int wtk_lua_str2array(lua_State *l)
{
	const char *p;
	size_t len;
	int n,cnt;
	int i;
	char *s,*e;
	int idx;

	cnt=0;
	i=1;
	if(!lua_isstring(l,i)){goto end;}
	p=lua_tolstring(l,i,&len);
	s=(char*)p;e=s+len;
	idx=0;
	lua_newtable(l);
	while(s<e)
	{
		n=wtk_utf8_bytes(*s);
		++idx;
		lua_pushnumber(l,idx);
		lua_pushlstring(l,s,n);
		lua_settable(l,-3);
		s+=n;
	}
	cnt=1;
end:
	return cnt;
}

wtk_act_t* wtk_act_new_str(char *data,int len)
{
	wtk_act_t *act;
	wtk_strkv_parser_t p;
	int ret;
	wtk_string_t *v;

	act=(wtk_act_t*)wtk_malloc(sizeof(wtk_act_t));
	act->v.hash=wtk_str_hash_new(13);
	act->type=WTK_ACT_HASH;
	wtk_strkv_parser_init(&(p),data,len);
	while(1)
	{
		ret=wtk_strkv_parser_next(&(p));
		if(ret!=0){break;}
		//wtk_debug("[%.*s]=[%.*s]\n",p.k.len,p.k.data,p.v.len,p.v.data);
		if(p.v.len>0 && p.k.len>0)
		{
			v=wtk_heap_dup_string(act->v.hash->heap,p.v.data,p.v.len);
			//wtk_debug("add\n");
			wtk_str_hash_add(act->v.hash,p.k.data,p.k.len,v);
		}
	}
	return act;
}

void wtk_act_delete(wtk_act_t *act)
{
	wtk_str_hash_delete(act->v.hash);
	wtk_free(act);
}

void wtk_act_lua_link(wtk_lua2_t *lua2)
{
	wtk_lua2_link_function(lua2,wtk_lua_act_print,"wtk_act_print");
	wtk_lua2_link_function(lua2,wtk_lua_act_get,"wtk_act_get");
	wtk_lua2_link_function(lua2,wtk_lua_act_set,"wtk_act_set");
	wtk_lua2_link_function(lua2,wtk_lua_chn2number,"wtk_chn2number");
	wtk_lua2_link_function(lua2,wtk_lua_itochn,"wtk_itochn");
	wtk_lua2_link_function(lua2,wtk_lua_str2array,"wtk_str2array");

	wtk_lua2_link_function(lua2,wtk_lua_json_get,"wtk_json_get");
	wtk_lua2_link_function(lua2,wtk_lua_json_set,"wtk_json_set");
}
