#ifndef WTK_CORE_WTK_TXTREE_H_
#define WTK_CORE_WTK_TXTREE_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_queue2.h"
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_str_encode.h"
#include "wtk/core/cfg/wtk_source.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_txtree wtk_txtree_t;
#define wtk_tnode_is_text_in_s(p,txt) wtk_tnode_is_text_in(p,txt,sizeof(txt)-1)

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_queue2_t child;			//wtk_tonde_t queue;
	char data[3];	//utf-8;
	unsigned len:7;
	unsigned is_leaf:1;
}wtk_tnode_t;

struct wtk_txtree
{
	wtk_tnode_t *root;
	wtk_heap_t *heap;
};

wtk_txtree_t* wtk_txtree_new();
void wtk_txtree_delete(wtk_txtree_t *tree);
wtk_tnode_t* wtk_tnode_find_child(wtk_tnode_t *p,char *data,int bytes);
void wtk_txtree_add_text(wtk_txtree_t *t,char *txt,int bytes);

/**
 * @return 0 on failed, 1 for text in, and -1 for match the last word;
 */
int wtk_tnode_is_text_in(wtk_txtree_t *p,char *txt,int bytes);
int wtk_txtree_load(wtk_txtree_t *t,wtk_source_t *src);
int wtk_txtree_load_file(wtk_txtree_t *t,char *fn);
#ifdef __cplusplus
};
#endif
#endif
