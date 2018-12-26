#include "wtk_sqlite.h"
#ifdef USE_SQL

wtk_sqlite_t* wtk_sqlite_new(char *fn)
{
	wtk_sqlite_t *s;
	int ret;

	//wtk_debug("fn=%s\n",fn);
	s=calloc(1,sizeof(*s));
	ret=wtk_sqlite_init(s,fn);
	if(ret!=0)
	{
		wtk_sqlite_delete(s);
		s=0;
	}
	return s;
}

int wtk_sqlite_delete(wtk_sqlite_t *s)
{
	wtk_sqlite_clean(s);
	free(s);
	return 0;
}

int wtk_sqlite_set_opt(wtk_sqlite_t *s)
{
	//sqlite3 *db=s->db;
	int ret;

	ret=wtk_sqlite_exe_s(s,"PRAGMA cache_size =8192;PRAGMA temp_store = MEMORY;");
	//wtk_debug("ret=%d\n",ret);
	return ret;
}

int wtk_sqlite_init(wtk_sqlite_t *s,char *fn)
{
	int ret;
//#define WIN32
#ifdef WIN32
	char *p;
#endif

	s->db=0;
	s->buf=wtk_strbuf_new(4096,1);
#ifdef WIN32
	fn=p=gbk_to_utf8(fn);
#endif
	ret=sqlite3_open(fn,&(s->db));
	if(ret!=SQLITE_OK){ret=-1;goto end;};
	//wtk_sqlite_set_opt(s);
end:
#ifdef WIN32
	if(p){wtk_free(p);}
#endif
	return ret;
}

int wtk_sqlite_clean(wtk_sqlite_t *s)
{
	if(s->db)
	{
		sqlite3_close(s->db);
	}
	wtk_strbuf_delete(s->buf);
	return 0;
}

int wtk_sqlite_select_str1(wtk_sqlite_t* s,char *sql,int sql_len,char *p,int p_len)
{
	sqlite3_stmt* stmt=0;
	sqlite3* db=s->db;
	int ret;
	char *data;
	int len;

	ret=sqlite3_prepare(db,sql,sql_len,&stmt,0);
	if(ret!=SQLITE_OK){ret=-1;goto end;}
	if(p_len>0)
	{
		ret=sqlite3_bind_text(stmt,1,p,p_len,0);
		if(ret!=SQLITE_OK){ret=-1;goto end;}
	}
	ret=sqlite3_step(stmt);
	if(ret!=SQLITE_ROW){ret=-1;goto end;}
	len=sqlite3_column_bytes(stmt,0);
    data=(char*)sqlite3_column_text(stmt,0);
    wtk_strbuf_reset(s->buf);
    wtk_strbuf_push(s->buf,data,len);
    wtk_strbuf_push_c(s->buf,0);
    ret=0;
end:
	if(stmt)
	{
		sqlite3_finalize(stmt);
	}
	return ret;
}

int wtk_sqlite_select_str(wtk_sqlite_t* s,wtk_array_t *a,char *sql,int sql_len)
{
	sqlite3_stmt* stmt=0;
	sqlite3* db=s->db;
	wtk_string_t **str;
	int ret;
	char *data;
	int len;

	wtk_array_reset(a);
	ret=sqlite3_prepare(db,sql,sql_len,&stmt,0);
	if(ret!=SQLITE_OK){ret=-1;goto end;}
	while(1)
	{
		ret=sqlite3_step(stmt);
		//wtk_debug("%d\n",ret);
		if(ret!=SQLITE_ROW)
		{
			break;
		}
		len=sqlite3_column_bytes(stmt,0);
		data=(char*)sqlite3_column_text(stmt,0);
		str=(wtk_string_t **)wtk_array_push(a);
		str[0]=wtk_heap_dup_string(a->heap,data,len);
		//wtk_strbuf_reset(s->buf);
		//wtk_strbuf_push(s->buf,data,len);
	}
	ret=ret==SQLITE_DONE?0:-1;
end:
	if(stmt)
	{
		sqlite3_finalize(stmt);
	}
	return ret;
}

