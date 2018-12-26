#include "wtk_cache.h"
#include "wtk/core/wtk_os.h"
#include "third/json/cJSON.h"
#define qf_s(d,k,v) qf(d,k,sizeof(k)-1,v)
#define sqlite3_prepare_s(db,s,stmt) sqlite3_prepare(db,s,sizeof(s)-1,stmt,0)
int wtk_cache_process_set(wtk_cache_t *c,void *data,wtk_cache_param_query_f qf,wtk_string_t *v);
int wtk_cache_process_get(wtk_cache_t *c,void *data,wtk_cache_param_query_f qf,wtk_string_t *v);
int wtk_cache_process_show(wtk_cache_t *c,void *data,wtk_cache_param_query_f qf,wtk_string_t *v);
int wtk_cache_process_del(wtk_cache_t *c,void *data,wtk_cache_param_query_f qf,wtk_string_t *v);
int wtk_cache_process_clear(wtk_cache_t *c,void *data,wtk_cache_param_query_f qf,wtk_string_t *v);
wtk_string_t* wtk_cache_hash_add(wtk_cache_t *c,char *k,int kl,char *v,int vl);
void wtk_cache_reset(wtk_cache_t *c);
void wtk_cache_httpc_add_port(wtk_cache_t *c,wtk_strbuf_t *buf,int add_and);
//#define DEBUG_CACHE

int wtk_cache_sqlite_table_exist(wtk_cache_t *c,wtk_sqlite_t *s)
{
	sqlite3_stmt* stmt=0;
	sqlite3* db=s->db;
	int exist=0;
	int ret;

	ret=sqlite3_prepare_s(db,"SELECT name FROM sqlite_master where name=\"cache\"",&stmt);
	if(ret!=SQLITE_OK){goto end;}
	ret=sqlite3_step(stmt);
	if(ret!=SQLITE_ROW){goto end;}
	exist=1;
end:
	if(stmt)
	{
		sqlite3_finalize(stmt);
	}
	return exist;
}

int wtk_cache_create_sqlite(wtk_cache_t *c,wtk_sqlite_t *s)
{
	sqlite3_stmt* stmt=0;
	sqlite3* db=s->db;
	int ret=-1;

	ret=sqlite3_prepare_s(db,"CREATE  TABLE \"main\".\"cache\" (\"key\" VARCHAR PRIMARY KEY  NOT NULL , \"value\" TEXT)",&stmt);
	if(ret!=SQLITE_OK){goto end;}
	ret=sqlite3_step(stmt);
	if(ret!=SQLITE_DONE){goto end;}
    ret=0;
end:
	if(stmt)
	{
		sqlite3_finalize(stmt);
	}
	return ret;
}

void wtk_cache_add_cmd_handler(wtk_cache_t *c)
{
	wtk_str_hash_t *hash=c->cmd_hash;

	c->del=0;
	c->set=0;
	c->clear=0;
	c->hook=0;
	wtk_str_hash_add_s(hash,"set",wtk_cache_process_set);
	wtk_str_hash_add_s(hash,"get",wtk_cache_process_get);
	wtk_str_hash_add_s(hash,"show",wtk_cache_process_show);
	wtk_str_hash_add_s(hash,"del",wtk_cache_process_del);
	//wtk_str_hash_add_s(hash,"clear",wtk_cache_process_clear);
}

wtk_cache_t* wtk_cache_new(wtk_cache_cfg_t *cfg)
{
	wtk_cache_t *c;
	char *fn;
	int ret=-1;

	c=(wtk_cache_t*)wtk_calloc(1,sizeof(*c));
	c->cfg=cfg;
	wtk_queue_init(&(c->kv_link_queue));
	if(cfg->use_httpc)
	{
		c->httpc=wtk_httpc_new(&(cfg->httpc));
	}else
	{
		c->httpc=0;
	}
	c->cmd_hash=wtk_str_hash_new(7);
	c->hash=wtk_str_hash_new(cfg->nslot);
	wtk_cache_add_cmd_handler(c);
	if(cfg->use_db)
	{
		fn=cfg->db_fn.data;
		c->sqlite=wtk_sqlite_new(fn);
		if(!c->sqlite){goto end;}
		if(wtk_cache_sqlite_table_exist(c,c->sqlite)==0)
		{
			ret=wtk_cache_create_sqlite(c,c->sqlite);
			if(ret!=0){goto end;}
		}
	}
	ret=0;
end:
	if(ret!=0)
	{
		wtk_cache_delete(c);
		c=0;
	}
	return c;
}

