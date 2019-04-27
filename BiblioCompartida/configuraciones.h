/*
 * configuraciones.h
 *
 *  Created on: 20 abr. 2019
 *      Author: utnso
 */

#ifndef CONFIGURACIONES_H_
#define CONFIGURACIONES_H_

//----------------DEFINICION DE MACROS----------------//

#define TRUE				1
#define FALSE				0

///----------------BIBLIOTECAS INCLUIDAS----------------///

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <pthread.h>

///----------------STRUCTS-----------------///

//Estructura para las conexiones
typedef struct {
	struct addrinfo* info;
	struct sockaddr_in address;			//Direccion contenida, en forma binaria
	socklen_t tamanioAddress;
	char* port;							//Puerto e ip, en forma de cadenas
	char* ip;
} Conexion;

///----------------FUNCIONES-----------------///

//Funciones de archivos de configuracion
t_config* archivoConfigCrear(char* path, char** campos);
bool archivoConfigTieneCampo(t_config* archivoConfig, char* campo);
bool archivoConfigFaltaCampo(t_config* archivoConfig, char* campo);
char* archivoConfigSacarStringDe(t_config* archivoConfig, char* campo);
int archivoConfigSacarIntDe(t_config* archivoConfig, char* campo);
long archivoConfigSacarLongDe(t_config* archivoConfig, char* campo);
double archivoConfigSacarDoubleDe(t_config* archivoConfig, char* campo);
char** archivoConfigSacarArrayDe(t_config* archivoConfig, char* campo);
int archivoConfigCantidadCampos(t_config* archivoConfig);
void archivoConfigDestruir(t_config* archivoConfig);
void archivoConfigSetearCampo(t_config* archivoConfig, char* campo, char* valor);
bool archivoConfigInvalido(t_config* archivoConfig, char** campos);
bool archivoConfigInexistente(t_config* archivoConfig);
bool archivoConfigIncompleto(t_config* archivoConfig, char** campos);

//Funciones de error
void fallo(char* mensajeError);
void terminarConError(int socket, char* error_msg, void* buffer);

#endif /* CONFIGURACIONES_H_ */