int wtk_sqlite_exe(wtk_sqlite_t* s,char *sql,int sql_len)
{
	sqlite3_stmt* stmt=0;
	sqlite3* db=s->db;
	int ret;

	//print_data(sql,sql_len);
	ret=sqlite3_prepare(db,sql,sql_len,&stmt,0);
	if(ret!=SQLITE_OK){ret=-1;goto end;}
	ret=sqlite3_step(stmt);
	ret=ret==SQLITE_DONE?0:-1;
end:
	if(stmt)
	{
		sqlite3_finalize(stmt);
	}
	//wtk_sqlite_print(s);
	return ret;
}

int wtk_sqlite_exe2(wtk_sqlite_t* s,char *sql,int sql_len,char *p,int p_len)
{
	sqlite3_stmt* stmt=0;
	sqlite3* db=s->db;
	int ret;

	ret=sqlite3_prepare(db,sql,sql_len,&stmt,0);
	if(ret!=SQLITE_OK){ret=-1;goto end;}
	ret=sqlite3_bind_text(stmt,1,p,p_len,0);
	if(ret!=SQLITE_OK){ret=-1;goto end;}
	ret=sqlite3_step(stmt);
	ret=ret==SQLITE_DONE?0:-1;
end:
	if(stmt)
	{
		sqlite3_finalize(stmt);
	}
	return ret;
}

int wtk_sqlite_exe3(wtk_sqlite_t *s,wtk_string_t *sql,void *ths,wtk_sqlite_add_param_f add_param,wtk_sqlite_notify_value_f notify)
{
	sqlite3_stmt* stmt=0;
	sqlite3* db=s->db;
	sqlite3_value *v;
	int ret;
	int i,ncol,index;

	//wtk_debug("%.*s\n",sql->len,sql->data);
	ret=sqlite3_prepare(db,sql->data,sql->len,&stmt,0);
	if(ret!=SQLITE_OK)
	{
		//wtk_debug("ret=%d\n",ret);
		wtk_debug("prepare[%.*s],ret=%d failed.\n",sql->len,sql->data,ret);
		ret=-1;goto end;
	}
	if(add_param)
	{
		ret=add_param(ths,stmt);
		if(ret!=0){goto end;}
	}
	index=0;
	while(1)
	{
		ret=sqlite3_step(stmt);
		//wtk_debug("%d\n",ret);
		if(ret!=SQLITE_ROW)
		{
			break;
		}
		ncol=sqlite3_column_count(stmt);
		for(i=0;i<ncol;++i)
		{
			v=sqlite3_column_value(stmt,i);
			if(v && notify)
			{
				notify(ths,index,i,v);
			}
		}
	}
	ret=ret==SQLITE_DONE?0:-1;
end:
	if(stmt)
	{
		sqlite3_finalize(stmt);
	}
	return ret;
}

int wtk_sqlite_exe4(wtk_sqlite_t *s,wtk_string_t *sql,void *ths,wtk_sqlite_add_param_f add_param,wtk_sqlite_notify_value2_f notify)
{
	sqlite3_stmt* stmt=0;
	sqlite3* db=s->db;
	sqlite3_value *v;
	int ret;
	int i,ncol,index;

	//wtk_debug("%.*s\n",sql->len,sql->data);
	ret=sqlite3_prepare(db,sql->data,sql->len,&stmt,0);
	if(ret!=SQLITE_OK)
	{
		//wtk_debug("ret=%d\n",ret);
		wtk_debug("prepare[%.*s],ret=%d failed.\n",sql->len,sql->data,ret);
		ret=-1;goto end;
	}
	if(add_param)
	{
		ret=add_param(ths,stmt);
		if(ret!=0){goto end;}
	}
	index=0;
	while(1)
	{
		ret=sqlite3_step(stmt);
		//wtk_debug("%d/%d\n",ret,SQLITE_ROW);
		if(ret!=SQLITE_ROW)
		{
			break;
		}
		ncol=sqlite3_column_count(stmt);
		for(i=0;i<ncol;++i)
		{
			v=sqlite3_column_value(stmt,i);
			if(v && notify)
			{
				if(notify(ths,index,i,v)==0)
				{
					ret=0;
					goto end;
				}
			}
		}
	}
	//wtk_debug("ret=%d\n",ret);
	//wtk_sqlite_print(s);
	ret=ret==SQLITE_DONE?0:-1;
end:
	if(stmt)
	{
		sqlite3_finalize(stmt);
	}
	//wtk_debug("ret=%d\n",ret);
	return ret;
}

