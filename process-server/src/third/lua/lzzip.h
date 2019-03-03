#ifndef __LZZIP_H__
#define __LZZIP_H__

#include <stdio.h>
#include <zzip/lib.h>

int luaZ_global_init();
int luaZ_global_clean();
int luaZ_open(const char *path);
int luaZ_close();
int luaZ_readable(const char *filename);
FILE* luaZ_openfile(const char *filename);

#endif
