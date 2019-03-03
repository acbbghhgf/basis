#ifndef QTK_UTIL_QTK_MAIL_H_
#define QTK_UTIL_QTK_MAIL_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_strbuf.h"
#ifdef __cplusplus
extern "C" {
#endif
#define qtk_mail_send_s(s) qtk_mail_send(s, sizeof(s)-1)
int qtk_mail_init(wtk_strbuf_t *tmpl);
int qtk_mail_send(const char *msg, int len);
int qtk_mail_clean();
#ifdef __cplusplus
}
#endif
#endif
