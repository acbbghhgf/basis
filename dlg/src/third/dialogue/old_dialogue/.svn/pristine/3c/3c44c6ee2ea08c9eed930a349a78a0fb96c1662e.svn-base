#ifndef WTK_CORE_WTK_QUEUE_SLOT_H_
#define WTK_CORE_WTK_QUEUE_SLOT_H_
#include "wtk/core/wtk_queue.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_queue_slot wtk_queue_slot_t;

struct wtk_queue_slot
{
    void *data;
    wtk_queue_node_t q;
};

wtk_queue_slot_t* wtk_queue_slot_new(void *data);
int wtk_queue_slot_delete(wtk_queue_slot_t *s,wtk_delete_handler_t dispose);
#ifdef __cplusplus
};
#endif
#endif
