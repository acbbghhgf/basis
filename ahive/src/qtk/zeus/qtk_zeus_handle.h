#ifndef QTK_ZEUS_QTK_ZEUS_HANDLE_H
#define QTK_ZEUS_QTK_ZEUS_HANDLE_H
#include "stdint.h"
#include "wtk/os/wtk_lock.h"
#include "qtk/core/qtk_map.h"

#ifdef __cplusplus
extern "C" {
#endif

struct qtk_zcontext;

typedef struct qtk_zhandle qtk_zhandle_t;

struct qtk_zhandle {
    wtk_lock_t lock;

    uint32_t handle_index;
    int slot_cnt;
    int slot_cap;
    struct qtk_zcontext **slots;

    qtk_map_t *name;
};

qtk_zhandle_t* qtk_zhandle_new();
int qtk_zhandle_delete(qtk_zhandle_t* zh);
uint32_t qtk_zhandle_register(qtk_zhandle_t* zh, struct qtk_zcontext*);
int qtk_zhandle_retire(qtk_zhandle_t *zh, uint32_t handle);
int qtk_zhandle_retireall(qtk_zhandle_t *zh);
struct qtk_zcontext* qtk_zhandle_grab(qtk_zhandle_t *zh, uint32_t handle);

uint32_t qtk_zhandle_findname(qtk_zhandle_t *zh, const char *name);
const char* qtk_zhandle_namehandle(qtk_zhandle_t *zh, uint32_t handle, const char *name);


#ifdef __cplusplus
}
#endif


#endif
