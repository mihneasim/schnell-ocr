.PHONY: all clean

ocr_path=../ocr
ocr_objs=$(ocr_path)/ocr.o
#ocr_sources:=$(OCR_PATH)/ocr.cpp

all: schneiden

clean:
	-rm schneiden
	-rm *.o

schneiden.o: schneiden.cpp
	$(CC) $(CFLAGS) `pkg-config --cflags opencv` -I $(ocr_path)  $< -c -o $@

schneiden: schneiden.o $(ocr_objs)
	$(CC) $(CFLAGS) `pkg-config --libs opencv` $^  -o $@
