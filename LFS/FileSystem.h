/*
 * FileSystem.h
 *
 *  Created on: 15 may. 2019
 *      Author: utnso
 */

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include "LFS.h"
#include <parser.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "LFS.h"

void levantarFileSystem();
void destruirFileSystem();

void obtenerMetadata(char* pathMetadata);
void obtenerBitmap(char* pathMetadata);
void obtenerTablas(char* puntoMontaje);

int bytesArchivoPath(char* path);
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

void createFS(char* nombreTabla, char* consistencia, int particiones, long tiempoCompactacion);
char* selectFS(char* tabla, int particiones, uint16_t key);

//Bajar la memtable al FS
void dump();
void crearNuevosBloques(char* registrosComprimidos, char* nombre);

#endif /* FILESYSTEM_H_ */
