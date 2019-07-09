/*
 * Memoria_nuevo.c
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

void cambiosConfigMemoria() {
	if (configModificado()) {
		t_config* archivoConfig = config_create(RUTA_CONFIG);
		configuracion->RETARDO_MEM = archivoConfigSacarIntDe(archivoConfig, "RETARDO_MEM");
		configuracion->RETARDO_FS = archivoConfigSacarIntDe(archivoConfig, "RETARDO_FS");
		configuracion->RETARDO_JOURNAL = archivoConfigSacarIntDe(archivoConfig, "RETARDO_JOURNAL");
		configuracion->RETARDO_GOSSIPING = archivoConfigSacarIntDe(archivoConfig, "RETARDO_GOSSIPING");
		archivoConfigDestruir(archivoConfig);
	}
}

int main()
{
	logger = log_create(logFile, "Memoria",true, LOG_LEVEL_INFO);
	configuracion = malloc(sizeof(ConfiguracionMemoria));
	for(int i=0;i<16;i++) configuracion->PUERTO_SEEDS[i]=0;
	socketLFS=0;
	configurar(configuracion);

	// cliente
	socketLFS = connectToServer(configuracion->IP_FS, configuracion->PUERTO_FS, logger);
	/*tPaquete* mensaje = malloc(sizeof(tPaquete));
	mensaje->type = HANDSHAKE;
	strcpy(mensaje->payload,"Memoria solicita tamanio del value");
	mensaje->length = sizeof(mensaje->payload);
	enviarPaquete(socketLFS, mensaje,logger,"Maximo tamanio del value");
	liberarPaquete(mensaje);
	char* tamanioValue;
	tMensaje* mensaje_handshake = NULL;
	recibirPaquete(socketLFS,mensaje_handshake,&tamanioValue,logger,"Maximo tamanio del value");
	maxValueSize = atoi(tamanioValue);*/
	levantarMemoria();
	sem_init(&mutexMemoria, 0, 1);

	//crearHiloIndependiente(&hiloJournal, (void*)journalization, NULL, "proceso Memoria");

	seed=0;
	while(configuracion->PUERTO_SEEDS[seed] != 0 && seed<16){
		socketSEED[seed] = connectToServer(configuracion->IP_SEEDS[seed], configuracion->PUERTO_SEEDS[seed], logger);
		seed++;
	}

	//servidor
	socketEscucha = crearSocketEscucha(configuracion->PUERTO, logger);
	fd_set setSocketsOrquestador;
	FD_ZERO(&setSocketsOrquestador);
	FD_SET(socketEscucha, &setSocketsOrquestador);
	maxSock = socketEscucha;
	tMensaje tipoMensaje;
	char * sPayload;

	crearHiloIndependiente(&hiloJournal,(void*)journalAutomatico, NULL, "proceso Memoria(Journal)");
	void* cierre;
	crearHilo(&hiloAPI,(void*)API_Memoria, NULL, "proceso Memoria");
	joinearHilo(hiloAPI, &cierre, "proceso Memoria");

	while ((int)cierre != 1) {

		puts("Escuchando");
		socketActivo = getConnection(&setSocketsOrquestador, &maxSock, socketEscucha, &tipoMensaje, &sPayload, logger);
		printf("Socket comunicacion: %d \n", socketActivo); //CORREGIR getConnection
		if (socketActivo != -1) {

			switch (tipoMensaje) {
			case SELECT:
				printf("\nRecibi %s\n", sPayload);
				//funcion SELECT
				ejecutarSelect(sPayload);
				break;
			case INSERT:
				printf("\nRecibi %s\n", sPayload);
				//funcion INSERT
				ejecutarInsert(sPayload);
				break;
			case CREATE:
				printf("\nRecibi %s\n", sPayload);
				//funcion CREATE
				ejecutarCreate(sPayload);
				break;
			case DESCRIBE:
				printf("\nRecibi %s\n", sPayload);
				//funcion DESCRIBE
				ejecutarDescribe(sPayload);
				break;
			case DROP:
				printf("\nRecibi %s\n", sPayload);
				//funcion DROP
				ejecutarDrop(sPayload);
				break;
			case JOURNAL:
				printf("\nRecibi %s\n", sPayload);
				//funcion JOURNAL
				ejecutarJournal(sPayload);
				break;
			case DESCONEXION:
				printf("\nSe desconecto un cliente\n");
				break;
			case HANDSHAKE:
				printf("\nKernel y Memoria realizan Handshake\n");
				tPaquete* mensaje = malloc(sizeof(tPaquete));
				mensaje->type = HANDSHAKE;
				strcpy(mensaje->payload, string_itoa(configuracion->MEMORY_NUMBER));
				mensaje->length = sizeof(mensaje->payload);
				enviarPaquete(socketActivo, mensaje, logger, "Enviar Número de Memoria a Kernel");
				liberarPaquete(mensaje);
				break;

			default:
				printf("Tipo de mensaje desconocido \n");
				break;

			}
		}
	}
	sem_destroy(&mutexMemoria);
	terminar(seed);
}

