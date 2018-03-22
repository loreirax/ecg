#include "frequenza.h"
static int	misure[5];
static int	index_m;

void	read_counter(){
	pthread_mutex_lock(&sqrs);
	misure[index_m] = contatore_qrs;
	contatore_qrs = 0;
	pthread_mutex_unlock(&sqrs);
	index_m = (index_m + 1) % 5;
}

void 	deseases_report(int frequenza_registrata){
	if (frequenza_registrata <= frequenza_minima){
		led(led_on, led_off);
	} else 
		if (frequenza_registrata >= frequenza_massima){
			led(led_off, led_on);
		} else {
			led(led_off, led_off);
		}
}

void	print_freq(int value){
char	s[3];
	sprintf(s,"%d", value);
	clear_to_color(freq, black);
	textout_ex(freq, font, s, 0, 0, white, transparent);
	stretch_blit(freq, screen, 0, 0, freq->w, freq->h, freq_x, freq_y, freq->w * zoom, freq->h * zoom);
}

void *task_frequenza(void *arg){
int	begin = 0;
int	sum = 0;
int	l;
struct task_param	*tp;
	tp = (struct task_param *)arg;
	index_m = 0;
	set_period(tp);
	wait_for_period(tp);
	for(;;){
		read_counter();
		if (begin < 4)
			begin++;
		else {
			sum = 0;
			for(l = 0; l < 5; ++l)
				sum += misure[l];
			print_freq(sum * 6);
			deseases_report(sum * 6); // sum * 6 è la frequenza cardiaca calcolata
		}
		if (deadline_miss(tp))
			printf("Deadline missed per la frequenza\n");
		wait_for_period(tp);
	}
}

