#include <stdio.h>
#include <string.h>

#include <highgui.h>
#include <cv.h>
#include <cxcore.h>

#include "ocr.h"

/* für Debug: */
#include "../muster.c"

/******************************************************************************/
/* das Zeichen manipulieren: */
static struct intern_bitmap* zeichen_umfang_schneiden(const struct intern_bitmap* zeichen)
{
	struct intern_bitmap *neu_bm;
	int i, j;
	int oben, untern, links, rechts;
	int width, height;
	char continue_flag;

	/* für Compiler Warnen: */
	j = 0;
	/* -------------------- */

	if (zeichen == NULL)
		return NULL;

	/* oben */
	continue_flag = 1;
	for (i = 0; i < zeichen->height; i++) {
		for (j = 0; j < zeichen->width; j++) {
			if (bm_getpixel(zeichen, i, j)) {
				continue_flag = 0;
				break;
			}
		}
		if (!continue_flag) break;
	}
	/* einmalige Prüfung reicht */
	if (i == zeichen->height && j == zeichen->width)
		return NULL;
	oben = i;

	/* untern */
	continue_flag = 1;
	for (i = zeichen->height - 1 ; i >= 0; i--) {
		for (j = 0; j < zeichen->width; j++) {
			if (bm_getpixel(zeichen, i, j)) {
				continue_flag = 0;
				break;
			}
		}
		if (!continue_flag) break;
	}
	untern = i;

	/* links */
	continue_flag = 1;
	for (j = 0; j < zeichen->width; j++) {
		for (i = 0; i < zeichen->height; i++) {
			if (bm_getpixel(zeichen, i, j)) {
				continue_flag = 0;
				break;
			}

		}
		if (!continue_flag) break;
	}
	links = j;

	/* rechts */
	continue_flag = 1;
	for (j = zeichen->width - 1; j >= 0; j--) {
		for (i = 0; i < zeichen->height; i++) {
			if (bm_getpixel(zeichen, i, j)) {
				continue_flag = 0;
				break;
			}
		}
		if (!continue_flag) break;
	}
	rechts = j;

	/* neues Bitmap herstellen */
	height = untern - oben + 1;
	width = rechts - links + 1;
	BM_ALLOC(neu_bm, height, width);

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			bm_setpixel(neu_bm, i, j,
				    bm_getpixel(zeichen, oben + i, links + j)
				   );
		}
	}

	return neu_bm;
}

struct intern_bitmap* zeichen_standardisieren(
		const struct intern_bitmap* zeichen)
{
	struct intern_bitmap *zwieschen_bm, *gleichartig_bm;
	zwieschen_bm = zeichen_umfang_schneiden(zeichen);
	gleichartig_bm = bm_skalieren(zwieschen_bm, STANDARD_ZEICHEN_HEIGHT,
						  STANDARD_ZEICHEN_WIDTH);
	bm_release(zwieschen_bm);

	/*
	#ifdef DEBUG
	for(int i = 0; i < gleichartig_bm->height; i++) {
		for (int j = 0; j < gleichartig_bm->width; j++) {
			if (bm_getpixel(gleichartig_bm, i, j))
				printf("X");
			else
				printf(" ");
		}
		printf("\n");
	}
	printf("\n\n");
	#endif
	*/

	return gleichartig_bm;
}


/******************************************************************************/
static int vektor_generieren_oben(vektor_t *vektor,
				const struct intern_bitmap *zeichen)
{
	/* Abtastensrichtung nach untern */
	int i, j;
	int verschiebung;

	if (vektor == NULL) return 0;

	verschiebung = 0;

	for (j = 0; j < zeichen->width; j++ ) {
		for (i = 0; i < zeichen->height; i++) {
			if (bm_getpixel(zeichen,i ,j))
				break;
		}
		vektor[verschiebung++] = i;
	}
	return verschiebung;
}

static int vektor_generieren_untern(vektor_t *vektor,
				const struct intern_bitmap *zeichen)
{
	/* Abtastensrichtung nach oben */
	int i, j;
	int verschiebung;

	if (vektor == NULL) return 0;

	verschiebung = 0;

	for (j = 0; j < zeichen->width; j++ ) {
		for (i = zeichen->height - 1; i >= 0; i--) {
			if (bm_getpixel(zeichen,i ,j))
				break;
		}
		vektor[verschiebung++] = zeichen->height - i - 1;
	}
	return verschiebung;
}

static int vektor_generieren_links(vektor_t *vektor,
				const struct intern_bitmap *zeichen)
{
	/* Abtastensrichtung nach rechts */
	int i, j;
	int verschiebung;

	if (vektor == NULL) return 0;

	verschiebung = 0;

	for (i = 0; i < zeichen->height; i++) {
		for (j = 0; j < zeichen->width; j++ ) {
			if (bm_getpixel(zeichen,i ,j))
				break;
		}
		vektor[verschiebung++] = j;
	}
	return verschiebung;
}

