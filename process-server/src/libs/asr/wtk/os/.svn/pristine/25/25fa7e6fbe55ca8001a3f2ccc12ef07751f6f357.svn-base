#ifndef WTK_OS_WTK_PROC_H_
#define WTK_OS_WTK_PROC_H_
#include "wtk/core/wtk_type.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_statm
{
	int size;
	int resident;
	int share;
	int text;
	int lib;
	int data;
	int dt;
}wtk_statm_t;

int wtk_statm_init(wtk_statm_t *m);
void wtk_statm_print(wtk_statm_t *m);
void wtk_print_statm();
int wtk_proc_get_dir(char* buf,int len);
int wtk_get_host_free_mem(double *v);
void wtk_proc_msleep(int ms);
void wtk_proc_dump_stack();
void wtk_proc_dump_stack2();
#ifndef W3IN32
double wtk_proc_mem();
void wtk_debug_mem();
#endif

#ifdef __cplusplus
};
#endif
#endif
