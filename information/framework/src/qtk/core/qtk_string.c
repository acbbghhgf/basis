#include "qtk_string.h"
#include <ctype.h>


void qtk_str_tolower(wtk_string_t *str) {
    int i;
    char *data = str->data;
    for (i = 0; i < str->len; i++) {
        data[i] = tolower(data[i]);
    }
}


void qtk_str_toupper(wtk_string_t *str) {
    int i;
    char *data = str->data;
    for (i = 0; i < str->len; i++) {
        data[i] = toupper(data[i]);
    }
}


void qtk_str_tolower2(char *dst, const char *src, int len) {
    int i;
    for (i = 0; i < len; i++) {
        dst[i] = tolower(src[i]);
    }
}


void qtk_str_toupper2(char *dst, const char *src, int len) {
    int i;
    for (i = 0; i < len; i++) {
        dst[i] = toupper(src[i]);
    }
}
