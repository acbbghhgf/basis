#ifndef WTK_CORE_MATH_WTK_LMS
#define WTK_CORE_MATH_WTK_LMS
#include "wtk/core/wtk_type.h" 
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lms wtk_lms_t;
struct wtk_lms
{
	int frame_size;
	float rate;
	short *x;
	float *win;
};

wtk_lms_t* wtk_lms_new(int frame_size,float rate);
void wtk_lms_delete(wtk_lms_t *lms);
void wtk_lms_process(wtk_lms_t *lms,short *mic,int mic_len,short *sp,int sp_len);
#ifdef __cplusplus
};
#endif
#endif
