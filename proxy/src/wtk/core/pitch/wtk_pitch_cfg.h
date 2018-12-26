#ifndef WTK_VITE_PITCH_WTK_PITCH_CFG
#define WTK_VITE_PITCH_WTK_PITCH_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
//#include "wtk/vite/f0/wtk_f0.h"
#include <math.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_pitch_cfg wtk_pitch_cfg_t;
struct wtk_pitch_cfg
{
	int sample_rate;
	int fft_frame_size;	//1024 pow 2
	int over_sampling;	//4 - 32
	int max_v;
	int step_size;
	int latency;
	float thresh;
	float scale;
	float expct;
};

int wtk_pitch_cfg_init(wtk_pitch_cfg_t *cfg);
int wtk_pitch_cfg_clean(wtk_pitch_cfg_t *cfg);
int wtk_pitch_cfg_update_local(wtk_pitch_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_pitch_cfg_update(wtk_pitch_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
