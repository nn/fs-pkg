#if	!defined(NN2_MEMORY)
#define	NN2_MEMORY
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "logger.h"
/*
 * memory allocation wrappers
 *	These are lightweight and should catch the common allocation problems
 *	-- One could add pretty easily hooks to a debugging malloc in here
 *	-- or a garbage collector. ;)
 */
static __inline void *mem_alloc(size_t size) {
   register void *ptr;

   if ((ptr = malloc(size)) == NULL)
      Log(LOG_ERROR, "%s:malloc %d:%s", __FUNCTION__, errno, strerror(errno));

   memset(ptr, 0, size);

   return ptr;
}

static __inline void *mem_realloc(void *ptr, size_t size) {
   register void *nptr;

   if ((nptr = realloc(ptr, size)) == NULL)
      Log(LOG_ERROR, "%s:realloc %d:%s", __FUNCTION__, errno, strerror(errno));

   return nptr;
}

static __inline void mem_free(void *ptr) {
   /*
    * Force a crash if double free or a NULL ptr, should aid debugging 
    */
   assert(ptr != NULL);
   free(ptr);
   ptr = NULL;

   return;
}

static __inline void *mem_calloc(size_t nmemb, size_t size) {
   register void *p;

   assert(nmemb != 0);
   assert(size != 0);

   if ((p = calloc(nmemb, size)) == NULL)
      Log(LOG_ERROR, "%s:calloc %d:%s", __FUNCTION__, errno, strerror(errno));

   return p;
}
#endif                                 /* !defined(NN2_MEMORY) */
