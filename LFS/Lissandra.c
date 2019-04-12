/*
 * Lissandra.c
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */
#include "LFS.h"
void configurar(ConfiguracionLFS* configuracion) {

	char* campos[] = {
					   "PUERTO",
					   "PUNTO_MONTAJE",
					   "RETARDO",
					   "TAMAÑO_VALUE",
					   "TIEMPO_DUMP"
					 };

	t_config* archivoConfig = archivoConfigCrear(RUTA_CONFIG, campos);

	//Relleno los campos con la info del archivo

	strcpy(configuracion->PUERTO, archivoConfigSacarStringDe(archivoConfig, "PUERTO"));
	strcpy(configuracion->PUNTO_MONTAJE, archivoConfigSacarStringDe(archivoConfig, "PUNTO_MONTAJE"));
	configuracion->RETARDO = archivoConfigSacarIntDe(archivoConfig, "RETARDO");
	configuracion->TAMAÑO_VALUE = archivoConfigSacarIntDe(archivoConfig, "TAMAÑO_VALUE");
	configuracion->TIEMPO_DUMP = archivoConfigSacarIntDe(archivoConfig, "TIEMPO_DUMP");

	archivoConfigDestruir(archivoConfig);
}
int main()
{
	configuracion = malloc(sizeof(ConfiguracionLFS));
	configurar(configuracion);

	//servidor
	//FUNCIONES SOCKETS (Usar dependiendo de la biblioteca que usemos)
	//socketEscucha= levantarServidorIPautomatica(configuracion->PUERTO, BACKLOG); //BACKLOG es la cantidad de clientes que pueden conectarse a este servidor
	//socketActivo = aceptarComunicaciones(socketEscucha);
	free(configuracion);

	crearHilo(&hiloCompactador,(void*)compactacion, NULL, "LFS");
	crearHilo(&hiloFileSystem,(void*)fileSystem, NULL, "LFS");

	pthread_join(hiloCompactador, NULL);
	pthread_join(hiloFileSystem, NULL);

	cerrarSocket(socketActivo);
	cerrarSocket(socketEscucha);
}

