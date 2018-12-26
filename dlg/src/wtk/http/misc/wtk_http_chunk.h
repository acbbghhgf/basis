#ifndef WTK_PROTO_MISC_HTTP_WTK_HTTP_CHUNK_H_
#define WTK_PROTO_MISC_HTTP_WTK_HTTP_CHUNK_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_strbuf.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef void(*wtk_http_chunk_notify_f)(void *usr_data);
typedef struct wtk_http_chunk wtk_http_chunk_t;
struct wtk_http_req;

/*
 * chunk define:
	chunked-Body = *chunk
	"0" CRLF
	footer
	CRLF
	chunk = chunk-size [ chunk-ext ] CRLF
*/

typedef enum
{
	WTK_HTTP_CHUNK_WAIT_SIZE=0,
	WTK_HTTP_CHUNK_SIZE,
	WTK_HTTP_CHUNK_WAIT_CR,
	WTK_HTTP_CHUNK_WAIT_LF,
	WTK_HTTP_CHUNK_HDR_DONE,
	WTK_HTTP_CHUNK_0_WAIT_CR,
	WTK_HTTP_CHUNK_0_WAIT_LF,
	//WTK_HTTP_CHUNK_0_LAST_WAIT_CR,
	//WTK_HTTP_CHUNK_0_LAST_WAIT_LF,
	WTK_HTTP_CHUNK_DONE,
}wtk_http_chunk_state_t;

struct wtk_http_chunk
{
	wtk_http_chunk_state_t state;
	int chunk_size;
	wtk_strbuf_t *buf;
};

wtk_http_chunk_t* wtk_http_chunk_new();
void wtk_http_chunk_delete(wtk_http_chunk_t *c);
void wtk_http_chunk_reset(wtk_http_chunk_t *c);
int wtk_http_chunk_feed(wtk_http_chunk_t *chunk,char *data,int bytes,int *consumed,int *done);
void wtk_http_chunk_print(wtk_http_chunk_t *c);
#ifdef __cplusplus
};
#endif
#endif
