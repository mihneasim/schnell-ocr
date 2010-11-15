/*!
 * \file global.h
 * \brief Die globalen Einstellungen werden in dieser Datei beschrieben
 */
#ifndef __GLOBAL_H__
#define __GLOBAL_H__

/*! Debug flag */
//#define DEBUG 1

/*! Die Auflösung einer Störung,
 *  etwas kleiner als der Wert wird als Störung behandelt. 
 * \warning noch nicht implementiert
 */
#define AUFLOESUNG_KONSTANT 2

#ifdef DEBUG
/*! für Debugzweck*/
#define OOPS() printf("OOPS\n");
#endif /*OOPS*/


/*! Maximale Speichergröße eines Zeichens plus das NULL Ende,
 * je nach der Zeichenkodierung ändert die Länge sich.
 * Diese Konstante bestimmt die Speichergröße eines Zeichens oder 
 * einer Zeichenkombination (z.B.: ung, sch ...)
 * Um die Kodierungskompatibilität zu erreichen, 
 * wird jeder Zeichen / Zeichenkombination in einen Puffer gespeichert.
 */
//#define ENCODE_LAENGE (6+2)
#define ENCODE_LAENGE 20

#endif /*__GLOBAL_H__*/
