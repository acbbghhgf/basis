#ifndef _WTK_NK_WTK_REQUEST_H_
#define _WTK_NK_WTK_REQUEST_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_queue.h"
#include "wtk/core/wtk_stack.h"
//#include "wtk/audio/wtk_decode.h"
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_str_hash.h"
#include "wtk/core/wtk_strbuf.h"
#include "wtk/os/wtk_pipequeue.h"
#include "wtk/http/nk/wtk_nk_event.h"
#include "wtk/core/param/wtk_param.h"
#include "wtk/core/wtk_audio_type.h"
#include "wtk/http/misc/wtk_http_response.h"
#include "wtk_request_cfg.h"
//#include "wtk_response.h"
#ifdef __cplusplus
extern "C" {
#endif
#define wtk_request_feed_back_s(req,code,s) wtk_request_feed_back(req,code,s,sizeof(s)-1)
#define wtk_request_set_response_hdr_s(r,k,v,v_len) wtk_request_set_response_hdr(r,k,sizeof(k)-1,v,v_len)
#define wtk_request_touch_s(r,s) wtk_request_touch(r,s,sizeof(s)-1)
struct wtk_http;
struct wtk_connection;
typedef struct wtk_request  wtk_request_t;
typedef int (*wtk_request_str_to_content_type_f)(char *data,int bytes);

#define DEBUG_TIME 1

typedef enum
{
	HTTP_OK=200,
	HTTP_BAD_REQUEST=400,
	HTTP_CONFLICT=409,
	HTTP_SERVER_ERR=500
}http_status;

typedef enum
{
	REQUEST_START=0,
	REQUEST_METHOD,
	REQUEST_URL_SPACE,
	REQUEST_URL,
	REQUEST_URL_PARAM,
	REQUEST_WAIT_VERSION,
	REQUEST_VERSION,
	REQUEST_CR,
	REQUEST_CL,
	REQUEST_KEY,
	REQUEST_KEY_SPACE,
	REQUEST_VALUE,
	REQUEST_ALMOST_DONE,
	REQUEST_HDR_DONE,
	REQUEST_CONTENT_DONE
}wtk_request_state;

typedef enum
{
	HTTP_UNKNOWN=0,
	HTTP_GET,
	HTTP_POST
}wtk_request_method;

struct wtk_request
{
	wtk_request_cfg_t *cfg;
	wtk_queue_node_t hoard_n;
	wtk_queue_node_t req_n;
    wtk_nk_event_t nk_to_vm_event;
    struct wtk_connection* c;
    struct wtk_response *response;
	//wtk_str_hash_t *hash;
	wtk_heap_t *heap;
	wtk_strbuf_t *body;
	wtk_request_state state;
	wtk_request_method method;

    wtk_strbuf_t *tmp_buf;				//used for save temp header;
    wtk_string_t key;					//used for save key value;

	//header section.
	wtk_string_t url;
	wtk_string_t params;
	wtk_string_t err;
	wtk_string_t *global_streamid;
	wtk_string_t *local_streamid;
	wtk_string_t *strhook;
	wtk_string_t *client;

	wtk_string_t *content_type;
	wtk_string_t *audio_tag;
	wtk_string_t *script;
	wtk_string_t *log;			//use for log request route by httpa->proxy->http
	wtk_strbuf_t *times;
    wtk_string_t *user_agent;   //use for send back http pkt to client 
	long content_length;
	void *data;					//used for attach some data with request,used for proxy.
	unsigned audio_type:4;		//wtk_audio_type_t;
	unsigned keep_alive:1;
    unsigned http1_1:1;
    //unsigned hook:1;
	unsigned is_stream:1;
	unsigned stream_eof:1;
	unsigned speech:1;
};

void wtk_request_set_str_to_content_type_f_g(wtk_request_str_to_content_type_f f);
wtk_request_t* wtk_request_new(wtk_request_cfg_t *cfg);
int wtk_request_delete(wtk_request_t *r);
int wtk_request_bytes(wtk_request_t *r);
int wtk_request_init(wtk_request_t *r,struct wtk_connection* c);
int wtk_request_reset(wtk_request_t *r);
int wtk_request_process_hdr(wtk_request_t *r,char* buf,int len,int *left);
int wtk_request_process_body(wtk_request_t *r,char* buf,int len,int *left);
void wtk_request_print(wtk_request_t *r);
void wtk_request_print_hdr(wtk_request_t *r);
int wtk_request_feed_back(wtk_request_t *r,int status,char *b,int bytes);
int wtk_request_is_valid(wtk_request_t *r);

/**
 * @brief request header to bin;
 */
void wtk_request_tobin(wtk_request_t *r,wtk_strbuf_t *buf,wtk_string_t *host,int hook);

/**
 * @brief request header and body to bin;
 */
void wtk_request_tobin2(wtk_request_t *r,wtk_strbuf_t *buf,wtk_string_t *host,int hook);
int wtk_request_reply(wtk_request_t *r);
void wtk_request_touch(wtk_request_t *r,char *s,int bytes);
void wtk_request_detach_body(wtk_request_t *r);

/**
 * @brief copy data from heap;
 */
void wtk_request_set_response_body(wtk_request_t *r,const char *data,int bytes);
void wtk_request_set_response_audio_url(wtk_request_t *r,const char *audio_url,int bytes);
void wtk_request_set_response_hdr(wtk_request_t *r,char *k,int k_len,char *v,int v_len);
void wtk_request_set_string(wtk_request_t *r,wtk_string_t *str,char *data,int bytes);
//void wtk_request_dup_result(wtk_request_t *r,wtk_param_t *src);
int wtk_request_is_speech(wtk_request_t *request);

//------------------------------ hdr parse section ----------------------
int wtk_request_update_content_length(wtk_request_t *r,char *v,int v_len);
int wtk_request_update_content_type(wtk_request_t *r,char *v,int v_len);
int wtk_request_update_connection(wtk_request_t *r,char *v,int v_len);
int wtk_request_update_stream_id(wtk_request_t *r,char *v,int v_len);
int wtk_request_update_stream_mode(wtk_request_t *r,char *v,int v_len);
int wtk_request_update_script(wtk_request_t *r,char *v,int v_len);
int wtk_request_update_params(wtk_request_t *r,char *v,int v_len);
int wtk_request_update_audio_tag(wtk_request_t *r,char *v,int v_len);
int wtk_request_update_hook(wtk_request_t *r,char *v,int v_len);
int wtk_request_update_log(wtk_request_t *r,char *v,int v_len);
int wtk_request_update_user_agent(wtk_request_t *r, char *v, int v_len);

//---------------------------- used for update request from Http response -------
void wtk_request_update_response_from_http_response(wtk_request_t *req,wtk_http_response_t *response);

/**
 * @brief set response of request from uplayer response;
 */
void wtk_request_update_response(wtk_request_t *request,wtk_http_response_t *response);
void wtk_request_set_disconnect_err(wtk_request_t *req);
int wtk_request_redirect(wtk_request_t *req,int fd,wtk_strbuf_t *buf);
int wtk_request_update_client(wtk_request_t *r,char *v,int v_len);
#ifdef __cplusplus
};
#endif
#endif
