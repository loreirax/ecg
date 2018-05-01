#include "main.h"
#define file_name "paziente_02.txt"

extern float 	ecg[n_shown_samples]; 	//valori dell'ecg
extern int 	index_in; 	//locazione prossimo dato da inserire
extern pthread_mutex_t  secg; 	//semaforo dati ecg
extern int 	sync_compute; 	//variabile che indica la presenza di un nuovo dato
extern pthread_mutex_t 	sync_mutex; 	//mutex per la var. condition
extern pthread_cond_t 	sync_var; 	//var. condition per dati nuovi

void 	*task_read(void * arg); 	//task per la lettura dei dati da file
void 	read_value(); 	//legge il nuovo dato dal file
void 	new_data(int s); 	//segnala il nuovo dato
void 	file_open(); 	//apre il file
