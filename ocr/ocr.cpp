/*!
 * \file ocr.cpp 
 * \brief Haupte Benutzerschnittstellesimplementierung
 *
 * Diese Datei enthält den Teil, den direkt mit Benutzer kommunizieren.
 * Sie bietet die Haupt Routine (Schnittstelle) an.
 */
#include <stdio.h>
#include <string.h>

#include <highgui.h>
#include <cv.h>
#include <cxcore.h>

#include "ocr.h"




//#define ocr_abs abs
/*! eine Implementierung vom Absolutenwertrechnen */
/*inline*/ int ocr_abs(int a)
{
	return a > 0 ? a : -a;
}

/*! quadrat eines long-typ Wertes. Die Funktion soll die Gewichtfunktion der Vektor sein, 
 * in dieser Verison ist sie Standardabweichung.*/
/*inline*/ long lquadrat(long a)
{
	return a*a;
}

/*!
 * \brief allgemeine Fehlerbehandelung.
 *
 * In dieser Version wird eine Fehlerbeschreibung in Standarder 
 * Feherausgang ausgeliefert.
 * \param msg die Fehlersbeschreibung
 */
void ocr_error(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(127);
}

/******************************************************************************/
/*
 * hier fängt die Operationen an,
 * die die Datenstruktur intern_bitmap bearbeiten
 *
 */

/*! entnehmen ein Punkt aus einer intern_bitmap Struktur
 *  \param bm	Zeiger zu einer intern_bitmap Struktur
 *  \param row	Zeile des Punktes, fängt mit 0 an
 *  \param col	Spaltzahl des Punktes, fängt mit 0 an
 *  \return	den Wert des Punktes
 */
int bm_getpixel(const struct intern_bitmap *bm, int row, int col)
{
	/* es wird angenommen, dass der Puffer von intern_bitmap
	 * der unsigned char typ ist */

	/* row und col fängt mit 0 an */
	if ((col >= bm->width) || (row >= bm->height)) {
		ocr_error("bm_getpixel: unrichtige Anforderung");
		return  256;
	}
	return bm->buffer[row * bm->width + col];
}

/*! ein Punkt in einer intern_bitmap Struktur einschreiben
 *  \param bm		Zeiger zu einer intern_bitmap Struktur
 *  \param row		Zeile des Punktes, fängt mit 0 an
 *  \param col		Spaltzahl des Punktes, fängt mit 0 an
 *  \param pixelvalue	der neue Wert des genannten Punktes
 *  \return 		den alten Wert des genannten Punktes
 */
int bm_setpixel(const struct intern_bitmap *bm, int row, int col,
						unsigned char pixelvalue)
{
	int altes_pixel;
	if ((col >= bm->width) || (row >= bm->height)) {
		ocr_error("bm_getpixel: unrichtige Anforderung");
		return 256;
	}
	altes_pixel = bm->buffer[row * bm->width + col];
	bm->buffer[row * bm->width + col] = pixelvalue;
	return altes_pixel;
}

/*!\brief verändern die Größe eines beliebigen viereckigen Bitmaps
 *
 * \param org_bm	das Original Bitmap (wird nicht geändert)
 * \param new_height	die neue Höhe des skalierte Bitmap
 * \param new_width	das neue Breite des skalierte Bitmap
 * \return		liefert einen neu allozierten Speicherbereich zurück,
 * 			die das skalierte Bitmap enthältet
 * \warning		neue Speicherplatz wird von Heap durch malloc() 
 * 			Funktion reserviert. Falls man sie nicht freisetzen,
 * 			wird ein 'Speicherloch' erzeugen.
 */
struct intern_bitmap* bm_skalieren(const struct intern_bitmap *org_bm,
					int new_height, int new_width)
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

	/*
#ifdef DEBUG
	printf("\n");
	for (i = 0; i < new_height; i++) {
		for (j = 0; j < new_width; j++) {
			printf("%c",bm_getpixel(neu_bm, i, j) == 0 ? ' ' : 'X');
		}
		printf("\n");
	}
#endif
	*/

	return neu_bm;
}

/*!
 * \brief freisetzen eines allozierten Bitmaps
 * \param bm	die intern_bitmap Struktur freizusetzen
 * \return	in dieser Version immer 0
 */
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

/*! \brief konvertieren eine CvMat Struktur ins intern_bitmap
 *
 * Eine Schnittstelle zum OpenCV, das Matrix in OpenCV ins intern_bitmap
 * umsetzen
 * \param mat	eine gültige CvMat Variable
 * \return	eine neue allozierte intern_bitmap Abbildung
 * \warning	neue Speicherplatz wird von Heap durch malloc()
 * 		Funktion reserviert. Falls man sie nicht freisetzen,
 *		wird ein 'Speicherloch' erzeugen.
 */
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

/*! \brief konvertieren ein intern_bitmap ins CvMat
 *
 * \param bm	ein gültiges intern_bitmap
 * \return	die vom Parameter neue erzeugte CvMat Struktur
 * \warning	Bitte löschen den rückgelieferte CvMat nachher!
 */
CvMat *bm_bm2cvmat(const struct intern_bitmap *bm)
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

/*! Ist eine Zelle nicht 0, dann ist sie 255 , für Debug*/
/*static*/ CvMat *bm_bm2cvmat_kontrast(const struct intern_bitmap *bm)
{
	/* Ist eine Zelle nicht 0, dann ist sie 255 , für Debug*/
	CvMat *mat;
	int i;
	unsigned char *p;

	mat = bm_bm2cvmat(bm);

	for (i = 0, p = mat->data.ptr;
	     i < mat->height * mat->width * (int)sizeof(unsigned char);
	     i++,p++) {
		if (*p) *p = 255;
	}


	return mat;
}



/******************************************************************************/
/*
 * hier fängt  ein einfachstes Trennungsschema an
 *
 */
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
			int grenze; /* relative verschiebung zu i in der Zeile */
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
			//printf("grenze: %d\n",grenze);

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


struct list_head*
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

/******************************************************************************/

static int rauschen_entfernen(const struct intern_bitmap *bm)
{
	int rauschen_ventil;
	rauschen_ventil = 1;
	/* bla bla.. +++++++ noch nicht fertig */
	return rauschen_ventil;
}

struct intern_bitmap *preprocess(IplImage *src)
{
	/* Die Umwandlung zwischen OpenCV und intern_bitmap */
	CvMat *mat;
	struct intern_bitmap *bm;


	mat = cvCreateMat(src->height, src->width, CV_8UC1);

	/* binärisieren : */
	cvAdaptiveThreshold(src, mat, 255, CV_ADAPTIVE_THRESH_MEAN_C,
						CV_THRESH_BINARY_INV, 35, 37);
	/* nun ist 'mat' eine binäre Matrix, die 0xFF und 0 enthält */

	/*
	#ifdef DEBUG
	cvNamedWindow("Demo Window", CV_WINDOW_AUTOSIZE);
	cvShowImage("Demo Window", mat);
	cvWaitKey(-1);
	cvDestroyWindow("Demo Window");
	#endif
	*/

	bm = bm_cvmat2bm(mat);
	cvReleaseMat(&mat);

	rauschen_entfernen(bm);

	return bm;
}

