/*
 * Memoria_nuevo.h
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */

#ifndef MEMORIA_H_
#define MEMORIA_H_


//Ruta del archivo de configuraciones
//#define RUTA_CONFIG "./Memoria.config" // Para correr desde Eclipse
//#define RUTA_CONFIG "../Memoria3.config" // Para correr desde Consola // Para memoria 1
//#define RUTA_CONFIG "../Memoria2.config" // Para memoria 2
#define logFile "../Memoria.log"
#define BACKLOG 17 //CANTIDAD MAXIMA DE SEEDS + KERNEL
#define CANT_MAX_SEEDS 16

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
#include <parser.h>
#include <inotify.h>
#include <malloc.h>

///---------------------VARIABLES A UTILIZAR-------------------------
char* RUTA_CONFIG;
int socketEscucha;
int maxSock;
int socketActivo;
int	socketLFS;
int	socketSEED[CANT_MAX_SEEDS];
int cliente[BACKLOG][2];
int cantidadClientes;
int seed;

t_log* logger;

sem_t mutexMemoria;
sem_t gossipMemoria;
sem_t loggerSemaforo;

pthread_t hiloAPI;
pthread_t hiloConfig;
pthread_t hiloJournal;
pthread_t hiloGossipMemoria;
///---------------------ESTRUCTURA DE CONFIGURACION DE MEMORIA-------------------------

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
	int PUERTO_SEEDS[CANT_MAX_SEEDS]; //Verificar

	int RETARDO_MEM;
	int RETARDO_FS;
	int TAM_MEM;
	int RETARDO_JOURNAL;
	int RETARDO_GOSSIPING;
	int MEMORY_NUMBER;
	char IP_PROPIA[20];
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

// Estructura para guardar las memorias que pertenecen al pool y datos de conexión
typedef struct {
	int IDMemoria; // ID o Número de Memoria (Viene dado por el Archivo de Configuración de Memoria)
	char IPMemoria[20]; // IP de la Memoria
	int puertoMemoria; // Puerto de la Memoria
	int socketMemoria;
} TablaGossip;

t_list* listaGossiping;

typedef struct {
	int segmentoID;
	int paginaID;
	int modificado;
} t_nodoLRU;

t_list* listaPaginasLRU;

int maxValueSize; //Obtener de LISS en el handshake

typedef struct {
		uint64_t timestamp;
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

void terminar();
void memoriaPrincipalDestruir();
void segmentoDestruir(Segmento*);
void paginaDestruir(Pagina*);
void registroDestruir(Registro* registro);
uint16_t getKey(void* registro);
uint64_t getTimestamp(void* registro);
Registro* getRegistro(void* registro);
void setValue(void* registro, char* value);
void setTimestamp(void* registro, uint64_t timestamp);
void setRegistro(void* registro, uint16_t key, uint64_t timestamp, char* value);
Segmento* segmentoCrear(char*, Pagina**);
Pagina* paginaCrear(int modificado, int nRegistro);
void levantarMemoria();
void resetearMemoria(void* punteroMemoria);
void vaciarMemoria();
void asignarRegistroANuevoSegmento(char* tabla, uint16_t key, char* value, uint64_t timestamp, int nSegmento, int nRegistro, int flagModificado);
void asignarRegistroASegmentoExistente(uint16_t key, char* value, uint64_t timestamp, int nSegmento, int nPagina, int nRegistro, int flagModificado);
void reasignarRegistroASegmentoExistente(uint16_t key, char* value, uint64_t timestamp, int nSegmento, int nPagina, int flagModificado);
int reasignarRegistroANuevoSegmento(char* tabla, uint16_t key, char* value, uint64_t timestamp, int nSegmento_nodo, int nSegmento, int nPagina_nodo, int flagModificado);
int buscarRegistroDisponible();
int buscarRegistro(t_nodoLRU* nodo_reemplazo);

void insertMemoria(char* tabla, uint16_t key, char* value, uint64_t timestamp, int flagModificado);
Registro* selectMemoria(char* tabla, uint16_t key);
void journalMemoria();
void dropMemoria(char* tabla);
char* describeMemoriaTabla(char* tabla);
t_list* describeMemoria();

void* API_Memoria();
char* ejecutarSelect(char*);
char* ejecutarInsert(char*);
char* ejecutarCreate(char*);
t_list* ejecutarDescribe(char*);
char* ejecutarDrop(char*);
int ejecutarJournal(char*);

void* journalAutomatico();
void ejecutarInsertJournal();

int memoriaEstaFull(t_list* lista);
void mostrarlistaPaginasLRU(t_list* lista);
void encolarNuevaPagina(t_list* lista, t_nodoLRU* nodo);
void encolarPaginaExistente(t_list* lista, t_link_element* nodo);
t_nodoLRU* desencolarPrimerElementoNoModificado(t_list *lista);
int estaEnListaDePaginas(t_list* lista, t_nodoLRU* nodo);
t_nodoLRU* insertarEnListaDePaginasLRU(t_list* lista, t_nodoLRU* nodo);
void removerElemento(t_list* lista, t_nodoLRU* nodo);

// Funciones para Gossiping
void realizarHandshake(int socket);
void eliminaMemoriaDeListaGossipingSERVER(int socketMem);
void eliminaMemoriaDeListaGossiping(int socketMem);
void conectarConNuevaMemoria(TablaGossip* nodo, int seed_gos);
void armarNodoMemoria(TablaGossip* nodo, int seed_gos);
int recibeLista(int socketMem, int seed_gos);
int pideListaGossiping(int socketMem, int seed_gos);
void pideListaGossiping_2(int socketMem);
void enviarListaGossiping(int socketEnvio);
void enviaLista(int socketMem);
void armarPropioNodo();
int nodoSocketEstaEnLista(int socketID);
void* gossipingMemoria();
void serializarNodo(TablaGossip* nodo, char* paqueteSerializado);
int recibirNodoYDeserializar(TablaGossip *nodo, int socketMem);
int nodoEstaEnLista(t_list* lista, TablaGossip* nodo);
void muestraLista(char* elemento);
void muestraListaGossip(TablaGossip* elemento);

#endif /* MEMORIA_H_ */
