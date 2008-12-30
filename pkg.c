/*
 * This file is ugly. It needs cleaned up and probably rewritten.
 *
 * Things are a bit intense in here as we try to do a bit of
 * caching of packages.
 *	* File is accessed causing package to be opened
 *		* ref count is increased (initially to 1)
 *	* File is closed
 *		* ref count is reduced
 *	* Another file in package is opened before the
 *	  garbage collect time out
 *		* Initial handle to package is reused
 *		* ref count increased
 *	....
 * This should help with things like compiling using several
 * headers from a package or starting programs which read
 * a bunch of config files.
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/file.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "atomicio.h"
#include "balloc.h"
#include "conf.h"
#include "db.h"
#include "evt.h"
#include "logger.h"
#include "memory.h"
#include "pkg.h"
#include "str.h"
#include "timestr.h"

/* private module-global stuff */
static BlockHeap *pkg_heap;            /* Block allocator heap */
static dlink_list pkg_list;            /* List of currently opened packages */
static time_t pkg_lifetime = 0;        /* see pkg_init() for initialization */

static dlink_node *pkg_findnode(struct pkg_handle *pkg) {
   dlink_node *ptr, *tptr;

   DLINK_FOREACH_SAFE(ptr, tptr, pkg_list.head) {
      if ((struct pkg_handle *)ptr->data == pkg)
         return ptr;
   }

   return NULL;
}

/*
 * release the package handle
 *
 * Should only be called by the Garbage Collector
 * unless we get tight on memory (XXX: Add this)
 */
static void pkg_release(struct pkg_handle *pkg) {
   dlink_node *ptr;

   /*
    * If name is allocated, free it 
    */
   if (pkg->name) {
      mem_free(pkg->name);
      pkg->name = NULL;
   }

   /*
    * Unlock the file 
    */
   flock(pkg->fd, LOCK_UN);

   /*
    * close handle, if exists 
    */
   if (pkg->fd) {
      close(pkg->fd);
      pkg->fd = -1;
   }

   if ((ptr = pkg_findnode((struct pkg_handle *)pkg)) != NULL) {
      dlink_delete(ptr, &pkg_list);
      blockheap_free(pkg_heap, ptr->data);
   }
}

/*
 * Find a package by name
 * Used by pkg_open to locate an existing reference
 * to a package that hasn't been garbage collected yet
 */
static struct pkg_handle *pkg_handle_byname(const char *path) {
   dlink_node *ptr, *tptr;
   struct pkg_handle *p;

   DLINK_FOREACH_SAFE(ptr, tptr, pkg_list.head) {
      p = (struct pkg_handle *)ptr->data;

      if (strcmp(p->name, path) == 0)
         return p;
   }

   return NULL;
}

/*
 * Scan for packages with refcnt == 0 every once in a while and close them
 * 	This helps to reduce closing and reopening packages unneededly
 *	Adjust tuning parameters in fs-pkg.cf as needed, based on your memory model.
 * if rfcnt == 0 && now > otime + (tuning.time.pkg_gc / 2): release it
 */
static void pkg_gc(int fd, short event, void *arg) {
   dlink_node *ptr, *tptr;
   struct pkg_handle *p;

   DLINK_FOREACH_SAFE(ptr, tptr, pkg_list.head) {
      p = (struct pkg_handle *)ptr->data;

      if (p->refcnt == 0 && (time(NULL) > p->otime + (pkg_lifetime / 2)))
         pkg_release(p);
   }
}

/*
 * Open a package, so that we can map files within it
 *
 * First, we try to see if the package already exists
 * in the cache. If it does, use it.
 *
 * Either way, we bump the ref count
 */
struct pkg_handle *pkg_open(const char *path) {
   struct pkg_handle *t;
   int         res = 0;

   /*
    * try to find an existing handle for the package
    * If this fails, create one and cache it...
    */
   if ((t = pkg_handle_byname(path)) == NULL) {
      t = blockheap_alloc(pkg_heap);
      t->name = str_dup(path);

      if ((t->fd = open(t->name, O_RDONLY)) < 0) {
         Log(LOG_ERROR, "failed opening pkg %s, bailing...", t->name);
         pkg_release(t);
         return NULL;
      }

      /*
       * Try to acquire an exclusive lock, fail if we cant 
       */
      if (flock(t->fd, LOCK_EX | LOCK_NB) == -1) {
         Log(LOG_ERROR, "failed locking package %s, bailing...", t->name);
         pkg_release(t);
         return NULL;
      }

      /*
       * Add handle to the cache list 
       */
      dlink_add_tail_alloc(t, &pkg_list);
   }

   /*
    * Adjust last used time and reference count either way... 
    */
   t->otime = time(NULL);
   t->refcnt++;

   return t;
}

/*
 * reduce the package's reference count
 *
 * We no longer free the package as the garbage collector
 * will do this for us after a delay, helping to keep packages
 * cached for a bit and reducing IO overhead
 */
void pkg_close(struct pkg_handle *pkg) {
   pkg->refcnt--;
   Log(LOG_DEBUG, "pkg_close: pkg %s refcnt == %d", pkg->name, pkg->refcnt);
}

/* Handle vfs_watch CREATED event */
int pkg_import(const char *path) {
   char       *tmp;
   int         infd, outfd;
   int         db_id = -1;

   /*
    * XXX: what do we do when an in-use package is modified?
    * XXX: for now I'm going to create a mandatory lock on
    * XXX: the package when it is opened, so this shouldn't happen
    */
   if (pkg_handle_byname(path)) {
      Log(LOG_HACK, "Active package %s was changed, ignoring...");
      return EXIT_SUCCESS;
   }

   Log(LOG_INFO, "importing pkg %s", path);

   db_begin();

   /*
    * Try to extract the package TOC 
    */
   if ((tmp = pkg_toc_extract(path)) == NULL) {
      Log(LOG_ERROR, "failed to export toc for %s", path);
      return EXIT_FAILURE;
   }

   Log(LOG_DEBUG, "TOC for %s extracted, processing", path);

   /*
    * Process the extracted TOC 
    */
   if (!pkg_toc_process(path, tmp)) {
      Log(LOG_DEBUG, "package %s seems valid, committing...", path);
      db_commit();
   } else {
      Log(LOG_ERROR, "failed adding package %s to database, skipping...", path);
      db_rollback();
   }

   /*
    * clean up: remove extracted toc and free its name buffer 
    */
   unlink(tmp);
   mem_free(tmp);
   return EXIT_SUCCESS;
}

/* Handle vfs_watch REMOVED event */
int pkg_forget(const char *path) {
   db_pkg_remove(path);
   return EXIT_SUCCESS;
}

void pkg_init(void) {
   if (!
       (pkg_heap =
        blockheap_create(sizeof(struct pkg_handle),
                         dconf_get_int("tuning.heap.pkg", 128), "pkg"))) {
      Log(LOG_FATAL, "pkg_init(): block allocator failed");
      raise(SIGTERM);
   }

   pkg_lifetime = nn2_timestr_to_time(dconf_get_str("tuning.timer.pkg_gc", NULL), 60);
   pkg_toc_zbufsize = dconf_get_int("tuning.toc.gz.buffer", 1310720);
   evt_timer_add_periodic(pkg_gc, "gc.pkg", pkg_lifetime);
}
