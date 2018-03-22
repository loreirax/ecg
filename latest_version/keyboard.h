#include "main.h"

#define max_admitted 200
#define min_admitted 0

extern int 	min_fr, max_fr; //frequenze di soglia per identificare le malattie
void 	*task_wait_keyboard();	//legge i comandi da tastiera
extern void 	print_limit_fr();	//stampa a video le frequenze di soglia