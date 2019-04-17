/*
 * Memoria.c
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */
#include "Memoria.h"
void configurar(ConfiguracionMemoria* configuracion) {

	char* campos[] = {
					   "PUERTO",
					   "IP_FS",
					   "PUERTO_FS",
					   "IP_SEEDS",
					   "PUERTO_SEEDS",
					   "RETARDO_MEM",
					   "RETARDO_FS",
					   "TAM_MEM",
					   "RETARDO_JOURNAL",
					   "RETARDO_GOSSIPING",
					   "MEMORY_NUMBER=1"
					 };

	t_config* archivoConfig = archivoConfigCrear(RUTA_CONFIG, campos);

	//Relleno los campos con la info del archivo

	strcpy(configuracion->PUERTO, archivoConfigSacarStringDe(archivoConfig, "PUERTO"));
	strcpy(configuracion->IP_FS, archivoConfigSacarStringDe(archivoConfig, "IP_FS"));
	strcpy(configuracion->PUERTO_FS, archivoConfigSacarStringDe(archivoConfig, "PUERTO_FS"));
	strcpy(configuracion->IP_SEEDS, archivoConfigSacarStringDe(archivoConfig, "IP_SEEDS"));
	strcpy(configuracion->PUERTO_SEEDS, archivoConfigSacarStringDe(archivoConfig, "PUERTO_SEEDS"));
	configuracion->RETARDO_MEM = archivoConfigSacarIntDe(archivoConfig, "RETARDO_MEM");
	configuracion->RETARDO_FS = archivoConfigSacarIntDe(archivoConfig, "RETARDO_FS");
	configuracion->TAM_MEM = archivoConfigSacarIntDe(archivoConfig, "TAM_MEM");
	configuracion->RETARDO_JOURNAL = archivoConfigSacarIntDe(archivoConfig, "RETARDO_JOURNAL");
	configuracion->RETARDO_GOSSIPING = archivoConfigSacarIntDe(archivoConfig, "RETARDO_GOSSIPING");
	configuracion->MEMORY_NUMBER = archivoConfigSacarIntDe(archivoConfig, "MEMORY_NUMBER");

	archivoConfigDestruir(archivoConfig);
}
int main()
{
	configuracion = malloc(sizeof(ConfiguracionMemoria));
	configurar(configuracion);

	//FUNCIONES SOCKETS (Usar dependiendo de la biblioteca que usemos)
	//servidor
	//socketEscucha= levantarServidorIPautomatica(configuracion->PUERTO, BACKLOG); //BACKLOG es la cantidad de clientes que pueden conectarse a este servidor
	//socketActivo = aceptarComunicaciones(socketEscucha);

	// cliente
	//int socketLFS = conectarAUnServidor(configuracion->IP_FS, configuracion->PUERTO_FS);
	//int socketSEED = conectarAUnServidor(configuracion->IP_SEEDS, configuracion->PUERTO_SEEDS);

	free(configuracion);

	cerrarSocket(socketActivo);
	cerrarSocket(socketEscucha);
	cerrarSocket(socketLFS);
	cerrarSocket(socketSEED);
}


