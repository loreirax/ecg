#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h> //time(null)
#include <pthread.h>

#define nsample 3000

int white, blue;
float ecg[nsample];
int baseY =  300;//500;
int baseX =  10;//100;
int basep = 0;
int first= 0;
int i, X1, X2, Y1, Y2, keyp, r, index_in;
BITMAP *grafico;
int altezza = 600;//500;
int lunghezza = 800; //600;
pthread_mutex_t secg = PTHREAD_MUTEX_INITIALIZER;
pthread_t ecg_id;
FILE *ecg_file;
char s[5];
struct timespec t1;
struct timespec t2;
long timediff;

void *task_ecg(){
	//srand(time(NULL));
	float dato = 0.0;
	if ((ecg_file = fopen("aami3dm.txt", "r")) == NULL) {
		printf("Non posso aprire il file %s\n", "aami3dm.txt");
		exit(1);
	}
	rewind(ecg_file);
	sleep(1);
	for(;;){
		//r = rand() % 50 - 25;
		clock_gettime(CLOCK_MONOTONIC, &t1);
		printf("t1 = %ld %ld\n", t1.tv_sec, t1.tv_nsec);
		fscanf(ecg_file, "%f", &dato);
		//printf("dato: %f\n", dato);
		pthread_mutex_lock(&secg);
		ecg[index_in] = dato;
		//printf("ecg: %f\n", ecg[index_in]);
		index_in = (index_in + 1) % nsample;
		pthread_mutex_unlock(&secg);
		clock_gettime(CLOCK_MONOTONIC, &t2);
		printf("t2 = %ld %ld\n", t2.tv_sec, t2.tv_nsec);
		timediff = (t2.tv_sec*1000000 + t2.tv_nsec) - (t1.tv_sec*1000000 + t1.tv_nsec);
		printf("Tempo lettura: %ld\n", timediff);
		usleep(1388); //frequenza a 720 Hz
	}	
}


void stampa(){
	clear_bitmap(grafico);
	pthread_mutex_lock(&secg);
	for(i = 1; i < nsample; ++i) {
		if (i != index_in) {	
			X1 = baseX + (i - 1) * 0.25;
			Y1 = baseY - ecg[i - 1] * 100.0F;
			X2 = baseX + i * 0.25;
			Y2 = baseY - ecg[i] * 100.0F;
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
	for (i = 0; i < nsample; ++i)
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
