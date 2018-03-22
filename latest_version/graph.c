#include "graph.h"

static int 	_x1, _y1, _x2, _y2;	//coordinate dei segmenti che compongono il grafico

void 	wait_to_draw() {
	pthread_mutex_lock(&sync_mutex);
	while(sync_graph == 0)
		pthread_cond_wait(&sync_var, &sync_mutex);
	sync_graph = 0;
	pthread_mutex_unlock(&sync_mutex);
}

void 	draw_axes() {
	clear_bitmap(graph);
	line(graph, base_x, base_y, base_x + n_shown_samples * unit_x, base_y, light_blue); // 0mV
	line(graph, base_x, base_y - 1 * unit_y, base_x + n_shown_samples * unit_x, base_y - 1 * unit_y, light_blue); // 1mV
	line(graph, base_x, base_y + 1 * unit_y, base_x + n_shown_samples * unit_x, base_y + 1 * unit_y, light_blue); // -1mV
}

void 	draw_graph(){
int	k;
	pthread_mutex_lock(&secg);
	for(k = 1; k < n_shown_samples; ++k) {
		if (k < index_in || k > index_in + 5) {	
			_x1 = base_x + (k - 1) * unit_x;
			_y1 = base_y - ecg[k - 1] * unit_y;
			_x2 = base_x + k * unit_x;
			_y2 = base_y - ecg[k] * unit_y;
			line(graph, _x1, _y1, _x2, _y2, white);
		}
	}
	pthread_mutex_unlock(&secg);
	blit(graph, screen, 0, 0, graph_x, graph_y, len_graph, hgt_graph);
}

void 	*task_graph(){
	for(;;){
		wait_to_draw();
		draw_axes();
		draw_graph();
	}
}

