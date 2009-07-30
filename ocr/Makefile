.PHONY: all clean

all: ocr.o

clean:
	-rm *.o

ocr.o: ocr.cpp
	$(CC) $(CFLAGS) `pkg-config --cflags opencv` $< -c -o $@
