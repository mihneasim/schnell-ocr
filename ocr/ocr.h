#ifndef __OCR__H__
#define __OCR__H__

#include <highgui.h>
#include <cv.h>
#include <cxcore.h>

#include "list.h"
#include "global.h"

/* Makros */

#define BM_ALLOC(BM,HEIGHT,WIDTH) do{\
			BM = (struct intern_bitmap *) \
				malloc(sizeof(struct intern_bitmap));\
			BM->buffer = (unsigned char *)malloc\
				(sizeof(unsigned char) * (HEIGHT) * (WIDTH));\
			BM->height = HEIGHT;\
			BM->width = WIDTH;\
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

int bm_release(struct intern_bitmap *bm);

struct intern_bitmap *preprocess(IplImage *src);

struct intern_bitmap *bm_cvmat2bm(const CvMat *mat);

struct list_head*
		einfach_trennen(const struct intern_bitmap *bild);

struct intern_bitmap* zeichen_standardisieren(
		const struct intern_bitmap* zeichen);

int ocr_bestpassend(struct intern_bitmap *bm, char *ergebnis, int laenge);

/*CvMat *bm_bm2cvmat_kontrast(struct intern_bitmap *bm);*/

CvMat *bm_bm2cvmat(const struct intern_bitmap *bm);

int vektor_generieren(int *vektor, const struct intern_bitmap *zeichen);

#endif /*__OCR__H__*/
