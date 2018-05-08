#include "keyboard.h"

void 	*task_wait_keyboard() {
char 	scan;
int	actual_ecg_frequency;
	pthread_mutex_lock(&sfecg);
	actual_ecg_frequency = ecg_frequency;
	pthread_mutex_unlock(&sfecg);
	do {
		scan = readkey() >> 8;
		switch (scan) {
		case KEY_UP:
			pthread_mutex_lock(&sth);
			if (max_fr < max_admitted)
				max_fr++;
			pthread_mutex_unlock(&sth);
			break;
		case KEY_DOWN:
			pthread_mutex_lock(&sth);
			if (max_fr > min_fr + 1)
				max_fr--;
			pthread_mutex_unlock(&sth);
			break;
		case KEY_LEFT:
			pthread_mutex_lock(&sth);
			if (min_fr > min_admitted)
				min_fr--;
			pthread_mutex_unlock(&sth);
			break;
		case KEY_RIGHT:
			pthread_mutex_lock(&sth);
			if (min_fr < max_fr -1)
				min_fr++;
			pthread_mutex_unlock(&sth);
			break;
		case KEY_M:
			if(actual_ecg_frequency < 490){
				actual_ecg_frequency += 10;
				pthread_mutex_lock(&sfecg);
				ecg_frequency = actual_ecg_frequency;
				pthread_mutex_unlock(&sfecg);
				pthread_mutex_lock(&sper);
				comp_param.period = 1000000 / actual_ecg_frequency;
				pthread_mutex_unlock(&sper);
			}
			break;
		case KEY_N:
			if (actual_ecg_frequency > 10){
				actual_ecg_frequency -= 10;
				pthread_mutex_lock(&sfecg);
				ecg_frequency = actual_ecg_frequency;
				pthread_mutex_unlock(&sfecg);
				pthread_mutex_lock(&sper);
				comp_param.period = 1000000 / actual_ecg_frequency;
				pthread_mutex_unlock(&sper);
			}
			break;
		default: 
			break;
		}
	} while (scan != KEY_ESC);
	//allegro_exit();
	pthread_exit(0);
}
