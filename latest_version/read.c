#include "read.h"

static FILE 	*ecg_file;	//file txt contenente i valori dell'ecg

void 	read_value() {
float 	value = 0.0;
	fscanf(ecg_file, "%f", &value);
	pthread_mutex_lock(&secg);
	ecg[index_in] = value;
	index_in = (index_in + 1) % n_shown_samples;
	pthread_mutex_unlock(&secg);
}

void 	new_data(int s1, int s2) {
	pthread_mutex_lock(&sync_mutex);
	sync_compute = s1;
	sync_graph = s2;
	pthread_mutex_unlock(&sync_mutex);
	pthread_cond_broadcast(&sync_var);
}

void 	file_open() {
	if ((ecg_file = fopen(file_name, "r")) == NULL) {
		printf("Can't open file '%s'\n", file_name);
		exit(1);
	}
	rewind(ecg_file);
}


void *task_read(void * arg){
struct task_param	*tp;
int 	h = 0;
	tp = (struct task_param *) arg;
	index_in = 0;
	file_open();
	set_period(tp);
	for(;;) {
		read_value();
		if (h < 4) {
			new_data(0, 1);
			++h;
		} else
			new_data(1, 1);
		if (deadline_miss(tp))
			printf("Reading task: deadline missed\n");
		wait_for_period(tp);
	}
}

