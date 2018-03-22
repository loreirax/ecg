#include "frequency.h"
static int	counted_peaks[5];	//picchi ogni 2 secondi, totale 10 secondi
static int	index_next;	//indice locazione prossimo dato

void	read_counter() {
	pthread_mutex_lock(&sqrs);
	counted_peaks[index_next] = counter_qrs;
	counter_qrs = 0;
	pthread_mutex_unlock(&sqrs);
	index_next = (index_next + 1) % 5;
}

void 	diseases_report(int recorded_fr) {
	if (recorded_fr <= min_fr) {
		led(led_on, led_off);
	} else 
		if (recorded_fr >= max_fr) {
			led(led_off, led_on);
		} else {
			led(led_off, led_off);
		}
}

void	print_freq(int value) {
char	s[3];
	sprintf(s,"%d", value);
	clear_to_color(freq, black);
	textout_ex(freq, font, s, 0, 0, white, transparent);
	stretch_blit(freq, screen, 0, 0, freq->w, freq->h, freq_x, freq_y, freq->w * zoom, freq->h * zoom);
}

void 	*task_frequency(void *arg) {
int		begin = 0;	//contatore per il riempimento di counted_peaks
int		sum = 0;	//somma dei valori di counted_peaks
int		i;	//indice per scorrere counted_peaks
struct task_param	*tp;
	tp = (struct task_param *)arg;
	index_next = 0;
	set_period(tp);
	wait_for_period(tp);
	for(;;){
		read_counter();
		if (begin < 4)
			begin++;
		else {
			sum = 0;
			for(i = 0; i < 5; ++i)
				sum += counted_peaks[i];
			print_freq(sum * 6);
			diseases_report(sum * 6); // sum * 6 è la frequenza cardiaca calcolata
		}
		if (deadline_miss(tp))
			printf("Frequency task: deadline missed\n");
		wait_for_period(tp);
	}
}

