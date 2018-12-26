#ifndef WTK_VITE_MATH_WTK_GUASSRAND_H_
#define WTK_VITE_MATH_WTK_GUASSRAND_H_
#include <math.h>
#include "wtk/core/wtk_type.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_guassrand wtk_guassrand_t;

struct wtk_guassrand
{
	float V1;
	float V2;
	float S;
	int phase;
};

void wtk_guassrand_init(wtk_guassrand_t *r);
void wtk_guassrand_clean(wtk_guassrand_t *r);
void wtk_guassrand_reset(wtk_guassrand_t *r);
float wtk_guassrand_rand(wtk_guassrand_t *r,float mean,float delta);
#ifdef __cplusplus
};
#endif
#endif
