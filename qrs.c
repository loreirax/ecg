#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <semaphore.h>
#define nsample 3000
#define ncampioni 17

float 	ecg[nsample];
float	X[5];
float 	Y0[3];
float 	Y1;
float	Y2;
float 	Y3;
int 	index_in;
int 	i, j, contatore_soglia, primo_vincolo, contatore_qrs;
float 	maxValue, soglia1, soglia2, value;
float	R[ncampioni + 1];
FILE 	*ecg_file;
pthread_mutex_t 	secg = PTHREAD_MUTEX_INITIALIZER;
pthread_t 	ecg_id, calc_id;
sem_t	event;

void setSoglie(float last){
	if (last > maxValue){
		maxValue = last;
		soglia1 = 0.8 * maxValue;
		soglia2 = 0.1 * maxValue;
	}
}

void *task_ecg(){
	//srand(time(NULL));
	float dato = 0.0;
	index_in = 0;
	if ((ecg_file = fopen("ecg.txt", "r")) == NULL) {
		printf("Non posso aprire il file %s\n", "ecg.txt");
		exit(1);
	}
	rewind(ecg_file);
	for(index_in = 0; index_in < 5; ){
		fscanf(ecg_file, "%f", &dato);
		pthread_mutex_lock(&secg);
		ecg[index_in] = dato;
		index_in = (index_in + 1) % nsample;
		pthread_mutex_unlock(&secg);
		printf("Dato: %f\n", dato);
		usleep(1388); //frequenza a 720 Hz
	}
	sem_post(&event);
	for(;;){
		fscanf(ecg_file, "%f", &dato);
		pthread_mutex_lock(&secg);
		ecg[index_in] = dato;
		index_in = (index_in + 1) % nsample;
		pthread_mutex_unlock(&secg);
		printf("Dato: %f\n", dato);
		sem_post(&event);
		usleep(1388); //frequenza a 720 Hz
	}
}

void *task_calcolo(){
	maxValue = 0;
	contatore_soglia = 0;
	contatore_qrs = 0;
	
	for(;;) {
		sem_wait(&event);
		pthread_mutex_lock(&secg);
		i = (index_in - 5 + nsample) % nsample; 
		for(j = 0; j < 5; ++j)
			X[j] = ecg[i + j];
		pthread_mutex_unlock(&secg);
		//algoritmo di Ahlstrom-Tompkins per un solo Y3
		for(i = 1; i < 3; ++i)
			Y0[i] = fabs(X[i - 1] + X[i + 1]);
		Y1 = Y0[i - 1] + 2 * Y0[i] + Y0[i + 1];
		Y2 = fabs(X[i - 2] - 2 * X[i] + X[i + 2]);
		Y3 = Y1 + Y2;
		printf("Y3: %f\n", Y3);
		setSoglie(Y3);
		if (Y3 >= soglia2 && primo_vincolo == 1){
			contatore_soglia++;
		} else {
			contatore_soglia = 0;
			primo_vincolo = 0;
		}
		
		if (Y3 >= soglia1 && primo_vincolo == 0)
			primo_vincolo = 1;
			
		if (contatore_soglia == ncampioni) {
			contatore_qrs++;
			contatore_soglia = 0;
			primo_vincolo = 0;
			printf("Un picco\n");
		}	
	}
}
	

int main(){
	sem_init(&event, 0, 0);	
	pthread_create(&ecg_id, NULL, task_ecg, NULL);
	pthread_create(&calc_id, NULL, task_calcolo, NULL);
	pthread_join(ecg_id, NULL);
	return 0;
}
