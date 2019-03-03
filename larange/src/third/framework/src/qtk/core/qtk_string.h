#ifndef QTK_CORE_QTK_STRING_H
#define QTK_CORE_QTK_STRING_H
#include "wtk/core/wtk_str.h"

#ifdef __cplusplus
extern "C" {
#endif

void qtk_str_tolower(wtk_string_t *str);
void qtk_str_toupper(wtk_string_t *str);
void qtk_str_tolower2(char *dst, const char *src, int len);
void qtk_str_toupper2(char *dst, const char *src, int len);

#ifdef __cplusplus
}
#endif

#endif
