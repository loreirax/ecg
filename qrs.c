#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
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

//coordinate
int 	_x1, _y1, _x2, _y2;
int 	baseY =  200;
int 	baseX =  10;
BITMAP 	*grafico;
int 	altezza = 400;
int 	lunghezza = 980;

//file dell'ecg
FILE 	*ecg_file;

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

void *task_ecg(){
	int	h;
	float 	dato = 0.0;
	index_in = 0;
	if ((ecg_file = fopen("n01m.txt", "r")) == NULL) {
		printf("Non posso aprire il file %s\n", "n01m.txt");
		exit(1);
	}
	rewind(ecg_file);
	for(h = 0; h < 4; ++h){
		fscanf(ecg_file, "%f", &dato);
		pthread_mutex_lock(&secg);
		ecg[index_in] = dato;
		index_in = (index_in + 1) % nsample;
		pthread_mutex_unlock(&secg);
		usleep(7813);
		//usleep(1388); //frequenza a 720 Hz
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
		usleep(7813);
		//usleep(1388); //frequenza a 720 Hz
	}
}

void *task_frequenza(){
	int	begin = 0;
	int	sum = 0;
	int	l;
	char	s[3];
	index_m = 0;
	sleep(2);
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
			clear_to_color(freq, blue);
			textout_ex(freq, font, s, 0, 0, white, transparent);
			blit(freq, screen, 0, 0, 100, 10, 20, 20);
		}
		sleep(2);
	}
}

void *task_calcolo(){
	int	i, j;
	int	start = 0;
	maxValue = 0;
	contatore_soglia = 0;
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
				pthread_create(&freq_id, NULL, task_frequenza, NULL);
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
			}
		}	
	}
}

void *task_grafico(){
	int	k;
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
			if (k != index_in) {	
				_x1 = baseX + (k - 1) * unit_x;
				_y1 = baseY - ecg[k - 1] * unit_y;
				_x2 = baseX + k * unit_x;
				_y2 = baseY - ecg[k] * unit_y;
				line(grafico, _x1, _y1, _x2, _y2, white);
			}
		}
		pthread_mutex_unlock(&secg);
		blit(grafico, screen, 0, 0, 50, 300, lunghezza, altezza);
	}
}

int main(){
	int 	m;
	for (m = 0; m < nsample; ++m)
		ecg[m] = 0.0;
	contatore_qrs = 0;
	pthread_cond_init(&sync_var, NULL);
	allegro_init();
	set_color_depth(8);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, 1080, 720, 0, 0);
	clear_to_color(screen, blue);
	grafico = create_bitmap(lunghezza, altezza);
	freq = create_bitmap(20, 20);
	clear_to_color(freq, blue);
	textout_ex(freq, font, "--", 0, 0, white, transparent);
	blit(freq, screen, 0, 0, 100, 10, 20, 20);
	blit(grafico, screen, 0, 0, 50, 300, lunghezza, altezza);
	pthread_create(&ecg_id, NULL, task_ecg, NULL);
	pthread_create(&calc_id, NULL, task_calcolo, NULL);
	pthread_create(&graph_id, NULL, task_grafico, NULL);
	pthread_join(ecg_id, NULL);
	return 0;
}
