CFLAGS = -O3 -Icubiomes
LDLIBS = -lm

main: main.c cubiomes/libcubiomes.a
	cc $(CFLAGS) -o $@ $^ $(LDLIBS)

spacinghelper: spacinghelper.c cubiomes/libcubiomes.a
	cc $(CFLAGS) -o $@ $^ $(LDLIBS)

cubiomes/libcubiomes.a:
	$(MAKE) -C cubiomes libcubiomes.a

.DEFAULT_GOAL := all
all: main spacinghelper cubiomes/libcubiomes.a
	$(MAKE) main 
	$(MAKE) spacinghelper
