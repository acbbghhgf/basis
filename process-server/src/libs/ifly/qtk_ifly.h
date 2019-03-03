#ifndef _QTK_SERVICE_LIBS_IFLY_QTK_IFLY_H
#define _QTK_SERVICE_LIBS_IFLY_QTK_IFLY_H
#include "wtk/core/wtk_type.h"
#include "qtk_ifly_cfg.h"
#include "qisr.h"
#include "msp_cmn.h"
#include "msp_errors.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct qtk_ifly qtk_ifly_t;
struct qtk_ifly
{
	qtk_ifly_cfg_t *cfg;
	wtk_strbuf_t *buf;
	wtk_strbuf_t *bufx;
	const char *session_id;
	int cnt;
	unsigned ready:1;
};

qtk_ifly_t* qtk_ifly_new(qtk_ifly_cfg_t *cfg);
void qtk_ifly_delete(qtk_ifly_t *ifly);
int qtk_ifly_start(qtk_ifly_t *ifly);
void qtk_ifly_process(qtk_ifly_t *ifly,char *data,int len,int is_end);
void qtk_ifly_get_result(qtk_ifly_t *ifly,wtk_string_t *v);
#ifdef __cplusplus
};
#endif
#endif
