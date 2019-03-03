#ifndef _QTK_HTTP_QTK_RESPONSE_H
#define _QTK_HTTP_QTK_RESPONSE_H
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_str_hash.h"
#include "wtk/core/wtk_strbuf.h"
#ifdef __cplusplus
extern "C" {
#endif

enum {
    HTTP_CONTINUE=100,
    HTTP_OK=200,
    HTTP_NOT_FOUND=404,
    HTTP_INTER_SERVER_ERROR=500,
};

#define qtk_response_new_from_str_s(heap, s, status) \
        qtk_response_new_from_str(heap, s, sizeof(s)-1, status)
#define qtk_response_add_hdr_s(r, s, v) qtk_response_add_hdr(r, s, sizeof(s)-1, v)
#define qtk_response_set_body_s(r, s) qtk_response_set_body(r, s, sizeof(s)-1)

struct qtk_request;
struct qtk_socket;

typedef struct qtk_response qtk_response_t;
struct qtk_response
{
    int status;
    int sock_id;
    wtk_queue_node_t hoard_n;
    wtk_heap_t *heap;             /* NOTE: use of this heap is not thread-safe */
    wtk_str_hash_t *hash;
    wtk_strbuf_t *buf;
    wtk_strbuf_t *body;
    struct qtk_request *request;
    int content_length;
    unsigned heap_ref:1;
};

qtk_response_t* qtk_response_new(void);
qtk_response_t* qtk_response_new_ref(wtk_heap_t *heap);
int qtk_response_init(qtk_response_t *rep, int sock_id);
void qtk_response_update_hdr(qtk_response_t *rep);
int qtk_response_process_body(qtk_response_t *rep, char *data, int len, int *left);
int qtk_response_delete(qtk_response_t *rep);
int qtk_response_send(qtk_response_t *rep, struct qtk_socket *s);
qtk_response_t *qtk_response_new_from_str(wtk_heap_t *heap, char *data, int len, int status);
void qtk_response_set_body(qtk_response_t *rep, const char *buf, int len);
void qtk_response_print(qtk_response_t *rep);
int qtk_response_reset(qtk_response_t *rep);
void qtk_response_add_hdr(qtk_response_t *rep, const char *key, int klen, wtk_string_t *value);
wtk_string_t* qtk_response_get_hdr(qtk_response_t *rep, const char *key, int klen);

#ifdef __cplusplus
};
#endif
#endif
