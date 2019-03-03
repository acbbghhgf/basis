#ifndef WTK_SEGMENTER_WTK_SEGMENTER_H_
#define WTK_SEGMENTER_WTK_SEGMENTER_H_
#include "wtk/core/wtk_type.h"
#include "wtk_segmenter_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_segmenter wtk_segmenter_t;
#define wtk_segmenter_parse_s(seg,data) wtk_segmenter_parse(seg,data,sizeof(data)-1)

struct wtk_segmenter
{
	wtk_segmenter_cfg_t *cfg;
	wtk_fkv2_t *fkv;
	wtk_heap_t *heap;
	wtk_strbuf_t *buf;
	//------------- output word array ------------
	wtk_string_t **wrd_array;
	int wrd_array_n;
};

wtk_segmenter_t* wtk_segmenter_new(wtk_segmenter_cfg_t *cfg,wtk_rbin2_t *rbin);
void wtk_segmenter_delete(wtk_segmenter_t *seg);
void wtk_segmenter_reset(wtk_segmenter_t *seg);
int wtk_segmenter_parse(wtk_segmenter_t *seg,char *data,int bytes,wtk_strbuf_t *output_buf);
int wtk_segmenter_parse2(wtk_segmenter_t *seg,char *data,int bytes);
#ifdef __cplusplus
};
#endif
#endif
