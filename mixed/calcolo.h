#include "main.h"

#define nsoglia 250 //numero campioni prima del calcolo della soglia
#define ncampioni 4 //campioni per la condizione del picco

extern int	contatore_qrs;
extern int	sync_calcolo;
extern pthread_mutex_t	sync_mutex;
extern pthread_cond_t	sync_var;
extern pthread_mutex_t	secg;

void	set_threshold(float last);
void	*task_calcolo();
void	wait_to_compute();
void	copy_data();
void	alg_AT();
