#include <stdio.h>
#include <string.h>

#include <highgui.h>
#include <cv.h>
#include <cxcore.h>


#include "global.h"
#include "ocr.h"

/* Diese Datei enthält den Teil, den direkt mit Benutzer kommunizieren.
 * Sie bietet die Haupt Routine (Schnittstelle) an.
 */

/*die Operationen an den Datenstruktur intern_bitmap: */
int bm_getpixel(const struct intern_bitmap *bm, int row, int col)
{
	/* es wird angenommen, dass der Puffer von intern_bitmap
	 * der unsigned char typ ist */

	/* row und col fängt mit 0 an */
	if ((col >= bm->width) || (row >= bm->height))
		return  256;
	return bm->buffer[row * bm->width + col];
}

int bm_setpixel(const struct intern_bitmap *bm, int row, int col,
						unsigned char pixelvalue)
{
	if ((col >= bm->width) || (row >= bm->height))
		return 256;
	bm->buffer[row * bm->width + col] = pixelvalue;
	return pixelvalue;
}

struct intern_bitmap* bm_skalieren(const struct intern_bitmap *org_bm, int new_height, int new_width)
{
	int i,j;
	struct intern_bitmap *neu_bm;
	/*float verhaeltnis_width, verhaeltnis_height;*/

	if (new_width <= 0 || new_height <= 0)
		return NULL;

	BM_ALLOC(neu_bm, new_height, new_width);
	
	/* initialisieren */
	for (i = 0; i < new_height; i++) {
		for (j = 0; j < new_width; j++) {
			bm_setpixel(neu_bm, i, j, 0);
		}
	}

	for (i = 0; i < new_height; i++) {
		for (j = 0; j < new_width; j++) {
			/* vermeiden die Float operation!! */
			int pixel;
			pixel = bm_getpixel(org_bm,
					i * org_bm->height / new_height,
					j * org_bm->width / new_width);
			bm_setpixel(neu_bm, i, j, pixel);
		}
	}
	
	#ifdef DEBUG
	for (i = 0; i < new_height; i++) {
		for (j = 0; j < new_width; j++) {
			printf("%c",bm_getpixel(neu_bm, i, j) == 0 ? ' ' : 'X');
		}
		printf("\n");
	}
	#endif

	return neu_bm;
}

int bm_release(struct intern_bitmap *bm)
{
	if (bm) {
		if (bm->buffer) {
			free(bm->buffer);
		}
		free(bm);
	}
	return 0;
}

struct intern_bitmap *bm_cvmat2bm(const CvMat *mat)
{
	struct intern_bitmap *bm;

	bm = (struct intern_bitmap *)malloc(sizeof(struct intern_bitmap));

	/* transformieren die OpenCV Struktur zur der internen Struktur*/
	bm->width = mat->cols;
	bm->height = mat->rows;
	bm->buffer = (unsigned char*) malloc(mat->height *
					mat->width * sizeof(unsigned char));
	 /* memcpy ist effektiver als die andere Routen,
	  * aber memcpy bei OpenCV wird nicht ganz richtig Ergebnis bekommen*/
	memcpy(bm->buffer, mat->data.ptr,
			mat->height * mat->width * sizeof(unsigned char));

	bm->aufloesung = AUFLOESUNG_KONSTANT;

	/*
	#ifdef DEBUG
	for(int i = 0; i < bm->height; i++) {
		for (int j = 0; j < bm->width; j++) {
			if (bm_getpixel(bm, i, j))
				printf("X");
			else
				printf(" ");
		}
		printf("\n");
	}
	printf("\n\n");
	#endif
	*/

	return bm;
}

CvMat *bm_bm2cvmat_cp(const struct intern_bitmap *bm)
{
	/* die Puffer einfach kopieren */
	CvMat *mat;
	mat = cvCreateMat(bm->height, bm->width, CV_8UC1);
	memcpy(mat->data.ptr, bm->buffer,
			mat->height * mat->width * sizeof(unsigned char));
	/*
	#ifdef DEBUG
	for(int i = 0; i < bm->height; i++) {
		for (int j = 0; j < bm->width; j++) {
			if (bm_getpixel(bm, i, j))
				printf("X");
			else
				printf(" ");
		}
		printf("\n");
	}
	printf("\n\n");
	#endif
	*/

	return mat;
}

