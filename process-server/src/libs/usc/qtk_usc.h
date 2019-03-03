#ifndef WTK_EXT_USC_WTK_USC_H_
#define WTK_EXT_USC_WTK_USC_H_
#include "wtk/core/wtk_type.h"
#include "libusc.h"
#include "qtk_usc_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct qtk_usc qtk_usc_t;
struct qtk_usc
{
    qtk_usc_cfg_t *cfg;
    USC_HANDLE handle;
    wtk_strbuf_t *buf;
    wtk_strbuf_t *bufx;
    unsigned ready:1;
};

qtk_usc_t* qtk_usc_new(qtk_usc_cfg_t *cfg);
void qtk_usc_delete(qtk_usc_t *usc);
int qtk_usc_start(qtk_usc_t *usc);
int qtk_usc_process(qtk_usc_t *usc,char *data,int len,int is_end);
void qtk_usc_get_result(qtk_usc_t *usc,wtk_string_t *v);
#ifdef __cplusplus
};
#endif
#endif
