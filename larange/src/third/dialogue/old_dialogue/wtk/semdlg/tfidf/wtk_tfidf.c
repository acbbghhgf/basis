#include <math.h>
#include <ctype.h>
#include "wtk_tfidf.h"
#include "wtk/core/wtk_str_encode.h"

wtk_tfidf_t* wtk_tfidf_new(wtk_tfidf_cfg_t *cfg)
{
	wtk_tfidf_t *tfidf;

	tfidf=(wtk_tfidf_t*)wtk_malloc(sizeof(wtk_tfidf_t));
	tfidf->cfg=cfg;
	if(cfg->use_seg)
	{
		tfidf->seg=wtk_segmenter_new(&(cfg->segmenter),NULL);
	}else
	{
		tfidf->seg=NULL;
	}
	tfidf->json=wtk_json_parser_new();
	tfidf->json2=wtk_json_parser_new();
	tfidf->sqlite=wtk_sqlite_new(cfg->bin_fn);
	tfidf->heap=wtk_heap_new(4096);
	tfidf->buf=wtk_strbuf_new(256,1);
	tfidf->out=(wtk_tfidf_out_t*)wtk_malloc(sizeof(wtk_tfidf_out_t)*(cfg->nbest+1));
	wtk_tfidf_reset(tfidf);
	return tfidf;
}

void wtk_tfidf_delete(wtk_tfidf_t *idf)
{
	wtk_free(idf->out);
	wtk_json_parser_delete(idf->json);
	wtk_json_parser_delete(idf->json2);
	wtk_sqlite_delete(idf->sqlite);
	if(idf->seg)
	{
		wtk_segmenter_delete(idf->seg);
	}
	wtk_strbuf_delete(idf->buf);
	wtk_heap_delete(idf->heap);
	wtk_free(idf);
}

void wtk_tfidf_reset(wtk_tfidf_t *idf)
{
	idf->nout=0;
	wtk_json_parser_reset(idf->json);
	wtk_json_parser_reset(idf->json2);
	idf->n_valid_wrd=0;
	wtk_queue_init(&(idf->wrd_q));
	wtk_segmenter_reset(idf->seg);
	wtk_heap_reset(idf->heap);
}

int wtk_tfidf_is_sep(wtk_tfidf_t *idf,char c)
{
	return isspace(c);
}

wtk_tfidf_wrd_t* wtk_tfidf_find_wrd(wtk_tfidf_t * idf,char *name,int bytes,unsigned int id,int insert)
{
	wtk_tfidf_wrd_t *wrd;
	wtk_queue_node_t *qn;

	for(qn=idf->wrd_q.pop;qn;qn=qn->next)
	{
		wrd=data_offset2(qn,wtk_tfidf_wrd_t,q_n);
		if(wrd->id==id)
		{
			return wrd;
		}
	}
	if(!insert){return NULL;}
	wrd=(wtk_tfidf_wrd_t*)wtk_heap_malloc(idf->heap,sizeof(wtk_tfidf_wrd_t));
	wtk_string_set(&(wrd->wrd),name,bytes);
	wrd->id=id;
	wrd->stop=0;
	wrd->cnt=0;
	wtk_queue_push(&(idf->wrd_q),&(wrd->q_n));
	return wrd;
}


void  wtk_tfidf_update_wrd(wtk_tfidf_t *idf,char *data,int len,int index)
{
	wtk_hash_str_node_t *node;
	wtk_tfidf_wrd_t *wrd;
	int id;

	//wtk_debug("[%.*s]\n",len,data);
	node=(wtk_hash_str_node_t*)wtk_str_hash_find_node3(idf->cfg->char_map,data,len,0);
	if(!node)
	{
		//wtk_debug("[%.*s] not found\n",len,data);
		goto end;
	}
	id=node->v.u;
	wrd=wtk_tfidf_find_wrd(idf,data,len,id,1);
	++wrd->cnt;
	node=(wtk_hash_str_node_t*)wtk_str_hash_find_node3(idf->cfg->stop_map,data,len,0);
	wrd->stop=node?1:0;
	if(!wrd->stop)
	{
		++idf->n_valid_wrd;
	}
end:
	return;
}

int wtk_tfidf_add_wrd_id(wtk_tfidf_t *idf,sqlite3_stmt *stmt)
{
	wtk_tfidf_wrd_t *wrd;
	int ret;

	wrd=(wtk_tfidf_wrd_t*)idf->hook;
	ret=sqlite3_bind_int(stmt,1,wrd->id);
	if(ret!=SQLITE_OK){ret=-1;goto end;}
end:
	//wtk_debug("ret=%d\n",ret);
	return ret;
}

