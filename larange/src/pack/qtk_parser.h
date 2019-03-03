#ifndef _PACK_QTK_PARSER_H_
#define _PACK_QTK_PARSER_H_
#include "wtk/core/wtk_str.h"
#include "qtk_pbase.h"
#include "qtk_pack.h"
#ifdef __cplusplus
extern "C" {
#endif

struct qtk_pipe;
typedef struct qtk_parser qtk_parser_t;
typedef int (*qtk_parser_handler) (void *ud, qtk_parser_t *ps);
typedef int (*qtk_parser_func)(qtk_parser_t *ps, struct qtk_pipe *p, const char *d, size_t len);

struct qtk_parser {
    qtk_pack_t *pack;
    qtk_pstate_t state;
    qtk_parser_func f;
    qtk_parser_handler pack_empty;
    qtk_parser_handler pack_ready;
    qtk_parser_handler pack_unlink;
    void *ud;
};

qtk_parser_t *qtk_parser_new(void *ud);
void qtk_parser_delete(qtk_parser_t *ps);
void qtk_parser_set_handler(qtk_parser_t *ps, qtk_parser_handler empty,
                            qtk_parser_handler ready, qtk_parser_handler unlink);
void qtk_parser_clone(qtk_parser_t *dst, qtk_parser_t *src);
void qtk_parser_clean(qtk_parser_t *ps);

#ifdef __cplusplus
};
#endif
#endif
