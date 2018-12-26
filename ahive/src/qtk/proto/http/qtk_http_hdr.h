#ifndef _QTK_PROTO_HTTP_QTK_HTTP_HDR_H
#define _QTK_PROTO_HTTP_QTK_HTTP_HDR_H
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_queue.h"
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_strbuf.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct qtk_http_hdr  qtk_http_hdr_t;
typedef struct qtk_http_hdr_item qtk_http_hdr_item_t;

typedef enum
{
    HTTP_CONTINUE=100,
	HTTP_OK=200,
	HTTP_PARTIAL=206,
	HTTP_BAD_REQUEST=400,
    HTTP_NOT_FOUND=404,
	HTTP_CONFLICT=409,
	HTTP_SERVER_ERR=500
}http_status;


typedef enum
{
	HTTP_UNKNOWN=0,
	HTTP_GET,
	HTTP_POST,
    HTTP_PUT,
    HTTP_HEAD,
    HTTP_DELETE,
}qtk_request_method;


struct qtk_http_hdr_item {
    wtk_queue_node_t node;
    wtk_string_t key;
    wtk_string_t value;
};

struct qtk_http_hdr
{
	wtk_heap_t *heap;
    union {
        struct request {
            qtk_request_method method;
            wtk_string_t url;
            wtk_string_t params;
        } req;
        struct response {
            int status;
        } rep;
    };
	long content_length;
    wtk_queue_t head_q;
};


qtk_http_hdr_t* qtk_httphdr_new();
int qtk_httphdr_delete(qtk_http_hdr_t *hdr);
int qtk_httphdr_reset(qtk_http_hdr_t *hdr);
int qtk_httphdr_process(qtk_http_hdr_t *hdr);
void qtk_httphdr_set_string(qtk_http_hdr_t *hdr, wtk_string_t *str, const char *data, int len);
int qtk_httphdr_add(qtk_http_hdr_t *hdr, const char *key, int klen, const char *value, int vlen);
wtk_string_t* qtk_httphdr_find_hdr(qtk_http_hdr_t *hdr, const char *key, int klen);
void qtk_httphdr_set_repstaus(qtk_http_hdr_t *hdr, const char *buf, int len);

#ifdef __cplusplus
};
#endif
#endif
