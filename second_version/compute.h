#include "main.h"

#define n_threshold 250 	//numero campioni prima del calcolo della soglia
#define n_peak_samples 4 	//campioni per la condizione del picco

extern int	counter_qrs; 	//contatore dei picchi (complessi qrs)
//extern int	sync_compute; 	//variabile che indica la presenza di un nuovo dato
//extern pthread_mutex_t	sync_mutex; 	//mutex per la var. condition
//extern pthread_cond_t	sync_var; 	//var. condition per dati nuovi
extern pthread_mutex_t	secg; 	//semaforo dati ecg

//parametri per il task che calcola la frequenza
extern pthread_t 	freq_id;
extern struct task_param 	freq_param, comp_param;
extern struct sched_param 	freq_par;
extern pthread_attr_t 	freq_attr;

void 	set_threshold(float last);	//calcola ed aggiorna le soglie dei picchi
void 	*task_compute();	//task che calcola la presenza di un picco e ne tiene il conteggio
//void 	wait_to_compute();	//attesa nuovo dato
void 	copy_data();	//copia dati
void 	alg_AT();	//calcola derivate filtrate del segnale
void 	check_peak();	//verifica se ci sia stato un picco
