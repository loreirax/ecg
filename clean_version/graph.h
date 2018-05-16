#include "main.h"

#define unit_x 1.72
#define unit_y 150
#define base_y 200
#define base_x 10


extern float 	ecg[duration]; 	//valori dell'ecg
extern int 	min_fr, max_fr; //frequenze di soglia per identificare le malattie
extern BITMAP 	*graph; 	//bitmap per preparare il grafico
extern BITMAP 	*freq; 	//bitmap per mostrare la frequenza cardiaca a video
extern BITMAP 	*ecg_freq; 	//bitmap per mostrare la frequenza dell'ecg a video

extern pthread_mutex_t 	secg; 	//semaforo dati ecg
extern pthread_mutex_t 	sth; 	//semaforo soglie malattie
extern pthread_mutex_t 	sfr; 	//semaforo frequenza
extern pthread_mutex_t 	sfecg; 	//semaforo frequenza ecg

void 	*task_graph(); 	//task che si occupa del grafico

void 	draw_axes(); 	//disegna gli assi del grafico dell'ecg
void 	draw_graph(); 	//rappresenta i valori dell'ecg
void 	print_freq(); 	//stampa a video la frequenza cardiaca registrata
void 	print_freq_ecg(); 	//stampa a video la frequenza dell'ecg
void 	diseases_report();  //controlla la frequenza e accende i led
extern void 	print_limit_fr(); 	//stampa a video le frequenze di soglia

