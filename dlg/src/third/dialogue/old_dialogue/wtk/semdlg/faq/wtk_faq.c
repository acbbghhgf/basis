#include "wtk_faq.h" 

wtk_faq_t* wtk_faq_new(wtk_faq_cfg_t *cfg,wtk_rbin2_t *rbin)
{
	wtk_faq_t *f;

	f=(wtk_faq_t*)wtk_malloc(sizeof(wtk_faq_t));
	f->cfg=cfg;
	if(cfg->use_vecdb)
	{
		f->db=wtk_vecdb_new(&(cfg->vecdb));
	}else
	{
		f->db=NULL;
	}
	if(cfg->use_tfidf)
	{
		f->v.idf=wtk_tfidf_new(&(cfg->tifidf));
	}else
	{
		f->v.vecfaq=wtk_vecfaq_new(&(cfg->vecfaq),rbin);
		if(f->db && !f->db->cfg->use_wrdvec)
		{
			f->db->wrdvec=f->v.vecfaq->wrdvec;
		}
	}
	return f;
}

void wtk_faq_set_wrdvec(wtk_faq_t *faq,wtk_wrdvec_t *wvec)
{
	if(!faq->cfg->use_tfidf)
	{
		wtk_vecfaq_set_wrdvec(faq->v.vecfaq,wvec);
		if(faq->db)
		{
			faq->db->wrdvec=wvec;
		}
	}
}

void wtk_faq_delete(wtk_faq_t *faq)
{
	if(faq->cfg->use_tfidf)
	{
		wtk_tfidf_delete(faq->v.idf);
	}else
	{
		wtk_vecfaq_delete(faq->v.vecfaq);
	}
	if(faq->db)
	{
		wtk_vecdb_delete(faq->db);
	}
	wtk_free(faq);
}

void wtk_faq_add(wtk_faq_t *faq,char *q,int q_bytes,char *a,int a_bytes)
{
	//wtk_debug("[%.*s]=[%.*s]\n",q_bytes,q,a_bytes,a);
	//wtk_faqdb_add(faq->db,q,q_bytes,a,a_bytes);
	if(faq->db)
	{
		wtk_vecdb_add(faq->db,q,q_bytes,a,a_bytes);
	}
}

wtk_string_t wtk_faq_get2(wtk_faq_t *faq,char *s,int slen,char *input,int input_len)
{
	wtk_string_t v;

	if(!faq->cfg->use_tfidf)
	{
		v=wtk_vecfaq_get3(faq->v.vecfaq,s,slen,input,input_len);
	}else
	{
		wtk_string_set(&(v),0,0);
	}
	return v;
}

wtk_string_t wtk_faq_get(wtk_faq_t *faq,char *data,int bytes)
{
	wtk_string_t t;

	//wtk_debug("[%.*s]\n",bytes,data);
	//t=wtk_faqdb_get(faq->db,data,bytes);
	if(faq->db)
	{
		t=wtk_vecdb_get(faq->db,data,bytes);
		if(t.len>0)
		{
			return t;
		}
	}
	if(faq->cfg->use_tfidf)
	{
		return wtk_tfidf_find(faq->v.idf,data,bytes);
	}else
	{
		return wtk_vecfaq_get(faq->v.vecfaq,data,bytes);
	}
}
