#ifndef WTK_CORE_FFT_WTK_QSTFT_CFG
#define WTK_CORE_FFT_WTK_QSTFT_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk_stft.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_qstft_cfg wtk_qstft_cfg_t;
struct wtk_qstft_cfg
{
	int lt; //
	int lf; //half-width of the frequency neighborhood for the computation of empirical covariance
};

int wtk_qstft_cfg_init(wtk_qstft_cfg_t *cfg);
int wtk_qstft_cfg_clean(wtk_qstft_cfg_t *cfg);
int wtk_qstft_cfg_update_local(wtk_qstft_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_qstft_cfg_update(wtk_qstft_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
