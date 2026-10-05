WARNINGS = -Wall -Wextra
# portable by default; `make native` or e.g. ARCHFLAGS="-arch arm64 -arch x86_64"
# (run `make clean` first when changing it, so cubiomes is rebuilt too)
ARCHFLAGS ?=
CFLAGS = -O3 -flto -fwrapv -pthread -Icubiomes $(ARCHFLAGS) $(WARNINGS)
DEBUGFLAGS = -g -O0 -fwrapv -pthread -Icubiomes $(ARCHFLAGS) $(WARNINGS)
LDLIBS = -lm

# everything except the two files that define main()
LIB_SRCS = seedsearch.c util.c parseargs.c cpucount.c
HEADERS = $(wildcard *.h)
CUBIOMES = cubiomes/libcubiomes.a

.DEFAULT_GOAL := release
release: main
debug: main-debug

native:
	$(MAKE) ARCHFLAGS=-march=native

main: main.c $(LIB_SRCS) $(HEADERS) $(CUBIOMES)
	$(CC) $(CFLAGS) -o $@ $(filter-out %.h,$^) $(LDLIBS)

main-debug: main.c $(LIB_SRCS) $(HEADERS) $(CUBIOMES)
	$(CC) $(DEBUGFLAGS) -o $@ $(filter-out %.h,$^) $(LDLIBS)

test: test.c $(LIB_SRCS) $(HEADERS) $(CUBIOMES)
	$(CC) $(DEBUGFLAGS) -o $@ $(filter-out %.h,$^) $(LDLIBS)
	./test
	$(RM) -r ./test ./test.dSYM

$(CUBIOMES): cubiomes/makefile $(wildcard cubiomes/*.c cubiomes/*.h)
	$(MAKE) -C cubiomes release CC="$(CC)" CFLAGS="$(ARCHFLAGS)"

cubiomes/makefile:
	@echo "cubiomes submodule missing, run: git submodule update --init" >&2
	@exit 1

.PHONY: release clean test debug native

clean:
	$(RM) -r main main-debug test *.dSYM
	if [ -f cubiomes/makefile ]; then $(MAKE) -C cubiomes clean; fi
