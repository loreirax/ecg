#include <stdlib.h>
#include "main.h"

static struct task_param 	graph_param;
static struct sched_param 	comp_par, graph_par, kb_par, comp_par;
static pthread_attr_t 	graph_attr, comp_attr, kb_attr;
static pthread_t 	comp_id, graph_id, kb_id;

static BITMAP 	*brad, *tac, *th_string, *heart, *clock_img;

void 	time_copy(struct timespec *td, struct timespec ts) {
	td->tv_sec = ts.tv_sec;
	td->tv_nsec = ts.tv_nsec;
}

void 	time_add_us(struct timespec *t, long us) {
	t->tv_sec += us / 1000000;
	t->tv_nsec += (us % 1000000) * 1000;
	if (t->tv_nsec > 1000000000) {
		t->tv_nsec -= 1000000000;
		t->tv_sec += 1;
	}
}

int 	time_cmp(struct timespec t1, struct timespec t2) {
	if (t1.tv_sec > t2.tv_sec) return 1;
	if (t1.tv_sec < t2.tv_sec) return -1;
	if (t1.tv_nsec > t2.tv_nsec) return 1;
	if (t1.tv_nsec < t2.tv_nsec) return -1;
	return 0;
}

void 	set_period(struct task_param *tp) {
struct timespec	t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	time_copy(&(tp->at), t);
	time_copy(&(tp->dl), t);
	time_add_us(&(tp->at), tp->period);
	time_add_us(&(tp->dl), tp->deadline);
}

int 	deadline_miss(struct task_param *tp) {
struct timespec	now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	if (time_cmp(now, tp->dl) > 0) {
		tp->dmiss++;
		return 1;
	}
	return 0;
}

void 	wait_for_period(struct task_param *tp) {
	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &(tp->at), NULL);
	time_add_us(&(tp->at), tp->period);
	time_add_us(&(tp->dl), tp->period);
}

void 	led(BITMAP *led1, BITMAP *led2) {
	stretch_blit(led1, screen, 0, 0, led1->w, led1->h, led1_x, led1_y, led_dim, led_dim);
	stretch_blit(led2, screen, 0, 0, led2->w, led2->h, led2_x, led2_y, led_dim, led_dim);
}

void 	close_button_handler() {
	pthread_cancel(kb_id);
	pthread_cancel(graph_id);
	pthread_cancel(freq_id);
	pthread_cancel(comp_id);
	allegro_exit();
	exit(0);
}
END_OF_FUNCTION(close_button_handler);

void 	print_limit_fr() {
char 	buf[16];
int 	f1, f2;
	clear_bitmap(min_freq);
	clear_bitmap(max_freq);
	pthread_mutex_lock(&sth);
	f1 = min_fr;
	f2 = max_fr;
	pthread_mutex_unlock(&sth);
	sprintf(buf, "Min %d", f1);
	textout_ex(min_freq, font, buf, 0, 0, white, transparent);
	stretch_blit(min_freq, screen, 0, 0, min_freq->w, min_freq->h, min_freq_x, min_freq_y, min_freq->w * zoom, min_freq->h * zoom);
	sprintf(buf, "Max %d", f2);
	textout_ex(max_freq, font, buf, 0, 0, white, transparent);
	stretch_blit(max_freq, screen, 0, 0, max_freq->w, max_freq->h, max_freq_x, max_freq_y, max_freq->w * zoom, max_freq->h * zoom);
}

void 	file_open() {
int 	m;
FILE 	*ecg_file;
	if ((ecg_file = fopen(file_name, "r")) == NULL) {
		printf("Can't open file '%s'\n", file_name);
		exit(1);
	}
	rewind(ecg_file);
	for (m = 0; m < duration; ++m)
		fscanf(ecg_file, "%f", &ecg[m]);
	fclose(ecg_file);
}


void 	init_variables() {
pthread_mutexattr_t 	matt;

	max_fr = 120;
	min_fr = 50;
	last_frequency = -1;
	index_in = 5;
	ecg_frequency = 100; //Hz

	pthread_mutexattr_init(&matt);
	pthread_mutexattr_setprotocol(&matt, PTHREAD_PRIO_INHERIT);
	pthread_mutex_init(&secg, &matt);
	pthread_mutex_init(&sqrs, &matt);
	pthread_mutex_init(&sper, &matt);
	
	pthread_mutex_init(&sfr, NULL);
	pthread_mutex_init(&sth, NULL);
	pthread_mutex_init(&sfecg, NULL);
	
	counter_qrs = 0;
}