void terminar(int seed){
	free(configuracion);
	memoriaPrincipalDestruir();
	if(socketLFS!=0)desconectarseDe(socketLFS);
	seed=0;
	while(configuracion->PUERTO_SEEDS[seed] != 0 && seed<16){
		desconectarseDe(socketSEED[seed]);
		seed++;
	}
	close(socketEscucha);
	return;
}

void insertMemoria(char* tabla, uint16_t key, char* value, int timestamp){
	int segmento = 0, pagina = 0, registro = -1;
	if(cantidadDeSegmentos == 0){
		registro = 0;
		asignarRegistroANuevoSegmento(tabla, key, value, timestamp, segmento, registro);
		t_nodoLRU* nodo = (t_nodoLRU*) malloc(sizeof(t_nodoLRU));
		nodo->modificado=0;nodo->paginaID=pagina;nodo->segmentoID=segmento;
		insertarEnListaDePaginasLRU(listaPaginasLRU,nodo);
		/*
		Registro* reg = getRegistro(granMalloc+registro*tamanioRealDeUnRegistro);
		Registro* reg2 = getRegistro(tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->frame);
		printf("\nInserto: %s %d %s %d", tabla,reg->key,reg->value,reg->timestamp );
		printf("\nInserto2: %s %d %s %d", tablaDeSegmentos[segmento]->tabla,reg->key,reg->value,reg->timestamp );
		free(reg);
		free(reg2);
		*/
	}
	else{
		for(segmento=0; segmento<cantidadDeSegmentos; segmento++){
			if(strcmp(tablaDeSegmentos[segmento]->tabla, tabla) == 0) break;
		}
		if(segmento<cantidadDeSegmentos){
			for(pagina=0; pagina<tablaDeSegmentos[segmento]->cantidadDePaginas;pagina++ ){
				if(getKey(tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->frame) == key) break;
			}
			if(pagina<tablaDeSegmentos[segmento]->cantidadDePaginas){
				setValue(tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->frame, value);
				setTimestamp(tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->frame, timestamp);
				t_nodoLRU* nodo = (t_nodoLRU*) malloc(sizeof(t_nodoLRU));
				nodo->modificado=1;nodo->paginaID=pagina;nodo->segmentoID=segmento;
				insertarEnListaDePaginasLRU(listaPaginasLRU,nodo);
			}else{
				registro = buscarRegistroDisponible();
				if(registro == -1){
					printf("Memoria sin frames vacios\n");
					t_nodoLRU* nodo = (t_nodoLRU*) malloc(sizeof(t_nodoLRU));
					nodo->modificado=0;nodo->paginaID=pagina;nodo->segmentoID=segmento;
					t_nodoLRU* nodo_reemplazo = insertarEnListaDePaginasLRU(listaPaginasLRU,nodo);
					registro = buscarRegistro(nodo_reemplazo);
				}
				if(registro == -1){
				asignarRegistroASegmentoExistente(key, value, timestamp, segmento, pagina, registro);
				t_nodoLRU* nodo = (t_nodoLRU*) malloc(sizeof(t_nodoLRU));
				nodo->modificado=0;nodo->paginaID=pagina;nodo->segmentoID=segmento;
				insertarEnListaDePaginasLRU(listaPaginasLRU,nodo);
				/*
				Registro* reg = getRegistro(granMalloc+registro*tamanioRealDeUnRegistro);
				Registro* reg2 = getRegistro(tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->frame);
				printf("\nInserto: %s %d %s %d", tabla,reg->key,reg->value,reg->timestamp );
				printf("\nInserto2: %s %d %s %d", tablaDeSegmentos[segmento]->tabla,reg->key,reg->value,reg->timestamp );
				free(reg);
				free(reg2);
				*/
				}
			}
		}
		else{
			registro = buscarRegistroDisponible();
			if(registro == -1){
				printf("Memoria sin frames vacios\n");
				t_nodoLRU* nodo = (t_nodoLRU*) malloc(sizeof(t_nodoLRU));
				nodo->modificado=0;nodo->paginaID=pagina;nodo->segmentoID=segmento;
				t_nodoLRU* nodo_reemplazo = insertarEnListaDePaginasLRU(listaPaginasLRU,nodo);
				registro = buscarRegistro(nodo_reemplazo);
			}
			if(registro == -1){
			asignarRegistroANuevoSegmento(tabla, key, value, timestamp, segmento, registro);
			t_nodoLRU* nodo = (t_nodoLRU*) malloc(sizeof(t_nodoLRU));
			nodo->modificado=0;nodo->paginaID=pagina;nodo->segmentoID=segmento;
			insertarEnListaDePaginasLRU(listaPaginasLRU,nodo);
			/*
			Registro* reg = getRegistro(granMalloc+registro*tamanioRealDeUnRegistro);
			Registro* reg2 = getRegistro(tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->frame);
			printf("\nInserto: %s %d %s %d", tabla,reg->key,reg->value,reg->timestamp );
			printf("\nInserto2: %s %d %s %d", tablaDeSegmentos[segmento]->tabla,reg->key,reg->value,reg->timestamp );
			free(reg);
			free(reg2);
			*/
			}
		}
	}
}

