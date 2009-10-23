.PHONY: all clean

all: ocr.o kennen_ohnefloat.o

clean:
	-rm *.o

ocr.o: ocr.cpp
	$(CC) $(CFLAGS) `pkg-config --cflags opencv` $< -c -o $@

kennen_ohnefloat.o: kennen_ohnefloat.cpp
	$(CC) $(CFLAGS) `pkg-config --cflags opencv` $< -c -o $@