int wtk_cache_bytes(wtk_cache_t *c)
{
	int b;

	b=sizeof(*c)+wtk_str_hash_bytes(c->cmd_hash)+wtk_str_hash_bytes(c->hash);
	return b;
}

int wtk_cache_delete_kv(wtk_cache_t *c,hash_str_node_t *n)
{
	wtk_kv_t *kv=(wtk_kv_t*)n->value;

	wtk_kv_delete(kv);
	return 0;
}

void wtk_cache_kvqueue_clear(wtk_cache_t *c)
{
	wtk_queue_node_t *n;
	wtk_kv_t *kv;

	while(1)
	{
		n=wtk_queue_pop(&(c->kv_link_queue));
		if(!n){break;}
		kv=data_offset(n,wtk_kv_t,link_n);
		wtk_kv_delete(kv);
	}
}

int wtk_cache_delete(wtk_cache_t *c)
{
	wtk_str_hash_delete(c->cmd_hash);
	if(c->hash)
	{
		//wtk_str_hash_walk(c->hash,(wtk_walk_handler_t)wtk_cache_delete_kv,c);
		wtk_str_hash_delete(c->hash);
	}
	wtk_cache_kvqueue_clear(c);
	if(c->sqlite)
	{
		wtk_sqlite_delete(c->sqlite);
	}
	if(c->httpc)
	{
		wtk_httpc_delete(c->httpc);
	}
	wtk_free(c);
	return 0;
}

void wtk_cache_reset(wtk_cache_t *c)
{
	//wtk_str_hash_walk(c->hash,(wtk_walk_handler_t)wtk_cache_delete_kv,c);
	wtk_str_hash_reset(c->hash);
	wtk_cache_kvqueue_clear(c);
}

wtk_string_t* wtk_cache_hash_get(wtk_cache_t *c,char *k,int kl)
{
	wtk_kv_t *kv;
	wtk_queue_node_t *n;

	kv=(wtk_kv_t*)wtk_str_hash_find(c->hash,k,kl);
	if(kv)
	{
		//move the most active kv to the tailer.
		n=&(kv->link_n);
		wtk_queue_remove(&(c->kv_link_queue),n);
		wtk_queue_push(&(c->kv_link_queue),n);
	}
	return kv?kv->v:0;
}

void wtk_cache_kv_del(wtk_cache_t *c,wtk_kv_t *kv)
{
	wtk_queue_remove(&(c->kv_link_queue),&(kv->link_n));
	wtk_str_hash_remove(c->hash,kv->k->data,kv->k->len);
	wtk_kv_delete(kv);
	return;
}

void wtk_cache_kv_remove_oldest(wtk_cache_t *c)
{
	wtk_queue_node_t *n;
	wtk_kv_t *kv;

	n=c->kv_link_queue.pop;
	if(!n){goto end;}
	kv=data_offset(n,wtk_kv_t,link_n);
	wtk_cache_kv_del(c,kv);
end:
	return;
}


void wtk_cache_hash_del(wtk_cache_t *c,char *k,int kl)
{
	wtk_kv_t *kv;

	kv=(wtk_kv_t*)wtk_str_hash_find(c->hash,k,kl);
	if(!kv){goto end;}
	wtk_cache_kv_del(c,kv);
end:
	return;
}

