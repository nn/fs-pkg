# enable more debugging output (slower, noisier)
CONFIG_DEBUG=y

# debug block allocator? should be unneeded
CONFIG_DEBUG_BALLOC=n

# strip binaries? (debug disables, makes smaller)
CONFIG_STRIP_BINS=y

# strip libraries (debug disable, not recommended)
CONFIG_STRIP_LIBS=n

# use libxml2 to parse the TOC? (broken)
CONFIG_TOC_LIBXML2=n

# use libxar to parse the TOC? (slower)
CONFIG_TOC_LIBXAR=y

# use libfuse for VFS? (broken)
CONFIG_VFS_FUSE=n

# use LD_PRELOAD (doesn't support setuid)
CONFIG_VFS_LDPRELOAD=y


####################
# Compiler options #
####################
CFLAGS += -Os -g -pipe
CFLAGS += -I. -I./fuse -I/usr/include/libxml2
CFLAGS += -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -fPIC
warn_noerror := -Wall -Wno-unused -Wno-strict-aliasing -ansi -std=c99
warn_flags := ${warn_noerror} #-Werror
LDFLAGS := -lxml2 -lz -lcrypto -pthread -lrt -lsqlite3
