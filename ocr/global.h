#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#define DEBUG 1

#define AUFLOESUNG_KONSTANT 2

#ifdef DEBUG
#define OOPS() printf("OOPS\n");
#endif /*OOPS*/


#define STANDARD_ZEICHEN_WIDTH	30
#define STANDARD_ZEICHEN_HEIGHT	30
/* Die Länge des Vektors, mit dem ein Zeichen beschrieben wird. */
#define ZEICHEN_VEKTOR_LAENGE	( 2 * \
					(STANDARD_ZEICHEN_WIDTH +\
					STANDARD_ZEICHEN_HEIGHT)\
				)

#endif /*__GLOBAL_H__*/