Registro* selectMemoria(char* tabla, uint16_t key){
	int segmento = 0, pagina = 0;
	for(segmento=0; segmento<cantidadDeSegmentos; segmento++){
		printf("tabla: %s\n", tablaDeSegmentos[segmento]->tabla);
		if(strcmp(tablaDeSegmentos[segmento]->tabla, tabla) == 0){printf("encontre tabla\n"); break;}
	}
	if(segmento<cantidadDeSegmentos){
		for(pagina=0; pagina<tablaDeSegmentos[segmento]->cantidadDePaginas ; pagina++){
			if(getKey(tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->frame) == key) {printf("encontre pagina\n"); break;}
		}
		if(pagina<tablaDeSegmentos[segmento]->cantidadDePaginas){
			t_nodoLRU* nodo = (t_nodoLRU*) malloc(sizeof(t_nodoLRU));
			nodo->modificado=0;nodo->paginaID=pagina;nodo->segmentoID=segmento;
			insertarEnListaDePaginasLRU(listaPaginasLRU,nodo);
			return getRegistro(tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->frame);
		}
		else{
			char* select_mensaje = string_from_format("SELECT %s %d",tabla,key);
			tPaquete* mensaje = malloc(sizeof(tPaquete));
			mensaje->type = SELECT;
			strcpy(mensaje->payload,select_mensaje);
			mensaje->length = sizeof(mensaje->payload);
			enviarPaquete(socketLFS, mensaje,logger,"Ejecutar comando SELECT desde Memoria.");
			liberarPaquete(mensaje);
			free(select_mensaje);
			char* sPayload;
			tMensaje* tipo_mensaje = NULL;
			recibirPaquete(socketLFS,tipo_mensaje,&sPayload,logger,"Value del SELECT de LFS");
			char* value = (char*) malloc(maxValueSize);
			strcpy(value,sPayload);
			insertMemoria(tabla, key, value, getCurrentTime());
			free(value);
			Registro* registro = selectMemoria(tabla, key);
			return registro;
		}
	}
	else{
		perror("No se encontro el segmento que contiene la tabla solicitada.");
		return NULL;
	}
}
void levantarMemoria(){
#include <malloc.h>

	tablaDeSegmentos = malloc(1);
	maxValueSize = 20; //ESTO TIENE QUE VENIR DE LFS
	tamanioRealDeUnRegistro = sizeof(int) + sizeof(uint16_t) + maxValueSize ;
	cantidadDeRegistros = configuracion->TAM_MEM / tamanioRealDeUnRegistro;
	granMalloc = malloc(cantidadDeRegistros*tamanioRealDeUnRegistro);
	for(int i=0; i <cantidadDeRegistros; i++)
			setTimestamp(granMalloc+i*tamanioRealDeUnRegistro,-1);
	printf("tamanio granMalloc: %d\n", malloc_usable_size(granMalloc));
}

