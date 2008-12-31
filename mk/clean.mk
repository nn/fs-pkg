
extra_clean += ${bin}.log
extra_clean += ${bin}.db
extra_clean += ${bin}.db-journal

clean:
	${RM} ${objs} ${bin} ${extra_clean}

distclean:
	@${MAKE} clean
