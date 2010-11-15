#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <highgui.h>
#include <cv.h>
#include <cxcore.h>


#include "ocr.h"
#include "kennen_ohnefloat.h"


#define ERGEBNIS_LAENGE 1000
char ergebnis[ERGEBNIS_LAENGE];

int main(int argc, char *argv[], char *env[])
{
	IplImage *src;
	struct intern_bitmap *bm;
	int i;

	/* Argumente testen */
	if (argc == 1) {
		printf("usage: \n\t%s <ein Bild>\n",argv[0]);
		return 1;
	}

	/* init OpenCV */
  #ifdef DEBUG
	cvInitSystem(argc, argv);
  #endif

	for (i = 1; i < argc; i++ ) {
		/* Bild Lesen */
		src = cvLoadImage(argv[i], CV_LOAD_IMAGE_GRAYSCALE);
		if (!src) {
			printf("Error: cvLoadImage()\n");
			return 1;
		}

		/* das original Bild anzeigen */
    #ifdef DEBUG
		cvNamedWindow("Demo Window", CV_WINDOW_AUTOSIZE);
		cvShowImage("Demo Window", src);
		cvWaitKey(-1);
		cvDestroyWindow("Demo Window");
    #endif

		bm = preprocess(src);
		ocr_bestpassend(bm, ergebnis, ERGEBNIS_LAENGE);
		bm_release(bm);

		cvReleaseData(src);

		printf("Ergebnis: %s\n", ergebnis);

	}
	return 0;
}
