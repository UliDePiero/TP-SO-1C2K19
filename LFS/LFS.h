/*
 * LFS.h
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */

#ifndef LFS_H_
#define LFS_H_

//Ruta del archivo de configuraciones
#define RUTA_CONFIG "LFS.config"
#define BACKLOG 16

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

pthread_t hiloCompactador;
pthread_t hiloFileSystem;
pthread_t hiloAPI;

t_log* logger;
char* logFile;

///---------------------ESTRUCTURA DE CONFIGURACION DE LFS-------------------------

//Estructura para datos del archivo de configuracion
typedef struct {
	//char PUERTO[10];
	int PUERTO_ESCUCHA;
	char PUNTO_MONTAJE[30];
	int RETARDO;
	int TAMANIO_VALUE;
	int TIEMPO_DUMP;
} ConfiguracionLFS;
/*
PUERTO_ESCUCHA=5003
PUNTO_MONTAJE=/mnt/LISSANDRA_FS/
RETARDO=500
TAMANIO_VALUE=4
TIEMPO_DUMP=5000
*/
//Estructura para guardar la configuracion del proceso
ConfiguracionLFS* configuracion;

void compactacion();
void fileSystem();
void API_LFS();

#endif /* LFS_H_ */
