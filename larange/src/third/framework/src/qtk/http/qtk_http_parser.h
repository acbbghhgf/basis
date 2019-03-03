#ifndef _QTK_HTTP_QTK_HTTP_PARSER_H
#define _QTK_HTTP_QTK_HTTP_PARSER_H
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_queue.h"
#include "wtk/core/wtk_str_hash.h"
#include "qtk_parser.h"

/*#include "wtk/nk/qtk_connection.h"*/
#include "qtk/http/qtk_request.h"
#ifdef __cplusplus
extern "C" {
#endif

struct qtk_connection;

typedef struct qtk_http_parser qtk_http_parser_t;
typedef struct qtk_http_parser_pack_handler qtk_http_parser_pack_handler_t;
typedef int (*qtk_http_parser_pack_f)(void *layer, qtk_http_parser_t *parser);

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
    HTTP_REQUEST_PARSER = 0,
    HTTP_RESPONSE_PARSER,
} parser_type_t;

struct qtk_http_parser_pack_handler {
    qtk_http_parser_pack_f empty;
    qtk_http_parser_pack_f ready;
    qtk_http_parser_pack_f unlink;
    void *layer;
};

struct qtk_http_parser
{
	QTK_PARSER
	wtk_queue_node_t n;
    wtk_strbuf_t *tmp_buf;
    wtk_string_t key;
	void *pack;
    parser_type_t type;
    qtk_http_parser_state_t state;
    qtk_http_parser_pack_handler_t handlers[2];
};

qtk_http_parser_t* qtk_http_parser_new();
qtk_http_parser_t* qtk_http_parser_clone(qtk_http_parser_t *p);
int qtk_http_parser_delete(qtk_http_parser_t *p);
int qtk_http_parser_init(qtk_http_parser_t *p);
int qtk_http_parser_bytes(qtk_http_parser_t *p);
int qtk_http_parser_reset(qtk_http_parser_t *p);
int qtk_http_parser_reset_state(qtk_http_parser_t *p);
void qtk_http_parser_set_handlers(qtk_http_parser_t *p, parser_type_t type,
                                  qtk_http_parser_pack_f empty,
                                  qtk_http_parser_pack_f ready,
                                  qtk_http_parser_pack_f unlink,
                                  void *layer);
const char* qtk_http_status_str(int status);

#ifdef __cplusplus
};
#endif
#endif
