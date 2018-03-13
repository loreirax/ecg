#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <signal.h>
#define nsample 550 //campioni mostrati a video
#define nsoglia 250 //numero campioni prima del calcolo della soglia
#define ncampioni 4 //campioni per la condizione del picco
#define white 15
#define blue 1
#define light_blue 9
#define transparent -1
#define black 0
#define unit_x 1.72
#define unit_y 150

//task
struct	task_param {
	//wcet
	long	period;
	long	deadline;
	int	priority;
	int	dmiss;
	struct timespec	at;
	struct timespec	dl;
};

struct task_param	ecg_param, freq_param, graph_param, calc_param;
struct sched_param	ecg_par, graph_par, calc_par, freq_par;
pthread_attr_t 	ecg_attr, graph_attr, calc_attr, freq_attr;



//condivisi
float 	ecg[nsample];
int 	index_in;
int	contatore_qrs;

//calcolo picco
float	X[5];
float 	Y0[3];
float 	Y1;
float	Y2;
float 	Y3;
int 	contatore_soglia, primo_vincolo;
float 	maxValue, soglia1, soglia2;

//calcolo frequenza
int	frequenza_attiva = 0;
int	misure[5];
int	index_m;
BITMAP	*freq;
//soglie per segnalazione
int	frequenza_massima = 88;
int	frequenza_minima = 80;
int	led1_x = 20;
int	led1_y = 40;
int	led2_x = 20;
int	led2_y = 130;
BITMAP	*led_on, *led_off;

//coordinate
int 	_x1, _y1, _x2, _y2;
int 	baseY =  200;
int 	baseX =  10;
BITMAP 	*grafico;
int 	altezza = 400;
int 	lunghezza = 980;

//file dell'ecg
FILE 	*ecg_file;

//file del suono
SAMPLE	*beep;

//semafori
pthread_mutex_t secg = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sqrs = PTHREAD_MUTEX_INITIALIZER;
int	sync_calcolo, sync_grafico;
pthread_mutex_t	sync_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t	sync_var;

//thread
pthread_t 	ecg_id, calc_id, graph_id, freq_id;

void setSoglie(float last){
	if (last > maxValue){
		maxValue = last;
		soglia1 = 0.8 * maxValue;
		soglia2 = 0.1 * maxValue;
	}
}

void time_copy(struct timespec *td, struct timespec ts){
	td->tv_sec = ts.tv_sec;
	td->tv_nsec = ts.tv_nsec;
}

void time_add_us(struct timespec *t, long us){
	t->tv_sec += us / 1000000;
	t->tv_nsec += (us % 1000000) * 1000;
	if (t->tv_nsec > 1000000000) {
		t->tv_nsec -= 1000000000;
		t->tv_sec += 1;
	}
}

int time_cmp(struct timespec t1, struct timespec t2){
	if (t1.tv_sec > t2.tv_sec) return 1;
	if (t1.tv_sec < t2.tv_sec) return -1;
	if (t1.tv_nsec > t2.tv_nsec) return 1;
	if (t1.tv_nsec < t2.tv_nsec) return -1;
	return 0;
}

void set_period(struct task_param *tp){
struct timespec	t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	time_copy(&(tp->at), t);
	time_copy(&(tp->dl), t);
	time_add_us(&(tp->at), tp->period);
	time_add_us(&(tp->dl), tp->deadline);
}

int deadline_miss(struct task_param *tp){
struct timespec	now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	if (time_cmp(now, tp->dl) > 0){
		tp->dmiss++;
		return 1;
	}
	return 0;
}

void wait_for_period(struct task_param *tp){
	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &(tp->at), NULL);
	time_add_us(&(tp->at), tp->period);
	time_add_us(&(tp->dl), tp->period);
}

void *task_ecg(void * arg){
struct task_param	*tp;
float 	dato = 0.0;
int	h;
	tp = (struct task_param *)arg;
	index_in = 0;
	if ((ecg_file = fopen("n01m.txt", "r")) == NULL) {
		printf("Non posso aprire il file %s\n", "n01m.txt");
		exit(1);
	}
	rewind(ecg_file);
	set_period(tp);
	for(h = 0; h < 4; ++h){
		fscanf(ecg_file, "%f", &dato);
		pthread_mutex_lock(&secg);
		ecg[index_in] = dato;
		index_in = (index_in + 1) % nsample;
		pthread_mutex_unlock(&secg);
		if (deadline_miss(tp))
			printf("Deadline missed per lettura\n");
		wait_for_period(tp);
	}
	for(;;){
		fscanf(ecg_file, "%f", &dato);
		pthread_mutex_lock(&secg);
		ecg[index_in] = dato;
		index_in = (index_in + 1) % nsample;
		pthread_mutex_unlock(&secg);
		pthread_mutex_lock(&sync_mutex);
		sync_calcolo = 1;
		sync_grafico = 1;
		pthread_mutex_unlock(&sync_mutex);
		pthread_cond_broadcast(&sync_var);
		if (deadline_miss(tp))
			printf("Deadline missed per lettu<ra\n");
		wait_for_period(tp);
	}
}
END_OF_FUNCTION(task_ecg);

