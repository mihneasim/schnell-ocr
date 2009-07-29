.PHONY: all clean

all: ocr.o lernen

clean:
	-rm lernen
	-rm *.o

ocr.o: ocr.cpp
	$(CC) $(CFLAGS) `pkg-config --cflags opencv` $< -c -o $@

lernen.o: lernen.cpp
	$(CC) $(CFLAGS) `pkg-config --cflags opencv` $< -c -o $@

lernen: lernen.o ocr.o
	$(CC) $(CFLAGS) `pkg-config --libs opencv` $^ -o $@