static CvMat *bm_bm2cvmat_kontrast(const struct intern_bitmap *bm)
{
	/* falls eine Zelle nicht 0, dann ist sie 255 , für Debug*/
	CvMat *mat;
	int i;
	unsigned char *p;
	
	mat = bm_bm2cvmat_cp(bm);

	for (i = 0, p = mat->data.ptr;
	     i < mat->height * mat->width * sizeof(unsigned char);
	     i++,p++) {
		if (*p) *p = 255;
	}


	return mat;
}


/* Die Umwandlung zwischen OpenCV und intern_bitmap */
struct intern_bitmap *preprocess(IplImage *src)
{
	CvMat *mat;
	struct intern_bitmap *bm;


	mat = cvCreateMat(src->height, src->width, CV_8UC1);

	/* binärisieren : */
	cvAdaptiveThreshold(src, mat, 255, CV_ADAPTIVE_THRESH_MEAN_C,
						CV_THRESH_BINARY_INV, 35, 37);
	/* nun ist 'mat' eine binäre Matrix, die 0xFF und 0 enthält */

	#ifdef DEBUG
	cvNamedWindow("Demo Window", CV_WINDOW_AUTOSIZE);
	cvShowImage("Demo Window", mat);
	cvWaitKey(-1);
	cvDestroyWindow("Demo Window");
	#endif
	bm = bm_cvmat2bm(mat);
	cvReleaseMat(&mat);
	return bm;
}

/*  ein einfachstes Trennungsschema */
static struct list_head*
		projektion_spalten_trennen(const struct intern_bitmap *zeile)
{
	/* liefert eine Liste, in den die einzelnen Zeichen als
	 * struct intern_bitmap zurück*/
	int i, ii, j;
	int *projektion;
	struct list_head *zeichenliste;

	projektion = (int *) malloc(sizeof(int) * zeile->width);

	for (i = 0; i < zeile->width; i++) {
		projektion[i] = 0;
		for (j = 0; j < zeile->height; j++) {
			if (bm_getpixel(zeile, j, i))
				projektion[i]++;
		}
	}

	/*
	#ifdef DEBUG
	printf("Projektiontrennung:\n");
	for (i = 0; i < zeile->width; i++) {
		printf("%d ",projektion[i]);
	}
	printf("\n");
	#endif
	*/

	zeichenliste = (struct list_head*)malloc(sizeof(struct list_head));
	INIT_LIST_HEAD(zeichenliste);

	/* zuerst wird das hässliche Verfahren implementiert.
	 * Wenn die andere Funktionen komplementiert werden,
	 * würde ich es optimieren
	 */

	/* zerlegen und kopieren die Zeile zu einzelnem Zeichen in einer Liste */
	for (i = 0; i < zeile->width; i++) {
		if (projektion[i] != 0) {
			int grenze; /* relative verschieb zu i in der Zeile */
			struct intern_bitmap *p;
			for (grenze = 0;((i + grenze < zeile->width) &&
					(projektion[i + grenze])); grenze++);

			/* ein neuer Knoten einzufügen  */
			BM_ALLOC(p, zeile->height, grenze);
			list_add_tail(&p->list, zeichenliste);

			/* kopieren Daten von einzelnem Zeichen */
			/* width und height muss unbedingt
			 * vor der Kopie initialisiert werden */
			/*p->width = grenze;
			p->height = zeile->height;*/
			p->aufloesung = AUFLOESUNG_KONSTANT;

			for (ii = 0; ii < grenze; ii++) {
				for (j = 0; j < zeile->height; j++) {
					bm_setpixel(p, j, ii,
					    bm_getpixel(zeile, j, i + ii));
				}
			}
			printf("grenze: %d\n",grenze);

			i += grenze;
		}
	}

	free(projektion);

	#ifdef DEBUG
	/* Das Ergebnis anzeigen*/
	/*
	{
		struct intern_bitmap *bm;
		struct list_head *p;
		list_for_each(p, zeichenliste){
			bm = list_entry(p, struct intern_bitmap, list);
			printf("DEBUG:%dx%d\n", bm->height, bm->width);
			cvNamedWindow("Debug Window", CV_WINDOW_AUTOSIZE);
			cvShowImage("Debug Window", bm_bm2cvmat(bm));
			cvWaitKey(-1);
			cvDestroyWindow("Debug Window");
		}
	}
	*/
	#endif

	return zeichenliste;
}


