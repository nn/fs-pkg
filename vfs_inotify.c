/*
 *      file: vfs_inotify.c
 *    author: nn <nn@nn2.us>
 * copyright: 2008 N2 Networks LLC, All rights reserved.
 *   license: MIT
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include "conf.h"
#include "dlink.h"
#include "evt.h"
#include "logger.h"
#include "pkg.h"
#include "vfs.h"

#define	INOTIFY_BUFSIZE	((sizeof(struct inotify_event) + FILENAME_MAX) * 1024)
static int vfs_inotify_fd = 0;
static ev_io vfs_inotify_evt;

static dlink_node *vfs_watch_findnode(vfs_watch_t *watch) {
        dlink_node *ptr, *tptr;

        DLINK_FOREACH_SAFE(ptr, tptr, vfs_watch_list.head) {
           if ((vfs_watch_t *)ptr->data == watch)
              return ptr;
        }

        return NULL;
}

static dlink_node *vfs_watch_findnode_byfd(const int fd) {
        dlink_node *ptr, *tptr;

        DLINK_FOREACH_SAFE(ptr, tptr, vfs_watch_list.head) {
           if (((vfs_watch_t *)ptr->data)->fd == fd)
              return ptr;
        }

        return NULL;
}

/*
 *    function: vfs_inotify_evt_get
 * description: process the events inotify has given us
 */
void vfs_inotify_evt_get(struct ev_loop *loop, ev_io *w, int revents) {
       ssize_t len, i = 0;
       char buf[INOTIFY_BUFSIZE] = {0};
       char path[PATH_MAX];
       dlink_node *ptr;

       len = read(w->fd, buf, INOTIFY_BUFSIZE);

       while (i < len) {
          struct inotify_event *e = (struct inotify_event *)&buf[i];

          if (e->name[0] == '.')
             continue;


          if ((ptr = vfs_watch_findnode_byfd(e->wd)) == NULL) {
             Log(LOG_ERROR, "got watch for unregistered fd %d.. wtf?", e->wd);
             return;
          }

          /* If we don't have a name, this is useless event... */
          if (!e->len)
             return;

          memset(path, 0, PATH_MAX);
          snprintf(path, PATH_MAX - 1, "%s/%s", ((vfs_watch_t *)ptr->data)->path, e->name);

          if (e->mask & IN_CLOSE_WRITE || e->mask & IN_MOVED_TO)
             pkg_import(path);
          else if (e->mask & IN_DELETE || e->mask & IN_MOVED_FROM)
             pkg_forget(path);
          else if (e->mask & IN_DELETE_SELF) {
                vfs_watch_remove((vfs_watch_t *)ptr->data);
          }

          i += sizeof(struct inotify_event) + e->len;
       }
}

int	vfs_watch_init(void) {
        char buf[PATH_MAX];
        char *p;
        if (vfs_inotify_fd)    
           return -1;

        /* Try to initialize inotify interface */
        if ((vfs_inotify_fd = inotify_init()) == -1) {
           Log(LOG_ERROR, "%s:inotify_init %d:%s", __FUNCTION__, errno, strerror(errno));
           return -2;
        }
 
        /* Setup the socket callback */      
        ev_io_init(&vfs_inotify_evt, vfs_inotify_evt_get, vfs_inotify_fd, EV_READ);
        ev_io_start(evt_loop, &vfs_inotify_evt);

        /* set up the blockheap */
        vfs_watch_heap = blockheap_create(sizeof(vfs_watch_t), dconf_get_int("tuning.heap.vfs_watch", 32), "vfs_watch");

        /* add all the paths in the config file */
        memcpy(buf, dconf_get_str("path.pkg", "/pkg"), PATH_MAX);
        for (p = strtok(buf, ":\n"); p; p = strtok(NULL, ":\n"))
            vfs_watch_add(p);

        return 0;
}

void	vfs_watch_fini(void) {
        blockheap_destroy(vfs_watch_heap);
}

vfs_watch_t *vfs_watch_add(const char *path) {
        vfs_watch_t *wh = blockheap_alloc(vfs_watch_heap);

        wh->mask = IN_CLOSE_WRITE|IN_MOVED_TO|IN_MOVED_FROM|IN_DELETE|IN_DELETE_SELF;
        memcpy(wh->path, path, sizeof(wh->path));

        if ((wh->fd = inotify_add_watch(vfs_inotify_fd, path, wh->mask)) <= 0) {
           Log(LOG_ERROR, "failed creating vfs watcher for path %s", wh->path);
           blockheap_free(vfs_watch_heap, wh);
           return NULL;
        }

        dlink_add_tail_alloc(wh, &vfs_watch_list);
        return wh;
}

/* XXX: We need to scan the watch lists and remove subdirs too -bk */
int	vfs_watch_remove(vfs_watch_t *watch) {
        int rv;
        dlink_node *ptr;

        if ((rv = inotify_rm_watch(watch->fd, vfs_inotify_fd)) != 0) {
           Log(LOG_ERROR, "error removing watch for %s (fd: %d)", watch->path, watch->fd);
        }

        if ((ptr = vfs_watch_findnode(watch)) != NULL) {
           dlink_delete(ptr, &vfs_watch_list);
           blockheap_free(vfs_watch_heap, ptr->data);
        }

        return rv;
}
