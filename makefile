WARNINGS = -Wall -Wextra
CFLAGS = -O3 -march=native -flto -pthread -Icubiomes $(WARNINGS)
DEBUGFLAGS = -g -O0 -march=native -pthread -Icubiomes $(WARNINGS)
LDLIBS = -lm

# everything except the two files that define main()
LIB_SRCS = seedsearch.c util.c parseargs.c cpucount.c
HEADERS = $(wildcard *.h)
CUBIOMES = cubiomes/libcubiomes.a

.DEFAULT_GOAL := release
release: main
debug: main-debug

main: main.c $(LIB_SRCS) $(HEADERS) $(CUBIOMES)
	cc $(CFLAGS) -o $@ $(filter-out %.h,$^) $(LDLIBS)

main-debug: main.c $(LIB_SRCS) $(HEADERS) $(CUBIOMES)
	cc $(DEBUGFLAGS) -o $@ $(filter-out %.h,$^) $(LDLIBS)

test: test.c $(LIB_SRCS) $(HEADERS) $(CUBIOMES)
	cc $(DEBUGFLAGS) -o $@ $(filter-out %.h,$^) $(LDLIBS)
	./test
	$(RM) -r ./test ./test.dSYM

$(CUBIOMES):
	$(MAKE) -C cubiomes release

.PHONY: release clean test debug

clean:
	$(RM) -r main main-debug test *.dSYM
	$(MAKE) -C cubiomes clean
