#include <sys/types.h>
#include <signal.h>
#include "conf.h"
#include "db.h"
#include "inode.h"
#include "logger.h"
BlockHeap  *inode_heap;

#if	0
void fill_statbuf(ext2_ino_t ino, pkg_inode_t * inode, struct stat *st) {
   /*
    * st_dev 
    */
   st->st_ino = ino;
   st->st_mode = inode->i_mode;
   st->st_nlink = inode->i_links_count;
   st->st_uid = inode->i_uid;          /* add in uid_high */
   st->st_gid = inode->i_gid;          /* add in gid_high */
   /*
    * st_rdev 
    */
   st->st_size = inode->i_size;
   st->st_blksize = EXT2_BLOCK_SIZE(fs->super);
   st->st_blocks = inode->i_blocks;

   /*
    * We don't have to implement nanosecs, fs's which don't can return
    * * 0 here
    */
   /*
    * Using _POSIX_C_SOURCE might also work 
    */
#ifdef __APPLE__
   st->st_atimespec.tv_sec = inode->i_atime;
   st->st_mtimespec.tv_sec = inode->i_mtime;
   st->st_ctimespec.tv_sec = inode->i_ctime;
   st->st_atimespec.tv_nsec = 0;
   st->st_mtimespec.tv_nsec = 0;
   st->st_ctimespec.tv_nsec = 0;
#else
   st->st_atime = inode->i_atime;
   st->st_mtime = inode->i_mtime;
   st->st_ctime = inode->i_ctime;
#ifdef __FreeBSD__
   st->st_atimespec.tv_nsec = 0;
   st->st_mtimespec.tv_nsec = 0;
   st->st_ctimespec.tv_nsec = 0;
#else
   st->st_atim.tv_nsec = 0;
   st->st_mtim.tv_nsec = 0;
   st->st_ctim.tv_nsec = 0;
#endif
#endif
}
#endif

void inode_init(void) {
   if (!
       (inode_heap =
        blockheap_create(sizeof(struct pkg_inode),
                         dconf_get_int("tuning.heap.inode", 128), "pkg"))) {
      Log(LOG_FATAL, "inode_init(): block allocator failed");
      raise(SIGTERM);
   }
}

void inode_fini(void) {
   blockheap_destroy(inode_heap);
}
