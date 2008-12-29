#include <sys/stat.h>
static __inline	int	is_dir(const char *path) {
        struct stat sb;

        stat(path, &sb);

        if (S_ISDIR(sb.st_mode))
           return 1;

        return 0;
}

static	__inline int	file_exists(const char *path) {
        struct stat sb;
        return (stat(path, &sb) ? 1 : 0);
}
