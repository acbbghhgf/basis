#include "qtk_dlg_res.h"

#include <string.h>

int qtk_dlg_res_parse(const char *res, qtk_dlg_res_t *r)
{
    char sep_literal[] = "://";
    char *sep = strstr(res, sep_literal);
    if (NULL == sep) return -1;
    int len = sep - res;
    switch (len) {
    case sizeof("builtin") - 1:
        if (memcmp(res, "builtin", len)) return -1;
        r->type = QTK_DLG_RES_BUILTIN;
        r->location = sep + sizeof(sep_literal) - 1;
        break;
    case sizeof("tmp") - 1:
        if (memcmp(res, "tmp", len)) return -1;
        r->type = QTK_DLG_RES_TMP;
        r->location = sep + sizeof(sep_literal) - 1;
        break;
    default:
        return -1;
    }
    return 0;
}
