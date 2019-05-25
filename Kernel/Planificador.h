/*
 * Planificador.h
 *
 *  Created on: 17 abr. 2019
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_


//Ruta del archivo de configuraciones
#define RUTA_CONFIG "./Kernel.config" // Para correr desde Eclipse
//#define RUTA_CONFIG "../Kernel.config" // Para correr desde Consola

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
#include <pthread.h>
#include <configuraciones.h>
#include <inotify.h>

///---------------------VARIABLES A UTILIZAR-------------------------
int socketMemoria;

t_log* logger;
char* logFile;

pthread_t hiloAPI;
pthread_t hiloPlanificacion;

//Colas de procesos
t_queue *New;
t_queue *Ready;
t_queue *Exec;
t_queue *Exit;
t_list *ListaLQL;
sem_t semContadorLQL;
sem_t semMultiprocesamiento;
sem_t semEjecutarLQL;

int LQLEnEjecucion;
int IDLQL; //autoincremental

///---------------------ESTRUCTURA DE CONFIGURACION DE LFS-------------------------

//Estructura para datos del archivo de configuracion
typedef struct {
	char IP_MEMORIA[20];
	//char PUERTO_MEMORIA[10];
	int PUERTO_MEMORIA;
	int QUANTUM;
	int MULTIPROCESAMIENTO;
	int METADATA_REFRESH;
	int SLEEP_EJECUCION;
} ConfiguracionKernel;
/*
IP_MEMORIA=127.0.0.2
PUERTO_MEMORIA=8001
QUANTUM=4
MULTIPROCESAMIENTO=3
METADATA_REFRESH=10000
SLEEP_EJECUCION=5000
*/
//Estructura para guardar la configuracion del proceso
ConfiguracionKernel* configuracion;

typedef struct {
	int ID;
	char Instruccion[100];
	int requestEjecutadas;
	//char PC[20];
	//char TablaArchivosAbiertos[10];
	int FlagIncializado;
} EstructuraLQL;
EstructuraLQL* LQL;
void API_Kernel();
void planificacion();
void cargarNuevoLQL(char* ScriptLQL);
void moverLQL(t_queue *colaOrigen, t_queue *colaDestino);
void actualizarRequestEjecutadas();

#endif /* PLANIFICADOR_H_ */