void 	init_tasks() {
	comp_param.period = 1000000 / ecg_frequency;
	comp_param.deadline = comp_param.period;
	comp_param.priority = 32;
	comp_param.dmiss = 0;
	freq_param.period = 2000000;
	freq_param.deadline = 45000; //entro 5 campioni (che sarebbero 50000)
	freq_param.priority = 28;
	freq_param.dmiss = 0;
	graph_param.period = 20000;
	graph_param.deadline = 20000;
	graph_param.priority = 30;
	graph_param.dmiss = 0;
	comp_par.sched_priority = comp_param.priority;
	freq_par.sched_priority = freq_param.priority;
	graph_par.sched_priority = graph_param.priority;
	kb_par.sched_priority = 26;
	pthread_attr_init(&graph_attr);
	pthread_attr_init(&comp_attr);
	pthread_attr_init(&freq_attr);
	pthread_attr_init(&kb_attr);
	pthread_attr_setschedparam(&freq_attr, &freq_par);
	pthread_attr_setschedparam(&graph_attr, &graph_par);
	pthread_attr_setschedparam(&comp_attr, &comp_par);
	pthread_attr_setschedparam(&kb_attr, &kb_par);
	pthread_create(&kb_id, &kb_attr, task_wait_keyboard, NULL);
	comp_param.arg = kb_id;
	pthread_create(&comp_id, &comp_attr, task_compute, NULL);
	pthread_create(&graph_id, &graph_attr, task_graph, &graph_param);
}

void 	init_diseases_alert() {
	brad = create_bitmap(11 * len_char, hgt_char);
	tac = create_bitmap(11 * len_char, hgt_char);
	clear_bitmap(brad);
	clear_bitmap(tac);
	textout_ex(brad, font, "Bradicardia", 0, 0, white, transparent);
	stretch_blit(brad, screen, 0, 0, brad->w, brad->h, brad_x, brad_y, brad->w * zoom, brad->h * zoom);
	textout_ex(tac, font, "Tachicardia", 0, 0, white, transparent);
	stretch_blit(tac, screen, 0, 0, tac->w, tac->h, tac_x, tac_y, tac->w * zoom, tac->h * zoom);
	led_on = load_bitmap("led_on.tga", NULL);
	if (!led_on)
		printf("Can't load led on image\n");
	led_off = load_bitmap("led_off.tga", NULL);
	if (!led_off)
		printf("Can't load led off image\n");
	led(led_off, led_off);
}

void 	init_freq() {
char	buf[7];
	heart = load_bitmap("heart.tga", NULL);
	if (!heart)
		printf("Can't load heart image!\n");
	clock_img = load_bitmap("clock.tga", NULL);
	if (!clock_img)
		printf("Can't load clock image!\n");
	stretch_blit(heart, screen, 0, 0, heart->w, heart->h, heart_x, heart_y, heart_dim, heart_dim * 4 / 5);
	stretch_blit(clock_img, screen, 0, 0, clock_img->w, clock_img->h, clock_x, clock_y, clock_dim, clock_dim * 4 / 5);
	freq = create_bitmap(7 * len_char, hgt_char);
	ecg_freq = create_bitmap(6 * len_char, hgt_char);
	clear_to_color(freq, black);
	textout_ex(freq, font, "--", 0, 0, white, transparent);
	stretch_blit(freq, screen, 0, 0, freq->w, freq->h, freq_x, freq_y, freq->w * zoom, freq->h * zoom);
	clear_to_color(ecg_freq, black);
	sprintf(buf, "%d Hz", ecg_frequency);
	textout_ex(ecg_freq, font, buf, 0, 0, white, transparent);
	stretch_blit(ecg_freq, screen, 0, 0, ecg_freq->w, ecg_freq->h, ecg_freq_x, ecg_freq_y, ecg_freq->w * zoom, ecg_freq->h * zoom);
}

void 	init_thresholds() {
	min_freq = create_bitmap(7 * len_char, hgt_char); //3 per scritta + spazio + 3 per numero
	max_freq = create_bitmap(7 * len_char, hgt_char);
	th_string = create_bitmap(7 * len_char, hgt_char);
	clear_bitmap(th_string);
	textout_ex(th_string, font, "Soglie:", 0, 0, white, transparent);
	stretch_blit(th_string, screen, 0, 0, th_string->w, th_string->h, th_x, th_y, th_string->w * zoom, th_string->h * zoom);
	print_limit_fr();
}

void 	init_graph() {
	graph = create_bitmap(len_graph, hgt_graph);
	blit(graph, screen, 0, 0, graph_x, graph_y, len_graph, hgt_graph);
}

void 	init_window() {
	allegro_init();
	LOCK_FUNCTION(close_button_handler);
	set_close_button_callback(close_button_handler);
	install_keyboard();
	set_color_depth(8);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, 1080, 720, 0, 0);
	clear_to_color(screen, black);
}

int 	main() {
	file_open();
	init_variables();
	init_window();
	init_diseases_alert();
	init_freq();
	init_thresholds();
	init_graph();
	init_tasks();
	pthread_join(kb_id, NULL);
	pthread_cancel(graph_id);
	pthread_cancel(freq_id);
	pthread_cancel(comp_id);
	allegro_exit();
	exit(0);
	return 0;
}
