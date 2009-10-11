.PHONY: all clean

ocr_path=../ocr
#ocr_objs=$(ocr_path)/ocr.o $(ocr_path)/kennen_ohnefloat.o
ocr_objs=$(ocr_path)/*.o
#ocr_sources:=$(OCR_PATH)/ocr.cpp

all:lernen tester

clean:
	-rm vektor.txt
	-rm lernen tester
	-rm *.o

lernen.o: lernen.cpp
	$(CC) $(CFLAGS) `pkg-config --cflags opencv` -I $(ocr_path)  $< -c -o $@

lernen: lernen.o $(ocr_objs)
	$(CC) $(CFLAGS) `pkg-config --libs opencv` $^  -o $@


tester: tester.o $(ocr_objs)
	$(CC) `pkg-config --libs opencv` $^ -o $@

tester.o: tester.cpp 
	$(CC) $(CFLAGS) `pkg-config --cflags opencv` -I $(ocr_path) $< -c -o $@
