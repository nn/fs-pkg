DEBUG=y

# strip binaries to make smaller? debug disables
STRIP_BINS=y

# Do we want to use FUSE? If not, build the database manager
# and LD_PRELOAD library libfs-pkg.so
USE_FUSE=y

# These should be good for most systems, adjust if not...
DISTCC := /usr/bin/distcc
CCACHE := /usr/bin/ccache

all: world

# Is DISTCC_HOSTS set?
ifneq (y${DISTCC_HOSTS}, y)
# Try to use distcc, if available
ifeq (${DISTCC}, $(wildcard ${DISTCC}))
CC := ${DISTCC}
endif
endif

# Try to use ccache, if available
ifeq (${CCACHE}, $(wildcard ${CCACHE}))
CC := ${CCACHE} ${CC}
endif
 
CFLAGS := -Os -g -pipe
CFLAGS += -I. -I./fuse -I/usr/include/libxml2
CFLAGS += -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -fPIC
warn_noerror := -Wall -Wno-unused -Wno-strict-aliasing -ansi -std=c99
warn_flags := ${warn_noerror} #-Werror
LDFLAGS := -lxml2 -lz -lcrypto -pthread -lrt -lsqlite3

bin := fs-pkg
objs += .obj/atomicio.o
objs += .obj/balloc.o
objs += .obj/conf.o
objs += .obj/db_sqlite.o
objs += .obj/dictionary.o
objs += .obj/dlink.o
objs += .obj/ev.o
objs += .obj/evt.o
objs += .obj/inode.o
objs += .obj/logger.o
objs += .obj/main.o
objs += .obj/pkg.o
objs += .obj/pkg_map.o
objs += .obj/pkg_toc.o
objs += .obj/signal_handler.o
objs += .obj/str.o
objs += .obj/support.o
objs += .obj/thread_pool.o
objs += .obj/timestr.o
objs += .obj/vfs.o
objs += .obj/vfs_inotify.o
objs += .obj/xar/archive.o
objs += .obj/xar/arcmod.o
objs += .obj/xar/b64.o
objs += .obj/xar/bzxar.o
objs += .obj/xar/darwinattr.o
objs += .obj/xar/data.o
objs += .obj/xar/ea.o
objs += .obj/xar/err.o
objs += .obj/xar/ext2.o
objs += .obj/xar/fbsdattr.o
objs += .obj/xar/filetree.o
objs += .obj/xar/hash.o
objs += .obj/xar/io.o
objs += .obj/xar/linuxattr.o
objs += .obj/xar/lzmaxar.o
objs += .obj/xar/macho.o
objs += .obj/xar/script.o
objs += .obj/xar/signature.o
objs += .obj/xar/stat.o
objs += .obj/xar/subdoc.o
objs += .obj/xar/util.o
objs += .obj/xar/zxar.o

ifeq (${USE_FUSE}, y)
CFLAGS += -DUSE_FUSE
objs += .obj/vfs_fuse.o
objs += .obj/fuse/fuse.o
objs += .obj/fuse/fuse_kern_chan.o
objs += .obj/fuse/fuse_loop.o
objs += .obj/fuse/fuse_loop_mt.o
objs += .obj/fuse/fuse_lowlevel.o
objs += .obj/fuse/fuse_mt.o
objs += .obj/fuse/fuse_opt.o
objs += .obj/fuse/fuse_session.o
objs += .obj/fuse/fuse_signals.o
objs += .obj/fuse/helper.o
objs += .obj/fuse/mount.o
objs += .obj/fuse/mount_util.o
endif

ifeq (${DEBUG}, y)
STRIP_BINS=n
CFLAGS += -DDEBUG
endif

extra_clean += ${bin}.log
extra_clean += ${bin}.db
extra_clean += ${bin}.db-journal

world: ${bin}
${bin}: ${objs}
	@${CC} -o $@ ${LDFLAGS} ${extra_libs} $^
ifeq (${STRIP_BINS}, y)
	strip ${bin}
endif

clean:
	${RM} ${objs} ${bin} ${extra_clean}

distclean:
	@${MAKE} clean

test:
	sudo xar --compression=none -c -f /pkg/irssi.pkg /usr/bin/irssi /usr/lib/irssi /usr/share/irssi \
		  /usr/share/irssi /usr/share/man/man1/irssi.1.gz

# Until i figure out how to work around these, let them throw warnings but dont abort comile....
.obj/fuse/%.o:fuse/%.c
	@echo "[CC] $< => $@"
	@${CC} ${warn_noerror} ${CFLAGS} -DFUSE_USE_VERSION=26 -DPACKAGE_VERSION="\"2.8.0-pre1\"" -DFUSERMOUNT_DIR="\"/bin\"" -DFUSERMOUNT_BIN="\"fusermount\"" -o $@ -c $<
.obj/vfs_fuse.o:vfs_fuse.c vfs_fuse.h vfs_pkg.h vfs_spill.h
	@echo "[CC] $< => $@"
	@${CC} ${warn_noerror} ${CFLAGS} -o $@ -c $<
.obj/sqlite3.o:sqlite3.c
	@echo "[CC] $< => $@"
	@${CC} ${warn_noerror} ${CFLAGS} -D_HAVE_SQLITE_CONFIG_H=1 -o $@ -c $<

# everything else is good...
.obj/%.o:%.c
	@echo "[CC] $< => $@"
	@${CC} ${warn_flags} ${CFLAGS} -o $@ -c $<

.obj/xar/%.o:xar/%.c

export DISTCC_HOSTS
