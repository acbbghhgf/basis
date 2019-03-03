#ifndef WTK_CORE_SEGMENTER_WTK_POSEG
#define WTK_CORE_SEGMENTER_WTK_POSEG
#include "wtk/core/wtk_type.h" 
#include "wtk_poseg_cfg.h"
#include "wtk_chnpos.h"
#include "wtk/core/wtk_fkv2.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_poseg wtk_poseg_t;

struct wtk_poseg
{
	wtk_poseg_cfg_t *cfg;
	wtk_chnpos_t *chnpos;
	wtk_heap_t *heap;
	wtk_array_t *input;
	wtk_string_t **wrds;
	wtk_string_t **pos;
	int nwrd;
};

wtk_poseg_t* wtk_poseg_new(wtk_poseg_cfg_t *cfg);
void wtk_poseg_delete(wtk_poseg_t *seg);
void wtk_poseg_reset(wtk_poseg_t *seg);
void wtk_poseg_process(wtk_poseg_t *seg,char *data,int bytes);
void wtk_poseg_print_output(wtk_poseg_t *seg);
#ifdef __cplusplus
};
#endif
#endif
