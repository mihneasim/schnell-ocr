#ifndef __OCR__H__
#define __OCR__H__

#include <highgui.h>
#include <cv.h>
#include <cxcore.h>

#include "list.h"

#define IB_ALLOC(IB,HEIGHT,WIDTH) do{\
			IB = (struct intern_bitmap *) \
				malloc(sizeof(struct intern_bitmap));\
			IB->buffer = (unsigned char *)malloc\
				(sizeof(unsigned char) * (HEIGHT) * (WIDTH));\
		     }while(0)

struct intern_bitmap {
	int width, height;
	int aufloesung;   /* die Länge, die bisschen größer als Störung ist */
	unsigned char *buffer;
	struct list_head list;
};

struct intern_bitmap *cvmat2intern(CvMat *mat);
int ocr_bestpassend(IplImage *src, char *ergebnis, int laenge);

#endif
