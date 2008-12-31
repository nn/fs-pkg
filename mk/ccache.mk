
CCACHE := /usr/bin/ccache

# Try to use ccache, if available
ifeq (${CCACHE}, $(wildcard ${CCACHE}))
CC := ${CCACHE} ${CC}
endif
