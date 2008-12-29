/*
 * XXX: This needs finished, it works but its very incomplete...
 * XXX: we should support other forms of threading and maybe
 * XXX: even a fork() based model for things too small for
 * XXX: pthreads lib, anyone have a use for that?
 * XXX; - jld 20081102 [bug: 33]
 */
#include "thread_pool.h"
#include <stdlib.h>
#include "memory.h"

thread_t   *thr_create(void *cb, void *arg, size_t stack_size) {
   thread_t   *ret = mem_alloc(sizeof(thread_t));

   if (ret == NULL)
      exit(EXIT_FAILURE);

   if ((ret->pa = mem_alloc(sizeof(pthread_attr_t))) == NULL)
      exit(EXIT_FAILURE);

   if (pthread_attr_init(ret->pa) != 0) {
      Log(LOG_ERROR, "%s:pthread_attr_init %d:%s", __FUNCTION__, errno, strerror(errno));
      exit(EXIT_FAILURE);
   }

   if (stack_size > 0)
      pthread_attr_setstacksize(ret->pa, stack_size);

   if (pthread_create(&ret->pth, ret->pa, cb, arg) != 0) {
      Log(LOG_ERROR, "%s:pthread_create %d:%s", __FUNCTION__, errno, strerror(errno));
      exit(EXIT_FAILURE);
   }

   return ret;
}
