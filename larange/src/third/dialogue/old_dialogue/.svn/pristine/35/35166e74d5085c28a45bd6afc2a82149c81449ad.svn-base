#include "wtk_faqdb.h" 

void wtk_faqdb_init_db(wtk_faqdb_t *db)
{
	char *sql="CREATE TABLE \"faq\" (\"q\" TEXT PRIMARY KEY  NOT NULL UNIQUE, \"a\" TEXT)";
	//CREATE  TABLE "main"."faq" ("q" TEXT PRIMARY KEY  NOT NULL  UNIQUE , "a" TEXT, "v" BLOB)

	//wtk_debug("%s\n",db->cfg->db);
	if(wtk_file_exist(db->cfg->db)!=0)
	{
		//wtk_debug("create sql\n");
		db->sqlite=wtk_sqlite_new(db->cfg->db);
		wtk_sqlite_exe(db->sqlite,sql,strlen(sql));
	}else
	{
		db->sqlite=wtk_sqlite_new(db->cfg->db);
	}
}

wtk_faqdb_t* wtk_faqdb_new(wtk_faqdb_cfg_t *cfg)
{
	wtk_faqdb_t *db;

	db=(wtk_faqdb_t*)wtk_malloc(sizeof(wtk_faqdb_t));
	db->cfg=cfg;
	db->buf=wtk_strbuf_new(256,1);
	db->buf2=wtk_strbuf_new(256,1);
	wtk_faqdb_init_db(db);
	return db;
}

void wtk_faqdb_delete(wtk_faqdb_t *db)
{
	wtk_strbuf_delete(db->buf2);
	wtk_strbuf_delete(db->buf);
	if(db->sqlite)
	{
		//wtk_debug("delete sqlite\n");
		wtk_sqlite_delete(db->sqlite);
	}
	wtk_free(db);
}

int wtk_faqdb_bind_search_query(void *ths,sqlite3_stmt *stmt)
{
	wtk_string_t *v;
	int ret;

	v=((wtk_string_t**)ths)[1];
	//wtk_debug("[%.*s]\n",v->len,v->data);
	ret=sqlite3_bind_text(stmt,1,v->data,v->len,0);
	if(ret!=SQLITE_OK){ret=-1;goto end;}
end:
	return ret;
}

int wtk_faqdb_bind_search_notify(void *ths,int index,int col,sqlite3_value *v)
{
	int type;
	char *p;
	int n;
	wtk_faqdb_t *db;

	db=((wtk_faqdb_t**)ths)[0];
	//wtk_debug("[%d,%d]\n",index,col);
	type=sqlite3_value_type(v);
	//wtk_debug("type=%d\n",type);
	switch(type)
	{
	case SQLITE_INTEGER:
		wtk_debug("%d\n",sqlite3_value_int(v));
		break;
	case SQLITE_FLOAT:
		wtk_debug("%f\n",sqlite3_value_double(v));
		break;
	case SQLITE_BLOB:
		p=(char*)sqlite3_value_blob(v);
		n=sqlite3_value_bytes(v);
		wtk_debug("[%.*s]\n",n,p);
		break;
	case SQLITE_NULL:
		break;
	case SQLITE_TEXT:
		p=(char*)sqlite3_value_text(v);
		//wtk_debug("[%s]=%d\n",p,(int)strlen(p));
		wtk_strbuf_reset(db->buf2);
		wtk_strbuf_push(db->buf2,p,(int)strlen(p));
		break;
	default:
		break;
	}
	return 0;
}

wtk_string_t wtk_faqdb_search(wtk_faqdb_t *db,char *q,int q_bytes)
{
	wtk_string_t sql=wtk_string("select a from faq where q=?");
	wtk_string_t t;
	void* p[2];

	wtk_string_set(&(t),q,q_bytes);
	p[0]=db;
	p[1]=&t;
	wtk_strbuf_reset(db->buf2);
	//wtk_debug("search %s\n",sql.data);
	wtk_sqlite_exe4(db->sqlite,&sql,p,(wtk_sqlite_add_param_f)wtk_faqdb_bind_search_query,
			(wtk_sqlite_notify_value2_f)wtk_faqdb_bind_search_notify);
	wtk_string_set(&(t),db->buf2->data,db->buf2->pos);
	return t;
}

