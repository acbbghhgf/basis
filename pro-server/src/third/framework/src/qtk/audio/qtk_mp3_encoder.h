#ifndef QTK_AUDIO_QTK_MP3_ENCODER_H_
#define QTK_AUDIO_QTK_MP3_ENCODER_H_
#include "wtk/core/wtk_stack.h"
#ifdef __cplusplus
extern "C" {
#endif

struct lame_global_struct;

typedef struct qtk_mp3encoder qtk_mp3encoder_t;
typedef int (*qtk_mp3enc_ready_f)(void *udata, char *buf, int len, int is_end);

struct qtk_mp3encoder {
    struct lame_global_struct *lame_gf;
    unsigned char *out_buf;
    int pos;
    int bufsize;
    qtk_mp3enc_ready_f handler;
    void *target;
};

int qtk_mp3enc_init(qtk_mp3encoder_t *enc, int bufsize);
int qtk_mp3enc_feed(qtk_mp3encoder_t *enc, char *buf, int len);
int qtk_mp3enc_reset(qtk_mp3encoder_t *enc);
int qtk_mp3enc_clean(qtk_mp3encoder_t *enc);
int qtk_mp3enc_set_notifier(qtk_mp3encoder_t *enc, qtk_mp3enc_ready_f handler, void* udata);

#ifdef __cplusplus
};
#endif

#endif
