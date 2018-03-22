#include "main.h"

extern float 	ecg[nsample];
extern int 	index_in;
extern pthread_mutex_t secg;
extern pthread_mutex_t sqrs;
extern int	sync_calcolo, sync_grafico;
extern pthread_mutex_t	sync_mutex;
void 	*task_ecg(void * arg);
void	read_value();
void	new_data(int s1, int s2);
void	file_open();
