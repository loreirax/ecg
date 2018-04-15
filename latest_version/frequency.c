#include "frequency.h"
static int 	counted_peaks[5];	//picchi ogni 2 secondi, totale 10 secondi
static int 	index_next;	//indice locazione prossimo dato

void 	read_counter() {
	pthread_mutex_lock(&sqrs);
	counted_peaks[index_next] = counter_qrs;
	counter_qrs = 0;
	pthread_mutex_unlock(&sqrs);
	index_next = (index_next + 1) % 5;
}

void 	*task_frequency(void *arg) {
int 	begin = 0; 	//contatore per il riempimento di counted_peaks
int 	sum = 0; 	//somma dei valori di counted_peaks
int 	i; 	//indice per scorrere counted_peaks
struct task_param 	*tp;
	tp = (struct task_param *)arg;
	index_next = 0;
	set_period(tp);
	wait_for_period(tp);
	for(;;){
		read_counter();
		if (begin < 4)
			begin++;
		else {
			sum = 0;
			for(i = 0; i < 5; ++i)
				sum += counted_peaks[i];
			pthread_mutex_lock(&sfr);
			last_frequency = sum * 6; 	// sum * 6 è la frequenza cardiaca calcolata
			pthread_mutex_unlock(&sfr);
		}
		if (deadline_miss(tp))
			printf("Frequency task: deadline missed\n");
		wait_for_period(tp);
	}
}