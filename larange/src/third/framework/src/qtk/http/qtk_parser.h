#ifndef QTK_HTTP_QTK_PARSER_H_
#define QTK_HTTP_QTK_PARSER_H_
#ifdef __cplusplus
extern "C" {
#endif

#define QTK_PARSER                              \
    qtk_parser_handler handler;                 \
    qtk_parser_close_handler close_handler;     \
    qtk_parser_reset_handler reset_handler;

struct qtk_socket;

typedef struct qtk_parser qtk_parser_t;

typedef int (*qtk_parser_handler)(void *data, struct qtk_socket *s, char *buf, int len);
typedef int (*qtk_parser_close_handler)(void *data);
typedef int (*qtk_parser_reset_handler)(void *data);

struct qtk_parser {
    QTK_PARSER
};

#ifdef __cplusplus
};
#endif
#endif
