#include <sys/types.h>
#include <sys/param.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <stdarg.h>
#include "balloc.h"
#include "conf.h"
#include "logger.h"
#include "pkg.h"
#include "util.h"
#include "vfs.h"

u_int32_t vfs_root_inode;

BlockHeap *vfs_handle_heap = NULL;
BlockHeap *vfs_watch_heap = NULL;
dlink_list vfs_watch_list;

/* We do not want to infinitely recurse... */
#define	MAX_RECURSE	16

static void vfs_dir_walk_recurse(const char *path, int depth) {
    DIR *d;
    struct dirent *r;
    char buf[PATH_MAX];

    if ((d = opendir(path)) == NULL) {
       Log(LOG_ERROR, "opendir %s failed, skipping", path);
       return;
    }

    while ((r = readdir(d)) != NULL) {
       /* Skip over hidden/disabled items */
       if (r->d_name[0] == '.')
          continue;
       memset(buf, 0, PATH_MAX);
       snprintf(buf, PATH_MAX - 1, "%s/%s", path, r->d_name);

       if (is_dir(buf)) {
          if (++depth < MAX_RECURSE)
             vfs_dir_walk_recurse(buf, depth);
          else
             Log(LOG_INFO, "%s: reached maximum depth (%d) in %s", __FUNCTION__, MAX_RECURSE, path);
       } else
          pkg_import(buf);
       
    }

    closedir(d);
}

int vfs_dir_walk(void) {
    char buf[PATH_MAX];
    char *p;

    memcpy(buf, dconf_get_str("path.pkg", "/pkg"), PATH_MAX);

    /* recurse each part of the optionally ':' seperated list */
    for (p = strtok(buf, ":\n"); p; p = strtok(NULL, ":\n")) {
       /* Run the recursive walker */
       vfs_dir_walk_recurse(p, 1);
    }
    return EXIT_SUCCESS;
}
