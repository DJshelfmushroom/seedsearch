CFLAGS = -O3 -march=native -flto -Icubiomes
LDLIBS = -lm

main: main.c cubiomes/libcubiomes.a
	cc $(CFLAGS) -o $@ $^ $(LDLIBS)

cubiomes/libcubiomes.a:
	$(MAKE) -C cubiomes release

.DEFAULT_GOAL := all
all: main

.PHONY: all clean

clean:
	$(RM) main
	$(MAKE) -C cubiomes clean
