#ifndef WTK_CORE_WTK_MEDIAN_FILTER
#define WTK_CORE_WTK_MEDIAN_FILTER
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_fwin.h"
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_sort.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_median_filter wtk_median_filter_t;
struct wtk_median_filter
{
	short *v;
	short *s;
	int idx;
	int win;
};

wtk_median_filter_t* wtk_median_filter_new(int window);
void wtk_median_filter_delete(wtk_median_filter_t *m);
void wtk_median_filter_feed(wtk_median_filter_t *m,short *v,int n,short *dst);

void wtk_median_filter_process(short *input,short *output,int len,int win);
#ifdef __cplusplus
};
#endif
#endif
