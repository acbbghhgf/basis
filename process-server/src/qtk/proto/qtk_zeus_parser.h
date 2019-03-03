#ifndef QTK_PROTO_QTK_ZEUS_PARSER_H_
#define QTK_PROTO_QTK_ZEUS_PARSER_H_
#ifdef __cplusplus
extern "C" {
#endif

struct qtk_deque;

#define QTK_PARSER \
    qtk_parser_handler handler; \
    qtk_parser_getbody_handler get_body;

/*
  return value > 0: raise the package
  return value < 0: error
  return value == 0: continue
 */
typedef struct qtk_zues_parser qtk_zeus_parser_t;
typedef int (*qtk_parser_handler)(void *data, struct qtk_deque *dq);
typedef int (*qtk_parser_getbody_handler)(qtk_zeus_parser_t *parser, struct qtk_deque *dq, struct qtk_deque *pack);

struct qtk_zeus_parser
{
    QTK_PARSER
};

#ifdef __cplusplus
};
#endif
#endif
