#ifndef WTK_NK_POLL_WTK_POLL_EVT
#define WTK_NK_POLL_WTK_POLL_EVT
#include "wtk/core/wtk_type.h" 
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_poll_evt wtk_poll_evt_t;

typedef enum
{
	WTK_CONNECT_EVT_READ=1,
	WTK_CONNECT_EVT_WRITE=2,
	WTK_CONNECT_EVT_ACCEPT=4,
}wtk_connect_evt_type_t;

struct wtk_poll_evt
{
	unsigned ref:1;
	unsigned polled:1;
	unsigned want_read:1;
	unsigned want_write:1;
	unsigned want_accept:1;

	unsigned readpolled:1;
	unsigned writepolled:1;
	unsigned writepending:1;

	unsigned read:1;
	unsigned write:1;
	unsigned error:1;
	unsigned eof:1;
	unsigned reof:1;

	unsigned want_close:1;
	unsigned use_ext_handler:1;
};

void wtk_poll_evt_init(wtk_poll_evt_t *evt,int evt_type);
void wtk_poll_evt_print(wtk_poll_evt_t *evt);
#ifdef __cplusplus
};
#endif
#endif
