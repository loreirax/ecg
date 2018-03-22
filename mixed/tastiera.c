#include "tastiera.h"

void *task_tastiera(){
char 	scan;
	do {
		scan = readkey() >> 8;
		switch (scan) {
		case KEY_UP:
			
			if (frequenza_massima < 200)
				frequenza_massima++;
		break;
		case KEY_DOWN:
			if (frequenza_massima > frequenza_minima + 1)
				frequenza_massima--;
		break;
		case KEY_LEFT:
			if (frequenza_minima > 0)
				frequenza_minima--;
		break;
		case KEY_RIGHT:
			if (frequenza_minima < frequenza_massima -1)
				frequenza_minima++;
		break;
		default: break;
		}
		stampa_frequenze();
	} while (scan != KEY_ESC);
}

