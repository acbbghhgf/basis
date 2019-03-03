#include "wtk_queue_slot.h"

wtk_queue_slot_t* wtk_queue_slot_new(void *data)
{
    wtk_queue_slot_t *s;

    s=(wtk_queue_slot_t*)wtk_malloc(sizeof(*s));
    s->data=data;
    return s;
}

int wtk_queue_slot_delete(wtk_queue_slot_t *s,wtk_delete_handler_t dispose)
{
    if(dispose)
    {
        dispose(s->data);
    }
    wtk_free(s);
    return 0;
}
