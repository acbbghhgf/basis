#ifndef QTK_IRIS_H
#define QTK_IRIS_H

#include "qtk/sframe/qtk_sframe.h"
#include "qtk_director.h"


struct qtk_director;


typedef struct qtk_iris qtk_iris_t;
struct qtk_iris{
    void *conf;
    void    *s;
    struct event_base *event;
    struct qtk_director *director;
    unsigned    run:1;
};



typedef int (*qtk_iris_process_f)(qtk_iris_t *, qtk_sframe_msg_t *);


int iris_stop(void *data);
int iris_start(void *data);
int iris_delete(void *data);
qtk_iris_t  *iris_new(void *s);

#endif
