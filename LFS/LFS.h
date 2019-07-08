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
#define logFile "../LFS.log"
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
	long RETARDO;
	int TAMANIO_VALUE;
	long TIEMPO_DUMP;
} ConfiguracionLFS;
typedef struct{
   char consistencia[3]; //opción 1
   //int consistencia; //opción 2 //SC, SHC, EC en protocolo.h
   int particiones;
   long tiempoCompactacion;
} MetadataLFS;
//Estructura para guardar la Metadata
typedef struct{
	uint64_t timestamp;
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
pthread_t hiloConfig;

t_log* logger;

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
sem_t dumpSemaforo;
sem_t loggerSemaforo;


//------------------------ FUNCIONES --------------------------------//
void configurar(ConfiguracionLFS* configuracion);
void* API_LFS();

void levantarLFS();
void destruirLFS();

void asd(char* a);

//------------------------ ESTRUCTURAS --------------------------------//
Tabla* crearTabla(char* nombreTabla, char* consistencia, int particiones, long tiempoCompactacion);
void tablaDestruir(Tabla* tabla);
void tablaRemover(char* nombreTabla);
Tabla* tablaEncontrar(char* nombre);
void mostrarTablas();
RegistroLFS* RegistroLFSCrear(uint16_t key, uint64_t timestamp, char* value);
void RegistroLFSDestruir(RegistroLFS* registro);
RegistroLFS* registroEncontrar(Tabla* tabla, uint16_t key);
RegistroLFS* registroEncontrarArray(uint16_t key, char* array);
RegistroLFS* registroEncontrarLista(t_list* lista, uint16_t key);
t_list* encontrarRegistros(t_list* lista, int particiones, int particion);
char* encontrarYComprimirRegistros(t_list* lista, int particiones, int particion);
void agregarRegistros(t_list* registros, char* array);
int cantDigitos(uint64_t numero);
char* comprimirRegistro(RegistroLFS* reg);
void mostrarRegistros(char* nombre);
void limpiarMemtable();
void limpiarTablaMemtable(Tabla* tabla);

void dumpLFS();
void compactacion(char* nombreTabla);

//------------------------ COMANDOS --------------------------------//
void createLFS(char* nombreTabla, char* consistencia, int particiones, long tiempoCompactacion);
void insertLFS(char* nombreTabla, uint16_t key, char* value, uint64_t timestamp);
void selectLFS(char* nombreTabla, uint16_t key);
void describeLFS(char* nombreTabla);
void dropLFS(char* nombreTabla);

#endif /* LFS_H_ */