wtk_string_t* wtk_cache_hash_add(wtk_cache_t *c,char *k,int kl,char *v,int vl)
{
	wtk_kv_t *kv;

	if(c->kv_link_queue.length>=c->cfg->max_active_slot)
	{
		wtk_cache_kv_remove_oldest(c);
	}
	kv=wtk_kv_new(k,kl,v,vl);
	wtk_str_hash_add_node(c->hash,kv->k->data,kv->k->len,kv,&(kv->hash_n));
	wtk_queue_push(&(c->kv_link_queue),&(kv->link_n));
	return kv->v;
}

int wtk_cache_sqlite_get(wtk_cache_t *c,char *k,int kl,wtk_string_t *v)
{
	int ret;

	ret=wtk_sqlite_select_str1_s(c->sqlite,"select value from cache where key=?",k,kl);
	if(ret!=0|| !v){goto end;}
	v->data=c->sqlite->buf->data;
	if(c->sqlite->buf->pos>0)
	{
		v->len=c->sqlite->buf->pos-1;
	}else
	{
		v->len=0;
	}
end:
	return ret;
}

int wtk_cache_httpc_get(wtk_cache_t* c,char *k,int kl,wtk_string_t *v)
{
	wtk_httpc_t* httpc=c->httpc;
	wtk_strbuf_t *buf=httpc->buf;
	int ret;

	wtk_strbuf_reset(buf);
	wtk_http_url_encode_kv_s(buf,"key",k,kl);
	wtk_strbuf_push_s(buf,"&cmd=get");
	wtk_cache_httpc_add_port(c,buf,1);
	ret=wtk_httpc_req(httpc,buf->data,buf->pos);
	//wtk_http_response_print(httpc->req->response);
	if( ret==0 &&(httpc->req->response->status==200) && httpc->req->response->body)
	{
		wtk_string_set(v,httpc->req->response->body->data,httpc->req->response->body->pos);
	}else
	{
		ret=-1;
		wtk_string_set(v,0,0);
	}
	return ret;
}

int wtk_cache_httpc_show(wtk_cache_t* c,wtk_string_t *v)
{
	wtk_httpc_t* httpc=c->httpc;
	wtk_strbuf_t *buf=httpc->buf;
	int ret;

	wtk_strbuf_reset(buf);
	wtk_strbuf_push_s(buf,"cmd=show");
	ret=wtk_httpc_req(httpc,buf->data,buf->pos);
	//wtk_http_response_print(httpc->req->response);
	if(ret==0 && httpc->req->response->body)
	{
		wtk_string_set(v,httpc->req->response->body->data,httpc->req->response->body->pos);
	}else
	{
		wtk_string_set(v,0,0);
	}
	return ret;
}

int wtk_cache_httpc_set(wtk_cache_t *c,char *k,int kl,char *v,int vl)
{
	wtk_httpc_t *httpc=c->httpc;
	wtk_strbuf_t *buf=httpc->buf;
	int ret=0;
	int i;

	wtk_strbuf_reset(buf);
	wtk_strbuf_push_s(buf,"cmd=set&");
	wtk_http_url_encode_kv_s(buf,"key",k,kl);
	wtk_strbuf_push_s(buf,"&");
	wtk_http_url_encode_kv_s(buf,"value",v,vl);
	wtk_cache_httpc_add_port(c,buf,1);
	for(i=0;i<c->cfg->httpc_retry;++i)
	{
		ret=wtk_httpc_post(httpc,buf->data,buf->pos);
		if(ret==0)
		{
			break;
		}
	}
	//wtk_debug("ret=%d\n",ret);
	//wtk_http_response_print(httpc->req->response);
	return ret;
}

