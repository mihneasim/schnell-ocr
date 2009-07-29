#include <stdio.h>
#include <string.h>

#include <highgui.h>
#include <cv.h>
#include <cxcore.h>

#include "ocr.h"

/******************************************************************************/
int lernen_zeichen(int *vektor, struct intern_bitmap *zeichen)
{
	/* annehmen ,
	 * dass der Vektor eine Puffer hat ,
	 * der größer als ZEICHEN_VEKTOR_LAENGE ist */
	int vektor_laenge;

	if (vektor == NULL) return 0;
	
	vektor_laenge = vektor_generieren(vektor, zeichen);
	
	#ifdef DEBUG
	{
		printf("Vektor:");
		for (int i = 0; i < ZEICHEN_VEKTOR_LAENGE; i++) {
			printf(" %d", vektor[i]);
		}
		printf("\n");

		cvNamedWindow("Demo Window", CV_WINDOW_AUTOSIZE);
		cvShowImage("Demo Window",bm_bm2cvmat(zeichen));
		cvWaitKey(-1);
		cvDestroyWindow("Demo Window");
	}
	#endif
	

	return vektor_laenge;
}

void lernen_text_pro_zeichen(struct intern_bitmap *text_bm)
{
	struct list_head *zeichenliste, *p;
	struct intern_bitmap *bm, *standard_bm;

	zeichenliste = einfach_trennen(text_bm);

	list_for_each(p, zeichenliste) {
		bm = list_entry(p, struct intern_bitmap, list);
		standard_bm = zeichen_standardisieren(bm);
		bm_release(standard_bm);
	}
}

int vektor[ZEICHEN_VEKTOR_LAENGE];
int main(int argc, char *argv[], char *env[])
{
	struct intern_bitmap *bm;
	IplImage *src;

	/* Argumente testen */
	if (argc == 1) {
		printf("usage: \n\t%s <ein Bild>\n",argv[0]);
		return 1;
	}

	/* init OpenCV */
	cvInitSystem(argc, argv);
	
	/* Bild Lesen */
	src = cvLoadImage(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
	if (!src) {
		printf("Error: cvLoadImage()\n");
		return 1;
	}
	
	bm = preprocess(src);

	bm_release(bm);
	
	/* das original Bild anzeigen */
	cvNamedWindow("Demo Window", CV_WINDOW_AUTOSIZE);
	cvShowImage("Demo Window", src);
	cvWaitKey(-1);
	cvDestroyWindow("Demo Window");

	return 0;
}
