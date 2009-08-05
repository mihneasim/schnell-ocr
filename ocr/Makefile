.PHONY: all clean

all: ocr.o methode1.o

clean:
	-rm *.o

ocr.o: ocr.cpp
	$(CC) $(CFLAGS) `pkg-config --cflags opencv` $< -c -o $@

methode1.o: methode1.cpp
	$(CC) $(CFLAGS) `pkg-config --cflags opencv` $< -c -o $@
