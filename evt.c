/*
 * evt.[ch]: Utility functions to make using libevent a bit more 'friendly' for neo-ircd
 * Copyright (C) 2008 N2 Networks LLC, Released under the BSD license. <neo@nn2.us>
 */
#include <stdlib.h>
#include "ev.h"
#include "memory.h"

struct ev_loop *evt_loop = NULL;

void evt_init(void) {
   evt_loop = ev_default_loop(0);
}

ev_timer *evt_timer_add_periodic(void *callback, const char *name, int interval) {
   ev_timer   *timer = mem_alloc(sizeof(ev_timer));
   ev_timer_init(timer, callback, 0, interval);
   ev_timer_start(evt_loop, timer);
   return timer;
}

/* MAKE DAMN SURE YOUR CALLBACK DOES A mem_free() ON TIMER!!! */
ev_timer *evt_timer_add_oneshot(void *callback, const char *name, int timeout) {
  ev_timer *timer = mem_alloc(sizeof(ev_timer));
  ev_timer_init(timer, callback, timeout, 0);
  ev_timer_start(evt_loop, timer);
  return timer;
}