void wtk_tfidf_notify_wrd_map(wtk_tfidf_t *idf,int index,int col,sqlite3_value *v)
{
	int type;
	char *p;
	wtk_json_parser_t *json=idf->json;
	int ret;

	//wtk_debug("[%d,%d]\n",index,col);
	type=sqlite3_value_type(v);
	if(type==SQLITE_TEXT)
	{
		p=(char*)sqlite3_value_text(v);
		//wtk_debug("[%s]=%d\n",p,(int)strlen(p));
		ret=wtk_json_parser_parse(json,p,strlen(p));
		if(ret!=0){return;}
	}
}

int wtk_tfidf_add_qa_id(wtk_tfidf_t *idf,sqlite3_stmt *stmt)
{
	wtk_json_item_t *wrd;
	int ret;

	wrd=(wtk_json_item_t*)idf->hook;
	//wtk_debug("id=%d\n",(int)(wrd->v.number));
	ret=sqlite3_bind_int(stmt,1,(int)(wrd->v.number));
	if(ret!=SQLITE_OK){ret=-1;goto end;}
end:
	//wtk_debug("ret=%d\n",ret);
	return ret;
}

void wtk_tfidf_notify_qa_mat(wtk_tfidf_t *idf,int index,int col,sqlite3_value *v)
{
	int type;
	char *p;
	wtk_json_parser_t *json=idf->json2;
	int ret;
	wtk_tfidf_wrd_t *wrd;
	wtk_queue_node_t *qn;
	wtk_json_item_t *item,*vi;
	char buf[64];
	int n;
	double f,f2;
	wtk_json_item_t *vti;

	type=sqlite3_value_type(v);
	if(type==SQLITE_TEXT)
	{
		p=(char*)sqlite3_value_text(v);
		//wtk_debug("[%s]=%d\n",p,(int)strlen(p));
		if(col!=0)
		{
			//wtk_debug("[%s]\n",p);
			return;
		}
		vti=(wtk_json_item_t*)idf->hook;
		wtk_json_parser_reset(json);
		ret=wtk_json_parser_parse(json,p,strlen(p));
		if(ret!=0){return;}
		item=json->json->main;
		///wtk_json_item_print3(item);
		f=0;
		for(qn=idf->wrd_q.pop;qn;qn=qn->next)
		{
			wrd=data_offset2(qn,wtk_tfidf_wrd_t,q_n);
			n=sprintf(buf,"%d",wrd->id);
			//wtk_debug("wrd=%p id=%d\n",wrd,wrd->id);
			vi=wtk_json_obj_get(item,buf,n);
			if(vi)
			{
				//wtk_json_item_print3(vi);
				f+=vi->v.number*wrd->cnt;
			}
		}
		vi=wtk_json_obj_get_s(item,"sum");
		f2=vi->v.number;
		f=f/(idf->a*f2);
		//wtk_debug("f=%f\n",f);
		if(f<idf->cfg->idf_thresh)
		{
			return;
		}
		//wtk_debug("%f/%f\n",idf->a,f2);
		//wtk_debug("f=%f\n",f);
		if(idf->nout>0)
		{
			if(f>idf->out->idf)
			{
				idf->out->id=vti->v.number;
				idf->out->idf=f;
			}
		}else
		{
			idf->out->id=vti->v.number;
			idf->out->idf=f;
			idf->nout=1;
		}
//		for(i=idf->nout-1;i>=0;--i)
//		{
//			if(f<idf->out[i].idf)
//			{
//				++i;
//				break;
//			}else
//			{
//				idf->out[i+1]=idf->out[i];
//			}
//		}
//		if(i<(idf->cfg->nbest-1))
//		{
//			if(idf->nout<idf->cfg->nbest)
//			{
//				++idf->nout;
//			}
//			if(i<0){i=0;}
//			idf->out[i].id=vti->v.number;
//			idf->out[i].idf=f;
//		}
		//exit(0);
	}
}

int wtk_tfidf_add_ans_id(wtk_tfidf_t *idf,sqlite3_stmt *stmt)
{
	int ret;

	//wtk_debug("id=%d\n",(int)(wrd->v.number));
	ret=sqlite3_bind_int(stmt,1,idf->out->id);
	if(ret!=SQLITE_OK){ret=-1;goto end;}
end:
	//wtk_debug("ret=%d\n",ret);
	return ret;
}

void wtk_tfidf_notify_ans(wtk_tfidf_t *idf,int index,int col,sqlite3_value *v)
{
	int type;
	char *p;

	//wtk_debug("[%d,%d]\n",index,col);
	type=sqlite3_value_type(v);
	if(type==SQLITE_TEXT)
	{
		p=(char*)sqlite3_value_text(v);
		//wtk_debug("[%s]=%d\n",p,(int)strlen(p));
		wtk_strbuf_push(idf->buf,p,strlen(p));
	}
}


wtk_string_t wtk_tfidf_get_answer(wtk_tfidf_t *idf)
{
	wtk_string_t sql=wtk_string("select answer from qa where rowid=?");
	int ret;
	wtk_string_t v;

	wtk_string_set(&(v),0,0);
	wtk_strbuf_reset(idf->buf);
	ret=wtk_sqlite_exe3(idf->sqlite,&sql,idf,(wtk_sqlite_add_param_f)wtk_tfidf_add_ans_id,(wtk_sqlite_notify_value_f)wtk_tfidf_notify_ans);
	if(ret!=0){goto end;}
	wtk_string_set(&(v),idf->buf->data,idf->buf->pos);
end:
	return v;
}


