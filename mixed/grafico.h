#include "main.h"

#define unit_x 1.72
#define unit_y 150
#define baseY 200
#define baseX 10


extern float 	ecg[nsample];
extern BITMAP	*grafico;
extern int	sync_grafico;
extern pthread_mutex_t	sync_mutex;
extern pthread_cond_t	sync_var;
extern pthread_mutex_t	secg;

void	*task_grafico();
void	wait_to_draw();
void	draw_axes();
void	draw_graph();

