#ifndef _QTK_UTIL_QTK_STR_HASHTABLE_H
#define _QTK_UTIL_QTK_STR_HASHTABLE_H
#include "qtk_abstruct_hashtable.h"

#ifdef __cplusplus
extern "C" {
#endif

/* typedef struct qtk_str_hashtable qtk_str_hashtable_t; */

/* struct qtk_str_hashtable { */

/* }; */

qtk_abstruct_hashtable_t* qtk_str_hashtable_new(int nslot, int offset);

qtk_abstruct_hashtable_t* qtk_str_growhash_new(int nslot, int offset);

int qtk_str_hashtable_test();


#ifdef __cplusplus
}
#endif

#endif
