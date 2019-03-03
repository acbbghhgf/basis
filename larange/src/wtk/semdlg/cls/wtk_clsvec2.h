#ifndef WTK_SEMDLG_CLS_WTK_CLSVEC2
#define WTK_SEMDLG_CLS_WTK_CLSVEC2
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_heap.h"
#include "wtk/core/math/wtk_sfvec.h"
#include "wtk/core/cfg/wtk_source.h"
#include "wtk/core/wtk_str_parser.h"
#include "wtk/core/math/wtk_vector.h"
#include "wtk/core/math/wtk_mat.h"
#include "wtk/core/wtk_os.h"
#include "wtk/os/wtk_cpu.h"
#include "wtk/os/wtk_thread2.h"
#include "wtk_clsvec_dat.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_clsvec2 wtk_clsvec2_t;

typedef struct
{
	wtk_queue_t item_q;
	wtk_sfvec_t vec;
	wtk_queue_t *thread_q;
}wtk_clsvec2_class_t;

struct wtk_clsvec2
{
	wtk_clsvec_dat_t *dat;
	wtk_heap_t *loc_heap;
	int ncls;
	wtk_clsvec2_class_t **clses;
	wtk_queue_t **list_q;
	int iter;
	int nthread;
	wtk_thread_t *threads;
	wtk_queue_t *thread_q;
	wtk_sem_t *sem;
	wtk_sem_t *notify_sem;
	unsigned run:1;
};


wtk_clsvec2_t* wtk_clsvec2_new(int nthread);
void wtk_clsvec2_delete(wtk_clsvec2_t *v);
void wtk_clsvec2_process(wtk_clsvec2_t *v,char *fn,int ncls);
void wtk_clsvec2_print(wtk_clsvec2_t *v);


#ifdef __cplusplus
};
#endif
#endif
