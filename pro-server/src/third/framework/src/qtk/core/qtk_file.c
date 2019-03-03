#include "qtk_file.h"
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_os.h"


#define FILE_STATE_NOERR 0
#define FILE_STATE_END -1
#define FILE_STATE_ERR -2


static int file_state(FILE* file)
{
    int state;

    if (feof(file))
    {
        state = FILE_STATE_END;
    }
    else if (ferror(file))
    {
        state = FILE_STATE_ERR;
    }
    else
    {
        state = FILE_STATE_NOERR;
    }
    return state;
}


int qtk_remove_dir(const char *dir_path) {
    int ret = -1;
    DIR* dirp = opendir(dir_path);
    char sub_path[512];
    char *p;
    int len;
    if (!dirp) {
        return -1;
    }
    struct dirent *dir;
    struct stat st;
    while ((dir = readdir(dirp)) != NULL) {
        if (strcmp(dir->d_name,".") == 0
           || strcmp(dir->d_name,"..") == 0) {
            continue;
        }
        len = strlen(dir_path) + strlen(dir->d_name) + 1;
        if (len >= sizeof(sub_path)) {
            p = wtk_malloc(len);
        } else {
            p = sub_path;
        }
        sprintf(p, "%s/%s", dir_path, dir->d_name);
        if (lstat(p, &st) == -1) {
            wtk_debug("rm_dir:lstat %s error", p);
        } else if (S_ISDIR(st.st_mode)) {
            if(qtk_remove_dir(p) == -1) // 如果是目录文件，递归删除
            {
                if (p != sub_path) wtk_free(p);
                goto end;
            }
            rmdir(p);
        } else if(S_ISREG(st.st_mode)) {
            unlink(p);     // 如果是普通文件，则unlink
        } else {
            wtk_debug("rm_dir:st_mode %s error", p);
        }
        if (p != sub_path) {
            wtk_free(p);
            p = sub_path;
        }
    }
    if(rmdir(dir_path) == -1)
    {
        goto end;
    }
    ret = 0;
end:
    closedir(dirp);
    return ret;
}


int qtk_remove_file(const char *file) {
    return unlink(file);
}


int qtk_write_file(FILE* file, const char* data, int len, int* writed)
{
    int ret = 0, index = 0, nLeft = len;

    if (!data)
    {
        return -1;
    }
    while (nLeft > 0)
    {
        ret = fwrite(&data[index], 1, nLeft, file);
        if (ret < nLeft)
        {
            ret = file_state(file);
            if (ret != FILE_STATE_NOERR)
            {
                break;
            }
        }
        index += ret;
        nLeft -= ret;
    }
    if (writed)
    {
        *writed = index;
    }
    return ret;
}


char* qtk_read_file_to_heap(const char* fn, int offset, int *n, wtk_heap_t *heap) {
    FILE* file = fopen(fn, "rb");
    char* p = NULL;
    int len;

    if (NULL == file) {
        wtk_debug("cannot open %s\r\n", fn);
        return NULL;
    }
    len=file_length(file) - offset;
    len -= offset;
    if (len <= 0) {
        wtk_debug("offset [%d] overflow\r\n", *n);
        goto end;
    }
    if (n && *n > 0) len = min(len, *n);
    fseek(file, offset, SEEK_SET);
    p = (char*) wtk_heap_malloc(heap, len);
    len = fread(p, 1, len, file);
    if (n) *n = len;
end:
    fclose(file);
    return p;
}


uint64_t qtk_file_length(const char *fn) {
    FILE* file = fopen(fn, "r");
    uint64_t len;
    if (NULL == file) {
        return 0;
    }
    len = file_length(file);
    fclose(file);
    return len;
}
