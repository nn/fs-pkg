#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "balloc.h"
#include "dlink.h"
#include "logger.h"
#include "memory.h"
#include "pkg.h"
#include "str.h"

/* This seems to be a BSD thing- it's not fatal if missing, so stub it */
#if	!defined(MAP_NOSYNC)
#define	MAP_NOSYNC	0
#endif                                 /* !defined(MAP_NOSYNC) */

static BlockHeap *pkg_file_mapping_heap = NULL;

void pkg_unmap_file(struct pkg_file_mapping *p) {
   if (p->addr != NULL && p->addr != MAP_FAILED)
      munmap(p->addr, p->len);

   if (p->pkg != NULL)
      mem_free(p->pkg);

   if (p->fd > 0)
      close(p->fd);

   blockheap_free(pkg_file_mapping_heap, p);
   p = NULL;
}

struct pkg_file_mapping *pkg_map_file(const char *path, size_t len, off_t offset) {
   struct pkg_file_mapping *p;

   p = blockheap_alloc(pkg_file_mapping_heap);

   p->pkg = str_dup(path);
   p->len = len;
   p->offset = offset;

   if (!(p->fd = open(p->pkg, O_RDONLY)) == -1) {
      Log(LOG_ERROR, "%s:open:%s %d:%s", __FUNCTION__, p->pkg, errno, strerror(errno));
      pkg_unmap_file(p);
      return NULL;
   }

   if ((p->addr =
        mmap(0, len, PROT_READ | PROT_EXEC, MAP_NOSYNC | MAP_PRIVATE,
             p->fd, offset)) == MAP_FAILED) {
      Log(LOG_ERROR, "%s:mmap: %d:%s", __FUNCTION__, errno, strerror(errno));
      pkg_unmap_file(p);
   }

   return p;
}
