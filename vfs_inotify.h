extern int vfs_watch_init(void);
extern void vfs_watch_fini(void);
extern vfs_watch_t *vfs_watch_add(const char *path);
extern int vfs_watch_remove(vfs_watch_t *watch);
