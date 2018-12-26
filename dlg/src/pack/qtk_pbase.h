#ifndef _PACK_QTK_PBASE_H_
#define _PACK_QTK_PBASE_H_
#ifdef __cplusplus
extern "C" {
#endif

#define US      037        /* unit separator */
#define RS      036        /* record separator */

typedef enum {
    QTK_PARSER_INIT = 0,
    QTK_PARSER_US,
    QTK_PARSER_RS,
    QTK_PARSER_DONE,
}qtk_pstate_t;

#ifdef __cplusplus
};
#endif
#endif
