#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h> //time(null)
#include <pthread.h>

int white, blue;
float ecg[50];
int baseY = 500;
int baseX = 100;
int basep = 0;
int first= 0;
int i, X1, X2, Y1, Y2, keyp, r, index_in;
BITMAP *grafico;
int altezza = 500;
int lunghezza = 600;
pthread_mutex_t secg = PTHREAD_MUTEX_INITIALIZER;
pthread_t ecg_id;
FILE *ecg_file;
char s[5];

void *task_ecg(){
	//srand(time(NULL));
	float dato = 0.0;
	if ((ecg_file = fopen("ecg_file.txt", "r")) == NULL) {
		printf("Non posso aprire il file %s\n", "ecg_file.txt");
		exit(1);
	}
	rewind(ecg_file);
	for(;;){
		//r = rand() % 50 - 25;
		fscanf(ecg_file, "%f", &dato);
		//printf("dato: %f\n", dato);
		pthread_mutex_lock(&secg);
		ecg[index_in] = dato;
		//printf("ecg: %f\n", ecg[index_in]);
		index_in = (index_in + 1) % 50;
		pthread_mutex_unlock(&secg);
		usleep(500000);
	}	
}


void stampa(){
	clear_bitmap(grafico);
	pthread_mutex_lock(&secg);
	for(i = 1; i < 50; ++i) {
		if (i != index_in) {	
			X1 = baseX + (i - 1) * 5;
			Y1 = baseY - ecg[i - 1] * 500.0F;
			X2 = baseX + i * 5;
			Y2 = baseY - ecg[i] * 500.0F;
			line(grafico, X1, Y1, X2, Y2, white);
			//printf("Stampata linea tra valori %f e %f\n", ecg[i - 1], ecg[i]);
			//printf("Coordinate (%d, %d) e (%d, %d)\n", X1, Y1, X2, Y2);
		}
	}
	pthread_mutex_unlock(&secg);
	blit(grafico, screen, 0, 0, 50, 50, lunghezza, altezza);
}

int main(){
	//pthread_mutex_init(&secg, null);
	for (i = 0; i < 50; ++i)
		ecg[i] = 0.0;
		
	allegro_init();
	set_color_depth(8);
	white = 15;
	blue = 9;
	grafico = create_bitmap(lunghezza, altezza);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, 1024, 768, 0, 0);
	clear_to_color(screen, blue);
	install_keyboard();
	stampa();
	index_in = 0;
	pthread_create(&ecg_id, NULL, task_ecg, NULL);
	for(;;){
		stampa();
	}	
	return 0;
}
