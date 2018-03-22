#include "main.h"

static struct task_param	ecg_param, graph_param, calc_param;
static struct sched_param	ecg_par, graph_par, calc_par;
static pthread_attr_t 		ecg_attr, graph_attr, calc_attr, tast_attr;
static pthread_t 	ecg_id, calc_id, graph_id, tast_id;

static BITMAP	*bradicardia, *tachicardia, *scritta_soglie, *cuore;

void	time_copy(struct timespec *td, struct timespec ts){
	td->tv_sec = ts.tv_sec;
	td->tv_nsec = ts.tv_nsec;
}

void	time_add_us(struct timespec *t, long us){
	t->tv_sec += us / 1000000;
	t->tv_nsec += (us % 1000000) * 1000;
	if (t->tv_nsec > 1000000000) {
		t->tv_nsec -= 1000000000;
		t->tv_sec += 1;
	}
}

int	time_cmp(struct timespec t1, struct timespec t2){
	if (t1.tv_sec > t2.tv_sec) return 1;
	if (t1.tv_sec < t2.tv_sec) return -1;
	if (t1.tv_nsec > t2.tv_nsec) return 1;
	if (t1.tv_nsec < t2.tv_nsec) return -1;
	return 0;
}

void	set_period(struct task_param *tp){
struct timespec	t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	time_copy(&(tp->at), t);
	time_copy(&(tp->dl), t);
	time_add_us(&(tp->at), tp->period);
	time_add_us(&(tp->dl), tp->deadline);
}

int	deadline_miss(struct task_param *tp){
struct timespec	now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	if (time_cmp(now, tp->dl) > 0){
		tp->dmiss++;
		return 1;
	}
	return 0;
}

void	wait_for_period(struct task_param *tp){
	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &(tp->at), NULL);
	time_add_us(&(tp->at), tp->period);
	time_add_us(&(tp->dl), tp->period);
}

void	led(BITMAP *led1, BITMAP *led2){
	stretch_blit(led1, screen, 0, 0, led1->w, led1->h, led1_x, led1_y, led_dim, led_dim);
	stretch_blit(led2, screen, 0, 0, led2->w, led2->h, led2_x, led2_y, led_dim, led_dim);
}

void	close_button_handler(){
	pthread_kill(tast_id, SIGKILL);
}
END_OF_FUNCTION(close_button_handler);

void 	stampa_frequenze(){
char	buf[16];
	clear_bitmap(freq_min);
	clear_bitmap(freq_max);
	sprintf(buf, "Min %d", frequenza_minima);
	textout_ex(freq_min, font, buf, 0, 0, white, transparent);
	stretch_blit(freq_min, screen, 0, 0, freq_min->w, freq_min->h, freq_min_x, freq_min_y, freq_min->w * zoom, freq_min->h * zoom);
	sprintf(buf, "Max %d", frequenza_massima);
	textout_ex(freq_max, font, buf, 0, 0, white, transparent);
	stretch_blit(freq_max, screen, 0, 0, freq_max->w, freq_max->h, freq_max_x, freq_max_y, freq_max->w * zoom, freq_max->h * zoom);
}

void	init_variabili(){
int 	m;
	frequenza_massima = 130;
	frequenza_minima = 60;
	sync_calcolo = 0;
	sync_grafico = 0;
	pthread_mutex_init(&secg, NULL);
	pthread_mutex_init(&sqrs, NULL);
	pthread_mutex_init(&sync_mutex, NULL);
	for (m = 0; m < nsample; ++m)
		ecg[m] = 0.0;
	contatore_qrs = 0;
	pthread_cond_init(&sync_var, NULL);
}

