#ifndef CORE_QTK_BASE_H_
#define CORE_QTK_BASE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <dirent.h>

#define qtk_debug(...)                                  \
do {                                                    \
    printf("%s[%05d]\t", __FUNCTION__, __LINE__);       \
    printf(__VA_ARGS__);                                \
    fflush(stdout);                                     \
} while(0)

#ifdef __cplusplus
};
#endif
#endif
