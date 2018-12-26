#ifndef _QTK_SERVICE_LIBS_IFLY_QTK_IFLY_TTS_H
#define _QTK_SERVICE_LIBS_IFLY_QTK_IFLY_TTS_H
#include "wtk/core/wtk_type.h"
#include "qtk_ifly_cfg.h"
#include "qtts.h"
#include "msp_cmn.h"
#include "msp_errors.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct qtk_ifly_tts qtk_ifly_tts_t;
typedef void (*qtk_ifly_tts_notify_f)(void* ths, const char *data, int len);
struct qtk_ifly_tts
{
	qtk_ifly_cfg_t *cfg;
	const char *session_id;
	qtk_ifly_tts_notify_f notify;
    void *ths;
	unsigned ready:1;
};

qtk_ifly_tts_t* qtk_ifly_tts_new(qtk_ifly_cfg_t *cfg);
void qtk_ifly_tts_delete(qtk_ifly_tts_t *ifly);
void qtk_ifly_tts_set_notify(qtk_ifly_tts_t *ifly, void *ths, qtk_ifly_tts_notify_f notify);
int qtk_ifly_tts_start(qtk_ifly_tts_t *ifly);
int qtk_ifly_tts_process(qtk_ifly_tts_t *ifly,char *data,int len);
#ifdef __cplusplus
};
#endif
#endif
