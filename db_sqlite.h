#include <sys/types.h>
#include "inode.h"

extern void *db_query(enum db_query_res_type type, const char *fmt, ...);
extern int  db_sqlite_open(const char *path);
extern void db_sqlite_close(void);
extern void db_sqlite_close(void);
extern int  db_pkg_add(const char *path);
extern int  db_file_add(int pkg, const char *path, const char type,
                        uid_t owner, gid_t grp, size_t size, off_t offset, time_t ctime);
extern int  db_pkg_remove(const char *path);
extern int  db_file_remove(int pkg, const char *path);
extern void db_begin(void);
extern void db_commit(void);
extern void db_rollback(void);
