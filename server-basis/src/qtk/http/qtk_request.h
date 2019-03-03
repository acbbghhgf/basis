#ifndef _QTK_HTTP_QTK_REQUEST_H
#define _QTK_HTTP_QTK_REQUEST_H
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_queue.h"
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_str_hash.h"
#include "wtk/core/wtk_strbuf.h"
#include "wtk/core/wtk_audio_type.h"
#include "qtk_response.h"
#ifdef __cplusplus
extern "C" {
#endif
#define qtk_request_add_hdr_s(r,k,v) qtk_request_add_hdr(r,k,sizeof(k)-1,v)
struct qtk_socket;

typedef struct qtk_request  qtk_request_t;
typedef int (*qtk_request_str_to_content_type_f)(char *data,int bytes);



typedef enum
{
	HTTP_UNKNOWN=0,
	HTTP_GET,
	HTTP_POST,
    HTTP_PUT,
    HTTP_HEAD,
    HTTP_DELETE,
}qtk_request_method;

struct qtk_request
{
	wtk_queue_node_t hoard_n;
	wtk_queue_node_t req_n;
    int sock_id;
	wtk_str_hash_t *hash;
    wtk_strbuf_t *buf;
    wtk_string_t url;
    wtk_string_t params;
	wtk_heap_t *heap;
	wtk_strbuf_t *body;
	qtk_request_method method;

	long content_length;
};

void qtk_request_set_str_to_content_type_f_g(qtk_request_str_to_content_type_f f);
qtk_request_t* qtk_request_new(void);
int qtk_request_delete(qtk_request_t *r);
int qtk_request_bytes(qtk_request_t *r);
int qtk_request_init(qtk_request_t *r, int sock_id);
int qtk_request_reset(qtk_request_t *r);
int qtk_request_process_hdr(qtk_request_t *r,char* buf,int len,int *left);
int qtk_request_process_body(qtk_request_t *r,char* buf,int len,int *left);
int qtk_request_send(qtk_request_t *r, struct qtk_socket* s);
int qtk_request_send2(qtk_request_t *r, int sock);
wtk_string_t* qtk_request_get_hdr(qtk_request_t *r, const char *key, int len);
void qtk_request_update_hdr(qtk_request_t *r);
void qtk_request_add_hdr(qtk_request_t *r, const char *key, int klen, wtk_string_t *value);
void qtk_request_set_body(qtk_request_t *r, const char *buf, int len);
void qtk_request_print(qtk_request_t *r);
const char *qtk_request_method_s(qtk_request_t *r);

void qtk_request_set_string(qtk_request_t *r,wtk_string_t *str,char *data,int bytes);

void qtk_request_copy(qtk_request_t *dest, qtk_request_t *src);
#ifdef __cplusplus
};
#endif
#endif
