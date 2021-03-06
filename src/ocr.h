/*!
 * \file ocr.h
 * \brief anmeldungen der haupten Benutzerschnittstelle
 *
 * Diese Datei enthält den Teil, den direkt mit Benutzer kommunizieren.
 * Sie bietet die Haupt Routine (Schnittstelle) an.
 */
#ifndef __OCR__H__
#define __OCR__H__

#include <highgui.h>
#include <cv.h>
#include <cxcore.h>

#include "list.h"
#include "global.h"

/* Makros */

/*! allozieren ein neues intern_bitmap mit den gegebenen Parameter
 * \param BM		der Zeiger des intern_bitmaps
 * \param HEIGHT	die Höhe des neuen Bitmaps
 * \param WIDTH		das Breite des neuen Bitmaps
 * \return		keine Rückgabe
 */
#define BM_ALLOC(BM,HEIGHT,WIDTH) do{\
			BM = (struct intern_bitmap *) \
				malloc(sizeof(struct intern_bitmap));\
			BM->buffer = (unsigned char *)malloc\
				(sizeof(unsigned char) * (HEIGHT) * (WIDTH));\
			BM->height = HEIGHT;\
			BM->width = WIDTH;\
		     }while(0)

/* Strukturen */

/*!
 * \brief die wichtigsten Zwieschenstruktur
 *
 * Alle Arbeiten beziehen sich auf diese Struktur.
 * Die Struktur beschreibt ein viereckiges Bitmap mit beliebiger Größe.
 * in meisten Fälle ist das Bitmap eine binärisierte (zweigrauwertige) Abbildung
 */
struct intern_bitmap {
	/* Breite und Höhe des Bitmaps */
	int width, height;
	/*! ein bisschen größer als die Störung */
	int aufloesung;
	/*! Pufferzeiger, normalerweise zeigt es in eine Heap-zelle */
	unsigned char *buffer;
	/*! damit die Kette kann in in eine 
	 * allgemeine Liste hingesetzt wernden*/
	struct list_head list;
};


/* Funktionen */

void ocr_error(const char *msg);

int ocr_abs(int a);

long lquadrat(long a);

int bm_getpixel(const struct intern_bitmap *bm, int row, int col);

int bm_setpixel(const struct intern_bitmap *bm, int row, int col,
						unsigned char pixelvalue);

int bm_release(struct intern_bitmap *bm);

struct intern_bitmap* bm_skalieren(const struct intern_bitmap *org_bm,
					int new_height, int new_width);

struct intern_bitmap *preprocess(IplImage *src);

struct intern_bitmap *bm_cvmat2bm(const CvMat *mat);

struct list_head*
		einfach_trennen(const struct intern_bitmap *bild);

/*CvMat *bm_bm2cvmat_kontrast(struct intern_bitmap *bm);*/

#endif /*__OCR__H__*/
