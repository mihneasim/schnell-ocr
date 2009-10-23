/*!
 * \file	kennen_ohnefloat.h
 * \brief	eine Kennungsmethode, ohne Manipulation der Floatzahle
 */
#ifndef __KENNEN_OHNEFLOAT__H__
#define __KENNEN_OHNEFLOAT__H__

#include "ocr.h"



/*! Die Länge eines getrennten Zeichenbitmaps */
#define STANDARD_ZEICHEN_WIDTH	30 
/*! Die Höhe eines getrennten Zeichenbitmaps */
#define STANDARD_ZEICHEN_HEIGHT	30
/*!
 * \brief Die Speichergröße des Vektors, mit dem ein Zeichen beschrieben wird.
 *
 *
 */

/* oben, untern, links, rechts : vier seiten Richtungen	(1)
 * +
 * die Richtungen aus vier Ecken			(2)
 *
 *
 * (1) und (2) muss verdoppelt werden. um das Innere des Zeichens abzutasten
 *
 */
#define ZEICHEN_VEKTOR_LAENGE	(2 * (\
					( 2 * \
						(STANDARD_ZEICHEN_WIDTH +\
						 STANDARD_ZEICHEN_HEIGHT)\
					)\
					+\
					( 4 * \
						(STANDARD_ZEICHEN_WIDTH +\
						 STANDARD_ZEICHEN_HEIGHT\
						 - 1)\
					)\
				     )\
				)



/*! der Typ des Vektorelementes */
typedef int vektor_t;

/*! \warning Diese Struktur kann total groß sein,  vorsicht auf dem Stack! */
struct zeichenvektor {
	/*! ein NULL endet Zeichenkette*/
	char zeichen_puffer[ENCODE_LAENGE];
	/*! der Vektor, der einen Zeichen(u. -kombination) beschreibt
	 * siehe die ausführlichen Kommentare in Funktion vektor_generieren()
	 */
	vektor_t vektor[ZEICHEN_VEKTOR_LAENGE];
};



/****************************************************************/

struct intern_bitmap* zeichen_standardisieren(
		const struct intern_bitmap* zeichen);

int ocr_bestpassend(struct intern_bitmap *bm, char *ergebnis, int laenge);


CvMat *bm_bm2cvmat(const struct intern_bitmap *bm);

int vektor_generieren(vektor_t *vektor, const struct intern_bitmap *zeichen);

long vektor_vergleichen(vektor_t *vektor, vektor_t *vektor_muster, int laenge);

#endif /*__KENNEN_OHNEFLOAT__H__*/
