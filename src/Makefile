.PHONY: all clean
CC=g++
all: ocr.o kennen_ohnefloat.o
CFLAGS=-fopenmp
clean:
	-rm *.o

ocr.o: ocr.cpp
	$(CC) $(CFLAGS) `pkg-config --cflags opencv` $< -c -o $@

kennen_ohnefloat.o: kennen_ohnefloat.cpp
	$(CC) $(CFLAGS) `pkg-config --cflags opencv` $< -c -o $@
