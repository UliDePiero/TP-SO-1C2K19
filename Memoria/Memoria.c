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
	levantarMemoria();

	//FUNCIONES SOCKETS (Usar dependiendo de la biblioteca que usemos)
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
	//servidor
	//socketEscucha= levantarServidorIPautomatica(configuracion->PUERTO, BACKLOG); //BACKLOG es la cantidad de clientes que pueden conectarse a este servidor
	//socketActivo = aceptarComunicaciones(socketEscucha);
	fd_set setSocketsOrquestador;
	FD_ZERO(&setSocketsOrquestador);

	// Inicializacion de sockets y actualizacion del log

	socketEscucha = crearSocketEscucha(configuracion->PUERTO, logger);

	//free(configuracion);

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
					//funcion SELECT
					break;
				case INSERT:
					printf("\nRecibi INSERT\n");
					//funcion INSERT
					break;
				case CREATE:
					printf("\nRecibi CREATE\n");
					//funcion CREATE
					break;
				case DESCRIBE:
					printf("\nRecibi DESCRIBE\n");
					//funcion DESCRIBE
					break;
				case DROP:
					printf("\nRecibi DROP\n");
					//funcion DROP
					break;
				case JOURNAL:
					printf("\nRecibi JOURNAL\n");
					//funcion JOURNAL
					break;

				default:
					printf("Tipo de mensaje desconocido \n");
					break;

			}
		}

	}

	free(configuracion);
	memoriaPrincipalDestruir();
	desconectarseDe(socketActivo);
	desconectarseDe(socketEscucha);
	desconectarseDe(socketLFS);
	seed=0;
	while(configuracion->PUERTO_SEEDS[seed] != 0 && seed<16){
		desconectarseDe(socketSEED[seed]);
	    seed++;
	}

}



//FALTA LRU + Que pasa si esta la memoria FULL + journal, esto ultimo dentro de la funcion LRU
void insertMemoria(char* tabla, uint16_t key, char* value, int timestamp){
	int pagina = 0, segmento = 0, registro = -1;
	if(cantidadDeSegmentos == 0){
		registro = 0;
		asignarRegistroANuevoSegmento(tabla, key, value, timestamp, segmento, registro);
		//printf("\nInserto: %s %d %s %d",tablaDeSegmentos[segmento]->tabla,tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->registro->key,tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->registro->value,tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->registro->timestamp);
		//printf("\nInsertoMEM: %s %d %s %d",tablaDeSegmentos[segmento]->tabla,memoriaPrincipal[registro]->key,memoriaPrincipal[registro]->value,memoriaPrincipal[registro]->timestamp);

	}
	else{
		for(segmento=0; segmento<cantidadDeSegmentos; segmento++){
			if(strcmp(tablaDeSegmentos[segmento]->tabla, tabla) == 0) break;
		}
		if(segmento<cantidadDeSegmentos){
			for(pagina=0; pagina<tablaDeSegmentos[segmento]->cantidadDePaginas;pagina++ ){
				if(tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->registro->key == key) break;
			}
			if(pagina<tablaDeSegmentos[segmento]->cantidadDePaginas){
				tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->registro->timestamp = timestamp;
			//printf("\nInserto: %s %d %s %d",tablaDeSegmentos[segmento]->tabla,tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->registro->key,tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->registro->value,tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->registro->timestamp);
			}else{
				registro = buscarRegistroDisponible();
				if(registro == -1) registro = ejecutarLRU(); //Funcion LRU
				asignarRegistroASegmentoExistente(key, value, timestamp, segmento, pagina, registro);
			}
		}
		else{
			registro = buscarRegistroDisponible();
			if(registro == -1) registro = ejecutarLRU(); //Funcion LRU
			asignarRegistroANuevoSegmento(tabla, key, value, timestamp, segmento, registro);
		}
	}
}
//FALTA en select Que pasa si esta la memoria FULL + journal
RegistroMemoria* selectMemoria(char* tabla, uint16_t key){
	int segmento = 0, pagina = 0;
	for(segmento=0; segmento<cantidadDeSegmentos; segmento++){
		if(strcmp(tablaDeSegmentos[segmento]->tabla, tabla) == 0) break;
	}
	if(segmento<cantidadDeSegmentos){
		for(pagina=0; pagina<tablaDeSegmentos[segmento]->cantidadDePaginas ; pagina++){
			if(tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->registro->key == key) break;
		}
		if(pagina<tablaDeSegmentos[segmento]->cantidadDePaginas)
			return tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->registro;
		else{
			/*
			 * Intercambio de mensajes con LFS para obtener un tablaDeSegmentos[registro]->tablaDePaginas[pagina]->registro
			 */
			/*
			tablaDeSegmentos[registro]->tablaDePaginas[pagina]->registro->key = key;
			strcpy(tablaDeSegmentos[registro]->tablaDePaginas[pagina]->registro->value, value);
			tablaDeSegmentos[registro]->tablaDePaginas[pagina]->registro->timestamp = timestamp;
			*/

			return NULL; //CAMBIAR CUANDO ESTE HECHO LO DE ARRIBA algo asi: return tablaDeSegmentos[registro]->tablaDePaginas[pagina]->registro;
		}
	}
	else{
		perror("No se encontro el segmento que contiene la tabla solicitada.");
		return NULL;
	}
}

