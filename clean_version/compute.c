#include "compute.h"
//variabili per il calcolo dell'algoritmo
static float 	X[5];
static float 	Y0[3];
static float 	Y1;
static float 	Y2;
static float 	Y3;

static int 	n_above; 	//numero di campioni oltre la seconda soglia
static int 	bool_t1; 	//booleano superata la prima soglia
static int 	active_fr; 	//calcolo della frequenza attivo
static float 	max_value; 	//massimo valore registrato
static float 	th_1, th_2; 	//soglie
static pthread_t 	id_exit; 	//id del task tastiera

void 	copy_data() {
int 	i, j;
	pthread_mutex_lock(&secg);
	i = index_in - 5;
	for(j = 0; j < 5; ++j)
		X[j] = ecg[i + j];
	index_in++;
	if (index_in == duration - 1){
		pthread_mutex_unlock(&secg);
		printf("File terminato\n");
		pthread_cancel(id_exit);
		pthread_exit(0);
	}
	else
		pthread_mutex_unlock(&secg);
}

void 	set_threshold(float last) {
	if (last > max_value){
		max_value = last;
		th_1 = 0.8 * max_value;
		th_2 = 0.1 * max_value;
	}
}

void 	alg_AT() { //algoritmo di Ahlstrom-Tompkins per un solo Y3
int 	i;
	for(i = 0; i < 3; ++i)
		Y0[i] = fabs(X[i - 1] + X[i + 1]);
	Y1 = Y0[0] + 2 * Y0[1] + Y0[2];
	Y2 = fabs(X[0] - 2 * X[2] + X[4]);
	Y3 = Y1 + Y2;
}

void 	check_peak() {
	if (bool_t1 == 0) {
		if (Y3 >= th_1) {
			bool_t1 = 1;
		}
	} else {		
		if (Y3 >= th_2) {
			n_above++;
			if (n_above == n_peak_samples) {
				pthread_mutex_lock(&sqrs);
				counter_qrs++;
				pthread_mutex_unlock(&sqrs);
				n_above = 0;
				bool_t1 = 0;
			}
		} else {
			n_above = 0;
			bool_t1 = 0;
		}
	}
}

void 	*task_compute() {
int 	start = 0;
struct task_param 	*tp;
	max_value = 0;
	n_above = 0;
	active_fr = 0;
	id_exit = comp_param.arg;
	set_period(&comp_param);
	for(;;) {
		copy_data();
		alg_AT();
		set_threshold(Y3);
		if (start <= n_threshold)
			start++;
		else {
			if (active_fr == 0) {
				pthread_create(&freq_id, &freq_attr, task_frequency, &freq_param);
				active_fr = 1;
			}
			check_peak();
		}
		if (deadline_miss(&comp_param))
			printf("Compute task: deadline missed\n");
		pthread_mutex_lock(&sper);
		tp = &comp_param;
		pthread_mutex_unlock(&sper);
		wait_for_period(tp);
	}
}
