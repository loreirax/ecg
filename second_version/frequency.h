#include "main.h"
extern int 	min_fr, max_fr; 	//frequenze di soglia per identificare le malattie
extern int 	counter_qrs; 	//contatore dei picchi (complessi qrs)
extern pthread_mutex_t 	sqrs; 	//semaforo per counter_qrs
void 	*task_frequency(void * arg); 	//calcola la frequenza
void 	read_counter(); 	//legge counter_qrs