int wtk_sqlite_begin_transaction(wtk_sqlite_t *s,wtk_string_t *sql)
{
	sqlite3* db=s->db;
	int ret;

	//wtk_debug("%.*s\n",sql->len,sql->data);
	s->trans_stmt=0;
	ret=sqlite3_prepare(db,sql->data,sql->len,&s->trans_stmt,0);
	if(ret!=SQLITE_OK)
	{
		//wtk_debug("ret=%d\n",ret);
		wtk_debug("prepare[%.*s],ret=%d failed.\n",sql->len,sql->data,ret);
		ret=-1;goto end;
	}
	ret=sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL,0);
	if(ret!=SQLITE_OK)
	{
		//wtk_debug("ret=%d\n",ret);
		wtk_debug("begine session [%.*s],ret=%d failed.\n",sql->len,sql->data,ret);
		ret=-1;goto end;
	}
	ret=0;
end:
	if(ret!=0 && s->trans_stmt)
	{
		sqlite3_finalize(s->trans_stmt);
		s->trans_stmt=0;
	}
	return ret;
}

int wtk_sqlite_exe_transaction(wtk_sqlite_t *s,void *ths,wtk_sqlite_add_param_f add_param,wtk_sqlite_notify_value2_f notify)
{
	sqlite3_stmt* stmt=s->trans_stmt;
	sqlite3_value *v;
	int ret;
	int i,ncol,index;

	//wtk_debug("%.*s\n",sql->len,sql->data);
	if(add_param)
	{
		ret=add_param(ths,stmt);
		if(ret!=0)
		{
			wtk_debug("add param failed\n");
			goto end;
		}
	}
	index=0;
	while(1)
	{
		ret=sqlite3_step(stmt);
		//wtk_debug("%d\n",ret);
		if(ret!=SQLITE_ROW)
		{
			break;
		}
		ncol=sqlite3_column_count(stmt);
		for(i=0;i<ncol;++i)
		{
			v=sqlite3_column_value(stmt,i);
			if(v && notify)
			{
				if(notify(ths,index,i,v)==0)
				{
					ret=0;
					goto end;
				}
			}
		}
	}
	wtk_debug("ret=%d\n",ret);
	ret=ret==SQLITE_DONE?0:-1;
end:
	sqlite3_clear_bindings(stmt);
	sqlite3_reset(stmt);
	wtk_debug("ret=%d\n",ret);
	return ret;
}

int wtk_sqlite_end_transaction(wtk_sqlite_t *s)
{
	sqlite3* db=s->db;

	sqlite3_exec(db, "END TRANSACTION", NULL, NULL,0);
	if(s->trans_stmt)
	{
		sqlite3_finalize(s->trans_stmt);
		s->trans_stmt=0;
	}
	return 0;
}


//--------------------- example section --------------------
int wtk_sqlite_add_param_test(void *ths,sqlite3_stmt *stmt)
{
	int ret;

	char *p="hello";
	ret=sqlite3_bind_text(stmt,1,p,strlen(p),0);
	if(ret!=SQLITE_OK){ret=-1;goto end;}
end:
	return ret;
}

void wtk_sqlite_add_param_notify_test(void *ths,int index,int col,sqlite3_value *v)
{
	int type;
	char *p;
	int n;

	wtk_debug("[%d,%d]\n",index,col);
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
}


void wtk_sqlite_print(wtk_sqlite_t *s)
{
	const char *msg;

	msg=sqlite3_errmsg(s->db);
	wtk_debug("err: %s\n",msg?msg:"succeed");
}
#endif
