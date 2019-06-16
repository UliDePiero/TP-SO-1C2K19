/*
 * LFS.h
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */

#ifndef LFS_H_
#define LFS_H_

//Ruta del archivo de configuraciones
//#define RUTA_CONFIG "./LFS.config" // Para correr desde Eclipse
#define RUTA_CONFIG "../LFS.config" // Para correr desde Consola
#define BACKLOG 16

#include "FileSystem.h"
#include <commons/bitarray.h>
#include <commons/string.h>
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
#include <inotify.h>
#include <inttypes.h>
#include <sys/wait.h>
#include <pthread.h>

///--------------------- ESTRUCTURAS -------------------------//

//Estructura para datos del archivo de configuracion
typedef struct {
	//char PUERTO[10];
	int PUERTO_ESCUCHA;
	char PUNTO_MONTAJE[100];
	int RETARDO;
	int TAMANIO_VALUE;
	unsigned int TIEMPO_DUMP;
} ConfiguracionLFS;
typedef struct{
   char consistencia[3]; //opción 1
   //int consistencia; //opción 2 //SC, SHC, EC en protocolo.h
   int particiones;
   int tiempoCompactacion;
} MetadataLFS;
//Estructura para guardar la Metadata
typedef struct{
   int timestamp;
   uint16_t key;
   char* value; // Siempre verificar que no sobrepase el tamaño indicado por archivo de configuración
} RegistroLFS;
//Estructura para guardar los Registros
typedef struct{
	char* nombreTabla;
	MetadataLFS* metadata;
	sem_t semaforo;
	pthread_t hiloCompactacion;
	t_list* registro;
} Tabla;
//Estructura para guardar las Tablas
typedef struct {
   int tamanioBloque;
   int cantidadBloques;
   char *magicNumber;
} Metadata;
//Estructura para guardar la Metadata del filesystem

///--------------------- VARIABLES A UTILIZAR -------------------------//

int socketEscucha;
int maxSock;
int socketActivo;

pthread_t hiloDump;
pthread_t hiloAPI;

t_log* logger;
char* logFile;

ConfiguracionLFS* configuracion;
char *bitarray;
t_bitarray *bitmap;
Metadata *metadata;
t_list *tablasLFS; //La memtable

sem_t memtableSemaforo;
sem_t bitmapSemaforo;
sem_t bitmapSemaforoFILE;
sem_t configSemaforo;
sem_t metadataSemaforo;
sem_t compactacionSemaforo;
sem_t dumpSemaforo;

//------------------------ FUNCIONES --------------------------------//
void configurar(ConfiguracionLFS* configuracion);
void API_LFS();

void destruirLFS();

//Estructuras
Tabla* crearTabla(char* nombreTabla, char* consistencia, int particiones, long tiempoCompactacion);
void tablaDestruir(Tabla* tabla);
Tabla* tablaEncontrar(char* nombre);
void mostrarTablas();
RegistroLFS* RegistroLFSCrear(uint16_t key, int timestamp, char* value);
void RegistroLFSDestruir(RegistroLFS* registro);
RegistroLFS* registroEncontrar(Tabla* tabla, uint16_t key);
RegistroLFS* registroEncontrarArray(uint16_t key, char* array);
char* comprimirRegistro(RegistroLFS* reg);
void mostrarRegistros(char* nombre);
void limpiarMemtable();
void limpiarTablaMemtable(Tabla* tabla);

void dumpLFS();
void compactacion(char* nombreTabla);

//Comandos
void createLFS(char* nombreTabla, char* consistencia, int particiones, long tiempoCompactacion);
void insertLFS(char* nombreTabla, uint16_t key, char* value, int timestamp);
char* selectLFS(char* nombreTabla, uint16_t key);
void dropLFS(char* nombreTabla);

#endif /* LFS_H_ */