int wtk_tfidf_add(wtk_tfidf_t *idf,wtk_tfidf_wrd_t *wrd)
{
	wtk_queue_node_t *qn;
	wtk_json_array_item_t *ai;
	//'CREATE  TABLE "qa" ("name" TEXT, "matrix" TEXT, "answer" TEXT)'
	//c.execute('INSERT INTO "map" (id,map) VALUES(?,?)',(i,v))
	wtk_string_t sql=wtk_string("select map from map where id=?");
	wtk_string_t sql2=wtk_string("select matrix from qa where rowid=?");
	int ret;
	wtk_json_parser_t *json=idf->json;

	//wtk_debug("[%.*s]\n",wrd->wrd.len,wrd->wrd.data);
	wtk_json_parser_reset(json);
	idf->hook=wrd;
	ret=wtk_sqlite_exe3(idf->sqlite,&sql,idf,(wtk_sqlite_add_param_f)wtk_tfidf_add_wrd_id,(wtk_sqlite_notify_value_f)wtk_tfidf_notify_wrd_map);
	if(ret!=0){goto end;}
	if(!json->json->main){ret=0;goto end;}
	//wtk_debug("len=%d\n",json->json->main->v.array->length);
	for(qn=json->json->main->v.array->pop;qn;qn=qn->next)
	{
		ai=data_offset2(qn,wtk_json_array_item_t,q_n);
		//id=(int)(ai->item->v.number);
		//wtk_debug("id=%d\n",id);
		idf->hook=ai->item;
		ret=wtk_sqlite_exe3(idf->sqlite,&sql2,idf,(wtk_sqlite_add_param_f)wtk_tfidf_add_qa_id,(wtk_sqlite_notify_value_f)wtk_tfidf_notify_qa_mat);
		//wtk_debug("ret=%d\n",ret);
		if(ret!=0){goto end;}
		//exit(0);
	}
end:
	//exit(0);
	return 0;
}

int wtk_tfidf_load_wrd(wtk_tfidf_t *idf)
{
	wtk_tfidf_wrd_t *wrd;
	wtk_queue_node_t *qn;
	int idx;

	//wtk_debug("len=%d/%d\n",idf->wrd_q.length,idf->n_valie_wrd);
	idf->a=0;
	for(qn=idf->wrd_q.pop;qn;qn=qn->next)
	{
		wrd=data_offset2(qn,wtk_tfidf_wrd_t,q_n);
		idf->a+=wrd->cnt*wrd->cnt;
	}
	//wtk_debug("a=%f\n",idf->a);
	idf->a=sqrt(idf->a);
	if(idf->n_valid_wrd>0)
	{
		for(qn=idf->wrd_q.pop;qn;qn=qn->next)
		{
			wrd=data_offset2(qn,wtk_tfidf_wrd_t,q_n);
			//wtk_debug("[%.*s]=%d\n",wrd->wrd.len,wrd->wrd.data,wrd->stop);
			if(!wrd->stop)
			{
				wtk_tfidf_add(idf,wrd);
			}
		}
	}else
	{
		srand(time_get_ms());
		idx=rand()%(idf->wrd_q.length);
		qn=wtk_queue_peek(&(idf->wrd_q),idx);
		if(qn)
		{
			wrd=data_offset2(qn,wtk_tfidf_wrd_t,q_n);
			wtk_tfidf_add(idf,wrd);
		}
	}
	return 0;
}

wtk_string_t wtk_tfidf_find(wtk_tfidf_t *idf,char *k,int bytes)
{
	wtk_strbuf_t *buf=idf->buf;
	wtk_string_t *v;
	wtk_string_t vx;
	int ret=-1;

	wtk_tfidf_reset(idf);
	wtk_string_set(&(vx),0,0);
	if(idf->seg)
	{
		wtk_segmenter_parse(idf->seg,k,bytes,buf);
		if(buf->pos<=0){goto end;}
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		v=wtk_heap_dup_string(idf->heap,buf->data,buf->pos);
		k=v->data;
		bytes=v->len;
	}
	//wtk_debug("[%.*s]\n",bytes,k);
	wtk_str_split2(k,bytes,idf,(wtk_str_split_f)wtk_tfidf_update_wrd,(wtk_str_split_is_sep_f)wtk_tfidf_is_sep);
	ret=wtk_tfidf_load_wrd(idf);
	if(ret!=0){goto end;}
	if(idf->nout<=0){ret=-1;goto end;}
	vx=wtk_tfidf_get_answer(idf);
	if(vx.len<=0){goto end;}
	ret=0;
end:
	//wtk_debug("[%.*s]\n",vx.len,vx.data);
	return vx;
}

