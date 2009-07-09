CFLAGS=-O2 -march=nocona -pipe -fomit-frame-pointer -mmmx -msse -msse2 -msse3

.PHONY: all ctags clean

MAKEFILE=Makefile

CFLAGS:=$(CFLAGS) -g

all: hauptteil tester

ctags:
	ctags -R `pkg-config --cflags opencv | sed -e 's/^.*-I\([^ \t]*\).*/\1/'` .

clean:
	-$(MAKE) -C ocr clean
	-rm *.o tester 

###############################################################
hauptteil:
	$(MAKE) -C ocr
	
###############################################################

tester_depend := ocr/ocr.o

tester: tester.o $(tester_depend)
	$(CC) `pkg-config --libs opencv` $^ -o $@

tester.o: tester.cpp
	$(CC) $(CFLAGS) `pkg-config --cflags opencv` -I ocr  $< -c -o $@
