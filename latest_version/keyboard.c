#include "keyboard.h"

void 	*task_wait_keyboard() {
char 	scan;
	do {
		scan = readkey() >> 8;
		pthread_mutex_lock(&sth);
		switch (scan) {
		case KEY_UP:
			if (max_fr < max_admitted)
				max_fr++;
			break;
		case KEY_DOWN:
			if (max_fr > min_fr + 1)
				max_fr--;
			break;
		case KEY_LEFT:
			if (min_fr > min_admitted)
				min_fr--;
			break;
		case KEY_RIGHT:
			if (min_fr < max_fr -1)
				min_fr++;
			break;
		default: 
			break;
		}
		pthread_mutex_unlock(&sth);
	} while (scan != KEY_ESC);
	//allegro_exit();
	pthread_exit(0);
}