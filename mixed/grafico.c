#include "grafico.h"

static int 	_x1, _y1, _x2, _y2;

void	wait_to_draw(){
	pthread_mutex_lock(&sync_mutex);
	while(sync_grafico == 0)
		pthread_cond_wait(&sync_var, &sync_mutex);
	sync_grafico = 0;
	pthread_mutex_unlock(&sync_mutex);
}

void	draw_axes(){
	clear_bitmap(grafico);
	line(grafico, baseX, baseY, baseX + nsample * unit_x, baseY, light_blue); // 0mV
	line(grafico, baseX, baseY - 1 * unit_y, baseX + nsample * unit_x, baseY - 1 * unit_y, light_blue); // 1mV
	line(grafico, baseX, baseY + 1 * unit_y, baseX + nsample * unit_x, baseY + 1 * unit_y, light_blue); // -1mV
}

void	draw_graph(){
int	k;
	pthread_mutex_lock(&secg);
	for(k = 1; k < nsample; ++k) {
		if (k < index_in || k > index_in + 5) {	
			_x1 = baseX + (k - 1) * unit_x;
			_y1 = baseY - ecg[k - 1] * unit_y;
			_x2 = baseX + k * unit_x;
			_y2 = baseY - ecg[k] * unit_y;
			line(grafico, _x1, _y1, _x2, _y2, white);
		}
	}
	pthread_mutex_unlock(&secg);
	blit(grafico, screen, 0, 0, grafico_x, grafico_y, lunghezza, altezza);
}

void *task_grafico(){
	for(;;){
		wait_to_draw();
		draw_axes();
		draw_graph();
	}
}

