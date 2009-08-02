#include <stdio.h>
#include <string.h>

#include <highgui.h>
#include <cv.h>
#include <cxcore.h>

#include "ocr.h"

/* für Debug: */
#include "../muster.c"


/* Diese Datei enthält den Teil, den direkt mit Benutzer kommunizieren.
 * Sie bietet die Haupt Routine (Schnittstelle) an.
 */

//#define ocr_abs abs
inline int ocr_abs(int a)
{
	return a > 0 ? a : -a;
}

inline long lquadrat(long a)
{
	return a*a;
}

void ocr_error(const char *msg)
{
	printf("%s\n", msg);
	exit(127);
}

/******************************************************************************/
/*
 * hier fängt die Operationen an,
 * die die Datenstruktur intern_bitmap bearbeiten
 *
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

int bm_setpixel(const struct intern_bitmap *bm, int row, int col,
						unsigned char pixelvalue)
{
	if ((col >= bm->width) || (row >= bm->height)) {
		ocr_error("bm_getpixel: unrichtige Anforderung");
		return 256;
	}
	bm->buffer[row * bm->width + col] = pixelvalue;
	return pixelvalue;
}

static struct intern_bitmap* bm_skalieren(const struct intern_bitmap *org_bm, int new_height, int new_width)
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

/*static*/ CvMat *bm_bm2cvmat_kontrast(const struct intern_bitmap *bm)
{
	/* falls eine Zelle nicht 0, dann ist sie 255 , für Debug*/
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
