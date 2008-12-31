libs += libfspkg.so
lib_objs += .obj/libfspkg.o

libfspkg.so:${lib_objs}
	@echo "[LD] $^ => $@"
	@${LD} -lc ${lib_ldflags} -o $@ $^

.objs/%.o:%.c
	@echo "[CC] $< => $@"
	@${CC} ${warn_flags} ${lib_cflags} -o $@ -c $<
