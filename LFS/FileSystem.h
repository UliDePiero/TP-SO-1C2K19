/*
 * FileSystem.h
 *
 *  Created on: 15 may. 2019
 *      Author: utnso
 */

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_
//#define PATH_METADATA "[Punto_Montaje]/Metadata/Metadata.bin"
//#define PATH_BITMAP "[Punto_Montaje]/Metadata/Bitmap.bin" 			// bitarray de las commons library.
//#define PATH_TABLAS "[Punto_Montaje]/Tables/[Nombre_Tabla]"
//#define PATH_DATOS "[Punto_Montaje]/Bloques/[nroBloque].bin"


typedef struct {
   int tamanioBloque;
   int cantidadBloques;
   char magicNumber[] = "LISSANDRA";
} Metadata;

typedef struct {
   int tamanio;
   char *bloques; 	/*Yo propongo guardarlo como un string “[bloque1,bloque2,...]” para no
   	   	   	   	   	  limitar la cantidad de bloques que se puedan poner, total hay una función
					  que transforma un string de este tipo en un array*/
   //Verificar
} ArchivoNoMetadata;

Metadata* metadata;
ArchivoNoMetadata* archivoNoMetadata;

#endif /* FILESYSTEM_H_ */