void levantarMemoria(){
	int i;
	tablaDeSegmentos = malloc(1);
	maxValueSize = 20; //ESTO TIENE QUE VENIR DE LFS
	//tamanioRealDeUnRegistro = sizeof(RegistroMemoria) + maxValueSize+1; //por que el +1 ???
	tamanioRealDeUnRegistro = sizeof(RegistroMemoria) + maxValueSize;
	tamanioRealDeUnaPagina = tamanioRealDeUnRegistro + sizeof(Pagina);
	cantidadDeRegistros = configuracion->TAM_MEM / tamanioRealDeUnRegistro;
	memoriaPrincipal = (RegistroMemoria**) malloc(cantidadDeRegistros*sizeof(RegistroMemoria*));
	for(i=0; i < cantidadDeRegistros; i++){
		//memoriaPrincipal[i] = (RegistroMemoria*) malloc(tamanioRealDeUnRegistro); //opcion 1
		memoriaPrincipal[i] = (RegistroMemoria*) malloc(sizeof(RegistroMemoria)); //opcion 2
		memoriaPrincipal[i]->value = (char*) malloc(maxValueSize*sizeof(char));
		memoriaPrincipal[i]->timestamp = -1;
	}
}
Segmento* segmentoCrear(char* tabla, Pagina** tablaDePaginas){
	Segmento* segmento = (Segmento*)malloc(sizeof(Segmento));
	strcpy(segmento->tabla, tabla);
	segmento->tablaDePaginas = tablaDePaginas;
	return segmento;
}
void segmentoDestruir(Segmento* segmento){
	free(segmento->tablaDePaginas);
	for(int i=0; i < segmento->cantidadDePaginas; i++)
		free(segmento->tablaDePaginas[i]);
	free(segmento);
}
Pagina* paginaCrear(int modificado, RegistroMemoria* registro){
	Pagina* pagina = (Pagina*)malloc(sizeof(Pagina));
	pagina->modificado = modificado;
	pagina->registro = registro;
	return pagina;
}
//Libera memoria de la pagina y del registro
void paginaDestruir(Pagina* pagina){
	registroDestruir(pagina->registro);
	free(pagina);
}
RegistroMemoria* registroCrear(int timeStamp, uint16_t key, char* value, int nRegistro){
	//RegistroMemoria* registro = (RegistroMemoria*)malloc(sizeof(RegistroMemoria));
	RegistroMemoria* registro = memoriaPrincipal[nRegistro];
	registro->key = key;
	registro->timestamp = timeStamp;
	registro->value = memoriaPrincipal[nRegistro]->value;
	strcpy(registro->value, value);
	return registro;
}
void registroDestruir(RegistroMemoria* registro){
	free(registro->value);
	free(registro);
}
void memoriaPrincipalDestruir(){
	for(int j=0; j < cantidadDeSegmentos; j++)
		segmentoDestruir(tablaDeSegmentos[j]);
	for(int i = 0; i < cantidadDeRegistros; i++){
		registroDestruir(memoriaPrincipal[i]);
	}
	free(memoriaPrincipal);
}
void asignarRegistroANuevoSegmento(char* tabla, uint16_t key, char* value, int timestamp, int nSegmento, int nRegistro){
	cantidadDeSegmentos++;
	tablaDeSegmentos = (Segmento**) realloc(tablaDeSegmentos, cantidadDeSegmentos*sizeof(Segmento*));
	RegistroMemoria* registroDelSegmento = registroCrear(timestamp, key, value, nRegistro);
	Pagina* paginaDelSegmento = paginaCrear(0, registroDelSegmento);
	Pagina** tablaDePaginasDelSegmento = (Pagina**) malloc(sizeof(Pagina*));
	Segmento* segmento = segmentoCrear(tabla, tablaDePaginasDelSegmento);
	tablaDeSegmentos[nSegmento] = segmento;
	tablaDeSegmentos[nSegmento]->cantidadDePaginas = 1;
	strcpy(tablaDeSegmentos[nSegmento]->tabla, tabla);
	tablaDeSegmentos[nSegmento]->tablaDePaginas = tablaDePaginasDelSegmento;
	tablaDeSegmentos[nSegmento]->tablaDePaginas[tablaDeSegmentos[nSegmento]->cantidadDePaginas - 1] = paginaDelSegmento;
}
void asignarRegistroASegmentoExistente(uint16_t key, char* value, int timestamp, int nSegmento, int nPagina, int nRegistro){
	RegistroMemoria* registroDelSegmento = registroCrear(timestamp, key, value, nRegistro);
	Pagina* paginaDelSegmento = paginaCrear(0, registroDelSegmento);
	tablaDeSegmentos[nSegmento]->cantidadDePaginas ++;
	tablaDeSegmentos[nSegmento]->tablaDePaginas = (Pagina**) realloc(tablaDeSegmentos[nSegmento]->tablaDePaginas, tablaDeSegmentos[nSegmento]->cantidadDePaginas*sizeof(Pagina*));
	tablaDeSegmentos[nSegmento]->tablaDePaginas[nPagina] = paginaDelSegmento;
	tablaDeSegmentos[nSegmento]->tablaDePaginas[nPagina]->registro = registroDelSegmento;
}
int buscarRegistroDisponible(){
	for(int i = 0; i < cantidadDeRegistros; i++){
		if (memoriaPrincipal[i]->timestamp < 0) return i;
	}
	return -1;
}
//FALTA CODEAR
int ejecutarLRU(){
	return 0;
}

