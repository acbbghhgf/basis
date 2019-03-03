#ifndef WTK_CORE_FFT_WTK_STFT_CFG
#define WTK_CORE_FFT_WTK_STFT_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_stft_cfg wtk_stft_cfg_t;
struct wtk_stft_cfg
{
	int channel;
	int win;
	int step;
	float overlap;
	unsigned use_sine:1;
	unsigned use_hann:1;
	unsigned use_hamming:1;
	unsigned use_fftscale:1;
	unsigned keep_win:1;
};

int wtk_stft_cfg_init(wtk_stft_cfg_t *cfg);
int wtk_stft_cfg_clean(wtk_stft_cfg_t *cfg);
int wtk_stft_cfg_update_local(wtk_stft_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_stft_cfg_update(wtk_stft_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
