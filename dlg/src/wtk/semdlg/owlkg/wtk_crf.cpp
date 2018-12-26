#ifdef USE_CRF
#include "wtk_crf.h"
#include "crfpp.h"
#include <iostream>
#include <vector>
#include <iterator>
#include <cmath>
#include <string>
#include <sstream>
#include "./third/CRF++-0.58/stream_wrapper.h"
#include "./third/CRF++-0.58/common.h"
#include "./third/CRF++-0.58/tagger.h"

wtk_crf_t* wtk_crf_new(char *fn)
{
	wtk_crf_t *crf;
	wtk_strbuf_t *buf;

	crf=(wtk_crf_t*)wtk_malloc(sizeof(wtk_crf_t));
	buf=wtk_strbuf_new(256,1);
	//"-m ./data/crf/model"
	wtk_strbuf_push_s(buf,"-m ");
	wtk_strbuf_push(buf,fn,strlen(fn));
	wtk_strbuf_push_s(buf," -v1");
	wtk_strbuf_push_c(buf,0);
#ifdef __ANDROID__
#else
	{
		CRFPP::TaggerImpl *tagger;
		tagger=new CRFPP::TaggerImpl();
		tagger->open(buf->data);
		crf->tager=tagger;
	}
#endif
	wtk_strbuf_delete(buf);
	return crf;
}

void wtk_crf_delete(wtk_crf_t *c)
{
#ifdef __ANDROID__
#else
	CRFPP::TaggerImpl *tagger;

	tagger=(CRFPP::TaggerImpl *)c->tager;
	delete tagger;
#endif
	wtk_free(c);
}

void wtk_crf_reset(wtk_crf_t *c)
{
#ifdef __ANDROID__
#else
	CRFPP::TaggerImpl *tagger;

	tagger=(CRFPP::TaggerImpl *)c->tager;
	tagger->clear();
#endif
}

int wtk_crf_add(wtk_crf_t *c,char *s)
{
#ifdef __ANDROID__
	return -1;
#else
	CRFPP::TaggerImpl *tagger;
	bool b;

	tagger=(CRFPP::TaggerImpl *)c->tager;
	b=tagger->add(s);
	return b?0:-1;
#endif
}

int wtk_crf_nresult(wtk_crf_t *c)
{
#ifdef __ANDROID__
	return -1;
#else
	CRFPP::TaggerImpl *tagger;

	tagger=(CRFPP::TaggerImpl *)c->tager;
	//wtk_debug("v=%d\n",tagger->size());
	return tagger->size();
#endif
}

const char* wtk_crf_get(wtk_crf_t *c,int i)
{
#ifdef __ANDROID__
	return NULL;
#else
	CRFPP::TaggerImpl *tagger;

	tagger=(CRFPP::TaggerImpl *)c->tager;
	return tagger->yname(tagger->y(i));
#endif
}

float wtk_crf_get_prob(wtk_crf_t *c,int i)
{
#ifdef __ANDROID__
	return 0.0;
#else
	CRFPP::TaggerImpl *tagger;

	tagger=(CRFPP::TaggerImpl *)c->tager;
	return tagger->prob(i);
#endif
}

int wtk_crf_process(wtk_crf_t *c)
{
#ifdef __ANDROID__
	return -1;
#else
	CRFPP::TaggerImpl *tagger;
	bool b;

	tagger=(CRFPP::TaggerImpl *)c->tager;
	b=tagger->parse();
	//wtk_debug("b=%d\n",b);
	if(0)
	{
		const char *s;

		s=tagger->toString();
		printf("%s\n",s);
		//exit(0);
	}
	return b?0:-1;
#endif
}

const char* wtk_crf_get_result(wtk_crf_t *c)
{
#ifdef __ANDROID__
	return NULL;
#else
	CRFPP::TaggerImpl *tagger;
	int i,n;
	const char *s;

	tagger=(CRFPP::TaggerImpl *)c->tager;
	tagger->parse();
	wtk_debug("x=%d y=%d\n",tagger->xsize(),tagger->ysize());
	n=tagger->xsize();
	for(i=0;i<n;++i)
	{
		s=tagger->yname(tagger->y(i));
		wtk_debug("v[%d]=%s\n",i,s);
	}
	return tagger->toString();
#endif
}
#endif
