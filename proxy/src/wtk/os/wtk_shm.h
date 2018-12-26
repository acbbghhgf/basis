#ifndef WTK_OS_WTK_SHM_H_
#define WTK_OS_WTK_SHM_H_
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "wtk/core/wtk_type.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_shm wtk_shm_t;

struct wtk_shm
{
	char *addr;
	int id;
};

wtk_shm_t* wtk_shm_new(int bytes);
void wtk_shm_delete(wtk_shm_t *s);
#ifdef __cplusplus
};
#endif
#endif
