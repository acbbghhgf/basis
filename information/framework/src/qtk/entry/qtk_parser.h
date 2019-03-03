#ifndef QTK_HTTP_QTK_PARSER_H_
#define QTK_HTTP_QTK_PARSER_H_
#ifdef __cplusplus
extern "C" {
#endif

#define QTK_PARSER                              \
    qtk_parser_handler handler;                 \
    qtk_parser_clone_handler clone_handler;     \
    qtk_parser_close_handler close_handler;     \
    qtk_parser_reset_handler reset_handler;

struct qtk_socket;

typedef struct qtk_parser qtk_parser_t;

typedef int (*qtk_parser_handler)(void *data, struct qtk_socket *s, char *buf, int len);
typedef int (*qtk_parser_close_handler)(void *data);
typedef int (*qtk_parser_reset_handler)(void *data);
typedef qtk_parser_t* (*qtk_parser_clone_handler)(void *data);

struct qtk_parser {
    QTK_PARSER
};

#define qtk_parser_call(p, handler, ret_type, err) ({    \
    ret_type ret;                                        \
    if ((p)->handler) {                                  \
        ret = (p)->handler(p);                           \
    } else {                                             \
        ret = err;                                       \
    }                                                    \
    ret;})

#define qtk_parser_reset(p) qtk_parser_call(p, reset_handler, int, -1)
#define qtk_parser_close(p) qtk_parser_call(p, close_handler, int, -1)
#define qtk_parser_clone(p) qtk_parser_call(p, clone_handler, qtk_parser_t*, NULL)

#define qtk_parser_delete(p) do {qtk_parser_close(p); wtk_free(p);} while(0)


#ifdef __cplusplus
};
#endif
#endif
