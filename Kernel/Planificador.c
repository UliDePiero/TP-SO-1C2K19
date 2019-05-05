/*
 * Planificador.c
 *
 *  Created on: 17 abr. 2019
 *      Author: utnso
 */

#include "Planificador.h"
void configurar(ConfiguracionKernel* configuracion) {

	char* campos[] = {
					   "IP_MEMORIA",
					   "PUERTO_MEMORIA",
					   "QUANTUM",
					   "MULTIPROCESAMIENTO",
					   "METADATA_REFRESH",
					   "SLEEP_EJECUCION"
					 };

	t_config* archivoConfig = archivoConfigCrear(RUTA_CONFIG, campos);

	//Relleno los campos con la info del archivo

	strcpy(configuracion->IP_MEMORIA, archivoConfigSacarStringDe(archivoConfig, "IP_MEMORIA"));
	//strcpy(configuracion->PUERTO_MEMORIA, archivoConfigSacarStringDe(archivoConfig, "PUERTO_MEMORIA"));
	configuracion->PUERTO_MEMORIA = archivoConfigSacarIntDe(archivoConfig, "PUERTO_MEMORIA");
	configuracion->QUANTUM = archivoConfigSacarIntDe(archivoConfig, "QUANTUM");
	configuracion->MULTIPROCESAMIENTO = archivoConfigSacarIntDe(archivoConfig, "MULTIPROCESAMIENTO");
	configuracion->METADATA_REFRESH = archivoConfigSacarIntDe(archivoConfig, "METADATA_REFRESH");
	configuracion->SLEEP_EJECUCION = archivoConfigSacarIntDe(archivoConfig, "SLEEP_EJECUCION");

	archivoConfigDestruir(archivoConfig);
}
//int planificador()
int main()
{
	logger = log_create(logFile, "Planificador",true, LOG_LEVEL_INFO);
	configuracion = malloc(sizeof(ConfiguracionKernel));
	configurar(configuracion);

	//FUNCIONES SOCKETS (Usar dependiendo de la biblioteca que usemos)

	// cliente
	//int socketMEMORIA = conectarAUnServidor(configuracion->IP_MEMORIA, configuracion->PUERTO_MEMORIA);
	int socketMEMORIA = connectToServer(configuracion->IP_MEMORIA, configuracion->PUERTO_MEMORIA, logger);

	free(configuracion);
	getchar();
	crearHiloIndependiente(&hiloAPI,(void*)API_Kernel, NULL, "Kernel");
	desconectarseDe(socketMemoria);
}




