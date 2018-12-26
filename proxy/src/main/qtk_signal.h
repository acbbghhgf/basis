#ifndef _MAIN_QTK_SIGNAL_H
#define _MAIN_QTK_SIGNAL_H

#include "os/qtk_spinlock.h"
#include <stdint.h>

typedef struct qtk_signal qtk_signal_t;

struct qtk_signal {
    qtk_spinlock_t lock;
    uint32_t signals;
};

int qtk_signal_init(qtk_signal_t *s);
int qtk_signal_clean(qtk_signal_t *s);
int qtk_signal_dispatch(qtk_signal_t *s);
int qtk_signal_kill(qtk_signal_t *s, int sig);

#endif
