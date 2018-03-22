#include "main.h"
extern int	min_fr, max_fr;	//frequenze di soglia per identificare le malattie
extern int 	counter_qrs;	//contatore dei picchi (complessi qrs)
extern BITMAP 	*freq;	//bitmap per mostrare la frequenza a video
extern pthread_mutex_t 	sqrs;	//semaforo per counter_qrs
void 	diseases_report(int recorded_fr);	//segnala le anomalie
void 	*task_frequency(void * arg);	//calcola la frequenza
void 	print_freq(int value);	//mostra a video la frequenza
void 	read_counter();	//legge counter_qrs
