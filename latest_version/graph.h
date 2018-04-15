#include "main.h"

#define unit_x 1.72
#define unit_y 150
#define base_y 200
#define base_x 10


extern float 	ecg[n_shown_samples]; 	//valori dell'ecg

extern BITMAP 	*graph; 	//bitmap per preparare il grafico
extern BITMAP 	*freq; 	//bitmap per mostrare la frequenza a video

extern pthread_mutex_t 	secg; 	//semaforo dati ecg

void 	*task_graph(); 	//task che si occupa del grafico

void 	draw_axes(); 	//disegna gli assi del grafico dell'ecg
void 	draw_graph(); 	//rappresenta i valori dell'ecg
void 	print_freq(); 	//stampa a video la frequenza registrata
void 	diseases_report();  //controlla la frequenza e accende i led
extern void 	print_limit_fr(); 	//stampa a video le frequenze di soglia

