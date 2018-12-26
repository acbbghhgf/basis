#include "lzzip.h"

#include <zzip/plugin.h>
#include <zzip/zzip.h>
#include <unistd.h>

static zzip_ssize_t xor_read (int f, void* p, zzip_size_t l)
{
    int xor_value = 250;
    zzip_ssize_t r = read(f, p, l);
    zzip_ssize_t x; char* q; for (x=0, q=p; x < r; x++) q[x] ^= xor_value;
    return r;
}

ZZIP_DIR *luaQ_opendir(const char *path)
{
    static zzip_plugin_io_handlers xor_handlers;
    static zzip_strings_t xor_fileext[] = { ".bin", ".BIN", "", 0 };
    zzip_init_io(&xor_handlers, 0);
    xor_handlers.fd.read = xor_read;
    return zzip_dir_open_ext_io(path, NULL, xor_fileext, &xor_handlers);
}

int luaQ_closedir(ZZIP_DIR *dir)
{
    return zzip_dir_close(dir);
}

int luaQ_dreadable(const char *path)
{
    ZZIP_DIR *dir = luaQ_opendir(path);
    if (NULL == dir) return 0;
    zzip_dir_close(dir);
    return 1;
}

int luaQ_freadable(ZZIP_DIR *dir, const char *filename)
{
    ZZIP_FILE *fp = zzip_file_open(dir, filename, O_RDONLY);
    if (NULL == fp) {
        return 0;
    }
    zzip_file_close(fp);
    return 1;
}
