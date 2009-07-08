CFLAGS=-O2 -march=nocona -pipe -fomit-frame-pointer -mmmx -msse -msse2 -msse3

.PHONY:all ctags clean

MAKEFILE=Makefile

CFLAGS:=$(CFLAGS) -g

all:tester 

ctags:
	ctags -R `pkg-config --cflags opencv | sed -e 's/^.*-I\([^ \t]*\).*/\1/'` .

clean:
	-rm *.o tester 

	
###############################################################

tester_depend :=

tester: tester.o $(tester_depend)
	$(CC) `pkg-config --libs opencv` $(CFLAGS) $^ -o $@

tester.o: tester.cpp
	$(CC) `pkg-config --cflags opencv`  $< -c -o $@
