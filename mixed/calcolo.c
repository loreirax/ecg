#include "calcolo.h"

static float	X[5];
static float 	Y0[3];
static float 	Y1;
static float	Y2;
static float 	Y3;
static int	contatore_soglia, primo_vincolo, frequenza_attiva;
static float 	maxValue, soglia1, soglia2;

void	wait_to_compute(){
	pthread_mutex_lock(&sync_mutex);
	while(sync_calcolo == 0)
		pthread_cond_wait(&sync_var, &sync_mutex);
	sync_calcolo = 0;
	pthread_mutex_unlock(&sync_mutex);
}

void	copy_data(){
int	i, j;
	pthread_mutex_lock(&secg);
	i = (index_in - 5 + nsample) % nsample; 
	for(j = 0; j < 5; ++j)
		X[j] = ecg[(i + j) % nsample];
	pthread_mutex_unlock(&secg);
}

void	set_threshold(float last){
	if (last > maxValue){
		maxValue = last;
		soglia1 = 0.8 * maxValue;
		soglia2 = 0.1 * maxValue;
	}
}

void	alg_AT(){ //algoritmo di Ahlstrom-Tompkins per un solo Y3
int	i;
	for(i = 0; i < 3; ++i)
			Y0[i] = fabs(X[i - 1] + X[i + 1]);
		Y1 = Y0[0] + 2 * Y0[1] + Y0[2];
		Y2 = fabs(X[0] - 2 * X[2] + X[4]);
		Y3 = Y1 + Y2;
}

void	check_peak(){
	if (Y3 >= soglia1 && primo_vincolo == 0)
		primo_vincolo = 1;
			
	if (Y3 >= soglia2 && primo_vincolo == 1)
		contatore_soglia++;
	else {
		contatore_soglia = 0;
		primo_vincolo = 0;
	}
					
	if (contatore_soglia == ncampioni) {
		pthread_mutex_lock(&sqrs);
		contatore_qrs++;
		pthread_mutex_unlock(&sqrs);
		contatore_soglia = 0;
		primo_vincolo = 0;
	}
}

void	*task_calcolo(){
int	start = 0;
	maxValue = 0;
	contatore_soglia = 0;
	frequenza_attiva = 0;
	for(;;) {
		wait_to_compute();
		copy_data();
		alg_AT();
		set_threshold(Y3);
		if (start <= nsoglia)
			start++;
		else {
			if (frequenza_attiva == 0){
				pthread_create(&freq_id, &freq_attr, task_frequenza, &freq_param);
				frequenza_attiva = 1;
			}
			check_peak();
		}
	}
}

