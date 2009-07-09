#ifndef __OCR__H__
#define __OCR__H__

#include <highgui.h>
#include <cv.h>
#include <cxcore.h>

#include "list.h"


struct intern_bitmap {
	int width, height;
	int aufloesung;   /* die Länge, die bisschen größer als Störung ist */
	unsigned char *buffer;
	struct list_head list;
};

int cvmat2intern(CvMat *mat, struct intern_bitmap *bm);
int ocr_bestpassend(IplImage *src, char *ergebnis, int laenge);

#endif
