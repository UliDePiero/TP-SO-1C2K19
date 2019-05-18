/*
 * Lissandra.c
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */
#include "LFS.h"
void configurar(ConfiguracionLFS* configuracion) {

	char* campos[] = {
					   "PUERTO_ESCUCHA",
					   "PUNTO_MONTAJE",
					   "RETARDO",
					   "TAMANIO_VALUE",
					   "TIEMPO_DUMP"
					 };

	t_config* archivoConfig = archivoConfigCrear(RUTA_CONFIG, campos);

	//Relleno los campos con la info del archivo

	//strcpy(configuracion->PUERTO, archivoConfigSacarStringDe(archivoConfig, "PUERTO"));
	configuracion->PUERTO_ESCUCHA = archivoConfigSacarIntDe(archivoConfig, "PUERTO_ESCUCHA");
	strcpy(configuracion->PUNTO_MONTAJE, archivoConfigSacarStringDe(archivoConfig, "PUNTO_MONTAJE"));
	configuracion->RETARDO = archivoConfigSacarIntDe(archivoConfig, "RETARDO");
	configuracion->TAMANIO_VALUE = archivoConfigSacarIntDe(archivoConfig, "TAMANIO_VALUE");
	configuracion->TIEMPO_DUMP = archivoConfigSacarIntDe(archivoConfig, "TIEMPO_DUMP");

	archivoConfigDestruir(archivoConfig);
}
int main()
{
	logger = log_create(logFile, "LFS",true, LOG_LEVEL_INFO);
	configuracion = malloc(sizeof(ConfiguracionLFS));
	configurar(configuracion);

	//servidor
	//FUNCIONES SOCKETS (Usar dependiendo de la biblioteca que usemos)

	/*socketEscucha= levantarServidorIPautomatica(configuracion->PUERTO_ESCUCHA, BACKLOG); //BACKLOG es la cantidad de clientes que pueden conectarse a este servidor
	socketActivo = aceptarComunicaciones(socketEscucha);*/

	fd_set setSocketsOrquestador;
	FD_ZERO(&setSocketsOrquestador);

	// Inicializacion de sockets y actualizacion del log

	socketEscucha = crearSocketEscucha(configuracion->PUERTO_ESCUCHA, logger);

	free(configuracion);

	//crearHiloIndependiente(&hiloCompactador,(void*)compactacion, NULL, "LFS");
	//pthread_create(&hiloCompactador, NULL, (void*)compactacion, NULL);
	//pthread_join(hiloCompactador, NULL);

	crearHilo(&hiloFileSystem,(void*)fileSystem, NULL, "LFS"); //LEVANTO FILESYSTEM
	joinearHilo(hiloFileSystem, NULL, "LFS");
	crearHiloIndependiente(&hiloAPI,(void*)API_LFS, NULL, "LFS");

	//ESTO TIENE QUE IR EN UN HILO APARTE PARA QUE QUEDE EN LOOP  ???
	FD_SET(socketEscucha, &setSocketsOrquestador);
	maxSock = socketEscucha;
	tMensaje tipoMensaje;
	char * sPayload;
	while (1) {

		puts("Escuchando");
		socketActivo = getConnection(&setSocketsOrquestador, &maxSock, socketEscucha, &tipoMensaje, &sPayload, logger);
		printf("Socket comunicacion: %d \n", socketActivo);//CORREGIR getConnection
		if (socketActivo != -1) {

			switch (tipoMensaje) {

			}
		}

	}

	desconectarseDe(socketActivo);
	desconectarseDe(socketEscucha);
}

void insertLFS(char* nombreTabla, uint16_t key, int value, int timestamp){
	int numeroTabla = 0;
	MetadataLFS* metadataTabla;
	while( strcmp(tablasLFS[numeroTabla]->nombreTabla, nombreTabla) != 0 && tablasLFS[numeroTabla]!=NULL) numeroTabla++;
	if(tablasLFS[numeroTabla] == NULL)
		perror("No se encontro la tabla solicitada.");
	else{
		metadataTabla = tablasLFS[numeroTabla]->metadata;
		//Verificar si existe en memoria una lista de datos a dumpear. De no existir, alocar dicha memoria.
		//Insertar en la memoria temporal del punto anterior una nueva entrada que contenga los datos enviados en la request.
		/*
		De esta manera, cada insert se realizará siempre sobre la porción de memoria temporal asignada a dicha tabla sin importarle
		si dentro de la misma ya existe la key. Esto es así, ya que al momento de obtener la misma se retornará el que tenga un Timestamp más
		reciente mientras que el proceso de Compactación (explicado en el Anexo I), posterior al proceso de dump, será el que se encargue
		de unificar dichas Keys dentro del archivo original.
		*/
		/*
		 Todo dato dentro de un archivo será persistido con el formato:
		 [TIMESTAMP];[KEY];[VALUE]
		 */
	}
}
RegistroLFS* selectLFS(char* nombreTabla, uint16_t key){
	int numeroTabla = 0;
	MetadataLFS* metadataTabla;
	while( strcmp(tablasLFS[numeroTabla]->nombreTabla, nombreTabla) != 0 && tablasLFS[numeroTabla]!=NULL) numeroTabla++;
	if(tablasLFS[numeroTabla]!=NULL){
		metadataTabla = tablasLFS[numeroTabla]->metadata;
		//Calcular cual es la partición que contiene dicho KEY.
		//Escanear la partición objetivo, todos los archivos temporales y la memoria temporal de dicha tabla (si existe) buscando la key deseada.
		//Encontradas las entradas para dicha Key, se retorna el valor con el Timestamp más grande.
		return NULL; //CAMBIAR RETURN
	}
	else{
		perror("No se encontro la tabla solicitada.");
		return NULL;
	}
}
