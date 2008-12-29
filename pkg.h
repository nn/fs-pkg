struct pkg_handle {
         int	fd;		/* file descriptor for mmap() */
         int	refcnt;		/* references to this package */
         time_t	otime;		/* open time (for garbage collector) */
         char	*name;		/* package name */
         u_int32_t id;		/* package ID */
};

struct pkg_file_mapping {
       char *pkg;		/* package file */
       size_t len;		/* length of subfile */
       off_t offset;		/* offset of subfile */
       int fd;			/* file descriptor for mmap */
       void *addr;		/* mmap return address */
};

/* Initialize package handling/caching subsystem */
extern void pkg_init(void);

/* Open a package */
extern struct pkg_handle *pkg_open(const char *path);

/* Release our instance of package a*/
extern void pkg_close(struct pkg_handle *pkg);

/* 'discover' a package, called by vfs watcher */
extern int pkg_import(const char *path);

/* 'forget' a package, called by vfs_watcher */
extern int pkg_forget(const char *path);

/* pkg_map.c */
extern void pkg_unmap_file(struct pkg_file_mapping *p);
extern struct pkg_file_mapping *pkg_map_file(const char *path, size_t len, off_t offset);

/* pkg_toc.c */
extern size_t pkg_toc_zbufsize;
extern char *pkg_toc_extract(const char *path);
extern int pkg_toc_process(const char *path, const char *toc, u_int32_t pkgid);
