/*
 * FUSE VFS Spillover file support
 *
 * This file should NEVER be included directly outside
 * of vfs_fuse.c, in fact we throw an error if this
 * is done
 */
#if	!defined(VFS_FUSE_C)
#error never include vfs_fuse_spill.h outside vfs_fuse.c
#error this file is for internal use only
#endif
#include "timestr.h"
/* Write-type operations which should return EROFS */
static void vfs_fuse_setattr(fuse_req_t req, fuse_ino_t ino,
                             struct stat *attr, int to_set,
                             struct fuse_file_info *fi) {
      DBG;
      fuse_reply_err(req, EROFS);
}

static void vfs_fuse_mknod(fuse_req_t req, fuse_ino_t ino,
                           const char *name, mode_t mode,
                           dev_t rdev) {
      DBG;
      fuse_reply_err(req, EROFS);
}

static void vfs_fuse_mkdir(fuse_req_t req, fuse_ino_t ino,
                           const char *name, mode_t mode) {
      DBG;
      fuse_reply_err(req, EROFS);
}

static void vfs_fuse_symlink(fuse_req_t req, const char *link,
                             fuse_ino_t parent, const char *name) {
      DBG;
      fuse_reply_err(req, EROFS);
}

static void vfs_fuse_unlink(fuse_req_t req, fuse_ino_t ino,
                            const char *name) {
      DBG;
      fuse_reply_err(req, EROFS);
}

static void vfs_fuse_rmdir(fuse_req_t req, fuse_ino_t ino,
                            const char *namee) {
      DBG;
     fuse_reply_err(req, EROFS);
}

static void vfs_fuse_rename(fuse_req_t req, fuse_ino_t parent, const char *name,
                            fuse_ino_t newparent, const char *newname) {
      DBG;
     fuse_reply_err(req, EROFS);
}

static void vfs_fuse_link(fuse_req_t req, fuse_ino_t ino,
                          fuse_ino_t newparent, const char *newname) {
      DBG;
     fuse_reply_err(req, EROFS);
}

static void vfs_fuse_write(fuse_req_t req, fuse_ino_t ino, const char *buf,
                           size_t size, off_t off, struct fuse_file_info *fi) {
      DBG;
      fuse_reply_err(req, EROFS);
}

static void vfs_fuse_setxattr(fuse_req_t req, fuse_ino_t ino, const char *name,
                              const char *value, size_t size, int flags) {
      DBG;
      fuse_reply_err(req, ENOTSUP);
}

static void vfs_fuse_removexattr(fuse_req_t req, fuse_ino_t ino, const char *name) {
      /* XXX: which is proper: ENOSUP, EACCES, ENOATTR? */
      DBG;
      fuse_reply_err(req, ENOTSUP);
}

static void vfs_fuse_create(fuse_req_t req, fuse_ino_t ino, const char *name,
                            mode_t mode, struct fuse_file_info *fi) {
      DBG;
      fuse_reply_err(req, EROFS);
}
