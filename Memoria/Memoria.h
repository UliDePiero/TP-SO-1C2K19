/*
 * Memoria.h
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */

#ifndef MEMORIA_H_
#define MEMORIA_H_


//Ruta del archivo de configuraciones
#define RUTA_CONFIG "Memoria.config"
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

///---------------------VARIABLES A UTILIZAR-------------------------
int socketEscucha;
int maxSock;
int socketActivo;
int	socketLFS;
int	socketSEED[16];

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

void journalization();
void API_Memoria();

#endif /* MEMORIA_H_ */
