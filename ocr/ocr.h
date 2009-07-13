#ifndef __OCR__H__
#define __OCR__H__

#include <highgui.h>
#include <cv.h>
#include <cxcore.h>

#include "list.h"

/* Makros */

#define IB_ALLOC(IB,HEIGHT,WIDTH) do{\
			IB = (struct intern_bitmap *) \
				malloc(sizeof(struct intern_bitmap));\
			IB->buffer = (unsigned char *)malloc\
				(sizeof(unsigned char) * (HEIGHT) * (WIDTH));\
			IB->height = HEIGHT;\
			IB->width = WIDTH;\
		     }while(0)

/* Strukturen */

struct intern_bitmap {
	int width, height;
	int aufloesung;   /* die Länge, die bisschen größer als Störung ist */
	unsigned char *buffer;
	struct list_head list;
};


/* Funktionen */
int bm_getpixel(const struct intern_bitmap *bm, int row, int col);

int bm_setpixel(const struct intern_bitmap *bm, int row, int col,
						unsigned char pixelvalue);

int release_intern_bitmap(struct intern_bitmap *bm);

struct intern_bitmap *preprocess(IplImage *src);

struct intern_bitmap *cvmat2intern(CvMat *mat);

static CvMat *intern2cvmat(struct intern_bitmap *bm);

int ocr_bestpassend(IplImage *src, char *ergebnis, int laenge);

#endif /*__OCR__H__*/
