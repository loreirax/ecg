#include <stdlib.h>
#include "main.h"

static struct task_param 	read_param, graph_param;
static struct sched_param 	read_par, graph_par;
static pthread_attr_t 	read_attr, graph_attr, comp_attr, kb_attr;
static pthread_t 	read_id, comp_id, graph_id, kb_id;

static BITMAP 	*brad, *tac, *th_string, *heart;

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
	pthread_cancel(read_id);
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

void 	init_variables() {
int 	m;
pthread_mutexattr_t 	matt;

	max_fr = 130;
	min_fr = 60;
	last_frequency = -1;
	sync_compute = 0;

	pthread_mutexattr_init(&matt);
	pthread_mutexattr_setprotocol(&matt, PTHREAD_PRIO_INHERIT);
	pthread_mutex_init(&secg, &matt);

	pthread_mutex_init(&sqrs, NULL);
	pthread_mutex_init(&sfr, NULL);
	pthread_mutex_init(&sth, NULL);
	pthread_mutex_init(&sync_mutex, NULL);
	for (m = 0; m < n_shown_samples; ++m)
		ecg[m] = 0.0;
	counter_qrs = 0;
	pthread_cond_init(&sync_var, NULL);
}

void 	init_tasks() {
	read_param.period = 8000;
	read_param.deadline = 8000;
	read_param.priority = 32;
	read_param.dmiss = 0;
	freq_param.period = 2000000;
	freq_param.deadline = 35000; //entro 5 campioni (che sarebbero 40000)
	freq_param.priority = 28;
	freq_param.dmiss = 0;
	graph_param.period = 16000;
	graph_param.deadline = 16000;
	graph_param.priority = 30;
	graph_param.dmiss = 0;
	read_par.sched_priority = read_param.priority;
	freq_par.sched_priority = freq_param.priority;
	graph_par.sched_priority = read_param.priority;
	pthread_attr_init(&read_attr);
	pthread_attr_init(&graph_attr);
	pthread_attr_init(&comp_attr);
	pthread_attr_init(&freq_attr);
	pthread_attr_init(&kb_attr);
	pthread_attr_setschedparam(&read_attr, &read_par);
	pthread_attr_setschedparam(&freq_attr, &freq_par);
	pthread_attr_setschedparam(&graph_attr, &graph_par);
	pthread_create(&read_id, &read_attr, task_read, &read_param);
	pthread_create(&comp_id, &comp_attr, task_compute, NULL);
	pthread_create(&graph_id, &graph_attr, task_graph, &graph_param);
	pthread_create(&kb_id, &kb_attr, task_wait_keyboard, NULL);
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
	heart = load_bitmap("heart.tga", NULL);
	if (!heart)
		printf("Can't load heart image!\n");
	stretch_blit(heart, screen, 0, 0, heart->w, heart->h, heart_x, heart_y, 51, 42);
	freq = create_bitmap(3 * len_char, hgt_char);
	clear_to_color(freq, black);
	textout_ex(freq, font, "--", 0, 0, white, transparent);
	stretch_blit(freq, screen, 0, 0, freq->w, freq->h, freq_x, freq_y, freq->w * zoom, freq->h * zoom);
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
	pthread_cancel(read_id);
	allegro_exit();
	exit(0);
	return 0;
}