int wtk_cache_sqlite_set(wtk_cache_t *c,char *k,int kl,char *v,int vl)
{
	wtk_sqlite_t *s=c->sqlite;
	sqlite3_stmt* stmt=0;
	sqlite3* db=s->db;
	int ret;

	ret=wtk_cache_sqlite_get(c,k,kl,0);
	if(ret==0)
	{
		ret=sqlite3_prepare_s(db,"update cache set value=? where key=?",&stmt);
	}else
	{
		ret=sqlite3_prepare_s(db,"insert into cache(value,key) values(?,?)",&stmt);
	}
	if(ret!=SQLITE_OK){ret=-1;goto end;}
	ret=sqlite3_bind_text(stmt,1,v,vl,0);
	if(ret!=SQLITE_OK){ret=-1;goto end;}
	ret=sqlite3_bind_text(stmt,2,k,kl,0);
	if(ret!=SQLITE_OK){ret=-1;goto end;}
	ret=sqlite3_step(stmt);
	if(ret!=SQLITE_ROW && ret!=SQLITE_DONE){ret=-1;goto end;}
    ret=0;
end:
	if(stmt)
	{
		sqlite3_finalize(stmt);
	}
	if(ret!=0)
	{
		wtk_debug("save db failed.\n");
		wtk_sqlite_print(s);
	}
	return ret;
}

int wtk_cache_sqlite_del(wtk_cache_t *c,char *k,int kl)
{
	return wtk_sqlite_exe2_s(c->sqlite,"delete from cache where key=?",k,kl);
}

void wtk_cache_httpc_add_port(wtk_cache_t *c,wtk_strbuf_t *buf,int add_and)
{
	if(c->cfg->http_port.len>0)
	{
		if(add_and)
		{
			wtk_strbuf_push_s(buf,"&");
		}
		wtk_http_url_encode_kv_s(buf,"port",c->cfg->http_port.data,c->cfg->http_port.len);
	}
}

int wtk_cache_httpc_del(wtk_cache_t *c,char *k,int kl)
{
	wtk_httpc_t *httpc=c->httpc;
	wtk_strbuf_t *buf=httpc->buf;
	int ret;

	wtk_strbuf_reset(buf);
	wtk_strbuf_push_s(buf,"cmd=del&");
	wtk_http_url_encode_kv_s(buf,"key",k,kl);
	wtk_cache_httpc_add_port(c,buf,1);
	ret=wtk_httpc_post(httpc,buf->data,buf->pos);
	//wtk_http_response_print(httpc->req->response);
	return ret;
}

int wtk_cache_del(wtk_cache_t *c,char *k,int kl,int update_httpc)
{
	int ret=0;

	wtk_cache_hash_del(c,k,kl);
	if(c->sqlite)
	{
		ret=wtk_cache_sqlite_del(c,k,kl);
		if(ret!=0){goto end;}
	}
	if(update_httpc && c->httpc)
	{
		ret=wtk_cache_httpc_del(c,k,kl);
		if(ret!=0){goto end;}
	}
	if(c->del)
	{
		ret=c->del(c->hook,k,kl);
		if(ret!=0){goto end;}
	}
end:
	return ret;
}

int wtk_cache_sqlite_clear(wtk_cache_t *c)
{
	return wtk_sqlite_exe_s(c->sqlite,"delete from cache");
}

int wtk_cache_httpc_clear(wtk_cache_t *c)
{
	wtk_httpc_t *httpc=c->httpc;
	wtk_strbuf_t *buf=httpc->buf;
	int ret;

	wtk_strbuf_reset(buf);
	wtk_strbuf_push_s(buf,"cmd=clear");
	wtk_cache_httpc_add_port(c,buf,1);
	ret=wtk_httpc_post(httpc,buf->data,buf->pos);
	//wtk_http_response_print(httpc->req->response);
	return ret;
}

int wtk_cache_clear(wtk_cache_t *c,int update_httpc)
{
	int ret=0;

	wtk_cache_reset(c);
	if(c->sqlite)
	{
		ret=wtk_cache_sqlite_clear(c);
		if(ret!=0){goto end;}
	}
	if(update_httpc && c->httpc)
	{
		ret=wtk_cache_httpc_clear(c);
		if(ret!=0){goto end;}
	}
	if(c->clear)
	{
		ret=c->clear(c->hook);
		if(ret!=0){goto end;}
	}
end:
	return ret;
}

