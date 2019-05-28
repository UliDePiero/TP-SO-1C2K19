/*
 * FileSystem.h
 *
 *  Created on: 15 may. 2019
 *      Author: utnso
 */

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include "LFS.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <openssl/md5.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/config.h>
#include <commons/log.h>
#include <arpa/inet.h>
#include <sockets.h>
#include <configuraciones.h>
#include <commons/string.h>
#include <parser.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>


typedef struct {
   int tamanioBloque;
   int cantidadBloques;
   char *magicNumber;
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

void levantarFileSystem();
void destruirFileSystem();

void createFS(char* nombreTabla, int consistencia, int particiones, long tiempoCompactacion);

#endif /* FILESYSTEM_H_ */
