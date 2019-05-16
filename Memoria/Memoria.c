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
					   "MEMORY_NUMBER"
					 };

	t_config* archivoConfig = archivoConfigCrear(RUTA_CONFIG, campos);
	//Relleno los campos con la info del archivo

	//strcpy(configuracion->PUERTO, archivoConfigSacarStringDe(archivoConfig, "PUERTO"));
	configuracion->PUERTO = archivoConfigSacarIntDe(archivoConfig, "PUERTO");
	strcpy(configuracion->IP_FS, archivoConfigSacarStringDe(archivoConfig, "IP_FS"));
	//strcpy(configuracion->PUERTO_FS, archivoConfigSacarStringDe(archivoConfig, "PUERTO_FS"));
	configuracion->PUERTO_FS = archivoConfigSacarIntDe(archivoConfig, "PUERTO_FS");
/*
	memcpy(configuracion->IP_SEEDS, archivoConfigSacarArrayDe(archivoConfig, "IP_SEEDS"), sizeof (configuracion->IP_SEEDS));
	memcpy(configuracion->PUERTO_SEEDS, archivoConfigSacarArrayDe(archivoConfig, "PUERTO_SEEDS"), sizeof (configuracion->PUERTO_SEEDS));
*/
	configuracion->IP_SEEDS = archivoConfigSacarArrayDe(archivoConfig, "IP_SEEDS");

	char** vectorStringPuertos = archivoConfigSacarArrayDe(archivoConfig, "PUERTO_SEEDS");
	for(int i=0;vectorStringPuertos[i]!=NULL;i++) configuracion->PUERTO_SEEDS[i] = atoi(vectorStringPuertos[i]);

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
	logger = log_create(logFile, "Memoria",true, LOG_LEVEL_INFO);
	configuracion = malloc(sizeof(ConfiguracionMemoria));
	for(int i=0;i<16;i++) configuracion->PUERTO_SEEDS[i]=0;
	configurar(configuracion);

	//FUNCIONES SOCKETS (Usar dependiendo de la biblioteca que usemos)
	//servidor
	//socketEscucha= levantarServidorIPautomatica(configuracion->PUERTO, BACKLOG); //BACKLOG es la cantidad de clientes que pueden conectarse a este servidor
	//socketActivo = aceptarComunicaciones(socketEscucha);
	fd_set setSocketsOrquestador;
	FD_ZERO(&setSocketsOrquestador);

	// Inicializacion de sockets y actualizacion del log

	socketEscucha = crearSocketEscucha(configuracion->PUERTO, logger);

	free(configuracion);

	crearHiloIndependiente(&hiloAPI,(void*)API_Memoria, NULL, "Memoria");
	//pthread_create(&hiloJournal, NULL, (void*)journalization, NULL);
	//pthread_join(hiloJournal, NULL);

	//ESTO TIENE QUE IR EN UN HILO APARTE PARA QUE QUEDE EN LOOP
	FD_SET(socketEscucha, &setSocketsOrquestador);
	maxSock = socketEscucha;
	tMensaje tipoMensaje;
	char * sPayload;
	while (1) {

		puts("Escuchando");
		socketActivo = getConnection(&setSocketsOrquestador, &maxSock, socketEscucha, &tipoMensaje, &sPayload, logger);
		printf("Socket comunicacion: %d \n", socketActivo); //CORREGIR getConnection
		if (socketActivo != -1) {

			switch (tipoMensaje) {
				case SELECT:
					printf("\nRecibi SELECT\n");
					//hilo con parametro SELECT
					break;
				case INSERT:
					printf("\nRecibi INSERT\n");
					//hilo con parametro INSERT
					break;
				case CREATE:
					printf("\nRecibi CREATE\n");
					//hilo con parametro CREATE
					break;
				case DESCRIBE:
					printf("\nRecibi DESCRIBE\n");
					//hilo con parametro DESCRIBE
					break;
				case DROP:
					printf("\nRecibi DROP\n");
					//hilo con parametro DROP
					break;
				case JOURNAL:
					printf("\nRecibi JOURNAL\n");
					//hilo con parametro JOURNAL
					break;

				default:
					printf("Tipo de mensaje desconocido \n");
					break;

			}
		}

	}

	//ESTE PUEDE QUEDAR EN EL HILO PRINCIPAL
	// cliente
	//int socketLFS = conectarAUnServidor(configuracion->IP_FS, configuracion->PUERTO_FS);
	//int socketSEED = conectarAUnServidor(configuracion->IP_SEEDS, configuracion->PUERTO_SEEDS);
	int socketLFS = connectToServer(configuracion->IP_FS, configuracion->PUERTO_FS, logger);
	int seed=0;
	while(configuracion->PUERTO_SEEDS[seed] != 0 && seed<16){
		socketSEED[seed] = connectToServer(configuracion->IP_SEEDS[seed], configuracion->PUERTO_SEEDS[seed], logger);
    	seed++;
	}

	free(configuracion);

	desconectarseDe(socketActivo);
	desconectarseDe(socketEscucha);
	desconectarseDe(socketLFS);
	seed=0;
	while(configuracion->PUERTO_SEEDS[seed] != 0 && seed<16){
		desconectarseDe(socketSEED[seed]);
	    seed++;
	}

}




void insertMemoria(char* tabla, uint16_t key, char* value, int timestamp){
	memoriaPrincipal->key = key;
	memoriaPrincipal->value = value;
	memoriaPrincipal->timestamp = timestamp;
}
RegistroMemoria* selectMemoria(char* tabla, uint16_t key){
	return memoriaPrincipal;
}

void levantarMemoria(){
	memoriaPrincipal = (RegistroMemoria*) malloc(configuracion->TAM_MEM % sizeof(RegistroMemoria));
}
Segmento* segmentoCrear(char* tabla, Pagina* pagina){
	Segmento* segmento = (Segmento*)malloc(sizeof(Segmento));
	strcpy(segmento->tabla, tabla);
	segmento->pagina = pagina;
	return segmento;
}
void segmentoDestruir(Segmento* segmento){
	free(segmento->tabla);
	free(segmento->pagina);
	free(segmento);
}
Pagina* paginaCrear(int modificado, RegistroMemoria* registro){
	Pagina* pagina = (Pagina*)malloc(sizeof(Pagina));
	pagina->modificado = modificado;
	pagina->registro = registro;
	return pagina;
}
void paginaDestruir(Pagina* pagina){
	free(pagina->registro);
	free(pagina);
}
void memoriaPrincipalDestruir(){
	for(int i = 0; i < configuracion->TAM_MEM % sizeof(RegistroMemoria); i++){
		free(memoriaPrincipal[i].value);
	}
	free(memoriaPrincipal);
}
