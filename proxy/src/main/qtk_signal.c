#include "qtk_signal.h"

int qtk_signal_init(qtk_signal_t *s)
{
    QTK_SPIN_INIT(s);
    s->signals = 0;
    return 0;
}

int qtk_signal_clean(qtk_signal_t *s)
{
    QTK_SPIN_DESTROY(s);
    return 0;
}

int qtk_signal_kill(qtk_signal_t *s, int sig)
{
    QTK_SPIN_LOCK(s);
    s->signals |= 1 << sig;
    QTK_SPIN_UNLOCK(s);
    return 0;
}

int qtk_signal_dispatch(qtk_signal_t *s)
{
    QTK_SPIN_LOCK(s);
    int ret =  __builtin_ffs(s->signals);
    s->signals = s->signals & (s->signals - 1);
    QTK_SPIN_UNLOCK(s);
    return ret;
}
