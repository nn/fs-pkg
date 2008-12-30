#include <sys/stat.h>
#include <errno.h>
static __inline int is_dir(const char *path) {
   struct stat sb;

   stat(path, &sb);

   if (S_ISDIR(sb.st_mode))
      return 1;

   return 0;
}

static __inline int file_exists(const char *path) {
   struct stat sb;
   stat(path, &sb);

   if (errno == ENOENT)
      return 0;

   return 1;
}
