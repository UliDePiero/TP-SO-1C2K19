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
#include <commons/bitarray.h>
#include <arpa/inet.h>
#include <sockets.h>
#include <configuraciones.h>
#include <commons/string.h>
#include <parser.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>


typedef struct {
   int tamanioBloque;
   int cantidadBloques;
   char *magicNumber;
} Metadata;


Metadata* metadata;
t_bitarray *bitmap;

void levantarFileSystem();
void destruirFileSystem();

void obtenerMetadata(char* pathMetadata);
void obtenerBitmap(char* pathMetadata);
void obtenerTablas(char* puntoMontaje);

int bytesArchivoPath(char* path);
size_t bytesArchivo(int Archivo);

//Funciones de bitmap
void crearBitmap(char* pathBitmap);
int bloquesLibres();
void limpiarBitmap();
void setBloqueOcupado(int index);
void setBloqueLibre(int index);
void guardarBitmap();
int proximoBloqueLibre();
void mostrarBitmap();

void createFS(char* nombreTabla, int consistencia, int particiones, long tiempoCompactacion);

#endif /* FILESYSTEM_H_ */