int wtk_cache_set(wtk_cache_t *c,char *k,int kl,char *v,int vl,int update_httpc)
{
	int ret=0;

	//print_data(k,kl);
	wtk_cache_hash_del(c,k,kl);
	wtk_cache_hash_add(c,k,kl,v,vl);
	if(c->sqlite)
	{
		ret=wtk_cache_sqlite_set(c,k,kl,v,vl);
#ifdef DEBUG_CACHE
		wtk_debug("sqlite set[%*.*s]: %d(%.*s)\n",kl,kl,k,ret,vl,v);
#endif
		if(ret!=0){goto end;}
	}
	if(update_httpc && c->httpc)
	{
		ret=wtk_cache_httpc_set(c,k,kl,v,vl);
#ifdef DEBUG_CACHE
		wtk_debug("httpc set[%*.*s]: %d,(%.*s)\n",kl,kl,k,ret,vl,v);
#endif
		if(ret!=0){goto end;}
	}
	if(c->set)
	{
		ret=c->set(c->hook,k,kl,v,vl);
		if(ret!=0){goto end;}
	}
end:
	return ret;
}


wtk_string_t* wtk_cache_get(wtk_cache_t *c,char *k,int kl)
{
	wtk_string_t x;
	wtk_string_t *v;
	int ret;

	//print_data(k,kl);
	v=wtk_cache_hash_get(c,k,kl);
	if(v){goto end;}
	wtk_string_set(&x,0,0);
	if(c->sqlite)
	{
		ret=wtk_cache_sqlite_get(c,k,kl,&x);
#ifdef DEBUG_CACHE
		wtk_debug("sqlite get[%*.*s]: len=%d,ret=%d,(%.*s)\n",kl,kl,k,x.len,ret,x.len,x.data);
#endif
		if(ret==0)
		{
#ifdef WIN32
            v=wtk_cache_hash_add(c,k,kl,x.data,x.len);
#endif
			//v=wtk_cache_hash_add(c,k,kl,x.data,x.len);
			goto end;
		}
	}
	if(c->httpc)
	{
		ret=wtk_cache_httpc_get(c,k,kl,&x);
#ifdef DEBUG_CACHE
		wtk_debug("httpc get[%*.*s]: len=%d,ret=%d,(%.*s)\n",kl,kl,k,x.len,ret,x.len,x.data);
#endif
		if(ret!=0){goto end;}
	}else
	{
		goto end;
	}
	v=wtk_cache_hash_add(c,k,kl,x.data,x.len);
end:
	return v;
}

int wtk_cache_sqlite_show(wtk_cache_t *c,wtk_string_t *value)
{
	wtk_sqlite_t *s=c->sqlite;
	wtk_strbuf_t *buf;
	sqlite3_stmt* stmt=0;
	sqlite3* db=s->db;
	int ret;
	char *k,*v;
	cJSON *json=0;

	ret=sqlite3_prepare_s(db,"select key,value from cache",&stmt);
	if(ret!=SQLITE_OK){ret=-1;goto end;}
	json=cJSON_CreateObject();
	while(1)
	{
		ret=sqlite3_step(stmt);
		//wtk_debug("%d\n",ret);
		if(ret!=SQLITE_ROW)
		{
			break;
		}
		//len=sqlite3_column_bytes(stmt,0);
		k=(char*)sqlite3_column_text(stmt,0);
		v=(char*)sqlite3_column_text(stmt,1);
		cJSON_AddStringToObject(json,k,v);
	}
	v=cJSON_PrintUnformatted(json);
	if(v)
	{
		buf=c->sqlite->buf;
		wtk_strbuf_reset(buf);
		wtk_strbuf_push(buf,v,strlen(v));
		wtk_string_set(value,buf->data,buf->pos);
		free(v);
	}else
	{
		wtk_string_set(value,0,0);
	}
	cJSON_Delete(json);
	ret=ret==SQLITE_DONE?0:-1;
end:
	if(stmt)
	{
		sqlite3_finalize(stmt);
	}
	return ret;
}

