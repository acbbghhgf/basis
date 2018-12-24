#ifndef MAIN_QTK_BIFROST_H_
#define MAIN_QTK_BIFROST_H_
#ifdef __cplusplus
extern "C" {
#endif

/* message from timer */
#define    QTK_TIMER        0
#define    QTK_ERROR        1
#define    QTK_SYS          2
#define    QTK_LUA          3
#define    QTK_RET          4

/* just used for delivery param to qtk_bifrost_send
 * if QTK_TAG_DONOTCOPY setted, caller must free data
 * after qtk_bifrost_send manually */
#define    QTK_TAG_ALLOCSESSION     0x100
#define    QTK_TAG_DONOTCOPY        0x200
#define BUF_SIZE    128

#ifdef __cplusplus
};
#endif
#endif