void led(BITMAP *led1, BITMAP *led2){
	stretch_blit(led1, screen, 0, 0, led1->w, led1->h, led1_x, led1_y, 50, 50);
	stretch_blit(led2, screen, 0, 0, led2->w, led2->h, led2_x, led2_y, 50, 50);
}

void segnalazione_anomalie(int frequenza_registrata){
	if (frequenza_registrata <= frequenza_minima){
		led(led_on, led_off);
	} else 
		if (frequenza_registrata >= frequenza_massima){
			led(led_off, led_on);
		} else {
			led(led_off, led_off);
		}
}




void *task_frequenza(void * arg){
int	begin = 0;
int	sum = 0;
int	l;
char	s[3];
struct task_param	*tp;
	tp = (struct task_param *)arg;
	index_m = 0;
	set_period(tp);
	wait_for_period(tp);
	for(;;){
		pthread_mutex_lock(&sqrs);
		misure[index_m] = contatore_qrs;
		contatore_qrs = 0;
		pthread_mutex_unlock(&sqrs);
		index_m = (index_m + 1) % 5;
		if (begin < 4)
			begin++;
		else {
			sum = 0;
			for(l = 0; l < 5; ++l)
				sum += misure[l];
			sprintf(s,"%d", sum * 6);
			clear_to_color(freq, black);
			textout_ex(freq, font, s, 0, 0, white, transparent);
			stretch_blit(freq, screen, 0, 0,freq->w, freq->h, 900, 50, 120, 80);
			segnalazione_anomalie(sum * 6); // sum * 6 è la frequenza cardiaca calcolata
		}
		if (deadline_miss(tp))
			printf("Deadline missed per la frequenza\n");
		wait_for_period(tp);
	}
}

void *task_calcolo(){
int	i, j;
int	start = 0;
	maxValue = 0;
	contatore_soglia = 0;
	//set_period(tp);
	for(;;) {
		pthread_mutex_lock(&sync_mutex);
		while(sync_calcolo == 0)
			pthread_cond_wait(&sync_var, &sync_mutex);
		sync_calcolo = 0;
		pthread_mutex_unlock(&sync_mutex);
		pthread_mutex_lock(&secg);
		i = (index_in - 5 + nsample) % nsample; 
		for(j = 0; j < 5; ++j)
			X[j] = ecg[(i + j) % nsample];
		pthread_mutex_unlock(&secg);
		//algoritmo di Ahlstrom-Tompkins per un solo Y3
		for(i = 0; i < 3; ++i)
			Y0[i] = fabs(X[i - 1] + X[i + 1]);
		Y1 = Y0[0] + 2 * Y0[1] + Y0[2];
		Y2 = fabs(X[0] - 2 * X[2] + X[4]);
		Y3 = Y1 + Y2;
		setSoglie(Y3);
		if (start <= nsoglia)
			start++;
		else {
			if (frequenza_attiva == 0){
				pthread_create(&freq_id, &freq_attr, task_frequenza, &freq_param);
				frequenza_attiva = 1;
			}
			if (Y3 >= soglia2 && primo_vincolo == 1)
				contatore_soglia++;
			else {
				contatore_soglia = 0;
				primo_vincolo = 0;
			}
			
			if (Y3 >= soglia1 && primo_vincolo == 0)
				primo_vincolo = 1;			
			if (contatore_soglia == ncampioni) {
				pthread_mutex_lock(&sqrs);
				contatore_qrs++;
				pthread_mutex_unlock(&sqrs);
				contatore_soglia = 0;
				primo_vincolo = 0;
				printf("Picco numero %d\n", contatore_qrs);
				//play_sample(beep, 100, 128, 500, 0);
				printf("\a");
			}
		}
		/*if (deadline_miss(tp))
			printf("Deadline missed per il calcolo\n");
		wait_for_period(tp);*/	
	}
}

