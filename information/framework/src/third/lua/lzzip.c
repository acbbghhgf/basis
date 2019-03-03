#include "lzzip.h"

#include <zzip/plugin.h>
#include <zzip/file.h>
#include <unistd.h>
#include <pthread.h>

static pthread_key_t key;

static zzip_ssize_t xor_read (int f, void* p, zzip_size_t l) {
    int xor_value = 250;
    zzip_ssize_t r = read(f, p, l);
    zzip_ssize_t x; char* q; for (x=0, q=p; x < r; x++) q[x] ^= xor_value;
    return r;
}

int luaZ_global_init() {
    pthread_key_create(&key, NULL);
    return 0;
}


int luaZ_global_clean() {
    return 0;
}


int luaZ_open(const char *path) {
    static zzip_plugin_io_handlers xor_handlers;
    static zzip_strings_t xor_fileext[] = { ".bin", ".BIN", "", 0 };
    ZZIP_DIR *zdir = (ZZIP_DIR*)pthread_getspecific(key);
    if (zdir) {
        zzip_dir_close(zdir);
        zdir = NULL;
    }
    zzip_init_io(&xor_handlers, 0);
    xor_handlers.fd.read = xor_read;
    zdir = zzip_dir_open_ext_io(path, NULL, xor_fileext, &xor_handlers);
    pthread_setspecific(key, zdir);
    return 0;
}

int luaZ_close() {
    ZZIP_DIR *zdir = (ZZIP_DIR*)pthread_getspecific(key);
    if (zdir) {
        return zzip_dir_close(zdir);
        pthread_setspecific(key, NULL);
    }
    return 0;
}

int luaZ_readable(const char *filename) {
    ZZIP_DIR *zdir = (ZZIP_DIR*)pthread_getspecific(key);
    ZZIP_FILE *fp;
    if (NULL == zdir) {
        return 0;
    }
    fp = zzip_file_open(zdir, filename, O_RDONLY);
    if (NULL == fp) {
        return 0;
    }
    zzip_file_close(fp);
    return 1;
}


FILE* luaZ_openfile(const char *filename) {
    ZZIP_DIR *zdir = (ZZIP_DIR*)pthread_getspecific(key);
    ZZIP_FILE *fp;
    if (NULL == zdir) {
        return NULL;
    }
    fp = zzip_file_open(zdir, filename, O_RDONLY);
    if (fp) {
        FILE* f = tmpfile();
        char buf[1024];
        int len;
        if (f) {
            while ((len = zzip_file_read(fp, buf, sizeof(buf))) > 0) {
                fwrite(buf, 1, len, f);
            }
            fseek(f, 0, SEEK_SET);
        }
        zzip_file_close(fp);
        return f;
    }
    return NULL;
}
