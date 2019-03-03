#ifndef WTK_VITE_PITCH_WTK_PITCH
#define WTK_VITE_PITCH_WTK_PITCH
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk/core/wtk_larray.h"
#include "wtk/core/wtk_strbuf.h"
#include "wtk_pitch_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_pitch wtk_pitch_t;

typedef void(*wtk_pitch_noityf_f)(void *ths,char *data,int bytes);


struct wtk_pitch
{
	wtk_pitch_cfg_t *cfg;
	wtk_flta_t *in;
	wtk_flta_t *out;
	wtk_flta_t *in_fifo;
	wtk_flta_t *out_fifo;
	wtk_flta_t *fft_worksp;
	wtk_flta_t *last_phase;
	wtk_flta_t *sum_phase;
	wtk_flta_t *output_accum;
	wtk_flta_t *ana_freq;
	wtk_flta_t *ana_magn;
	wtk_flta_t *syn_freq;
	wtk_flta_t *syn_magn;
	int rover;
	wtk_strbuf_t *buf;
	double f1;
	double f2;
	double f3;
	double f4;
	void* notify_ths;
	wtk_pitch_noityf_f notify;
};

wtk_pitch_t* wtk_pitch_new(wtk_pitch_cfg_t *cfg);
void wtk_pitch_delete(wtk_pitch_t *p);
void wtk_pitch_reset(wtk_pitch_t *p);

void wtk_pitch_set(wtk_pitch_t *p,void *ths,wtk_pitch_noityf_f notify);

/*
	bytes must be pow 2
*/
void wtk_pitch_process(wtk_pitch_t *p,float pitch_shift,char *data,int bytes);

int wtk_pitch_convert(wtk_pitch_t *p,float shift,char *in,char *out);
#ifdef __cplusplus
};
#endif
#endif