static int vektor_generieren_rechts(vektor_t *vektor,
				const struct intern_bitmap *zeichen)
{
	/* Abtastensrichtung nach links */
	int i, j;
	int verschiebung;

	if (vektor == NULL) return 0;

	verschiebung = 0;

	for (i = 0; i < zeichen->height; i++) {
		for (j = zeichen->width - 1; j >= 0; j-- ) {
			if (bm_getpixel(zeichen,i ,j))
				break;
		}
		vektor[verschiebung++] = zeichen->width - j - 1;
	}
	return verschiebung;
}


static int vektor_generieren_linksoben(vektor_t *vektor,
				const struct intern_bitmap *zeichen)
{
	/* Abtastensrichtung 45 Grad nach rechts untern */
	int x, y;
	int i;
	int verschiebung;

	if (vektor == NULL) return 0;

	verschiebung = 0;

	/* zuerst senkrecht, von untern nach oben */

	for (i = zeichen->height - 1; i >= 0; i--) {
		x = 0;
		y = i;
		while ((x < zeichen->width) && (y < zeichen->height)) {
			if (bm_getpixel(zeichen, y, x))
				break;

			x++;
			y++;
		}
		/* x ist die aktuellen Schritte, die der Schleifer läuft*/
		vektor[verschiebung++] = x;
	}

	/* waagerecht von links nach rechts */

	/*for (i = 0; i < zeichen->width; i++) {*/
	for (i = 1; i < zeichen->width; i++) {
		x = i;
		y = 0;
		while ((x < zeichen->width) && (y < zeichen->height)) {
			if (bm_getpixel(zeichen, y, x))
				break;

			x++;
			y++;
		}

		/* y ist die aktuellen Schritte, die der Schleifer läuft*/
		vektor[verschiebung++] = y;
	}

	return verschiebung;
}

static int vektor_generieren_rechtsoben(vektor_t *vektor,
				const struct intern_bitmap *zeichen)
{
	/* Abtastensrichtung 45 Grad nach links untern */
	int x, y;
	int i;
	int verschiebung;

	if (vektor == NULL) return 0;

	verschiebung = 0;

	/* zuerst waagerecht, von links nach rechts */

	for (i = 0; i < zeichen->width; i++) {
		x = i;
		y = 0;
		while ((x >= 0) && (y < zeichen->height)) {
			if (bm_getpixel(zeichen, y, x))
				break;

			x--;
			y++;
		}

		/* y ist die aktuellen Schritte, die der Schleifer läuft*/
		vektor[verschiebung++] = y;
	}

	/* senkrecht, von oben nach untern */

	/*for (i = 0; i < zeichen->height; i++) {*/
	for (i = 1; i < zeichen->height; i++) {
		x = zeichen->width - 1;
		y = i;
		while ((x >= 0) && (y < zeichen->height)) {
			if (bm_getpixel(zeichen, y, x))
				break;

			x--;
			y++;
		}
		/* die aktuellen Schritte, die der Schleifer läuft*/
		vektor[verschiebung++] = y - i;
	}


	return verschiebung;
}


static int vektor_generieren_rechtsuntern(vektor_t *vektor,
				const struct intern_bitmap *zeichen)
{
	/* Abtastensrichtung 45 Grad nach links oben */
	int x, y;
	int i;
	int verschiebung;

	if (vektor == NULL) return 0;

	verschiebung = 0;

	/* zuerst senkrecht, von oben nach untern */

	for (i = 0; i < zeichen->height; i++) {
		x = zeichen->width - 1;
		y = i;
		while ((x >= 0) && (y >= 0)) {
			if (bm_getpixel(zeichen, y, x))
				break;

			x--;
			y--;
		}
		/* die aktuellen Schritte, die der Schleifer läuft*/
		vektor[verschiebung++] = i - y;
	}

	/* waagerecht von rechts nach links */

	/*for (i = zeichen->width - 1; i >= 0; i--) {*/
	for (i = zeichen->width - 2; i >= 0; i--) {
		x = i;
		y = zeichen->height - 1;
		while ((x >= 0) && (y >= 0)) {
			if (bm_getpixel(zeichen, y, x))
				break;

			x--;
			y--;
		}

		/* die aktuellen Schritte, die der Schleifer läuft*/
		/*printf("i = %d, x = %d\n", i, x);*/
		vektor[verschiebung++] = i - x;
	}

	return verschiebung;
}


static int vektor_generieren_linksuntern(vektor_t *vektor,
				const struct intern_bitmap *zeichen)
{
	/* Abtastensrichtung 45 Grad nach rechts oben */
	int x, y;
	int i;
	int verschiebung;

	if (vektor == NULL) return 0;

	verschiebung = 0;

	/* zuerst waagerecht von rechts nach links */

	for (i = zeichen->width - 1; i >=0;  i--) {
		x = i;
		y = zeichen->height - 1;
		while ((x < zeichen->width) && (y >= 0)) {
			if (bm_getpixel(zeichen, y, x))
				break;

			x++;
			y--;
		}

		/* die aktuellen Schritte, die der Schleifer läuft*/
		vektor[verschiebung++] = x - i;
	}

	/* senkrecht, von untern nach oben */

	/* for (i = zeichen->height - 1; i >= 0; i--) { */
	for (i = zeichen->height - 2; i >= 0; i--) {
		x = 0;
		y = i;
		while ((x < zeichen->width) && (y >= 0)) {
			if (bm_getpixel(zeichen, y, x))
				break;

			x++;
			y--;
		}
		/* x ist die aktuellen Schritte, die der Schleifer läuft*/
		vektor[verschiebung++] = x;
	}


	return verschiebung;
}

