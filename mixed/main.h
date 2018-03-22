#include <allegro.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <signal.h>

//colori
#define white 15
#define blue 1
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
#define soglie_x 75
#define	soglie_y 260
#define	freq_min_x 350
#define	freq_min_y 260
#define	freq_max_x 550
#define	freq_max_y 260
#define	cuore_x 820
#define	cuore_y 50
#define	grafico_x 50
#define	grafico_y 300

//dimensioni
#define	lun_char 10
#define	alt_char 20
#define zoom 3
#define	led_dim 50
#define	cuore_dim 50
#define	altezza 400
#define	lunghezza 980

//utilità del calcolo
#define nsample 550

struct	task_param {
	//wcet
	long	period;
	long	deadline;
	int	priority;
	int	dmiss;
	struct timespec	at;
	struct timespec	dl;
};

pthread_t 	freq_id;
struct task_param	freq_param;
struct sched_param	freq_par;
pthread_attr_t 		freq_attr;


float 	ecg[nsample];
int 	index_in;
int	contatore_qrs;

BITMAP	*freq;
BITMAP	*led_on, *led_off;
BITMAP	*freq_max, *freq_min;
BITMAP 	*grafico;

extern void	*task_calcolo();
extern void 	*task_frequenza(void *arg);
extern void	*task_grafico();
extern void	*task_ecg(void * arg);
extern void 	*task_tastiera();

int	frequenza_massima;
int	frequenza_minima;

pthread_mutex_t	secg;
pthread_mutex_t sqrs;
int	sync_calcolo, sync_grafico;
pthread_mutex_t	sync_mutex;
pthread_cond_t	sync_var;
void	time_copy(struct timespec *td, struct timespec ts);
void	time_add_us(struct timespec *t, long us);
int	time_cmp(struct timespec t1, struct timespec t2);
void	set_period(struct task_param *tp);
int 	deadline_miss(struct task_param *tp);
void 	wait_for_period(struct task_param *tp);
void 	led(BITMAP *led1, BITMAP *led2);
void 	close_button_handler();
void	stampa_frequenze();
void	init_variabili();
void	init_finestra();
void	init_segnalazione();
void	init_freq();
void	init_soglie();
void	init_grafico();
void	init_task();
