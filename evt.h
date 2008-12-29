#include "ev.h"
extern struct ev_loop *evt_loop;
extern void evt_init(void);
extern ev_timer *evt_timer_add_periodic(void *callback, const char *name, int interval);
/* MAKE DAMN SURE YOUR CALLBACK DOES A mem_free() ON TIMER!!! */
extern ev_timer *evt_timer_add_oneshot(void *callback, const char *name, int timeout);