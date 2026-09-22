CFLAGS = -O3 -march=native -flto -Icubiomes
DEBUGFLAGS = -g -O0 -march=native -Icubiomes
LDLIBS = -lm

.DEFAULT_GOAL := release
release: main
debug: main-debug

main: seedsearch.c cpucount.c main.c seedsearch.h cpucount.h cubiomes/libcubiomes.a
	cc $(CFLAGS) -o $@ $(filter-out %.h,$^) $(LDLIBS)

main-debug: seedsearch.c cpucount.c main.c seedsearch.h cpucount.h cubiomes/libcubiomes.a
	cc $(DEBUGFLAGS) -o $@ $(filter-out %.h,$^) $(LDLIBS)

test: seedsearch.c test.c seedsearch.h cubiomes/libcubiomes.a
	cc $(DEBUGFLAGS) -o $@ $(filter-out %.h,$^) $(LDLIBS)
	./test
	$(RM) ./test

cubiomes/libcubiomes.a:
	$(MAKE) -C cubiomes release


.PHONY: release clean test debug

clean:
	$(RM) main main-debug test
	$(MAKE) -C cubiomes clean
