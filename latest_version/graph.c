#include "graph.h"

static int 	_x1, _y1, _x2, _y2; 	//coordinate dei segmenti che compongono il grafico
static int 	recorded_fr; 	//variabile di appoggio per la frequenza

void 	diseases_report() {
int	aux_min, aux_max;
	pthread_mutex_lock(&sth);
	aux_min = min_fr;
	aux_max = max_fr;
	pthread_mutex_unlock(&sth);
	if (recorded_fr <= aux_min && recorded_fr >= 0) {
		led(led_on, led_off);
	} else 
		if (recorded_fr >= aux_max) {
			led(led_off, led_on);
		} else {
			led(led_off, led_off);
		}
}

void 	print_freq() {
char 	s[3];
	pthread_mutex_lock(&sfr);
	recorded_fr = last_frequency;
	pthread_mutex_unlock(&sfr);
	if (recorded_fr < 0)
		sprintf(s,"%s", "--");
	else
		sprintf(s,"%d", recorded_fr);
	clear_to_color(freq, black);
	textout_ex(freq, font, s, 0, 0, white, transparent);
	stretch_blit(freq, screen, 0, 0, freq->w, freq->h, freq_x, freq_y, freq->w * zoom, freq->h * zoom);
	diseases_report(recorded_fr);
}

void 	draw_axes() {
	clear_bitmap(graph);
	line(graph, base_x, base_y, base_x + n_shown_samples * unit_x, base_y, light_blue); // 0mV
	line(graph, base_x, base_y - 1 * unit_y, base_x + n_shown_samples * unit_x, base_y - 1 * unit_y, light_blue); // 1mV
	line(graph, base_x, base_y + 1 * unit_y, base_x + n_shown_samples * unit_x, base_y + 1 * unit_y, light_blue); // -1mV
}

void 	draw_graph() {
int 	k;
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

void 	*task_graph(void * arg) {
struct task_param 	*tp;
	tp = (struct task_param *)arg;
	set_period(tp);
	for(;;){
		draw_axes();
		draw_graph();
		print_freq();
		diseases_report();
		print_limit_fr();
		if (deadline_miss(tp))
			printf("Graphic task: deadline missed\n");
		wait_for_period(tp);
	}
}
