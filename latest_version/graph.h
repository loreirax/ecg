#include "main.h"

#define unit_x 1.72
#define unit_y 150
#define base_y 200
#define base_x 10


extern float 	ecg[n_shown_samples];	//valori dell'ecg
extern BITMAP 	*graph;	//bitmap per preparare il grafico
extern int 	sync_graph;	//booleano nuovo dato disponibile
extern pthread_mutex_t 	sync_mutex;	//mutex per la var. condition
extern pthread_cond_t 	sync_var;	//var. condition per dati nuovi
extern pthread_mutex_t 	secg;	//semaforo dati ecg

void 	*task_graph();	//task che si occupa del grafico
void 	wait_to_draw();	//aspetta un nuovo dato dell'ecg
void 	draw_axes();	//disegna gli assi del grafico dell'ecg
void 	draw_graph();	//rappresenta i valori dell'ecg

