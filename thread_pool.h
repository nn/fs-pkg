#include <pthread.h>
typedef struct thread_t {
   pthread_t   pth;
   pthread_attr_t *pa;
   int         refcnt;                 /* Reference count */
} thread_t;

extern thread_t *thr_create(void *cb, void *arg, size_t stack_size);
