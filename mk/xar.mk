# NEED_XAR is misleading, we always need xar....
ifeq (${CONFIG_TOC_LIBXAR}, y)
CFLAGS += -DCONFIG_TOC_LIBXAR
NEED_XAR=y
endif
ifeq (${CONFIG_VFS_FUSE}, y)
CFLAGS += -DCONFIG_VFS_FUSE
NEED_XAR=y
endif

ifeq (${NEED_XAR}, y)
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
else
LDFLAGS += -lxar
endif

.obj/xar/%.o:xar/%.c
	@echo "[CC] $< => $@"
	@${CC} ${warn_flags} ${CFLAGS} -o $@ -c $<
