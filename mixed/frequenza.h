#include "main.h"
extern BITMAP	*freq;
extern pthread_t 	freq_id;
extern struct task_param	freq_param;
extern struct sched_param	freq_par;
extern pthread_attr_t 	freq_attr;
extern pthread_mutex_t	sqrs;
void 	deseases_report(int frequenza_registrata);
void 	*task_frequenza(void * arg);
void	print_freq(int value);
void	read_counter();