uint16_t getKey(void* registro){
	uint16_t key = *(uint16_t*)registro;
	return key;
}
int getTimestamp(void* registro){
	int timestamp = *(int*)(registro+sizeof(uint16_t));
	return timestamp;
}
Registro* getRegistro(void* registro){
	Registro* reg = (Registro*) malloc(sizeof(Registro));
	reg->value = (char*) malloc(maxValueSize);
	reg->key = *(uint16_t*)registro;
	reg->timestamp = *(int*)(registro+sizeof(uint16_t));
	strcpy(reg->value,(char*)(registro+sizeof(uint16_t)+sizeof(int)));
	return reg;
}
void setValue(void* registro, char* value){
	memcpy(registro+sizeof(uint16_t)+sizeof(int),value,maxValueSize);
}
void setTimestamp(void* registro, int timestamp){
	memcpy(registro+sizeof(uint16_t),&timestamp,sizeof(int));
}
void setRegistro(void* registro, uint16_t key, int timestamp, char* value){
	memcpy(registro,&key,sizeof(uint16_t));
	memcpy(registro+sizeof(uint16_t),&timestamp,sizeof(int));
	memcpy(registro+sizeof(uint16_t)+sizeof(int),value,maxValueSize);
}
Segmento* segmentoCrear(char* tabla, Pagina** tablaDePaginas){
	Segmento* segmento = (Segmento*)malloc(sizeof(Segmento));
	strcpy(segmento->tabla, tabla);
	segmento->tablaDePaginas = tablaDePaginas;
	return segmento;
}
void segmentoDestruir(Segmento* segmento){
	for(int i=0; i < segmento->cantidadDePaginas; i++)
		free(segmento->tablaDePaginas[i]);
	free(segmento->tablaDePaginas);
	free(segmento);
}
Pagina* paginaCrear(int modificado, int nRegistro){
	Pagina* pagina = (Pagina*)malloc(sizeof(Pagina));
	pagina->modificado = modificado;
	pagina->frame = granMalloc + nRegistro*tamanioRealDeUnRegistro;
	return pagina;
}
void paginaDestruir(Pagina* pagina){
	free(pagina);
}
void registroDestruir(Registro* registro){
	free(registro->value);
	free(registro);
}
void memoriaPrincipalDestruir(){
	for(int j=0; j < cantidadDeSegmentos; j++)
		segmentoDestruir(tablaDeSegmentos[j]);
	free(granMalloc);
}
void asignarRegistroANuevoSegmento(char* tabla, uint16_t key, char* value, int timestamp, int nSegmento, int nRegistro){
	cantidadDeSegmentos++;
	tablaDeSegmentos = (Segmento**) realloc(tablaDeSegmentos, cantidadDeSegmentos*sizeof(Segmento*));
	Pagina* paginaDelSegmento = paginaCrear(0, nRegistro);
	Pagina** tablaDePaginasDelSegmento = (Pagina**) malloc(sizeof(Pagina*));
	Segmento* segmento = segmentoCrear(tabla, tablaDePaginasDelSegmento);
	tablaDeSegmentos[nSegmento] = segmento;
	tablaDeSegmentos[nSegmento]->cantidadDePaginas = 1;
	strcpy(tablaDeSegmentos[nSegmento]->tabla, tabla);
	tablaDeSegmentos[nSegmento]->tablaDePaginas = tablaDePaginasDelSegmento;
	tablaDeSegmentos[nSegmento]->tablaDePaginas[tablaDeSegmentos[nSegmento]->cantidadDePaginas - 1] = paginaDelSegmento;
	setRegistro(granMalloc+nRegistro*tamanioRealDeUnRegistro,key,timestamp,value);
}
void asignarRegistroASegmentoExistente(uint16_t key, char* value, int timestamp, int nSegmento, int nPagina, int nRegistro){
	Pagina* paginaDelSegmento = paginaCrear(0, nRegistro);
	tablaDeSegmentos[nSegmento]->cantidadDePaginas ++;
	tablaDeSegmentos[nSegmento]->tablaDePaginas = (Pagina**) realloc(tablaDeSegmentos[nSegmento]->tablaDePaginas, tablaDeSegmentos[nSegmento]->cantidadDePaginas*sizeof(Pagina*));
	tablaDeSegmentos[nSegmento]->tablaDePaginas[nPagina] = paginaDelSegmento;
	tablaDeSegmentos[nSegmento]->tablaDePaginas[nPagina]->frame = granMalloc + nRegistro*tamanioRealDeUnRegistro;
	setRegistro(granMalloc+nRegistro*tamanioRealDeUnRegistro,key,timestamp,value);
}
int buscarRegistroDisponible(){
	for(int i = 0; i < cantidadDeRegistros; i++){
		if (getTimestamp(granMalloc+i*tamanioRealDeUnRegistro) < 0) return i;
	}
	return -1;
}
int buscarRegistro(t_nodoLRU* nodo_reemplazo){
	uint16_t key = getKey(tablaDeSegmentos[nodo_reemplazo->segmentoID]->tablaDePaginas[nodo_reemplazo->paginaID]->frame);
	for(int i = 0; i < cantidadDeRegistros; i++){
		if (getKey(granMalloc+i*tamanioRealDeUnRegistro) == key) return i;
	}
	return -1;
}

