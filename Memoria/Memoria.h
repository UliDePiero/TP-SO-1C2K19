/*
 * Memoria_nuevo.h
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */

#ifndef MEMORIA_H_
#define MEMORIA_H_


//Ruta del archivo de configuraciones
#define RUTA_CONFIG "./Memoria.config" // Para correr desde Eclipse
//#define RUTA_CONFIG "../Memoria.config" // Para correr desde Consola
#define BACKLOG 5 //Creo que se pueden conectar solo 2 clientes pero hay que verificar

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

///---------------------VARIABLES A UTILIZAR-------------------------
int socketEscucha;
int maxSock;
int socketActivo;
int	socketLFS;
int	socketSEED[16];
int seed;

t_log* logger;
char* logFile;

pthread_t hiloAPI;
pthread_t hiloJournal;

///---------------------ESTRUCTURA DE CONFIGURACION DE LFS-------------------------

//Estructura para datos del archivo de configuracion
typedef struct {
	//char PUERTO[10];
	int PUERTO;
	char IP_FS[20];
	//char PUERTO_FS[10];
	int PUERTO_FS;

	//char IP_SEEDS[16][20]; //Verificar
	char** IP_SEEDS;
	//char PUERTO_SEEDS[16][10]; //Verificar
	int PUERTO_SEEDS[16]; //Verificar

	int RETARDO_MEM;
	int RETARDO_FS;
	int TAM_MEM;
	int RETARDO_JOURNAL;
	int RETARDO_GOSSIPING;
	int MEMORY_NUMBER;
} ConfiguracionMemoria;
/*
PUERTO=8001
IP_FS=172.0.0.1
PUERTO_FS=5003
IP_SEEDS=[172.0.0.2,172.0.0.3]
PUERTO_SEEDS=[8000,8001]
RETARDO_MEM=600
RETARDO_FS=600
TAM_MEM=2048
RETARDO_JOURNAL=60000
RETARDO_GOSSIPING=30000
MEMORY_NUMBER=1
*/
//Estructura para guardar la configuracion del proceso

ConfiguracionMemoria* configuracion;

int maxValueSize; //Obtener de LISS en el handshake

typedef struct {
		int timestamp;
		uint16_t key;
		char* value;
} Registro;
typedef struct{
	int modificado;
	void* frame;
} Pagina;
typedef struct{
	char tabla[20];
	int cantidadDePaginas;
	Pagina** tablaDePaginas;
} Segmento;

Segmento** tablaDeSegmentos;

int tamanioRealDeUnRegistro;
unsigned int cantidadDeRegistros;
unsigned int cantidadDeSegmentos;
void* granMalloc;

void terminar(int seed);
void memoriaPrincipalDestruir();
void segmentoDestruir(Segmento*);
void paginaDestruir(Pagina*);
void registroDestruir(Registro* registro);
uint16_t getKey(void* registro);
int getTimestamp(void* registro);
Registro* getRegistro(void* registro);
void setTimestamp(void* registro, int timestamp);
void setRegistro(void* registro, uint16_t key, int timestamp, char* value);
Segmento* segmentoCrear(char*, Pagina**);
Pagina* paginaCrear(int modificado, int nRegistro);
void levantarMemoria();
void asignarRegistroANuevoSegmento(char* tabla, uint16_t key, char* value, int timestamp, int nSegmento, int nRegistro);
void asignarRegistroASegmentoExistente(uint16_t key, char* value, int timestamp, int nSegmento, int nPagina, int nRegistro);
int buscarRegistroDisponible();
int ejecutarLRU();
void insertMemoria(char* tabla, uint16_t key, char* value, int timestamp);
Registro* selectMemoria(char* tabla, uint16_t key);

void journalization();
void* API_Memoria();
void ejecutarSelect(char*);
void ejecutarInsert(char*);
void ejecutarCreate(char*);
void ejecutarDescribe(char*);
void ejecutarDrop(char*);
void ejecutarJournal(char*);

#endif /* MEMORIA_H_ */
