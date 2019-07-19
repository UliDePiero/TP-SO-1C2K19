/*
 * Planificador.h
 *
 *  Created on: 17 abr. 2019
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

//Ruta del archivo de configuraciones
//#define RUTA_CONFIG "./Kernel.config" // Para correr desde Eclipse
#define RUTA_CONFIG "../Kernel.config" // Para correr desde Consola
#define logFile "../Kernel.log"

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
#include <commons/collections/list.h>
#include "API_Kernel.h"

///---------------------VARIABLES A UTILIZAR-------------------------
int socketMemoria;

t_log* logger;

pthread_t hiloAPI;
pthread_t hiloPlanificacion;
pthread_t hiloRespuestasRequest;

//Colas de procesos
t_queue *New;
t_queue *Ready;
t_queue *Exec;
t_queue *Exit;
t_list *ListaLQL;
sem_t semContadorLQL;
sem_t semMultiprocesamiento;
sem_t semEjecutarLQL;
sem_t loggerSemaforo;

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

// Listas de memorias asignadas a cada criterio de consistencia:
t_list* memoriaSC; // Listado de Memorias asignadas al criterio SC (Cantidad = 0 o 1)
t_list* memoriasSHC; // Listado de Memorias asignadas al criterio SHC (Cantidad = 0 o >0)
t_list* memoriasEC; // Listado de Memorias asignadas al criterio EC (Cantidad = 0 o >0)

// Estructura para guardar las memorias que pertenecen al pool, datos de conexión y criterios asignados
typedef struct {
	int IDMemoria; // ID o Número de Memoria (Viene dado por el Archivo de Configuración de Memoria)
	char IPMemoria[20]; // IP de la Memoria
	int puertoMemoria; // Puerto de la Memoria
	int socketMemoria;
	unsigned char criterioSC; // Vale 1 si la Memoria está asignada al criterio SC, sino 0
	unsigned char criterioSHC; // Vale 1 si la Memoria está asignada al criterio SHC, sino 0
	unsigned char criterioEC; // Vale 1 si la Memoria está asignada al criterio EC, sino 0
} TablaGossip;

t_list* listaGossiping;

typedef struct{
   char consistencia[3];
   int particiones;
   long tiempoCompactacion;
} Metadata;
typedef struct{
	char* nombreTabla;
	Metadata* metadata;
} Tabla;

t_list* listaTablas;

/* --------------------    Definición de Funciones    -------------------- */
//Funciones usadas en API
void limpiarListaTablas(Tabla* tabla);
Tabla* encontrarTabla(char* nombreTabla);
// Funciones de Configuración
void configurar(ConfiguracionKernel* configuracion);
void cambiosConfigKernel();
// Funciones para Planificación
void API_Kernel();
void planificacion();
void cargarNuevoLQL(char* ScriptLQL);
void moverLQL(t_queue *colaOrigen, t_queue *colaDestino);
void actualizarRequestEjecutadas();
// Funciones de Criterios de Consistencia
void crearListasDeCriteriosMemorias();
TablaGossip* buscarNodoMemoria(int nroMemoria);
void asociarACriterioSC(int nroMemoria);
void asociarACriterioSHC(int nroMemoria);
void asociarACriterioEC(int nroMemoria);
void desasociarDeCriterioSC(int nroMemoria);
int buscarMemoriaEnListaCriterio(int nroMemoria, t_list* listaMemoriasCriterio);
void desasociarDeCriterioSHC(int nroMemoria);
void desasociarDeCriterioEC(int nroMemoria);
TablaGossip* elegirMemoriaCriterioSC();
int funcionHash(int key, int cantMaxMemoriasSHC);
TablaGossip* elegirMemoriaCriterioSHC(int key);
int generarNumeroRandom(int nroMax);
TablaGossip* elegirMemoriaCriterioEC();
// Funciones para Gossiping
int pideRetardoGossiping();
void armarNodoMemoria(TablaGossip* nodo);
void pideListaGossiping(int socketMem);
void conectarConNuevaMemoria(TablaGossip* nodo);
void gossipingKernel();
int recibirNodoYDeserializar(TablaGossip *nodo, int socketMem);
int nodoEstaEnLista(t_list* lista, TablaGossip* nodo);

#endif /* PLANIFICADOR_H_ */
