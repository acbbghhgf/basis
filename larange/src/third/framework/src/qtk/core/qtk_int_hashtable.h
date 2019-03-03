#ifndef _QTK_CORE_QTK_INT_HASHTABLE_H
#define _QTK_CORE_QTK_INT_HASHTABLE_H
#include "qtk_abstruct_hashtable.h"

#ifdef __cplusplus
extern "C" {
#endif

qtk_abstruct_hashtable_t* qtk_int_hashtable_new(int nslot, int offset);

qtk_abstruct_hashtable_t* qtk_int_growhash_new(int nslot, int offset);

int qtk_int_hashtable_test();


#ifdef __cplusplus
}
#endif

#endif
