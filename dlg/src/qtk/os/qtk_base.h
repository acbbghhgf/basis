#ifndef _OS_QTK_BASE_H_
#define _OS_QTK_BASE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#define qtk_debug(...)                                  \
do {                                                    \
    printf("%s[%05d]\t", __FUNCTION__, __LINE__);       \
    printf(__VA_ARGS__);                                \
    fflush(stdout);                                     \
} while(0)

#define cast(t, exp)            ((t)(exp))
#define unuse(v)                ((void)v)

#ifdef __cplusplus
};
#endif
#endif
