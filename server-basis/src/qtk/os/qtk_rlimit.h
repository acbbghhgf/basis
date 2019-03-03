#ifndef QTK_OS_RLIMIT_H
#define QTK_OS_RLIMIT_H


#ifdef __cplusplus
extern "C" {
#endif

int qtk_rlimit_set_nofile(int max_files);

#ifdef __cplusplus
}
#endif

#endif
