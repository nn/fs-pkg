ifeq (${CONFIG_VFS_FUSE}, y)
CFLAGS += -DCONFIG_VFS_FUSE
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

# Until i figure out how to work around these, let them throw warnings but dont abort comile....
.obj/fuse/%.o:fuse/%.c
	@echo "[CC] $< => $@"
	@${CC} ${warn_noerror} ${CFLAGS} -DFUSE_USE_VERSION=26 -DPACKAGE_VERSION="\"2.8.0-pre1\"" -DFUSERMOUNT_DIR="\"/bin\"" -DFUSERMOUNT_BIN="\"fusermount\"" -o $@ -c $<
.obj/vfs_fuse.o:vfs_fuse.c vfs_fuse.h vfs_pkg.h vfs_spill.h
	@echo "[CC] $< => $@"
	@${CC} ${warn_noerror} ${CFLAGS} -o $@ -c $<
