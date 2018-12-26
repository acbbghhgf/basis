#ifndef WTK_NK_WTK_NK_EVENT_H_
#define WTK_NK_WTK_NK_EVENT_H_
#include "wtk/core/wtk_queue.h"
#include "wtk/os/wtk_pipequeue.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_nk_event wtk_nk_event_t;
typedef enum
{
	WTK_NK_REQUEST,
	WTK_NK_CON_CLOSE,
}wtk_nk_event_type_t;

struct wtk_nk_event
{
	wtk_nk_event_type_t type;
	wtk_queue_node_t pipe_n;
	wtk_pipequeue_t *pipe_queue;
	void *data;					//WTK_NK_REQUEST -> wtk_request_t;WTK_NK_CON_CLOSE -> wtk_http_parser;
	unsigned used:1;
};

int wtk_nk_event_send_back(wtk_nk_event_t *nke);
#ifdef __cplusplus
};
#endif
#endif