void journalMemoria (){
	int j=0;
	char *instruccion;
	Registro *registro = malloc(sizeof(Registro));
	//for revisando todas las páginas
	sem_wait(&mutexMemoria);
	if(cantidadDeSegmentos != 0){
		while(tablaDeSegmentos[j]){
			for(int i=0; i<tablaDeSegmentos[j]->cantidadDePaginas; i++){
				if(tablaDeSegmentos[j]->tablaDePaginas[i]->modificado == 1){
					instruccion = malloc(15+sizeof(tablaDeSegmentos[j]->tabla)+sizeof(registro->key)+sizeof(registro->value)+sizeof(registro->timestamp));
					registro = getRegistro(tablaDeSegmentos[j]->tablaDePaginas[i]->frame);
					sprintf (instruccion, "INSERT %s %hd %s %d \n", tablaDeSegmentos[j]->tabla, registro->key, registro->value, registro->timestamp);
					ejecutarInsertJournal(instruccion);
					free(instruccion);
				}
			}
			segmentoDestruir(tablaDeSegmentos[j]);
			j++;
		}
	}
	sem_post(&mutexMemoria);
	free(registro);
}

void ejecutarInsertJournal(char *instruccion){
	tPaquete* mensaje = malloc(sizeof(tPaquete));
	mensaje->type = INSERT;
	strcpy(mensaje->payload,instruccion);
	mensaje->length = sizeof(mensaje->payload);
	enviarPaquete(socketLFS, mensaje,logger,"Ejecutar comando INSERT desde Memoria.");
	liberarPaquete(mensaje);
}

void* journalAutomatico (){
    clock_t start, diff;
    int elapsedsec;
    while (1) {
    	start = clock();
    	while (1) {
    		diff = clock() - start;
    		elapsedsec = diff / CLOCKS_PER_SEC;
    		if (elapsedsec >= (configuracion->RETARDO_JOURNAL / 1000)){
    			journalMemoria();
    			break;
    		}
        }
    }
}
