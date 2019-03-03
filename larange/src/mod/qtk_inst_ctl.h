#ifndef _MOD_QTK_INST_CTL_H_
#define _MOD_QTK_INST_CTL_H_

#include <glob.h>
#include "main/qtk_dlg_worker.h"

#define qtk_ictl_update_cnt_s(s, diff)      qtk_ictl_update_cnt(s, sizeof(s)-1, diff)
#define qtk_ictl_get_cnt_s(s)               qtk_ictl_get_cnt(s, sizeof(s)-1)
#define qtk_ictl_get_pending_cnt_s(s)       qtk_ictl_get_pending_cnt(s, sizeof(s)-1)
#define qtk_ictl_pend_s(s, ref, ud)         qtk_ictl_pend(s, sizeof(s)-1, ref, ud)
#define qtk_ictl_flush_s(s)                 qtk_ictl_flush(s, sizeof(s)-1)

int qtk_ictl_init(qtk_dlg_worker_t *dw);
int qtk_ictl_update_cnt(const char *n, size_t l, int diff);
int qtk_ictl_get_cnt(const char *n, size_t l);
void qtk_ictl_try_remove(const char *n, size_t l);
void qtk_ictl_clean();
int qtk_ictl_pend(const char *n, size_t l, int ref, wtk_string_t *ud);
int qtk_ictl_flush(const char *n, size_t l);
int qtk_ictl_get_pending_cnt(const char *n, size_t l);

#endif
