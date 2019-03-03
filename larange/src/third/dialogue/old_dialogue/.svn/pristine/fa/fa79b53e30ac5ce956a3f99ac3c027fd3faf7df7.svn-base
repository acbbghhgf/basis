#ifndef WTK_SEMDLG_NLPEMOT_WTK_NLPEMOT
#define WTK_SEMDLG_NLPEMOT_WTK_NLPEMOT
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_fkv.h"
#include "wtk/core/json/wtk_json_parse.h"
#include "wtk_nlpemot_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_nlpemot wtk_nlpemot_t;

#define NLP_EMOT_VEC_SIZE 23

typedef struct
{
	float v[NLP_EMOT_VEC_SIZE];
}wtk_nlpemot_vec_t;


struct wtk_nlpemot
{
	wtk_nlpemot_cfg_t *cfg;
	wtk_lexr_t *lexr;
	wtk_segmenter_t *seg;
	wtk_fkv_t *dict;
	wtk_nlpemot_vec_t vec;
	wtk_strbuf_t *type;
	float value;
};

wtk_nlpemot_t* wtk_nlpemot_new(wtk_nlpemot_cfg_t *cfg,wtk_rbin2_t *rbin,wtk_lexr_t *lexr);
void wtk_nlpemot_delete(wtk_nlpemot_t *e);
void wtk_nlpemot_process(wtk_nlpemot_t *e,char *txt,int bytes);
void wtk_nlpemot_vec_print(wtk_nlpemot_vec_t *vec);
wtk_string_t* wtk_nlpemot_get_index_type(int i);
int wtk_nlpemot_get_max_index(wtk_nlpemot_t *e);
void wtk_nlpemot_print(wtk_nlpemot_t *e);
#ifdef __cplusplus
};
#endif
#endif
