/*
 * FileSystem.h
 *
 *  Created on: 15 may. 2019
 *      Author: utnso
 */

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include "LFS.h"
#include <commons/bitarray.h>
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

char *bitarray;
Metadata *metadata;
t_bitarray *bitmap;

void levantarFileSystem();
void destruirFileSystem();

void obtenerMetadata(char* pathMetadata);
void obtenerBitmap(char* pathMetadata);
void obtenerTablas(char* puntoMontaje);

int bytesArchivoPath(char* path);
size_t bytesArchivo(FILE* Archivo);
int cantidadBloques(char** bloquesArray);
char* encontrarRegistroParticion(char* pathParticion, uint16_t key);
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
void insertFS(char* nombreTabla, uint16_t key, char* value, int timestamp);
char* selectFS(char* tabla, int particiones, uint16_t key);

#endif /* FILESYSTEM_H_ */
