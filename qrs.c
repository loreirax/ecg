#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#define nsample 34511
#define ncampioni 17

float 	ecg[nsample];
float 	Y0[nsample];
float 	Y1[nsample];
float	Y2[nsample];
float 	Y3[nsample];
int 	i, j, h, contatore_soglia, primo_vincolo, contatore_qrs;
float 	maxValue, soglia1, soglia2, value;
float	R[ncampioni + 1];
FILE 	*ecg_file;

float max (float v[], int start, int end){
	float aux = v[start];
	for(h = start + 1; h < end; ++h)
		if (v[h] > aux)
			aux = v[h];
	return aux;
}

int main(){
	float dato = 0.0;
	//lettura dei valori
	if ((ecg_file = fopen("ecg.txt", "r")) == NULL) {
		printf("Non posso aprire il file %s\n", "ecg.txt");
		exit(1);
	}
	for(i = 0; i < nsample; ++i){
		fscanf(ecg_file, "%f", &dato);
		ecg[i] = dato;
		//printf("Dato %d: %f\n", i, dato);
	}
	
	//algoritmo di Ahlstrom-Tompkins
	for(i = 1; i < nsample - 1; ++i)
		Y0[i] = fabs(ecg[i - 1] + ecg[i + 1]);
	for(i = 2; i < nsample - 2; ++i){
		Y1[i] = Y0[i - 1] + 2 * Y0[i] + Y0[i + 1];
		Y2[i] = fabs(ecg[i - 2] - 2 * ecg[i] + ecg[i + 2]);
		Y3[i] = Y1[i] + Y2[i];
	}
	//calcolo max(Y3)
	maxValue = max(Y3, 2, nsample - 2);
	printf("Max %f\n", maxValue);
	//c'è un picco se un punto supera soglia 1 e i 6 successivi superano soglia 2
	soglia1 = 0.8 * maxValue;
	soglia2 = 0.1 * maxValue;
	printf("S1 %f\n", soglia1);
	printf("S2 %f\n", soglia2);
	contatore_qrs = 0;
	for(i = 2; i < nsample - 2; ++i){
		//printf("i prima: %d\n", i);
		if (Y3[i] >= soglia2 && primo_vincolo == 1){
			contatore_soglia++;
			R[contatore_soglia] = ecg[i];
		}
		else {
			contatore_soglia = 0;
			primo_vincolo = 0;
		}
		if (Y3[i] >= soglia1 && primo_vincolo == 0){
			primo_vincolo = 1;
			R[0] = ecg[i];
		}
		
		if (contatore_soglia == ncampioni) {
			contatore_qrs++;
			contatore_soglia = 0;
			primo_vincolo = 0;
			value = max(R, 0, ncampioni);
			printf("Picco di valore: %f\nIndice prima soglia: %d\n", value, i - 6);
		}
		//printf("i dopo: %d\n", i);
	}
	printf("I picchi trovati sono %d\n", contatore_qrs);
	return 0;
}
	