void	init_task(){
	ecg_param.period = 16000;//7813;
	ecg_param.deadline = 16000;
	ecg_param.priority = 32;
	ecg_param.dmiss = 0;
	freq_param.period = 2000000;
	freq_param.deadline = 35000; //entro 5 campioni (che sarebbero 40000)
	freq_param.priority = 28;
	freq_param.dmiss = 0;
	ecg_par.sched_priority = ecg_param.priority;
	freq_par.sched_priority = freq_param.priority;
	pthread_attr_init(&ecg_attr);
	pthread_attr_init(&graph_attr);
	pthread_attr_init(&calc_attr);
	pthread_attr_init(&freq_attr);
	pthread_attr_init(&tast_attr);
	pthread_attr_setschedparam(&ecg_attr, &ecg_par);
	pthread_attr_setschedparam(&freq_attr, &freq_par);
	pthread_create(&ecg_id, &ecg_attr, task_ecg, &ecg_param);
	pthread_create(&calc_id, &calc_attr, task_calcolo, NULL);
	pthread_create(&graph_id, &graph_attr, task_grafico, NULL);
	pthread_create(&tast_id, &tast_attr, task_tastiera, NULL);
}

void	init_segnalazione(){
	bradicardia = create_bitmap(11 * lun_char, alt_char);
	tachicardia = create_bitmap(11 * lun_char, alt_char);
	clear_bitmap(bradicardia);
	clear_bitmap(tachicardia);
	textout_ex(bradicardia, font, "Bradicardia", 0, 0, white, transparent);
	stretch_blit(bradicardia, screen, 0, 0, bradicardia->w, bradicardia->h, brad_x, brad_y, bradicardia->w * zoom, bradicardia->h * zoom);
	textout_ex(tachicardia, font, "Tachicardia", 0, 0, white, transparent);
	stretch_blit(tachicardia, screen, 0, 0, tachicardia->w, tachicardia->h, tac_x, tac_y, tachicardia->w * zoom, tachicardia->h * zoom);
	led_on = load_bitmap("led_on.tga", NULL);
	if (!led_on)
		printf("Impossibile caricare led on!\n");
	led_off = load_bitmap("led_off.tga", NULL);
	if (!led_off)
		printf("Impossibile caricare led off!\n");
	led(led_off, led_off);
}

void	init_freq(){
	cuore = load_bitmap("heart.tga", NULL);
	if (!cuore)
		printf("Impossibile caricare il cuore!\n");
	stretch_blit(cuore, screen, 0, 0, cuore->w, cuore->h, cuore_x, cuore_y, 51, 42);
	freq = create_bitmap(3 * lun_char, alt_char);
	clear_to_color(freq, black);
	textout_ex(freq, font, "--", 0, 0, white, transparent);
	stretch_blit(freq, screen, 0, 0, freq->w, freq->h, freq_x, freq_y, freq->w * zoom, freq->h * zoom);
}

void	init_soglie(){
	freq_min = create_bitmap(7 * lun_char, alt_char); //3 per scritta + spazio + 3 per numero
	freq_max = create_bitmap(7 * lun_char, alt_char);
	scritta_soglie = create_bitmap(7 * lun_char, alt_char);
	clear_bitmap(scritta_soglie);
	textout_ex(scritta_soglie, font, "Soglie:", 0, 0, white, transparent);
	stretch_blit(scritta_soglie, screen, 0, 0, scritta_soglie->w, scritta_soglie->h, soglie_x, soglie_y, scritta_soglie->w * zoom, scritta_soglie->h * zoom);
	stampa_frequenze();
}

void	init_grafico(){
	grafico = create_bitmap(lunghezza, altezza);
	blit(grafico, screen, 0, 0, grafico_x, grafico_y, lunghezza, altezza);
}

void	init_finestra(){
	allegro_init();
	LOCK_FUNCTION(close_button_handler);
	set_close_button_callback(close_button_handler);
	install_keyboard();
	set_color_depth(8);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, 1080, 720, 0, 0);
	clear_to_color(screen, black);
}

int	main(){
char	scan;
	init_variabili();
	init_finestra();
	init_segnalazione();
	init_freq();
	init_soglie();
	init_grafico();
	init_task();
	pthread_join(tast_id, NULL);
	//pthread_join(ecg_id, NULL);
	return 0;
}
