#ifndef CORE_QTK_THREAD_H_
#define CORE_QTK_THREAD_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

typedef struct qtk_thread qtk_thread_t;
typedef void *(*qtk_thread_routine)(void *);

struct qtk_thread {
    qtk_thread_routine routine;
    pthread_t tid;
    void *data;
};

int qtk_thread_init(qtk_thread_t *thread, qtk_thread_routine routine, void *data);
int qtk_thread_start(qtk_thread_t *thread);
int qtk_thread_join(qtk_thread_t *thread);

#ifdef __cplusplus
};
#endif
#endif
