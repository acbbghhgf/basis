#ifndef _OS_QTK_LIMIT_H_
#define _OS_QTK_LIMIT_H_
#ifdef __cplusplus
extern "C" {
#endif
int qtk_limit_set_maxfiles(int max_files);
int qtk_limit_set_pipe_sz(int sz, int fd);
#ifdef __cplusplus
};
#endif
#endif