static struct list_head*
		zeilen_trennen(const struct intern_bitmap *bild)
{
	int i, j, ii, jj;
	int alt_zeilennummer; /* letzte leer zeile */
	struct list_head *zeilenliste;
	struct intern_bitmap *neuezeile;


	zeilenliste = (struct list_head*)malloc(sizeof(struct list_head));
	INIT_LIST_HEAD(zeilenliste);

	alt_zeilennummer = -1;
	for (i = 0; i <= bild->height; i++) {

		for (j = 0;
		     (i != bild->height)
		     && (j < bild->width)
		     && (!bm_getpixel(bild, i, j))
		     ; j++);

		if (j == bild->width || i == bild->height) {
			if (alt_zeilennummer + 1 != i) {
				/* eine gültige zeile gefunden */
				/* initialisieren und in die liste einfügen*/
				BM_ALLOC(neuezeile, i - alt_zeilennummer - 1,
								bild->width);
				list_add_tail(&neuezeile->list, zeilenliste);
				/* Zeiledaten kopieren */
				/* muss ein Funktion bm_bitblt individuell
				 * implementieren!!! */
				neuezeile->aufloesung = AUFLOESUNG_KONSTANT;
				for (ii = alt_zeilennummer + 1; ii < i; ii++) {
					for (jj = 0; jj < bild->width; jj++) {
						bm_setpixel(neuezeile,
							    ii - alt_zeilennummer - 1,
							    jj,
							    bm_getpixel(
								    bild,
								    ii,
								    jj)
							   );
					}
				}
			}
			alt_zeilennummer = i;

			/*
			#ifdef DEBUG
			printf("alt_zeilennummer: %d\n", alt_zeilennummer);
			#endif
			*/
		}
	}
	return zeilenliste;
}


static struct list_head*
		einfach_trennen(const struct intern_bitmap *bild)
{
	struct list_head  *p;
	struct list_head *zeichenliste, *teil_zeichenliste, *zeilenliste;


	zeichenliste = (struct list_head*) malloc(sizeof(struct list_head));
	INIT_LIST_HEAD(zeichenliste);

	zeilenliste = zeilen_trennen(bild);
	list_for_each(p, zeilenliste) {
		teil_zeichenliste = projektion_spalten_trennen(
					list_entry(p,
						   struct intern_bitmap,
						   list)
				    );
		//list_splice_tail(zeichenliste, teil_zeichenliste);
		list_splice_tail(teil_zeichenliste, zeichenliste);
		free(teil_zeichenliste);
	}
	free(zeilenliste);

	return zeichenliste;
}


struct intern_bitmap* zeichen_umfang_schneiden(const struct intern_bitmap* zeichen) {
	+++++++++
}

struct intern_bitmap* zeichen_standardisieren(const struct intern_bitmap* zeichen)
{
	struct intern_bitmap *zwieschen_bm, *gleichartig_bm;
	zwieschen_bm = zeichen_umfang_schneiden(zeichen);
	gleichartig_bm = bm_skalieren(bm, STANDARD_ZEICHEN_HEIGHT,
					  STANDARD_ZEICHEN_WIDTH);
	bm_release(zwieschen_bm);
	return gleichartig_bm;
}

int ocr_bestpassend(IplImage *src, char *ergebnis, int laenge)
{
	/* liefert länge der erkannte Zeichen zurück*/

	struct intern_bitmap *bm, *standard_bm;
	struct list_head *zeichenliste, *p;

	bm = preprocess(src);

	/*zeichenliste = projektion_spalten_trennen(bm);*/
	zeichenliste = einfach_trennen(bm);
	bm_release(bm);



	list_for_each(p, zeichenliste) {
		bm = list_entry(p, struct intern_bitmap, list);
		standard_bm = zeichen_standardisieren(bm);
		#ifdef DEBUG
		cvNamedWindow("Demo Window", CV_WINDOW_AUTOSIZE);
		cvShowImage("Demo Window",bm_bm2cvmat_cp(bm));
		cvWaitKey(-1);
		cvDestroyWindow("Demo Window");

		cvNamedWindow("Demo Window", CV_WINDOW_AUTOSIZE);
		cvShowImage("Demo Window",bm_bm2cvmat_cp(standard_bm));
		cvWaitKey(-1);
		cvDestroyWindow("Demo Window");
		#endif
		bm_release(standard_bm);
	}

	strcpy(ergebnis, "noch nicht implementiert");
	return 0;
}
