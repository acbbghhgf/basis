#include "qtk_thread.h"
#include "qtk_base.h"

int qtk_thread_init(qtk_thread_t *thread, qtk_thread_routine routine, void *data)
{
    thread->routine = routine;
    thread->data = data;
    return 0;
}


int qtk_thread_start(qtk_thread_t *thread)
{
    int ret = -1;

    ret = pthread_create(&thread->tid, NULL, thread->routine, thread->data);
    if (ret != 0) {
        qtk_debug("error pthread_create\n");
        goto end;
    }

    ret = 0;
end:
    return ret;
}


int qtk_thread_join(qtk_thread_t *thread)
{
    pthread_join(thread->tid, NULL);
    return 0;
}
