/*
 * FUSE VFS Package file operations
 *
 * This file should NEVER be included directly outside
 * of vfs_fuse.c, in fact we throw an error if this
 * is done
 */
#if	!defined(VFS_FUSE_C)
#error never include vfs_fuse_pkg.h outside vfs_fuse.c
#error this file is for internal use only
#endif
#include "db_sqlite.h"
#include "timestr.h"

static void vfs_fuse_getattr(fuse_req_t req, fuse_ino_t ino,
                            struct fuse_file_info *fi) {
      pkg_inode_t *i;
      struct stat sb;
      DBG;

      if (ino == FUSE_ROOT_ID) {
         Log(LOG_INFO, "got root inode %lu", ino);
         ino = vfs_root_inode;
         sb.st_ino = ino;
      } else if ((i = db_inode_lookup((u_int32_t)ino)) != NULL) {
         /* XXX: do stuff ;) */
      }

      /* Did we get a valid response? */
      if (sb.st_ino)
         fuse_reply_attr(req, &sb, 0.0);
      else
        fuse_reply_err(req, ENOENT);
}

static void vfs_fuse_access(fuse_req_t req, fuse_ino_t ino, int mask) {
      DBG;
      fuse_reply_err(req, 0);	/* success */
}

static void vfs_fuse_readlink(fuse_req_t req, fuse_ino_t ino) {
      DBG;
      fuse_reply_err(req, ENOSYS);
}

static void vfs_fuse_opendir(fuse_req_t req, fuse_ino_t ino,
                             struct fuse_file_info *fi) {
      DBG;
      fuse_reply_err(req, ENOENT);
}

static void vfs_fuse_readdir(fuse_req_t req, fuse_ino_t ino,
                             size_t size, off_t off,
                             struct fuse_file_info *fi) {
      DBG;
      fuse_reply_err(req, ENOENT);
}

static void vfs_fuse_releasedir(fuse_req_t req, fuse_ino_t ino,
                                 struct fuse_file_info *fi) {
      DBG;
      fuse_reply_err(req, ENOENT);
}

static void vfs_fuse_open(fuse_req_t req, fuse_ino_t ino,
                          struct fuse_file_info *fi) {
      struct vfs_handle *fh;
      fh = blockheap_alloc(vfs_handle_heap);
      fi->fh = ((uint64_t)fh);
      DBG;
      fuse_reply_open(req, fi);
}

static void vfs_fuse_release(fuse_req_t req, fuse_ino_t ino,
                             struct fuse_file_info *fi) {
      DBG;
      fuse_reply_err(req, 0); /* success */
}

static void vfs_fuse_read(fuse_req_t req, fuse_ino_t ino,
                          size_t size, off_t off,
                          struct fuse_file_info *fi) {
      DBG;
/*      reply_buf_limited(req, hello_str, strlen(hello_str), off, size); */
      fuse_reply_err(req, EBADF);
}

static void vfs_fuse_statfs(fuse_req_t req, fuse_ino_t ino) {
      DBG;
      fuse_reply_err(req, ENOSYS);
}

static void vfs_fuse_getxattr(fuse_req_t req, fuse_ino_t ino,
                              const char *name, size_t size) {
      DBG;
      fuse_reply_err(req, ENOTSUP);
}

static void vfs_fuse_listxattr(fuse_req_t req, fuse_ino_t ino, size_t size) {
      DBG;
      fuse_reply_err(req, ENOTSUP);
}

static void vfs_fuse_lookup(fuse_req_t req, fuse_ino_t parent,
                           const char *name) {
      struct fuse_entry_param e;

      DBG;
#if	0
      if (parent != 1) /* XXX: need checks here */
#endif
         fuse_reply_err(req, ENOENT);
#if	0
      else {
         memset(&e, 0, sizeof(e));
         e.ino = 2; /* Inode number */
         e.attr_timeout = 1.0;
         e.entry_timeout = 1.0;
         /* XXX: stat */
         vfs_fuse_stat(e.ino, &e.attr);
         fuse_reply_entry(req, &e);
      }
#endif
}