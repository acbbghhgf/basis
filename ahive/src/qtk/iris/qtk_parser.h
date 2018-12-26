#ifndef QTK_HTTP_QTK_PARSER_H_
#define QTK_HTTP_QTK_PARSER_H_
#ifdef __cplusplus
extern "C" {
#endif

#define QTK_PARSER \
    qtk_parser_handler handler; \
    qtk_parser_close_handler close; \
    qtk_parser_shot_handler shot; \
    qtk_parser_close_notify_handler close_notify;

struct qtk_connection;

typedef int (*qtk_parser_handler)(void *data, struct qtk_connection *conn, char *buf, int len);
typedef int (*qtk_parser_close_handler)(void *data);
typedef int (*qtk_parser_shot_handler)(void *data);
typedef int (*qtk_parser_close_notify_handler)(void *data);

#ifdef __cplusplus
};
#endif
#endif
