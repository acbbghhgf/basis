#ifndef WTK_OS_WTK_CPU_H_
#define WTK_OS_WTK_CPU_H_
#include "wtk/core/wtk_os.h"
#include "wtk/core/cfg/wtk_source.h"
#include "wtk/core/wtk_stack.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_cpu wtk_cpu_t;
#define WTK_CPU_SLOT 9
//typedef unsigned long long ullong;
//typedef double ullong;
typedef enum
{
	wtk_cpu_user=0,
	wtk_cpu_nice=1,
	wtk_cpu_system=2,
	wtk_cpu_idle=3,
	wtk_cpu_iowait=4,
	wtk_cpu_irq=5,
	wtk_cpu_softirq=6,
	wtk_cpu_stead=7,
	wtk_cpu_guest=8,
}wtk_cpu_index_t;
extern wtk_string_t cpu_names[];

/*
//cpu  11486564 27488 2668210 54539206 595774 186 145060 0 0 0

	ullong user;
	ullong nice;
	ullong system;
	ullong idle;
	ullong iowait;
	ullong irq;
	ullong softirq;
	ullong stead; //Since Linux 2.6.11, there is an eighth column, steal - stolen time, which is the time spent in other operating systems when running in a virtualized environment
	ullong guest; //Since Linux 2.6.24, there is a ninth column, guest, which is the time spent running a virtual CPU for guest operating systems under the control of the Linux kernel
*/
struct wtk_cpu
{
	double last_times[WTK_CPU_SLOT];
	double cur_times[WTK_CPU_SLOT];
	double diff[WTK_CPU_SLOT];
	double rate[WTK_CPU_SLOT];
	double tot;
	int ncpu;
	int index;		//used for count shot count;
	wtk_strbuf_t *buf;
	wtk_source_t src;
};

wtk_string_t* wtk_cpu_get_name(wtk_cpu_index_t i);
wtk_cpu_t* wtk_cpu_new();
int wtk_cpu_delete(wtk_cpu_t *c);
int wtk_cpu_update(wtk_cpu_t *c);
float wtk_cpu_rate(wtk_cpu_t *c);
void wtk_cpu_to_string(wtk_cpu_t *cpu,wtk_strbuf_t *buf);
void wtk_cpu_to_string2(wtk_cpu_t *cpu,wtk_stack_t *stack);

/**
 * @biref cfile format: a=b;c=d;
 */
void wtk_cpu_to_string3(wtk_cpu_t *cpu,wtk_stack_t *stack);
void wtk_cpu_print(wtk_cpu_t *c);
int wtk_cpu_shot(wtk_source_t *p,wtk_strbuf_t *buf,double *v);
int wtk_cpu_shot2(wtk_source_t *p,wtk_strbuf_t *buf,double *v);

/**
 * @brief get the number of cpu;
 */
int wtk_get_cpus();

/**
 * @brief get frequence of cpu in GHZ; if Hyper-Threading is
 */
float wtk_cpu_get_frequence();

/**
 *	@brief hyber threading is enable or not.
 */
int wtk_cpu_ht_is_enable();

int wtk_get_phy_cpus();
#ifdef __cplusplus
};
#endif
#endif
