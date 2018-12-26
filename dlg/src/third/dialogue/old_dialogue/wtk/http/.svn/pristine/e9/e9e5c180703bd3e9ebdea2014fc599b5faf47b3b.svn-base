#ifndef WTK_HTTPC_WTK_RESPONSE_H_
#define WTK_HTTPC_WTK_RESPONSE_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_alloc.h"
#include "wtk/core/wtk_stack.h"
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_strbuf.h"
#include "wtk/core/wtk_str_hash.h"
#include "wtk_http_chunk.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_http_response wtk_http_response_t;
#define wtk_http_response_get_hdr(rep,key) wtk_hash_find(rep->hdr,key)

typedef enum 
{
    RESPONSE_START=0,
    RESPONSE_STATUS_CODE,
    RESPONSE_STATUS_INFO,
    RESPONSE_WAIT_CR,
    RESPONSE_CR,
    RESPONSE_CL,
    RESPONSE_KEY,
    RESPONSE_KEY_SPACE,
    RESPONSE_VALUE,
    RESPONSE_HDR_ALMOST_DONE,
    RESPONSE_HDR_DONE,
    RESPONSE_CONTENT_DONE
}wtk_response_state_t;

struct wtk_http_response
{
    wtk_heap_t *heap;
    wtk_str_hash_t *hash;		//HTTP response header, value is wtk_string_t*
    wtk_strbuf_t *body;
    wtk_strbuf_t *tmp_buf;

    wtk_string_t key;
    int status;
    int content_length;
    wtk_response_state_t state;

    wtk_http_chunk_t *chunk;
    unsigned unknown_content_length:1;
    unsigned is_chunk:1;
};

wtk_http_response_t* wtk_http_response_new();
wtk_http_response_t* wtk_http_response_new2(int hdr_bytes,int hdr_slot,int body_bytes);
int wtk_http_response_delete(wtk_http_response_t *rep);
int wtk_http_response_reset(wtk_http_response_t* rep);

int wtk_http_response_process_hdr(wtk_http_response_t *rep,char *buf,int bytes,int *left);
int wtk_http_response_process_content(wtk_http_response_t *rep,char *buf,int bytes,int *left);
int wtk_http_response_feed(wtk_http_response_t *rep,char *buf,int bytes,int *left);
int wtk_http_response_is_finish(wtk_http_response_t *rep);
void wtk_http_response_print(wtk_http_response_t *rep);
int wtk_http_response_feed_fd(wtk_http_response_t *rep,int fd,wtk_strbuf_t *buf);
#ifdef __cplusplus
};
#endif
#endif
