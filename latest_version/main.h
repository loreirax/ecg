#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <signal.h>

//colori
#define white 15
#define light_blue 9
#define transparent -1
#define black 0

//coordinate
#define	freq_x 900
#define freq_y 65
#define brad_x 140
#define brad_y 65
#define tac_x 140
#define tac_y 155
#define led1_x 70
#define	led1_y 50
#define	led2_x 70
#define	led2_y 140
#define th_x 75
#define	th_y 260
#define	min_freq_x 350
#define	min_freq_y 260
#define	max_freq_x 550
#define	max_freq_y 260
#define	heart_x 820
#define	heart_y 50
#define	graph_x 50
#define	graph_y 300

//dimensioni
#define	len_char 10
#define	hgt_char 20
#define zoom 3
#define	led_dim 50
#define	heart_dim 50
#define	hgt_graph 400
#define	len_graph 980

//utilità del calcolo
#define n_shown_samples 550

struct 	task_param {
	//wcet
	long 	period;
	long 	deadline;
	int 	priority;
	int 	dmiss;
	struct timespec 	at;
	struct timespec 	dl;
};

//parametri task_frequency
pthread_t 	freq_id;
struct task_param 	freq_param;
struct sched_param 	freq_par;
pthread_attr_t 	freq_attr;


float 	ecg[n_shown_samples]; 	//valori dell'ecg
int 	index_in; 	//locazione prossimo dato da inserire
int 	counter_qrs; 	//contatore dei picchi (complessi qrs)


BITMAP 	*freq; 	//bitmap per mostrare la frequenza a video
BITMAP 	*led_on, *led_off; 	//bitmap con i led
BITMAP 	*max_freq, *min_freq; 	//bitmap per mostrare a video le soglie delle anomalie
BITMAP 	*graph; 	//bitmap per preparare il grafico

extern void 	*task_compute();
extern void 	*task_frequency(void *arg);
extern void 	*task_graph(void * arg);
extern void 	*task_read(void * arg);
extern void 	*task_wait_keyboard();

int 	max_fr, min_fr; 	//frequenze di soglia per identificare le malattie
int 	last_frequency; 	//frequenza calcolata

int 	sync_compute; 	//variabile che indica la presenza di un nuovo dato
pthread_mutex_t 	secg; 	//semaforo dati ecg
pthread_mutex_t 	sqrs; 	//semaforo per counter_qrs
pthread_mutex_t 	sfr; 	//semaforo frequenza
pthread_mutex_t 	sth; 	//semaforo soglie malattie
pthread_mutex_t 	sync_mutex; 	//mutex per la var. condition
pthread_cond_t 		sync_var; 	//var. condition per dati nuovi

void 	time_copy(struct timespec *td, struct timespec ts); 	//ricopia ts in td
void 	time_add_us(struct timespec *t, long us); 	//aggiunge us microsecondi a t
int 	time_cmp(struct timespec t1, struct timespec t2); 	//compara t1 e t2
void 	set_period(struct task_param *tp); 	//imposta il momento della prossima riattivazione e della deadline
int 	deadline_miss(struct task_param *tp); 	//controlla se la deadline sia stata rispettata 
void  	wait_for_period(struct task_param *tp); 	//aspetta la nuova riattivazione
void 	led(BITMAP *led1, BITMAP *led2); 	//aggiorna i led che segnalano le anomalie
void 	close_button_handler(); 	//azione alla pressione del tasto di chiusura della finestra
void 	print_limit_fr(); 	//stampaa video le frequenze limite

void 	init_variables();
void 	init_window();
void 	init_diseases_alert();
void 	init_freq();
void 	init_thresholds();
void 	init_graph();
void 	init_tasks();