int vektor_generieren(vektor_t *vektor, const struct intern_bitmap *zeichen)
{
	/* vektor in Uhrzeigerrichtung von dem Zeichen generieren */
	int laenge, verschiebung;

	if (vektor == NULL) return 0;

	verschiebung = 0;

	laenge = vektor_generieren_oben(&vektor[verschiebung], zeichen);
	verschiebung += laenge;

	laenge = vektor_generieren_rechts(&vektor[verschiebung], zeichen);
	verschiebung += laenge;

	laenge = vektor_generieren_untern(&vektor[verschiebung], zeichen);
	verschiebung += laenge;

	laenge = vektor_generieren_links(&vektor[verschiebung], zeichen);
	verschiebung += laenge;

	////////

	laenge = vektor_generieren_linksoben(&vektor[verschiebung], zeichen);
	verschiebung += laenge;

	laenge = vektor_generieren_rechtsoben(&vektor[verschiebung], zeichen);
	verschiebung += laenge;

	laenge = vektor_generieren_rechtsuntern(&vektor[verschiebung], zeichen);
	verschiebung += laenge;

	laenge = vektor_generieren_linksuntern(&vektor[verschiebung], zeichen);
	verschiebung += laenge;

	/*
	#ifdef DEBUG
	printf("Debug: vektor_generieren():\n");
	for (int i = 0; i < verschiebung; i++) {
		printf("%d ",vektor[i]);
	}
	printf("\n\n");
	#endif
	*/

	return verschiebung;
}

long vektor_vergleichen(vektor_t *vektor, vektor_t *vektor_muster, int laenge)
{
	int i;
	long ergebnis;

	/* für die Sicherheit:*/
	if (laenge > ZEICHEN_VEKTOR_LAENGE) return -1;

	ergebnis = 0;

	for (i = 0; i < laenge; i++) {
		//printf("| %d - %d | = %d\n", vektor[i], vektor_muster[i], ocr_abs(vektor[i] - vektor_muster[i]));
		ergebnis += lquadrat(ocr_abs(vektor[i] - vektor_muster[i]));
	}

	return ergebnis;
}

int ocr_bestpassend(struct intern_bitmap *bm, char *ergebnis, int laenge)
{
	/* liefert länge der erkannte Zeichen zurück*/
	vektor_t vektor[ZEICHEN_VEKTOR_LAENGE];
	int kleinst, zurzeit, kleinst_index;
	int vektor_laenge;
	struct intern_bitmap *standard_bm;
	struct list_head *zeichenliste, *p;


	if (laenge <= 1) return -1;

	ergebnis[0] = '\0';

	/*zeichenliste = projektion_spalten_trennen(bm);*/
	zeichenliste = einfach_trennen(bm);
	/*bm_release(bm);*/ /* das wird außer dieser Funktion getan */

	list_for_each(p, zeichenliste) {
		bm = list_entry(p, struct intern_bitmap, list);
		standard_bm = zeichen_standardisieren(bm);
		vektor_laenge = vektor_generieren(vektor, standard_bm);

		kleinst = vektor_vergleichen(vektor, daten_muster[0].vektor, vektor_laenge);
		kleinst_index = 0;

		for (int i = 0; i < ZEICHEN_MUSTER_MENGE; i++) {
			//printf("%05ld %s\n",vektor_vergleichen(vektor, daten_muster[i].vektor, vektor_laenge), daten_muster[i].zeichen_puffer);
			zurzeit = vektor_vergleichen(vektor, daten_muster[i].vektor, vektor_laenge);
			//printf("zurzeit: %d, kleinst %d\n", zurzeit, kleinst);
			if (zurzeit < kleinst) {
				kleinst = zurzeit;
				kleinst_index = i;
			}
		}

		printf("%s kennbarkeit = -%d\n", daten_muster[kleinst_index].zeichen_puffer, kleinst);
		strcat(ergebnis, daten_muster[kleinst_index].zeichen_puffer);
		//+++++++++
		#ifdef DEBUG
		/*
		printf("%s\n", daten_muster[kleinst_index].zeichen_puffer);
		cvNamedWindow("Demo Window", CV_WINDOW_AUTOSIZE);
		cvShowImage("Demo Window",bm_bm2cvmat(bm));
		cvWaitKey(-1);
		cvDestroyWindow("Demo Window");
		*/

		cvNamedWindow("Demo Window", CV_WINDOW_AUTOSIZE);
		cvShowImage("Demo Window",bm_bm2cvmat(standard_bm));
		cvWaitKey(-1);
		cvDestroyWindow("Demo Window");
		#endif
		bm_release(standard_bm);
	}

	//strcpy(ergebnis, "noch nicht implementiert");
	return 0;
}
