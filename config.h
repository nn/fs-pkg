/* this should be suitable for linux... */

#define HAVE_SYS_STATFS_H 1
/* #undef HAVE_SYS_XATTR_H */
/* #undef HAVE_SYS_EXTATTR_H */
/* #undef HAVE_SYS_PARAM_H 1 */
/* #undef HAVE_LGETXATTR */
/* #undef HAVE_LSETXATTR */
/* #undef HAVE_GETXATTR */
/* #undef HAVE_SETXATTR */
/* #undef HAVE_GETATTRLIST */
/* #undef HAVE_SETATTRLIST */
/* #undef HAVE_CHFLAGS */
#define HAVE_STATVFS 1
#define HAVE_STATFS 1
#define HAVE_EXT2FS_EXT2_FS_H 1
/* #undef HAVE_STRUCT_STAT_ST_FLAGS */
/* #undef HAVE_STRUCT_STATVFS_F_FSTYPENAME */
/* #undef HAVE_STRUCT_STATFS_F_FSTYPENAME */
/* #undef HAVE_SYS_ACL_H */
/* #undef HAVE_LIBUTIL_H */
#define HAVE_ASPRINTF 1
/* #undef HAVE_LIBBZ2 */
/* #undef HAVE_LIBLZMA */
#define HAVE_LCHOWN 1
/* #undef HAVE_LCHMOD */
/* #undef HAVE_STRMODE */
#define UID_STRING RId32
#define UID_CAST (uint32_t)
#define GID_STRING PRId32
#define GID_CAST (uint32_t)
#define INO_STRING PRId64
#define INO_HEXSTRING PRIx64
#define INO_CAST (uint64_t)
#define DEV_STRING PRId64
#define DEV_HEXSTRING PRIx64
#define DEV_CAST (uint64_t)

/* Once spillover files are ready for use */
#undef	SPILLOVER
#define	SQLITE_OMIT_LOAD_EXTENSION
#define	SQLITE_THREADSAFE 1
#define	SQLITE_OMIT_SHARED_CACHE
