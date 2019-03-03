#ifndef WTK_OS_WTK_LOCKHOARD_H_
#define WTK_OS_WTK_LOCKHOARD_H_
#include "wtk/core/wtk_hoard.h"
#include "wtk/os/wtk_lock.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lockhoard wtk_lockhoard_t;

struct wtk_lockhoard
{
	WTK_HOARD
	wtk_lock_t lock;
};

int wtk_lockhoard_init(wtk_lockhoard_t *h,int offset,int max_free,wtk_new_handler_t newer,wtk_delete_handler_t deleter,void *data);
int wtk_lockhoard_clean(wtk_lockhoard_t *h);
void* wtk_lockhoard_pop(wtk_lockhoard_t *h);
int wtk_lockhoard_push(wtk_lockhoard_t *h,void* data);
#ifdef __cplusplus
};
#endif
#endif
