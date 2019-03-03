#include "wtk_pitch_cfg.h" 
#include <math.h>

int wtk_pitch_cfg_init(wtk_pitch_cfg_t *cfg)
{
	cfg->sample_rate=16000;
	cfg->fft_frame_size=512;
	cfg->over_sampling=4;
	cfg->max_v=32768;
	cfg->thresh=0.8;
	wtk_pitch_cfg_update(cfg);
	return 0;
}

int wtk_pitch_cfg_clean(wtk_pitch_cfg_t *cfg)
{
	return 0;
}

int wtk_pitch_cfg_update_local(wtk_pitch_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_i(lc,cfg,sample_rate,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,fft_frame_size,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,over_sampling,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,max_v,v);
	wtk_local_cfg_update_cfg_f(lc,cfg,thresh,v);
	return 0;
}

int wtk_pitch_cfg_update(wtk_pitch_cfg_t *cfg)
{
	cfg->scale=1.0/cfg->max_v;
	cfg->step_size=cfg->fft_frame_size/cfg->over_sampling;
	cfg->latency=cfg->fft_frame_size-cfg->step_size;
	cfg->expct=(2.0*M_PI*cfg->step_size)/cfg->fft_frame_size;
	return 0;
}
