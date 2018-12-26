#ifndef WTK_TFIDF_WTK_TFIDF_H_
#define WTK_TFIDF_WTK_TFIDF_H_
#include "wtk/core/wtk_hash.h"
#include "wtk/core/wtk_larray.h"
#include "wtk_tfidf_cfg.h"
#include "wtk/core/wtk_sqlite.h"
#include "wtk/core/wtk_str.h"
#include "wtk/core/json/wtk_json_parse.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_tfidf wtk_tfidf_t;
#define wtk_tfidf_find_s(i,k) wtk_tfidf_find(i,k,sizeof(k)-1)

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_string_t wrd;
	int id;
	int cnt;
	unsigned stop:1;
}wtk_tfidf_wrd_t;

typedef struct
{
	float idf;
	int id;
}wtk_tfidf_out_t;


struct wtk_tfidf
{
	wtk_tfidf_cfg_t *cfg;
	wtk_segmenter_t *seg;
	wtk_tfidf_out_t *out;
	int nout;
	wtk_sqlite_t *sqlite;
	wtk_json_parser_t *json;
	wtk_json_parser_t *json2;
	wtk_heap_t *heap;
	wtk_strbuf_t *buf;
	wtk_queue_t wrd_q;
	int n_valid_wrd;
	void *hook;
	double a;
};

wtk_tfidf_t* wtk_tfidf_new(wtk_tfidf_cfg_t *cfg);
void wtk_tfidf_delete(wtk_tfidf_t *idf);
void wtk_tfidf_reset(wtk_tfidf_t *idf);
wtk_string_t wtk_tfidf_find(wtk_tfidf_t *idf,char *k,int bytes);
#ifdef __cplusplus
};
#endif
#endif
