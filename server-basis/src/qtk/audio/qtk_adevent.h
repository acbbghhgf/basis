#ifndef WTK_AUDIO_WTK_ADEVENT_H_
#define WTK_AUDIO_WTK_ADEVENT_H_
#include "wtk/core/wtk_strbuf.h"
#include "wtk/core/wtk_queue.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct qtk_adevent qtk_adevent_t;
typedef enum
{
	WTK_ADBLOCK_APPEND,
	WTK_ADBLOCK_END,
	WTK_ADSTREAM_END,
}wtk_adstate_t;

struct qtk_adevent
{
	wtk_queue_node_t hoard_n;
	wtk_queue_node_t q_n;
	wtk_adstate_t state;
	wtk_strbuf_t *buf;
};

qtk_adevent_t* qtk_adevent_new(int init_size);
int qtk_adevent_delete(qtk_adevent_t *e);
int qtk_adevent_reset(qtk_adevent_t *e);
int qtk_adevent_push(qtk_adevent_t *e,char *c,int bytes);
#ifdef __cplusplus
};
#endif
#endif
