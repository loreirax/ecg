#include "main.h"

#define max_admitted 200
#define min_admitted 0

extern int 	min_fr, max_fr; //frequenze di soglia per identificare le malattie
extern pthread_mutex_t sth;	//semaforo soglie malattie
void 	*task_wait_keyboard();	//legge i comandi da tastiera