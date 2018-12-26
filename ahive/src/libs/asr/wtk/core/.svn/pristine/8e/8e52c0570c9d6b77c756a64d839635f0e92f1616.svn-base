#ifndef WTK_CORE_FFT_WTK_QSTFT
#define WTK_CORE_FFT_WTK_QSTFT
#include "wtk/core/wtk_type.h" 
#include "wtk_qstft_cfg.h"
#include "wtk_stft.h"
#include "wtk/core/wtk_robin.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_qstft wtk_qstft_t;

typedef enum
{
	WTK_QSTFT_INIT,
	WTK_QSTFT_UPDATE,
}wtk_qstft_state_t;

typedef void(*wtk_qstft_notify_f)(void *ths,wtk_complex_t ***XX,int is_end);
typedef void(*wtk_qstft_delete_msg_f)(void *ths,wtk_stft_msg_t *msg);

struct wtk_qstft
{
	wtk_qstft_cfg_t *cfg;
	wtk_stft_cfg_t *stft_cfg;
	wtk_qstft_state_t state;
	wtk_robin_t *robin;
	float *winf;
	float *wint;
	wtk_complex_t ***XX;//|channel*channel*nbin|
	wtk_complex_t **xx;
	float *wei;
	int max_ind;
	void *notify_ths;
	wtk_qstft_notify_f notify;
	void *delete_msg_ths;
	wtk_qstft_delete_msg_f delete_msg;
};

wtk_qstft_t* wtk_qstft_new(wtk_qstft_cfg_t *cfg,wtk_stft_cfg_t *stft_cfg);
void wtk_qstft_delete(wtk_qstft_t *qf);
void wtk_qstft_reset(wtk_qstft_t *qf);
void wtk_qstft_set_notify(wtk_qstft_t *qf,void *ths,wtk_qstft_notify_f notify);
void wtk_qstft_set_delete_msg(wtk_qstft_t *qf,void *ths,wtk_qstft_delete_msg_f delete_msg);

void wtk_qstft_feed(wtk_qstft_t *qf,wtk_stft_msg_t *msg,int len,int is_end);
#ifdef __cplusplus
};
#endif
#endif