void *task_grafico(void * arg){
int	k;
struct task_param	*tp;
	tp = (struct task_param *)arg;
	//set_period(tp);
	for(;;){
		pthread_mutex_lock(&sync_mutex);
		while(sync_grafico == 0)
			pthread_cond_wait(&sync_var, &sync_mutex);
		sync_grafico = 0;
		pthread_mutex_unlock(&sync_mutex);
		clear_bitmap(grafico);
		line(grafico, baseX, baseY, baseX + nsample * unit_x, baseY, light_blue); // 0mV
		line(grafico, baseX, baseY - 1 * unit_y, baseX + nsample * unit_x, baseY - 1 * unit_y, light_blue); // 1mV
		line(grafico, baseX, baseY + 1 * unit_y, baseX + nsample * unit_x, baseY + 1 * unit_y, light_blue); // -1mV
		pthread_mutex_lock(&secg);
		for(k = 1; k < nsample; ++k) {
			if (k < index_in || k > index_in + 5) {	
				_x1 = baseX + (k - 1) * unit_x;
				_y1 = baseY - ecg[k - 1] * unit_y;
				_x2 = baseX + k * unit_x;
				_y2 = baseY - ecg[k] * unit_y;
				line(grafico, _x1, _y1, _x2, _y2, white);
			}
		}
		pthread_mutex_unlock(&secg);
		blit(grafico, screen, 0, 0, 50, 300, lunghezza, altezza);
		/*if (deadline_miss(tp))
			printf("Deadline missed per il grafico\n");
		wait_for_period(tp);*/
	}
}

void close_button_handler(){
	pthread_kill(ecg_id, SIGKILL);
	pthread_kill(calc_id, SIGKILL);
	pthread_kill(graph_id, SIGKILL);
	pthread_kill(freq_id, SIGKILL);
}
END_OF_FUNCTION(close_button_handler);
void *task_
do {
scan = get_scancode();
switch (scan) {
case KEY_SPACE:
if (naf < MAXFLY)
task_create(nab++, btask, PER, DL,...);
break;
case KEY_UP:
g = g + 1; // increase gravity
break;
case KEY_DOWN:
if (g > 1) g = g - 1; // decrease gravity
break;
default: break;
}
} while (scan != KEY_ESC);

int main(){
int 	m;
	//variabili condivise
	for (m = 0; m < nsample; ++m)
		ecg[m] = 0.0;
	contatore_qrs = 0;
	pthread_cond_init(&sync_var, NULL);
	//grafica
	allegro_init();
	LOCK_FUNCTION(close_button_handler);
	set_close_button_callback(close_button_handler);
	install_sound(DIGI_AUTODETECT, MIDI_NONE, 0);
	set_config_int("sound", "quality", 1);
	beep = load_sample("beep.wav");
	if(beep == NULL){
		printf("Couldn't load beep!\n");
	}
	set_color_depth(8);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, 1080, 720, 0, 0);
	clear_to_color(screen, black);
	grafico = create_bitmap(lunghezza, altezza);
	freq = create_bitmap(30, 20);
	led_on = load_bitmap("led_on.tga", NULL);
	if (!led_on)
		printf("Couldn't load on!\n");
	led_off = load_bitmap("led_off.tga", NULL);
	if (!led_off)
		printf("Couldn't load off!\n");
	led(led_off, led_off);
	clear_to_color(freq, black);
	textout_ex(freq, font, "--", 0, 0, white, transparent);
	stretch_blit(freq, screen, 0, 0,freq->w, freq->h, 900, 50, 120, 80);
	blit(grafico, screen, 0, 0, 50, 300, lunghezza, altezza);
	
	//task
	ecg_param.period = 8000;//7813;
	ecg_param.deadline = 8000;
	ecg_param.priority = 32;
	ecg_param.dmiss = 0;
	/*calc_param.period = 7813;
	calc_param.deadline = 7813;
	calc_param.priority = 23;
	calc_param.dmiss = 0;
	graph_param.period = 7813;
	graph_param.deadline = 7813;
	graph_param.priority = 23;
	graph_param.dmiss = 0;*/
	freq_param.period = 2000000;
	freq_param.deadline = 35000; //entro 5 campioni (che sarebbero 40000)
	freq_param.priority = 28;
	freq_param.dmiss = 0;
	ecg_par.sched_priority = ecg_param.priority;
	freq_par.sched_priority = freq_param.priority;
	pthread_attr_init(&ecg_attr);
	pthread_attr_init(&graph_attr);
	pthread_attr_init(&calc_attr);
	pthread_attr_init(&freq_attr);
	/*
	pthread_attr_setinheritsched(&ecg_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&ecg_attr, SCHED_RR);
	
	pthread_attr_setinheritsched(&calc_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&calc_attr, SCHED_RR);
	
	pthread_attr_setinheritsched(&graph_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&graph_attr, SCHED_RR);
	
	pthread_attr_setinheritsched(&freq_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&freq_attr, SCHED_RR);
	*/
	pthread_attr_setschedparam(&ecg_attr, &ecg_par);
	pthread_attr_setschedparam(&freq_attr, &freq_par);
	pthread_create(&ecg_id, &ecg_attr, task_ecg, &ecg_param);
	pthread_create(&calc_id, &calc_attr, task_calcolo, NULL);
	pthread_create(&graph_id, &graph_attr, task_grafico, NULL);
	pthread_join(ecg_id, NULL);
	return 0;
}
