#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <highgui.h>
#include <cv.h>
#include <cxcore.h>


#include "ocr.h"


#define ERGEBNIS_LAENGE 100
char ergebnis[ERGEBNIS_LAENGE];

char dateiname[1000];

int main(int argc, char *argv[], char *env[])
{
	IplImage *src;
	FILE *f_txt;
	struct intern_bitmap *bm;
	int i;

	/* Argumente testen */
	if (argc == 1) {
		printf("usage: \n\t%s <ein Bild>\n",argv[0]);
		return 1;
	}

	/* init OpenCV */
	cvInitSystem(argc, argv);

	for (i = 1; i < argc; i++ ) {
		/* Bild Lesen */
		strcpy(dateiname, "/media/d/PROJECTS/vc2008/ocr/ref/Viertelbilder/ViertelBilder/");
		strcat(dateiname, argv[i]);
		strcat(dateiname, ".jpg");
		src = cvLoadImage(dateiname, CV_LOAD_IMAGE_GRAYSCALE);
		if (!src) {
			printf("Error: cvLoadImage() :\n\t%s\n", dateiname);
			return 1;
		}

		/* das original Bild anzeigen */
		cvNamedWindow("Demo Window", CV_WINDOW_AUTOSIZE);
		cvShowImage("Demo Window", src);
		cvWaitKey(-1);
		cvDestroyWindow("Demo Window");


		strcpy(dateiname, "/media/d/PROJECTS/vc2008/ocr/ref/Viertelbilder/ViertelBilder/");
		strcat(dateiname, argv[i]);
		strcat(dateiname, ".txt");

		f_txt = fopen(dateiname, "r");

		if (!f_txt) {
			printf("Datei konnte nicht geöffnet werden:\n\t%s", dateiname);
			return 1;
		}

		fclose(f_txt);

		/* FIXME ++++++*/

		cvReleaseData(src);
		/*
		bm = preprocess(src);
		ocr_bestpassend(bm, ergebnis, ERGEBNIS_LAENGE);
		bm_release(bm);
		printf("Ergebnis: %s\n", ergebnis);
		*/

	}
	return 0;
}
