#include "lettura.h"

static FILE 	*ecg_file;

void	read_value(){
float 	dato = 0.0;
	fscanf(ecg_file, "%f", &dato);
	pthread_mutex_lock(&secg);
	ecg[index_in] = dato;
	index_in = (index_in + 1) % nsample;
	pthread_mutex_unlock(&secg);
}

void	new_data(int s1, int s2){
	pthread_mutex_lock(&sync_mutex);
	sync_calcolo = s1;
	sync_grafico = s2;
	pthread_mutex_unlock(&sync_mutex);
	pthread_cond_broadcast(&sync_var);
}

void	file_open(){
	if ((ecg_file = fopen("prova_ecg.txt", "r")) == NULL) {
		printf("Non posso aprire il file %s\n", "prova_ecg.txt");
		exit(1);
	}
	rewind(ecg_file);
}


void *task_ecg(void * arg){
struct task_param	*tp;
int	h;
	tp = (struct task_param *)arg;
	index_in = 0;
	file_open();
	set_period(tp);
	for(h = 0; h < 4; ++h){
		read_value();
		new_data(0, 1);
		if (deadline_miss(tp))
			printf("Deadline mancata per lettura\n");
		wait_for_period(tp);
	}
	for(;;){
		read_value();
		new_data(1, 1);
		if (deadline_miss(tp))
			printf("Deadline mancata per lettura\n");
		wait_for_period(tp);
	}
}