int wtk_faqdb_bind_add_query(void *ths,sqlite3_stmt *stmt)
{
	wtk_string_t **p;
	wtk_string_t *q;
	wtk_string_t *a;
	int ret;

	p=(wtk_string_t**)ths;
	q=p[1];
	a=p[2];
	//wtk_debug("[%.*s]\n",q->len,q->data);
	//wtk_debug("[%.*s]\n",a->len,a->data);
	ret=sqlite3_bind_text(stmt,1,a->data,a->len,NULL);
	if(ret!=SQLITE_OK){ret=-1;goto end;}
	ret=sqlite3_bind_text(stmt,2,q->data,q->len,NULL);
	if(ret!=SQLITE_OK){ret=-1;goto end;}
	ret=0;
end:
	//wtk_debug("bind query ret=%d\n",ret);
	return ret;
}

int wtk_faqdb_bind_add_notify(void *ths,int index,int col,sqlite3_value *v)
{
	int type;
	char *p;
	int n;
	//wtk_faqdb_t *db;

	//db=((wtk_faqdb_t**)ths)[0];
	//wtk_debug("[%d,%d]\n",index,col);
	type=sqlite3_value_type(v);
	wtk_debug("type=%d\n",type);
	switch(type)
	{
	case SQLITE_INTEGER:
		wtk_debug("%d\n",sqlite3_value_int(v));
		break;
	case SQLITE_FLOAT:
		wtk_debug("%f\n",sqlite3_value_double(v));
		break;
	case SQLITE_BLOB:
		p=(char*)sqlite3_value_blob(v);
		n=sqlite3_value_bytes(v);
		wtk_debug("[%.*s]\n",n,p);
		break;
	case SQLITE_NULL:
		break;
	case SQLITE_TEXT:
		p=(char*)sqlite3_value_text(v);
		wtk_debug("[%s]=%d\n",p,(int)strlen(p));
		break;
	default:
		break;
	}
	return 0;
}

void wtk_faqdb_add(wtk_faqdb_t *db,char *q,int q_bytes,char *a,int a_bytes)
{
	wtk_string_t sql1=wtk_string("insert into faq(a,q) values(?,?)");
	wtk_string_t sql2=wtk_string("UPDATE faq set a=? where q=?");
	wtk_strbuf_t *buf=db->buf;
	wtk_string_t x1,x2,t;
	void *p[3];
	//int ret;

	//wtk_debug("[%.*s]=[%.*s]\n",q_bytes,q,a_bytes,a);
	wtk_strbuf_reset(buf);
	wtk_strbuf_push(buf,q,q_bytes);
	if(buf->pos>0)
	{
		wtk_string_set(&(x1),buf->data,buf->pos);
		wtk_string_set(&(x2),a,a_bytes);
		p[0]=db;
		p[1]=&(x1);
		p[2]=&(x2);
		t=wtk_faqdb_search(db,buf->data,buf->pos);
		//wtk_debug("[%.*s]\n",t.len,t.data);
		if(t.len==0)
		{
			wtk_sqlite_exe4(db->sqlite,&sql1,p,(wtk_sqlite_add_param_f)wtk_faqdb_bind_add_query,
					(wtk_sqlite_notify_value2_f)wtk_faqdb_bind_add_notify);
		}else
		{
			wtk_sqlite_exe4(db->sqlite,&sql2,p,(wtk_sqlite_add_param_f)wtk_faqdb_bind_add_query,
					(wtk_sqlite_notify_value2_f)wtk_faqdb_bind_add_notify);
		}
		//wtk_debug("ret=%d\n",ret);
	}
}


wtk_string_t wtk_faqdb_get(wtk_faqdb_t *db,char *q,int q_bytes)
{
	return wtk_faqdb_search(db,q,q_bytes);
}

