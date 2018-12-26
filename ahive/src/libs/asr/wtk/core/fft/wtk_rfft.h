#ifndef WTK_DLG_AUDIO_DSB_WTK_RFFT
#define WTK_DLG_AUDIO_DSB_WTK_RFFT
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_larray.h"
#include "wtk/core/wtk_str.h"
#include "wtk/core/math/wtk_mat.h"
#include "wtk/core/math/wtk_math.h"
#include "wtk/core/wtk_complex.h"
#include "wtk/core/wtk_kcls.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_rfft wtk_rfft_t;

#define TRIGO_BD_LIMIT 12

typedef struct
{
	float pos_cos;// Current phase expressed with sin and cos. [-1 ; 1]
	float pos_sin;// -
	float step_cos;// Phase increment per step, [-1 ; 1]
	float step_sin;// -
}wtk_oscsincos_t;


struct wtk_rfft
{
	int win;
	int len;
	int nbr_bits;
	int *br_lut;
	float *trigo_lut;
	float *buffer;
	wtk_oscsincos_t *trigo_osc;
};

int wtk_rfft_next_pow (long x);

/**
 * len=length(data)/2;
 */
wtk_rfft_t* wtk_rfft_new(int len);
void wtk_rfft_delete(wtk_rfft_t *rf);
void wtk_rfft_process_fft(wtk_rfft_t *rf,float *f,float *x);
void wtk_rfft_process_ifft(wtk_rfft_t *rf,float *f,float *x);

/**
 *  fft2 在　fft1 中的位置与偏移;
 */
//fft,ref_fft
void wtk_xcorr_freq_mult(float *fft1,float *fft2,float *xy,int fft_size);

//vs values float array
//ps values int array
void wtk_xcorr_find_nbest_values(wtk_larray_t *vs,wtk_larray_t *ps,float *xcorr_value,int nx,int margin,int *delays,float *values,int nbest);

void wtk_rfft_nbest_xcorr_int(int *mic,int mic_len,int *sp,int sp_len,int margin,int *delays,int nbest);
void wtk_rfft_nbest_xcorr_float(float *mic,int mic_len,float *sp,int sp_len,int margin,int *delays,int nbest);
void wtk_rfft_nbest_xcorr(char *mic,int mic_len,char *sp,int sp_len,int margin,int *delays,int nbest);
void wtk_rfft_nbest_xcorr2(char *mic,int mic_len,char *sp,int sp_len,int *delays,int nbest);
void wtk_rfft_nbest_xcorr3(int channel,char **mic,int mic_len,char *sp,int sp_len,int *delays);
void wtk_rfft_nbest_xcorr4(int channel,char **mic,int mic_len,char *sp,int sp_len,int nbest,int *nbest_delays);
void wtk_rfft_nbest_xcorr_mult_channel(int channel,char **mic,int mic_len,char *sp,int sp_len,int *delays);

int wtk_rfft_xcorr(char *mic,int mic_len,char *sp,int sp_len);
int wtk_rfft_xcorr2(char *mic,int mic_len,char *sp,int sp_len);
float wtk_rfft_xcorr3(char *mic,int mic_len,char *sp,int sp_len,int margin,float *max_v);

void wtk_rfft_print_fft(float *f,int n);
wtk_complex_t wtk_rfft_get_value(float *f,int n,int idx);
void wtk_rfft_set_value(float *f,int n,int idx,wtk_complex_t c);
void wtk_rfft_set_value2(float *f,int n,int idx,float a,float b);

int wtk_int_max(int *delay,int n);
int wtk_int_min(int *delay,int n);

//do the multiplication: (a+jb)(c+jd) = (ac-bd)+j(bc+ad);
void wtk_rfft_mult(float *fft1,float *fft2,float *xy,int win);

//do the multiplication: (a+jb)'(c+jd) =(a-jb)(c+jd)=(ac+bd)+j(ad-bc);
void wtk_rfft_mult2(float *fft1,float *fft2,float *xy,int win);

//do the multiplication: (a+jb).conj(c+jd) =(a+jb)(c-jd)=(ac+bd)+j(-ad+bc);
void wtk_rfft_mult3(register float *fft1,register float *fft2,float *xy,int win);
#ifdef __cplusplus
};
#endif
#endif
