DISTCC := /usr/bin/distcc
# Is DISTCC_HOSTS set?
ifneq (y${DISTCC_HOSTS}, y)
# Try to use distcc, if available
ifeq (${DISTCC}, $(wildcard ${DISTCC}))
CC := ${DISTCC}
endif
endif

export DISTCC_HOSTS
