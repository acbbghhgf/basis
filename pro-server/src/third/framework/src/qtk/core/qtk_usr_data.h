#ifndef QTK_CORE_QTK_USR_DATA_H
#define QTK_CORE_QTK_USR_DATA_H

#include    <errno.h>
#include    <stdio.h>
#include    <limits.h>
#include    <stdlib.h>
#include    <string.h>
#include    <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define QTK_USR_DATA \
    void    *usr_data; \
    void    (*ctor)(void *); \
    void    (*dtor)(void *); \

#ifndef qtk_offsetof
#define qtk_offsetof(TYPE, MEMBER)    ((size_t) &((TYPE *)0)->MEMBER)
#endif

#ifndef qtk_data_offset2
#define qtk_data_offset2(q, type, link) (type*)((void*)((char*)q - qtk_offsetof(type, link)))
#endif

typedef struct qtk_usr_data qtk_usr_data_t;
struct qtk_usr_data{
    QTK_USR_DATA
};

typedef void (*qtk_ctor_f)(void *);
typedef void (*qtk_dtor_f)(void *);

static inline void
qtk_update_usr_data(qtk_usr_data_t *up, void *usr_data)
{
    up->usr_data = usr_data;
}

static inline void
qtk_update_usr_handler(qtk_usr_data_t *up, void (*ctor)(void *), void (*dtor)(void *))
{
    if(up->usr_data && ctor)
        ctor(up->usr_data);
    up->ctor = ctor;
    up->dtor = dtor;
}

static inline void
qtk_usr_data_remove(qtk_usr_data_t *up)
{
    if(up->usr_data && up->dtor)
        up->dtor(up->usr_data);
}

#ifdef __cplusplus
};
#endif

#endif
