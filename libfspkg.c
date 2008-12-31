#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

struct file_op {
  /* file ops */
  int (*open)(const char *pathname, int flags, ...);
  int (*close)(int fd);
  ssize_t (*readlink)(const char *path, char *buf, size_t bufsiz);
  ssize_t (*read)(int fd, void *buf, size_t count);

  /* file ops: spillover */

  /* access ops */
  int (*access)(const char *pathname, int mode);
  int (*statfs)(const char *path, struct statfs *buf);

  /* access ops: spillover */

  /* directory ops */
  DIR *(*opendir)(const char *name);
  struct dirent *(*readdir)(DIR *dirp);
  int (*closedir)(DIR *dirp);

  /* directory ops: spillover */
};

struct file_op real_ops;
struct file_op pkg_ops;

static void _init_lib(void) {
    real_ops.open = dlsym(RTLD_NEXT, "open");
    real_ops.close = dlsym(RTLD_NEXT, "close");
    real_ops.readlink = dlsym(RTLD_NEXT, "readlink");
    real_ops.read = dlsym(RTLD_NEXT, "read");
    real_ops.access = dlsym(RTLD_NEXT, "access");
    real_ops.statfs = dlsym(RTLD_NEXT, "statfs");
    real_ops.opendir = dlsym(RTLD_NEXT, "opendir");
    real_ops.readdir = dlsym(RTLD_NEXT, "readdir");
    real_ops.closedir = dlsym(RTLD_NEXT, "closedir");
}

static void _init(void) {
    _init_lib();
}

int open(const char *pathname, int flags, ...) {
    return real_ops.open(pathname, flags);
}

int close(int fd) {
    return real_ops.close(fd);
}

ssize_t readlink(const char *path, char *buf, size_t bufsiz) {
    return real_ops.readlink(path, buf, bufsiz);
}

ssize_t read(int fd, void *buf, size_t count) {
    return real_ops.read(fd, buf, count);
}

int access(const char *path, int mode) {
    return real_ops.access(path, mode);
}

int statfs(const char *path, struct statfs *buf) {
    return real_ops.statfs(path, buf);
}

DIR *opendir(const char *name) {
    return real_ops.opendir(name);
}

struct dirent *readdir(DIR *dirp) {
    return real_ops.readdir(dirp);
}

int closedir(DIR *dirp) {
    return real_ops.closedir(dirp);
}
  