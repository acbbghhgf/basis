#ifndef WTK_OS_PS_WTK_PSYS_H_
#define WTK_OS_PS_WTK_PSYS_H_
#include "wtk/core/wtk_type.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_psys wtk_psys_t;
struct wtk_psys
{
	int parent_write_fd[2];	//0 for read and 1 for write
	int parent_read_fd[2];
	int pid;
};

int wtk_psys_init(wtk_psys_t *sys,char *cmd);
void wtk_psys_clean(wtk_psys_t *sys);

/*
 * @return write bytes;
 */
int wtk_psys_write(wtk_psys_t *sys,char *data,int bytes);

/**
 * @return read bytes;
 */
int wtk_psys_read(wtk_psys_t *sys,char *data,int bytes);
#ifdef __cplusplus
};
#endif
#endif
