#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <time.h>
#include "balloc.h"

struct	vfs_handle {
        char	pkg_file[PATH_MAX];	/* package path */
        off_t	pkg_offset;		/* offset within package */
        char	file[PATH_MAX];		/* file name */
        size_t	len;			/* length of file */
        off_t	offset;			/* current offset in file */
        char	*maddr;			/* mmap()'d region of package */
};

struct vfs_watch {
        u_int32_t mask;
        int fd;
        char path[PATH_MAX];
};

struct vfs_fake_stat {
        mode_t st_mode;
        off_t st_size;
        uid_t st_uid;
        gid_t st_gid;
        time_t st_time;
        char path[PATH_MAX];
};

typedef struct vfs_handle vfs_handle_t;
typedef struct vfs_watch vfs_watch_t;

extern int vfs_dir_walk(void);

extern BlockHeap *vfs_handle_heap;
extern BlockHeap *vfs_watch_heap;
extern u_int32_t vfs_root_inode;
extern dlink_list vfs_watch_list;

#include "vfs_fuse.h"
#include "vfs_inotify.h"
