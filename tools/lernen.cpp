#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <highgui.h>
#include <cv.h>
#include <cxcore.h>

#include "ocr.h"



int vektor[ZEICHEN_VEKTOR_LAENGE];


int lernen_zeichen(int *vektor, struct intern_bitmap *zeichen)
{
	/* annehmen ,
	 * dass der Vektor eine Puffer hat ,
	 * der größer als ZEICHEN_VEKTOR_LAENGE ist */
	int vektor_laenge;

	if (vektor == NULL) return 0;
	
	vektor_laenge = vektor_generieren(vektor, zeichen);
	/*
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
	*/

	return vektor_laenge;
}

void lernen_text_pro_zeichen(struct intern_bitmap *text_bm)
{
	FILE *f_vektor;
	struct list_head *zeichenliste, *p;
	struct intern_bitmap *bm, *standard_bm;
	int i;
	char str[10];

	zeichenliste = einfach_trennen(text_bm);
	
	f_vektor = fopen("vektor.txt","wa");
	if (!f_vektor) {
		ocr_error("Datei konnte nicht geöffnet werden");
	}
	/*f_vektor = stdout;*/

	list_for_each(p, zeichenliste) {
		bm = list_entry(p, struct intern_bitmap, list);
		standard_bm = zeichen_standardisieren(bm);
		vektor_generieren(vektor, standard_bm);

		cvNamedWindow("Zeigensfenster", CV_WINDOW_AUTOSIZE);
		cvShowImage("Zeigensfenster",bm_bm2cvmat(standard_bm));
		cvWaitKey(-1);
		cvDestroyWindow("Zeigensfenster");
		printf("Das ist ");
		fgets(str,9,stdin);

		fprintf(f_vektor, "%s", str);
		for (i = 0; i < ZEICHEN_VEKTOR_LAENGE; i++) {
			fprintf(f_vektor, "%d ", vektor[i]);
		}
		fprintf(f_vektor, "\n");
		bm_release(standard_bm);
	}


	fclose(f_vektor);
}


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
	lernen_text_pro_zeichen(bm);
	bm_release(bm);
	
	/* das original Bild anzeigen */
	/*
	cvNamedWindow("Demo Window", CV_WINDOW_AUTOSIZE);
	cvShowImage("Demo Window", src);
	cvWaitKey(-1);
	cvDestroyWindow("Demo Window");
	*/

	return 0;
}
