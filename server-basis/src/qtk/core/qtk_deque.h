#ifndef QTK_UTIL_QTK_DEQUE_H
#define QTK_UTIL_QTK_DEQUE_H
#include "wtk/core/wtk_type.h"
#include <stdio.h>
#include "wtk/core/wtk_alloc.h"
#include "wtk/core/wtk_queue.h"
#include "wtk/core/wtk_os.h"
#include "wtk/core/wtk_strbuf.h"

#ifdef __cplusplus
extern "C" {
#endif

#define qtk_deque_push_s(s,msg) qtk_deque_push(s,msg,sizeof(msg)-1)

struct deque_block;

struct qtk_deque
{
        wtk_queue_node_t            q;
        uint32_t                last_alloc_size;
        uint32_t		max_size;
        uint32_t		len;
        float			growf;
        struct deque_block	*pop;
        struct deque_block	*push;
        struct deque_block	*end;
};
typedef struct qtk_deque qtk_deque_t;

/**
 * @brief allocate stack.
 */
qtk_deque_t* qtk_deque_new(int init_size,int max_size,float growf);

int qtk_deque_bytes(qtk_deque_t *s);
/**
 * @brief init stack.
 */
int qtk_deque_init(qtk_deque_t *s,int init_size,int max_size,float growf);

/**
 * @brief clone deque from src. Please keep dst is space memory
 */
int qtk_deque_clone(qtk_deque_t *dst, qtk_deque_t* src);

/**
 * @brief dispose stack.
 */
int qtk_deque_delete(qtk_deque_t* s);

/**
 * @brief clean stack.
 */
int qtk_deque_clean(qtk_deque_t *s);

/**
 * @brief reset stack.
 */
int qtk_deque_reset(qtk_deque_t *s);

/**
 * @brief push data.
 */
int qtk_deque_push(qtk_deque_t* s,const char* data,int len);

int qtk_deque_push_front(qtk_deque_t* s,const char* data,size_t len);

void qtk_deque_push_f(qtk_deque_t *s,const char *fmt,...);
/**
 * @brief push string.
 */
int qtk_deque_push_string(qtk_deque_t* s,const char* str);

/**
 * @brief pop data
 * @return 0 if pop len bytes else failed;
 */
int qtk_deque_pop(qtk_deque_t* s,char* data,int len);

/**
 * @return copied bytes;
 */
int qtk_deque_pop2(qtk_deque_t* s,char* data,int len);

/**
 * @brief check stack is valid or not.
 */
int qtk_deque_is_valid(qtk_deque_t * s);

void qtk_deque_merge(qtk_deque_t *s,char* p);
void qtk_deque_add(qtk_deque_t *dst,qtk_deque_t *src);
void qtk_deque_copy(qtk_deque_t *s,wtk_strbuf_t *to);
int qtk_deque_write(qtk_deque_t *s,FILE *f);
int qtk_deque_write2(qtk_deque_t *s,char *fn);
int qtk_deque_read(qtk_deque_t *s,char *fn);

/*
  @brief move first len bytes of src to the tail of dst
 */
int qtk_deque_move(qtk_deque_t *dst,qtk_deque_t *src, int len);

/*
  @brief peek first len bytes from dq
 */
int qtk_deque_front(qtk_deque_t *dq, char *data, int len);

/*
  @brief peek last len bytes from dq
 */
int qtk_deque_tail(qtk_deque_t *dq, char *data, int len);

#ifdef __cplusplus
}
#endif

#endif
