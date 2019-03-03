#ifndef _QTK_FWD_QTK_FWD_CFG_H
#define _QTK_FWD_QTK_FWD_CFG_H
#include "wtk/core/wtk_str.h"
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "qtk/event/qtk_event_cfg.h"
#include "qtk/entry/qtk_entry_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct qtk_fwd_cfg qtk_fwd_cfg_t;

struct qtk_fwd_cfg
{
    qtk_event_cfg_t event;
    qtk_entry_cfg_t entry;
    wtk_local_cfg_t *uplayer;
    char *uplayer_name;
    wtk_string_t log_fn;
    wtk_string_t pid_fn;
    wtk_string_t mail_tmpl;
    wtk_strbuf_t *mail_buf;
    int statis_time;
    unsigned daemon:1;
    unsigned send_mail:1;
    unsigned ld_deepbind:1;
};

int qtk_fwd_cfg_init(qtk_fwd_cfg_t *cfg);
int qtk_fwd_cfg_clean(qtk_fwd_cfg_t *cfg);
int qtk_fwd_cfg_update(qtk_fwd_cfg_t *cfg);
int qtk_fwd_cfg_update_local(qtk_fwd_cfg_t *cfg,wtk_local_cfg_t *lc);
void qtk_fwd_cfg_print(qtk_fwd_cfg_t *cfg);
void qtk_http_cfg_update_debug(qtk_fwd_cfg_t *cfg,int debug);
int qtk_http_cfg_update_depend(qtk_fwd_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
