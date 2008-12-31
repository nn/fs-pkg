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
#include "timestr.h"

static void vfs_fuse_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
   pkg_inode_t *i;
   struct stat sb;
   Log(LOG_DEBUG, "[%lu] %s:%d:%s", time(NULL), __FILE__, __LINE__, __FUNCTION__);

   if ((i = db_query(QUERY_INODE, "SELECT * FROM files WHERE inode = %d", (u_int32_t) ino)) != NULL) {
      /*
       * XXX: do stuff ;) 
       */
      sb.st_ino = ino;
      sb.st_mode = i->st_mode;
      sb.st_size = i->st_size;
      sb.st_uid = i->st_uid;
      sb.st_gid = i->st_gid;
      blockheap_free(inode_heap, i);
   }

   /*
    * Did we get a valid response? 
    */
   if (sb.st_ino)
      fuse_reply_attr(req, &sb, 0.0);
   else
      fuse_reply_err(req, ENOENT);
}

static void vfs_fuse_access(fuse_req_t req, fuse_ino_t ino, int mask) {
   Log(LOG_DEBUG, "[%lu] %s:%d:%s", time(NULL), __FILE__, __LINE__, __FUNCTION__);
   fuse_reply_err(req, 0);             /* success */
}

static void vfs_fuse_readlink(fuse_req_t req, fuse_ino_t ino) {
   Log(LOG_DEBUG, "[%lu] %s:%d:%s", time(NULL), __FILE__, __LINE__, __FUNCTION__);
   fuse_reply_err(req, ENOSYS);
}

static void vfs_fuse_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
   Log(LOG_DEBUG, "[%lu] %s:%d:%s", time(NULL), __FILE__, __LINE__, __FUNCTION__);
   fuse_reply_err(req, ENOENT);
}

static void vfs_fuse_readdir(fuse_req_t req, fuse_ino_t ino,
                             size_t size, off_t off, struct fuse_file_info *fi) {
   Log(LOG_DEBUG, "[%lu] %s:%d:%s", time(NULL), __FILE__, __LINE__, __FUNCTION__);
   fuse_reply_err(req, ENOENT);
}

static void vfs_fuse_releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
   Log(LOG_DEBUG, "[%lu] %s:%d:%s", time(NULL), __FILE__, __LINE__, __FUNCTION__);
   fuse_reply_err(req, ENOENT);
}

static void vfs_fuse_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
   struct vfs_handle *fh;
   fh = blockheap_alloc(vfs_handle_heap);
   fi->fh = ((uint64_t) fh);
   pkg_inode_t *i;
   struct stat sb;
   Log(LOG_DEBUG, "[%lu] %s:%d:%s", time(NULL), __FILE__, __LINE__, __FUNCTION__);

   /*
    * Look for fill inside a package 
    */
   if ((i = db_query(QUERY_INODE, "SELECT * FROM files WHERE inode = %d", (u_int32_t) ino))) {
      /*
       * XXX: do stuff ;) 
       */
   } else {
      /*
       * XXX: look it up in spill over 
       */
   }

   if (!i) {
      if (!fi->flags & O_CREAT) {
         fuse_reply_err(req, ENOENT);
      } else {
         /*
          * XXX: create spillover file instead of EROFS 
          */
         fuse_reply_err(req, EROFS);
      }
   }

   blockheap_free(inode_heap, i);
   fuse_reply_open(req, fi);
}

static void vfs_fuse_release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
   Log(LOG_DEBUG, "[%lu] %s:%d:%s", time(NULL), __FILE__, __LINE__, __FUNCTION__);
   fuse_reply_err(req, 0);             /* success */
}

static void vfs_fuse_read(fuse_req_t req, fuse_ino_t ino,
                          size_t size, off_t off, struct fuse_file_info *fi) {
   Log(LOG_DEBUG, "[%lu] %s:%d:%s", time(NULL), __FILE__, __LINE__, __FUNCTION__);
/*      reply_buf_limited(req, hello_str, strlen(hello_str), off, size); */
   fuse_reply_err(req, EBADF);
}

static void vfs_fuse_statfs(fuse_req_t req, fuse_ino_t ino) {
   Log(LOG_DEBUG, "[%lu] %s:%d:%s", time(NULL), __FILE__, __LINE__, __FUNCTION__);
   fuse_reply_err(req, ENOSYS);
}

static void vfs_fuse_getxattr(fuse_req_t req, fuse_ino_t ino, const char *name, size_t size) {
   Log(LOG_DEBUG, "[%lu] %s:%d:%s", time(NULL), __FILE__, __LINE__, __FUNCTION__);
   fuse_reply_err(req, ENOTSUP);
}

static void vfs_fuse_listxattr(fuse_req_t req, fuse_ino_t ino, size_t size) {
   Log(LOG_DEBUG, "[%lu] %s:%d:%s", time(NULL), __FILE__, __LINE__, __FUNCTION__);
   fuse_reply_err(req, ENOTSUP);
}

static void vfs_fuse_lookup(fuse_req_t req, fuse_ino_t parent, const char *name) {
   struct fuse_entry_param e;

   Log(LOG_DEBUG, "[%lu] %s:%d:%s", time(NULL), __FILE__, __LINE__, __FUNCTION__);
#if	0
   if (parent != 1)                    /* XXX: need checks here */
#endif
      fuse_reply_err(req, ENOENT);
#if	0
   else {
      memset(&e, 0, sizeof(e));
      e.ino = 2;                       /* Inode number */
      e.attr_timeout = 1.0;
      e.entry_timeout = 1.0;
      /*
       * XXX: stat 
       */
      vfs_fuse_stat(e.ino, &e.attr);
      fuse_reply_entry(req, &e);
   }
#endif
}
