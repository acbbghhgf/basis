#ifndef __LZZIP_H__
#define __LZZIP_H__

#include "zzip/zzip.h"

ZZIP_DIR *luaQ_opendir(const char *path);
int luaQ_closedir(ZZIP_DIR *dir);
int luaQ_dreadable(const char *path);
int luaQ_freadable(ZZIP_DIR *dir, const char *filename);

#endif
