#ifndef WTK_NK_WTK_RESPONSE_H_
#define WTK_NK_WTK_RESPONSE_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_queue.h"
#include "wtk/core/param/wtk_param.h"
#include "wtk/http/nk/wtk_connection.h"
#include "wtk/http/nk/wtk_nk.h"
#include "wtk/core/wtk_array.h"
#include "wtk_request.h"
#include "wtk_response_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_response wtk_response_t;
#define wtk_response_feedback_s(rep,s,c,req,b) wtk_response_feedback(rep,s,c,req,b,sizeof(b)-1)
#define wtk_response_set_stream_status_s(rep,id,s)  wtk_response_set_stream_status(rep,id,s,sizeof(s)-1)
#define wtk_response_set_body_s(rep,b) wtk_response_set_body(rep,b,sizeof(b)-1)

typedef enum
{
	WTK_STREAM_NO_ERR=0,
	WTK_STREAM_NO_AUDIO_TAG=1,
	WTK_STREAM_NO_SCRIPT=2,
	WTK_STREAM_INVALID_LUA=3,
	WTK_STREAM_INVALID_PARAM=4,
	WTK_STREAM_CALC_FAILED=5,
	WTK_STREAM_INVALID_SEQUENCE=6,
	WTK_STREAM_UPSERVER_NOT_FOUND=7,
}wtk_stream_status_t;

typedef struct
{
	wtk_string_t *k;
	wtk_string_t *v;
}wtk_response_hdr_t;

struct wtk_response
{
	wtk_queue_node_t n;
	wtk_string_t stream_status;
	wtk_string_t body;
	wtk_string_t up;
	//wtk_strbuf_t* audio_url;
    int stream_data_received;
    int stream_status_id;
	int status;
	wtk_array_t *custom_hdr_array;			//used for store cutstom hdr array; wtk_response_hdr_t array;
	wtk_string_t *custom_hdr_key;
	wtk_string_t *custom_hdr_value;
	wtk_string_t *audio_url;
	wtk_string_t *content_type;
	wtk_string_t *lua_hdr;
	wtk_param_t *result;
	wtk_param_t *hint_result;
	wtk_string_t *test;
    unsigned writed:1;
    unsigned discard:1;
	unsigned eof:1;
};

wtk_response_t* wtk_response_new();
int wtk_response_delete(wtk_response_t *rep);
int wtk_response_bytes(wtk_response_t *rep);
int wtk_response_init(wtk_response_t *rep);
int wtk_response_reset(wtk_response_t *rep);
int wtk_response_feedback(wtk_response_t *rep,int status,wtk_connection_t *con,wtk_request_t *req,char *body,int body_len);
int wtk_response_write(wtk_response_t *rep,wtk_connection_t *con,wtk_request_t *req);
int wtk_response_set_result(wtk_response_t *rep,wtk_param_t* result);
int wtk_response_set_hint_result(wtk_response_t *rep,wtk_param_t* result);
int wtk_response_set_hint_result2(wtk_response_t *rep,char *s,int len);
int wtk_response_set_stream_status(wtk_response_t *rep,int id,const char *s,int len);
int wtk_response_is_finished(wtk_response_t *rep);
int wtk_response_set_lua_hdr(wtk_response_t *rep,wtk_string_t *lua);
void wtk_response_set_body(wtk_response_t *rep,char *data,int len);
void wtk_response_print_hdr_array(wtk_response_t *rep);
#ifdef __cplusplus
};
#endif
#endif
