#ifndef _QTK_PROTO_HTTP_QTK_ZHTTP_PARSER_H
#define _QTK_PROTO_HTTP_QTK_ZHTTP_PARSER_H
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_queue.h"
#include "wtk/core/wtk_strbuf.h"
#include "qtk/proto/qtk_zeus_parser.h"
#include "qtk/proto/http/qtk_http_hdr.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct qtk_zhttp_parser qtk_zhttp_parser_t;
typedef struct qtk_http_parser_pack_handler qtk_http_parser_pack_handler_t;
typedef int (*qtk_http_parser_pack_f)(void *layer, qtk_zhttp_parser_t *parser);

typedef enum
{
    HTTP_START=0,
    HTTP_METHOD,
    HTTP_URL_SPACE,
    HTTP_URL,
    HTTP_URL_PARAM,
    HTTP_WAIT_VERSION,
    HTTP_VERSION,
    HTTP_STATUS_CODE,
    HTTP_STATUS_INFO,
    HTTP_WAIT_CR,
    HTTP_CR,
    HTTP_CL,
    HTTP_KEY,
    HTTP_KEY_SPACE,
    HTTP_VALUE,
    HTTP_HDR_ALMOST_DONE,
    HTTP_HDR_DONE,
    HTTP_CONTENT_DONE
} qtk_http_parser_state_t;

typedef enum {
    HTTP_REQUEST_PARSER = 1,
    HTTP_RESPONSE_PARSER,
} parser_type_t;

typedef enum {
    HTTP_CONTENT_LENGTH = 0,
    HTTP_KEY_UNKNOWN = -1,
} http_key_t;

struct qtk_http_parser_pack_handler {
    qtk_http_parser_pack_f empty;
    qtk_http_parser_pack_f ready;
    qtk_http_parser_pack_f unlink;
    void *layer;
};

struct qtk_zhttp_parser
{
	QTK_PARSER
    wtk_string_t key;
    wtk_strbuf_t *tmp_buf;
	qtk_http_hdr_t *pack;
    parser_type_t type;
    qtk_http_parser_state_t state;
    qtk_http_parser_pack_handler_t handlers[2];
};

qtk_zhttp_parser_t* zqtk_http_parser_new();
int qtk_zhttp_parser_delete(qtk_zhttp_parser_t *p);
int qtk_zhttp_parser_clean(qtk_zhttp_parser_t *p);
int qtk_zhttp_parser_init(qtk_zhttp_parser_t *p);
int qtk_zhttp_parser_reset(qtk_zhttp_parser_t *p);
int qtk_zhttp_parser_reset_state(qtk_zhttp_parser_t *p);
int qtk_zhttp_parser_get_body(qtk_zhttp_parser_t *p, struct qtk_deque *dq,
                             struct qtk_deque *pack);
const char *qtk_zhttp_status_str(int status);

#ifdef __cplusplus
};
#endif
#endif
