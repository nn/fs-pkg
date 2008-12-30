#include <signal.h>
#include <errno.h>
#define	FUSE_USE_VERSION 26
#include <fuse/fuse_i.h>
#include <fuse/fuse_lowlevel.h>
#include "conf.h"
#include "db.h"
#include "evt.h"
#include "logger.h"
#include "memory.h"
#include "pkg.h"
#include "vfs.h"
#define	DBG	      fprintf(stderr, "[%lu] %s:%d:%s\n", conf.now - conf.born, __FILE__, __LINE__, __FUNCTION__)
#define	VFS_FUSE_C                    /* required for vfs_fuse_spill.h */
#include "vfs_pkg.h"
#include "vfs_spill.h"
#undef	VFS_FUSE_C

static ev_io vfs_fuse_evt;
static struct fuse_chan *vfs_fuse_chan = NULL;
static struct fuse_session *vfs_fuse_sess = NULL;
struct fuse_args vfs_fuse_args = { 0, NULL, 0 };

/* FUSE operations */
static struct fuse_lowlevel_ops vfs_fuse_ops = {
/* File operations */
   .lookup = vfs_fuse_lookup,
   .readlink = vfs_fuse_readlink,
   .open = vfs_fuse_open,
   .release = vfs_fuse_release,
   .read = vfs_fuse_read,
/* Stat/permissions */
   .getattr = vfs_fuse_getattr,
   .access = vfs_fuse_access,
   .statfs = vfs_fuse_statfs,
   .getxattr = vfs_fuse_getxattr,
   .listxattr = vfs_fuse_listxattr,
/* Directory operations */
   .opendir = vfs_fuse_opendir,
   .readdir = vfs_fuse_readdir,
   .releasedir = vfs_fuse_releasedir,
/* Spillover operations, unsupported for now */
   .create = vfs_fuse_create,
   .mknod = vfs_fuse_mknod,
   .mkdir = vfs_fuse_mkdir,
   .symlink = vfs_fuse_symlink,
   .unlink = vfs_fuse_unlink,
   .rmdir = vfs_fuse_rmdir,
   .rename = vfs_fuse_rename,
   .link = vfs_fuse_link,
   .write = vfs_fuse_write,
   .setxattr = vfs_fuse_setxattr,
   .removexattr = vfs_fuse_removexattr
};

/* This is a hacked up version of fuse/fuse_loop.c, hope it works -bk */
static void vfs_fuse_read_cb(struct ev_loop *loop, ev_io * w, int revents) {
   int         res = 0;
   struct fuse_chan *ch = fuse_session_next_chan(vfs_fuse_sess, NULL);
   struct fuse_chan *tmpch = ch;
   size_t      bufsize = fuse_chan_bufsize(ch);
   char       *buf;

   if (!(buf = mem_alloc(bufsize))) {
      Log(LOG_FATAL, "fuse: failed to allocate read buffer\n");
      conf.dying = 1;
      return;
   }

   res = fuse_chan_recv(&tmpch, buf, bufsize);

   if (!(res == -EINTR || res <= 0))
      fuse_session_process(vfs_fuse_sess, buf, res, tmpch);

   mem_free(buf);
   fuse_session_reset(vfs_fuse_sess);
}

void vfs_fuse_fini(void) {
   if (vfs_fuse_sess != NULL)
      fuse_session_destroy(vfs_fuse_sess);

   if (vfs_fuse_chan != NULL) {
      fuse_session_remove_chan(vfs_fuse_chan);
      fuse_unmount(dconf_get_str("path.mountpoint", "/"), vfs_fuse_chan);
   }

   if (vfs_fuse_args.allocated)
      fuse_opt_free_args(&vfs_fuse_args);
}

void vfs_fuse_init(void) {
   if ((vfs_fuse_chan = fuse_mount(conf.mountpoint, &vfs_fuse_args)) == NULL) {
      Log(LOG_FATAL, "FUSE: mount error");
      conf.dying = 1;
      raise(SIGTERM);
   }

   if ((vfs_fuse_sess = fuse_lowlevel_new(&vfs_fuse_args, &vfs_fuse_ops,
                                          sizeof(vfs_fuse_ops), NULL)) != NULL) {
      fuse_session_add_chan(vfs_fuse_sess, vfs_fuse_chan);
   } else {
      Log(LOG_FATAL, "FUSE: unable to create session");
      conf.dying = 1;
      raise(SIGTERM);
   }

   /*
    * Register an interest in events on the fuse fd 
    */
   ev_io_init(&vfs_fuse_evt, vfs_fuse_read_cb, fuse_chan_fd(vfs_fuse_chan), EV_READ);
   ev_io_start(evt_loop, &vfs_fuse_evt);

   /*
    * Set up our various blockheaps 
    */
   vfs_handle_heap =
       blockheap_create(sizeof(vfs_handle_t),
                        dconf_get_int("tuning.heap.vfs_handle", 128), "vfs_handle");
}
