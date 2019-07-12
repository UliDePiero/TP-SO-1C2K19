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

//----------------- FUNCIONES AUXILIARES -----------------//

int bytesArchivoPath(char* path);
int cantidadBloques(char** bloquesArray);
char* encontrarRegistroParticion(char* pathParticion, uint16_t key);
int calcularBloques(long sizeTotal);

//----------------- FUNCIONES DE BITMAP -----------------//

void crearBitmap(char* pathBitmap);
int bloquesLibres();
void limpiarBitmap();
void setBloqueOcupado(int index);
void setBloqueLibre(int index);
void guardarBitmap();
int proximoBloqueLibre();
void mostrarBitmap();

void createFS(char* nombreTabla, char* consistencia, int particiones, long tiempoCompactacion);
char* selectFS(char* nombreTabla, int particiones, uint16_t key);
void dropFS(char* nombreTabla);

void dump();
void crearNuevosBloques(char* registrosComprimidos, char* nombre);

void compactar(char* nombreTabla);
void crearNuevosBloquesCompactacion(char* registrosComprimidos, char* particionPath);

#endif /* FILESYSTEM_H_ */
