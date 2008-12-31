all: world

include mk/config.mk
include mk/distcc.mk
include mk/ccache.mk
include mk/fuse.mk
include mk/indent.mk
include mk/xar.mk
include mk/bin.mk
include mk/lib.mk
include mk/clean.mk
 
ifeq (${CONFIG_DEBUG}, y)
CONFIG_STRIP_BINS=n
CFLAGS += -DCONFIG_DEBUG
endif

world:${bin} ${libs}

testpkg:
	xar --compression=none -c -f pkg/irssi.pkg /usr/bin/irssi /usr/lib/irssi /usr/share/irssi \
		  /usr/share/irssi /usr/share/man/man1/irssi.1.gz