int wtk_cache_show(wtk_cache_t *c,wtk_string_t *value)
{
	if(c->sqlite)
	{
		return wtk_cache_sqlite_show(c,value);
	}
	if(c->httpc)
	{
		return wtk_cache_httpc_show(c,value);
	}
	return -1;
}


int wtk_cache_has_update(wtk_cache_t *c,void *data,wtk_cache_param_query_f qf)
{
	wtk_string_t v;
	int ret;
	int update_httpc=1;

	ret=qf_s(data,"update",&v);
	if(ret!=0){goto end;}
	if(wtk_string_cmp_s(&v,"False")==0)
	{
		update_httpc=0;
	}
end:
	return update_httpc;
}


int wtk_cache_process_set(wtk_cache_t *c,void *data,wtk_cache_param_query_f qf,wtk_string_t *v)
{
	wtk_string_t key;
	wtk_string_t value;
	int ret,update;

	ret=qf_s(data,"key",&key);
	if(ret!=0){goto end;}
	ret=qf_s(data,"value",&value);
	if(ret!=0){goto end;}
	update=wtk_cache_has_update(c,data,qf);
	ret=wtk_cache_set(c,key.data,key.len,value.data,value.len,update);
	if(ret!=0){goto end;}
	wtk_string_set_s(v,"True");
end:
	if(ret!=0)
	{
		wtk_string_set_s(v,"False");
	}
	return ret;
}

int wtk_cache_process_del(wtk_cache_t *c,void *data,wtk_cache_param_query_f qf,wtk_string_t *v)
{
	wtk_string_t key;
	int ret;
	int update;

	ret=qf_s(data,"key",&key);
	if(ret!=0){goto end;}
	update=wtk_cache_has_update(c,data,qf);
	ret=wtk_cache_del(c,key.data,key.len,update);
	if(ret!=0){goto end;}
	wtk_string_set_s(v,"True");
end:
	if(ret!=0)
	{
		wtk_string_set_s(v,"False");
	}
	return ret;
}

int wtk_cache_process_get(wtk_cache_t *c,void *data,wtk_cache_param_query_f qf,wtk_string_t *v)
{
	wtk_string_t key;
	wtk_string_t *x;
	int ret;

	ret=qf_s(data,"key",&key);
	if(ret!=0){goto end;}
	x=wtk_cache_get(c,key.data,key.len);
	//wtk_debug("x=%p\n",x);
	if(!x){ret=-1;goto end;}
	//wtk_debug("get:[%.*s]\n",x->len,x->data);
	*v=*x;
end:
	return ret;
}

int wtk_cache_process_show(wtk_cache_t *c,void *data,wtk_cache_param_query_f qf,wtk_string_t *v)
{
	return wtk_cache_show(c,v);
}

int wtk_cache_process_clear(wtk_cache_t *c,void *data,wtk_cache_param_query_f qf,wtk_string_t *v)
{
	int ret;
	int update;

	update=wtk_cache_has_update(c,data,qf);
	ret=wtk_cache_clear(c,update);
	if(ret==0)
	{
		wtk_string_set_s(v,"True");
	}
	return ret;
}

int wtk_cache_process(wtk_cache_t *c,void *data,wtk_cache_param_query_f qf,wtk_string_t *v)
{
	wtk_string_t cmd;
	wtk_cache_process_f pf;
	int ret;

	ret=qf_s(data,"cmd",&cmd);
	if(ret!=0){goto end;}
	//print_data(cmd.data,cmd.len);
	pf=(wtk_cache_process_f)wtk_str_hash_find(c->cmd_hash,cmd.data,cmd.len);
	if(!pf){ret=-1;goto end;}
	ret=pf(c,data,qf,v);
	if(ret!=0)
	{
		if(wtk_string_cmp_s(&(cmd),"get")==0)
		{
			wtk_string_set(v,0,0);
		}else
		{
			wtk_string_set_s(v,"False");
		}
	}
	ret=0;
end:
	//if(ret!=0){wtk_sqlite_print(c->sqlite);}
	return ret;
}
