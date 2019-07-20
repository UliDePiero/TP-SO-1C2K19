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
					   "MEMORY_NUMBER",
					   "IP_PROPIA"
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
	strcpy(configuracion->IP_PROPIA, archivoConfigSacarStringDe(archivoConfig, "IP_PROPIA"));

	archivoConfigDestruir(archivoConfig);
}

void cambiosConfigMemoria() {
	while (1) {
		if (configModificado()) {
			char* campos[] = { "PUERTO", "IP_FS", "PUERTO_FS", "IP_SEEDS",
					"PUERTO_SEEDS", "RETARDO_MEM", "RETARDO_FS", "TAM_MEM",
					"RETARDO_JOURNAL", "RETARDO_GOSSIPING", "MEMORY_NUMBER",
					"IP_PROPIA" };
			t_config* archivoConfig = archivoConfigCrear(RUTA_CONFIG, campos);
			int nuevoRetardoMem = archivoConfigSacarIntDe(archivoConfig,
					"RETARDO_MEM");
			int nuevoRetardoFS = archivoConfigSacarIntDe(archivoConfig,
					"RETARDO_FS");
			int nuevoRetardoJournal = archivoConfigSacarIntDe(archivoConfig,
					"RETARDO_JOURNAL");
			int nuevoRetardoGossiping = archivoConfigSacarIntDe(archivoConfig,
					"RETARDO_GOSSIPING");

			if (configuracion->RETARDO_MEM != nuevoRetardoMem) {
				sem_wait(&loggerSemaforo);
				log_info(logger,
						"Cambio de Retardo de acceso a Memoria Principal %d -> %d",
						configuracion->RETARDO_MEM, nuevoRetardoMem);
				sem_post(&loggerSemaforo);
				configuracion->RETARDO_MEM = nuevoRetardoMem;
			}
			if (configuracion->RETARDO_FS != nuevoRetardoFS) {
				sem_wait(&loggerSemaforo);
				log_info(logger,
						"Cambio de Retardo de acceso a File System %d -> %d",
						configuracion->RETARDO_FS, nuevoRetardoFS);
				sem_post(&loggerSemaforo);
				configuracion->RETARDO_FS = nuevoRetardoFS;
			}
			if (configuracion->RETARDO_JOURNAL != nuevoRetardoJournal) {
				sem_wait(&loggerSemaforo);
				log_info(logger, "Cambio de Tiempo de Journal %d -> %d",
						configuracion->RETARDO_JOURNAL, nuevoRetardoJournal);
				sem_post(&loggerSemaforo);
				configuracion->RETARDO_JOURNAL = nuevoRetardoJournal;
			}
			if (configuracion->RETARDO_GOSSIPING != nuevoRetardoGossiping) {
				sem_wait(&loggerSemaforo);
				log_info(logger, "Cambio de Tiempo de Gossiping %d -> %d",
						configuracion->RETARDO_GOSSIPING,
						nuevoRetardoGossiping);
				sem_post(&loggerSemaforo);
				configuracion->RETARDO_GOSSIPING = nuevoRetardoGossiping;
			}
			archivoConfigDestruir(archivoConfig);
		}
	}
}

int main()
{
	logger = log_create(logFile, "Memoria",true, LOG_LEVEL_TRACE);
	configuracion = malloc(sizeof(ConfiguracionMemoria));
	for (int i = 0; i < CANT_MAX_SEEDS; i++) {
		configuracion->PUERTO_SEEDS[i] = 0;
		socketSEED[i] = 0;
	}
	configurar(configuracion);

	// cliente
	socketLFS = connectToServer(configuracion->IP_FS, configuracion->PUERTO_FS, logger);

	if(socketLFS == 1){
		log_error(logger, "LFS no conectado");
		log_debug(logger, "Modulo Memoria cerrada");
		log_destroy(logger);
		free(configuracion);
		exit(EXIT_FAILURE);
	}

	tPaquete* mensajeHAND = malloc(sizeof(tPaquete));
	mensajeHAND->type = HANDSHAKE;
	strcpy(mensajeHAND->payload,"Memoria solicita tamanio del value");
	mensajeHAND->length = sizeof(mensajeHAND->payload);
	enviarPaquete(socketLFS, mensajeHAND,logger,"Maximo tamanio del value");
	liberarPaquete(mensajeHAND);
	char* tamanioValue;
	tMensaje mensaje_handshake;
	recibirPaquete(socketLFS,&mensaje_handshake,&tamanioValue,logger,"Maximo tamanio del value");
	maxValueSize = atoi(tamanioValue);
	sem_init(&mutexMemoria, 0, 1);
	sem_init(&loggerSemaforo, 1, 1);
	levantarMemoria();
	listaPaginasLRU = list_create();

	crearHiloIndependiente(&hiloGossipMemoria,(void*)gossipingMemoria, NULL, "proceso Memoria(Gossip)");
	crearHiloIndependiente(&hiloConfig,(void*)cambiosConfigMemoria, NULL, "proceso Memoria(Config)");
	crearHiloIndependiente(&hiloJournal,(void*)journalAutomatico, NULL, "proceso Memoria(Journal)");

	seed=0;
	while (configuracion->PUERTO_SEEDS[seed] != 0 && seed < CANT_MAX_SEEDS) {
		socketSEED[seed] = connectToServer(configuracion->IP_SEEDS[seed], configuracion->PUERTO_SEEDS[seed], logger);
		seed++;
	}

	//servidor
	socketEscucha = crearSocketEscucha(configuracion->PUERTO, logger);
	fd_set setSocketsOrquestador;
	FD_ZERO(&setSocketsOrquestador);
	FD_SET(socketEscucha, &setSocketsOrquestador);
	maxSock = socketEscucha;
	//Recepcion de mensajes
	tMensaje tipoMensaje;
	char * sPayload;
	//Envio de mensajes
	tPaquete* mensaje;
	char* retorno;
	int retornoINT;
	t_list* retornoLista;
	crearHiloIndependiente(&hiloAPI,(void*)API_Memoria, NULL, "proceso Memoria(API)");

	while (1) {
		//puts("Escuchando");
		socketActivo = getConnection(&setSocketsOrquestador, &maxSock, socketEscucha, &tipoMensaje, &sPayload, logger);
		if (socketActivo != -1) {
			sem_wait(&loggerSemaforo);
			log_info(logger, "Comunicacion establecida en el socket %d", socketActivo);
			sem_post(&loggerSemaforo);
			switch (tipoMensaje) {
			case SELECT:
				sem_wait(&loggerSemaforo);
				log_info(logger, "Memoria recibió '%s'", sPayload);
				sem_post(&loggerSemaforo);
				//funcion SELECT
				retorno = ejecutarSelect(sPayload);
				mensaje = malloc(sizeof(tPaquete));
				if(retorno != NULL)
				{
					mensaje->type = SELECT;
					strcpy(mensaje->payload,retorno);
					free(retorno);
				}
				else
				{
					mensaje->type = ERROR_EN_COMANDO;
					strcpy(mensaje->payload,"SELECT");
				}
				mensaje->length = sizeof(mensaje->payload);
				enviarPaquete(socketActivo, mensaje,logger,"Value del SELECT de MEMORIA.");
				liberarPaquete(mensaje);
				sem_wait(&loggerSemaforo);
				log_info(logger, "Resultado de SELECT enviado a Kernel");
				sem_post(&loggerSemaforo);
				break;
			case INSERT:
				sem_wait(&loggerSemaforo);
				log_info(logger, "Memoria recibió '%s'", sPayload);
				sem_post(&loggerSemaforo);
				//funcion INSERT
				retorno = ejecutarInsert(sPayload);
				mensaje = malloc(sizeof(tPaquete));
				if(retorno != NULL)
				{
					mensaje->type = INSERT;
					strcpy(mensaje->payload,retorno);
					free(retorno);
				}
				else
				{
					mensaje->type = ERROR_EN_COMANDO;
					strcpy(mensaje->payload,"INSERT");
				}
				mensaje->length = sizeof(mensaje->payload);
				enviarPaquete(socketActivo, mensaje,logger,"Ejecucucion del INSERT en MEMORIA.");
				liberarPaquete(mensaje);
				sem_wait(&loggerSemaforo);
				log_info(logger, "Resultado de INSERT enviado a Kernel");
				sem_post(&loggerSemaforo);
				break;
			case CREATE:
				sem_wait(&loggerSemaforo);
				log_info(logger, "Memoria recibió '%s'", sPayload);
				sem_post(&loggerSemaforo);
				//funcion CREATE
				retorno = ejecutarCreate(sPayload);
				mensaje = malloc(sizeof(tPaquete));
				if(retorno != NULL)
				{
					mensaje->type = CREATE;
					strcpy(mensaje->payload,retorno);
					free(retorno);
				}
				else
				{
					mensaje->type = ERROR_EN_COMANDO;
					strcpy(mensaje->payload,"CREATE");
				}
				mensaje->length = sizeof(mensaje->payload);
				enviarPaquete(socketActivo, mensaje,logger,"Ejecucion del CREATE en MEMORIA.");
				liberarPaquete(mensaje);
				sem_wait(&loggerSemaforo);
				log_info(logger, "Resultado de CREATE enviado a Kernel");
				sem_post(&loggerSemaforo);
				break;
			case DESCRIBE:
				sem_wait(&loggerSemaforo);
				log_info(logger, "Memoria recibió '%s'", sPayload);
				sem_post(&loggerSemaforo);
				//funcion DESCRIBE
				retornoLista = ejecutarDescribe(sPayload);
				mensaje = malloc(sizeof(tPaquete));
				if(retornoLista != NULL)
				{
					mensaje->type = DESCRIBE;
					char* ret = string_new();
					t_link_element* nodo = retornoLista->head;
					while (nodo) {
						string_append(&ret,(char*)nodo->data);
						nodo = nodo->next;
					}
					strcpy(mensaje->payload,ret);
					free(ret);
					free(nodo);
					list_destroy(retornoLista);
				}
				else
				{
					mensaje->type = ERROR_EN_COMANDO;
					strcpy(mensaje->payload,"DESCRIBE");
				}
				mensaje->length = sizeof(mensaje->payload);
				enviarPaquete(socketActivo, mensaje,logger,"Ejecucion del DESCRIBE en MEMORIA.");
				liberarPaquete(mensaje);
				sem_wait(&loggerSemaforo);
				log_info(logger, "Resultado de DESCRIBE enviado a Kernel");
				sem_post(&loggerSemaforo);
				break;
			case DROP:
				sem_wait(&loggerSemaforo);
				log_info(logger, "Memoria recibió '%s'", sPayload);
				sem_post(&loggerSemaforo);
				//funcion DROP
				retorno = ejecutarDrop(sPayload);
				mensaje = malloc(sizeof(tPaquete));
				if(retorno != NULL)
				{
					mensaje->type = DROP;
					strcpy(mensaje->payload,retorno);
					free(retorno);
				}
				else
				{
					mensaje->type = ERROR_EN_COMANDO;
					strcpy(mensaje->payload,"DROP");
				}
				mensaje->length = sizeof(mensaje->payload);
				enviarPaquete(socketActivo, mensaje,logger,"Ejecucion del DROP en MEMORIA.");
				liberarPaquete(mensaje);
				sem_wait(&loggerSemaforo);
				log_info(logger, "Resultado de DROP enviado a Kernel");
				sem_post(&loggerSemaforo);
				break;
			case JOURNAL:
				sem_wait(&loggerSemaforo);
				log_info(logger, "Memoria recibió '%s'", sPayload);
				sem_post(&loggerSemaforo);
				//funcion JOURNAL
				retornoINT = ejecutarJournal(sPayload);
				mensaje = malloc(sizeof(tPaquete));
				if(retornoINT != 0)
				{
					mensaje->type = JOURNAL;
					itoa(retornoINT,mensaje->payload,10);
				}
				else
				{
					mensaje->type = ERROR_EN_COMANDO;
					strcpy(mensaje->payload,"JOURNAL");
				}
				mensaje->length = sizeof(mensaje->payload);
				enviarPaquete(socketActivo, mensaje,logger,"Ejecucion del JOURNAL en MEMORIA.");
				liberarPaquete(mensaje);
				sem_wait(&loggerSemaforo);
				log_info(logger, "Resultado de JOURNAL enviado a Kernel");
				sem_post(&loggerSemaforo);
				break;
			case ERROR_EN_COMANDO:
				sem_wait(&loggerSemaforo);
				log_error(logger, "Hubo un error en la ejecucion del comando %s en LFS", sPayload);
				sem_post(&loggerSemaforo);
				break;
			case DESCONEXION:
				sem_wait(&loggerSemaforo);
				log_info(logger, "Se desconecto un cliente");
				sem_post(&loggerSemaforo);
				break;
			case HANDSHAKE:
				sem_wait(&loggerSemaforo);
				log_info(logger, "Kernel y Memoria realizan Handshake");
				sem_post(&loggerSemaforo);
				mensaje = malloc(sizeof(tPaquete));
				mensaje->type = HANDSHAKE;
				strcpy(mensaje->payload, string_itoa(configuracion->RETARDO_GOSSIPING));
				mensaje->length = sizeof(mensaje->payload);
				enviarPaquete(socketActivo, mensaje, logger, "Enviar Retardo de Gossiping a Kernel");
				liberarPaquete(mensaje);
				break;
			case GOSSIPING:
				sem_wait(&loggerSemaforo);
				log_info(logger, "Pedido de Lista de Gossiping a Memoria");
				sem_post(&loggerSemaforo);
				enviarListaGossiping(socketActivo);
				break;
			case GOSSIPING_RECIBE:
				sem_wait(&loggerSemaforo);
				log_info(logger, "Envío de Lista de Gossiping a Memoria");
				sem_post(&loggerSemaforo);
				recibeLista(socketActivo);
				break;
			default:
				//printf("Tipo de mensaje desconocido \n");
				sem_wait(&loggerSemaforo);
				log_error(logger, "El tipo de mensaje ingresado es desconocido", sPayload);
				sem_post(&loggerSemaforo);
				break;

			}
		}
	}
}

void terminar(){
	memoriaPrincipalDestruir();
	list_destroy(listaPaginasLRU);
	if(socketLFS!=1)desconectarseDe(socketLFS);
	int s=0;
	while(configuracion->PUERTO_SEEDS[s] != 0 && s<seed){
		desconectarseDe(socketSEED[s]);
		s++;
	}
	pthread_cancel(hiloJournal);
	pthread_cancel(hiloGossipMemoria);
	pthread_cancel(hiloConfig);
	sem_wait(&loggerSemaforo);
	log_debug(logger, "Modulo Memoria cerrada");
	sem_post(&loggerSemaforo);
	log_destroy(logger);
	sem_destroy(&loggerSemaforo);
	sem_destroy(&mutexMemoria);
	free(configuracion);
	close(socketEscucha);
	exit(0);
}

void insertMemoria(char* tabla, uint16_t key, char* value, uint64_t timestamp, int flagModificado){
	int segmento = 0, pagina = 0, registro = -1;
	if(cantidadDeSegmentos == 0){
		registro = 0;
		asignarRegistroANuevoSegmento(tabla, key, value, timestamp, segmento, registro, flagModificado);
		t_nodoLRU* nodo = (t_nodoLRU*) malloc(sizeof(t_nodoLRU));
		nodo->modificado=flagModificado;nodo->paginaID=pagina;nodo->segmentoID=segmento;
		insertarEnListaDePaginasLRU(listaPaginasLRU,nodo);
		sem_wait(&loggerSemaforo);
		log_info(logger, "'INSERT %s %hd %s %" PRIu64 "' ejecutado exitosamente", tabla, key, value, timestamp);
		sem_post(&loggerSemaforo);
/*
		Registro* reg = getRegistro(granMalloc+registro*tamanioRealDeUnRegistro);
		Registro* reg2 = getRegistro(tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->frame);
		printf("\nInserto: %s %d %s %llu", tabla,reg->key,reg->value,reg->timestamp );
		printf("\nInserto2: %s %d %s %llu", tablaDeSegmentos[segmento]->tabla,reg->key,reg->value,reg->timestamp );
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
				tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->modificado = flagModificado;
				t_nodoLRU* nodo = (t_nodoLRU*) malloc(sizeof(t_nodoLRU));
				nodo->modificado=flagModificado;nodo->paginaID=pagina;nodo->segmentoID=segmento;
				insertarEnListaDePaginasLRU(listaPaginasLRU,nodo);
				sem_wait(&loggerSemaforo);
				log_info(logger, "'INSERT %s %hd %s %" PRIu64 "' ejecutado exitosamente", tabla, key, value, timestamp);
				sem_post(&loggerSemaforo);
			}else{
				registro = buscarRegistroDisponible();
				if(registro == -1){
					//printf("Memoria sin frames vacios\n");
					t_nodoLRU* nodo = (t_nodoLRU*) malloc(sizeof(t_nodoLRU));
					nodo->modificado=flagModificado;nodo->paginaID=pagina;nodo->segmentoID=segmento;
					t_nodoLRU* nodo_reemplazo = insertarEnListaDePaginasLRU(listaPaginasLRU,nodo);
					if(nodo_reemplazo->segmentoID != -1)
						registro = buscarRegistro(nodo_reemplazo);
					else
						registro = -2;
				}
				if(registro != -1 && registro != -2){
					asignarRegistroASegmentoExistente(key, value, timestamp, segmento, pagina, registro, flagModificado);
					t_nodoLRU* nodo = (t_nodoLRU*) malloc(sizeof(t_nodoLRU));
					nodo->modificado=flagModificado;nodo->paginaID=pagina;nodo->segmentoID=segmento;
					insertarEnListaDePaginasLRU(listaPaginasLRU,nodo);
					sem_wait(&loggerSemaforo);
					log_info(logger, "'INSERT %s %hd %s %" PRIu64 "' ejecutado exitosamente", tabla, key, value, timestamp);
					sem_post(&loggerSemaforo);
/*
					Registro* reg = getRegistro(granMalloc+registro*tamanioRealDeUnRegistro);
					Registro* reg2 = getRegistro(tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->frame);
					printf("\nInserto: %s %d %s %llu", tabla,reg->key,reg->value,reg->timestamp );
					printf("\nInserto2: %s %d %s %llu", tablaDeSegmentos[segmento]->tabla,reg->key,reg->value,reg->timestamp );
					free(reg);
					free(reg2);
*/
				}
				else if(registro == -2)
					insertMemoria(tabla, key,value, timestamp, flagModificado);
			}
		}
		else{
			registro = buscarRegistroDisponible();
			if(registro == -1){
				//printf("Memoria sin frames vacios\n");
				t_nodoLRU* nodo = (t_nodoLRU*) malloc(sizeof(t_nodoLRU));
				nodo->modificado=flagModificado;nodo->paginaID=pagina;nodo->segmentoID=segmento;
				t_nodoLRU* nodo_reemplazo = insertarEnListaDePaginasLRU(listaPaginasLRU,nodo);
				if(nodo_reemplazo->segmentoID != -1)
					registro = buscarRegistro(nodo_reemplazo);
				else
					registro = -2;
			}
			if(registro != -1 && registro != -2){
				asignarRegistroANuevoSegmento(tabla, key, value, timestamp, segmento, registro, flagModificado);
				t_nodoLRU* nodo = (t_nodoLRU*) malloc(sizeof(t_nodoLRU));
				nodo->modificado=flagModificado;nodo->paginaID=pagina;nodo->segmentoID=segmento;
				insertarEnListaDePaginasLRU(listaPaginasLRU,nodo);
				sem_wait(&loggerSemaforo);
				log_info(logger, "'INSERT %s %hd %s %" PRIu64 "' ejecutado exitosamente", tabla, key, value, timestamp);
				sem_post(&loggerSemaforo);
/*
				Registro* reg = getRegistro(granMalloc+registro*tamanioRealDeUnRegistro);
				Registro* reg2 = getRegistro(tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->frame);
				printf("\nInserto: %s %d %s %llu", tabla,reg->key,reg->value,reg->timestamp );
				printf("\nInserto2: %s %d %s %llu", tablaDeSegmentos[segmento]->tabla,reg->key,reg->value,reg->timestamp );
				free(reg);
				free(reg2);
*/
			}
			else if(registro == -2)
				insertMemoria(tabla, key,value, timestamp, flagModificado);
		}
	}
}

Registro* selectMemoria(char* tabla, uint16_t key){
	int segmento = 0, pagina = 0;
	for(segmento=0; segmento<cantidadDeSegmentos; segmento++){
		//printf("tabla: %s\n", tablaDeSegmentos[segmento]->tabla);
		if(strcmp(tablaDeSegmentos[segmento]->tabla, tabla) == 0){ break;}
	}
	if(segmento<cantidadDeSegmentos){
		for(pagina=0; pagina<tablaDeSegmentos[segmento]->cantidadDePaginas; pagina++){
			if(getKey(tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->frame) == key) { break;}
		}
		if(pagina<tablaDeSegmentos[segmento]->cantidadDePaginas){
			t_nodoLRU* nodo = (t_nodoLRU*) malloc(sizeof(t_nodoLRU));
			nodo->modificado=tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->modificado;nodo->paginaID=pagina;nodo->segmentoID=segmento;
			insertarEnListaDePaginasLRU(listaPaginasLRU,nodo);
			sem_wait(&loggerSemaforo);
			log_info(logger, "'SELECT %s %hd' ejecutado exitosamente", tabla, key);
			sem_post(&loggerSemaforo);
			return getRegistro(tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->frame);
		}
	}
	if(socketLFS!=1){
		char* select_mensaje = string_from_format("SELECT %s %d",tabla,key);
		tPaquete* mensaje = malloc(sizeof(tPaquete));
		mensaje->type = SELECT;
		strcpy(mensaje->payload,select_mensaje);
		mensaje->length = sizeof(mensaje->payload);
		enviarPaquete(socketLFS, mensaje,logger,"Ejecutar comando SELECT desde Memoria.");
		liberarPaquete(mensaje);
		free(select_mensaje);
		char* sPayload;
		tMensaje tipo_mensaje;
		recibirPaquete(socketLFS,&tipo_mensaje,&sPayload,logger,"Value del SELECT de LFS");
		if(tipo_mensaje == ERROR_EN_COMANDO)
		{
			free(sPayload);
			sem_wait(&loggerSemaforo);
			log_error(logger, "No se pudo encontrar la key %d o la tabla %s en LFS", key, tabla);
			sem_post(&loggerSemaforo);
			return NULL;
		}
		else
		{
			insertMemoria(tabla, key, sPayload, getCurrentTime(), 0);
			free(sPayload);
			Registro* registro = selectMemoria(tabla, key);
			return registro;
		}
	}else{
		sem_wait(&loggerSemaforo);
		log_error(logger, "No se pudo encontrar la key %d: LFS no conectado", key);
		sem_post(&loggerSemaforo);
		return NULL;
	}
}

/*//VIEJO
Registro* selectMemoria(char* tabla, uint16_t key){
	int segmento = 0, pagina = 0;
	for(segmento=0; segmento<cantidadDeSegmentos; segmento++){
		//printf("tabla: %s\n", tablaDeSegmentos[segmento]->tabla);
		if(strcmp(tablaDeSegmentos[segmento]->tabla, tabla) == 0){ break;}
	}
	if(segmento<cantidadDeSegmentos){
		for(pagina=0; pagina<tablaDeSegmentos[segmento]->cantidadDePaginas; pagina++){
			if(getKey(tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->frame) == key) { break;}
		}
		if(pagina<tablaDeSegmentos[segmento]->cantidadDePaginas){
			t_nodoLRU* nodo = (t_nodoLRU*) malloc(sizeof(t_nodoLRU));
			nodo->modificado=0;nodo->paginaID=pagina;nodo->segmentoID=segmento;
			insertarEnListaDePaginasLRU(listaPaginasLRU,nodo);
			sem_wait(&loggerSemaforo);
			log_info(logger, "'SELECT %s %hd' ejecutado exitosamente", tabla, key);
			sem_post(&loggerSemaforo);
			return getRegistro(tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->frame);
		}
		else{
			if(socketLFS!=1){
				char* select_mensaje = string_from_format("SELECT %s %d",tabla,key);
				tPaquete* mensaje = malloc(sizeof(tPaquete));
				mensaje->type = SELECT;
				strcpy(mensaje->payload,select_mensaje);
				mensaje->length = sizeof(mensaje->payload);
				enviarPaquete(socketLFS, mensaje,logger,"Ejecutar comando SELECT desde Memoria.");
				liberarPaquete(mensaje);
				free(select_mensaje);
				char* sPayload;
				tMensaje tipo_mensaje;
				recibirPaquete(socketLFS,&tipo_mensaje,&sPayload,logger,"Value del SELECT de LFS");
				insertMemoria(tabla, key, sPayload, getCurrentTime());
				free(sPayload);
				Registro* registro = selectMemoria(tabla, key);
				return registro;
			}else{
				sem_wait(&loggerSemaforo);
				log_error(logger, "No se pudo encontrar la key %d: LFS no conectado", key);
				sem_post(&loggerSemaforo);
				return NULL;
			}
		}
	}
	else{
		sem_wait(&loggerSemaforo);
		log_error(logger, "No se encontro el segmento que contiene la tabla %s", tabla);
		sem_post(&loggerSemaforo);
		return NULL;
	}
}
*/
void dropMemoria(char* tabla){
	int segmento = 0, pagina = 0;
	void *pKey;
	t_nodoLRU* nodo;
	for(segmento=0; segmento<cantidadDeSegmentos; segmento++){
		if(strcmp(tablaDeSegmentos[segmento]->tabla, tabla) == 0)
		{
			//printf("encontre tabla\n");
			t_list* listaPunteros = list_create();
			for(pagina=0; pagina<tablaDeSegmentos[segmento]->cantidadDePaginas; pagina++){
				pKey = tablaDeSegmentos[segmento]->tablaDePaginas[pagina]->frame;
				list_add(listaPunteros,pKey);
				nodo = (t_nodoLRU*) malloc(sizeof(t_nodoLRU));
				nodo->modificado=-1;nodo->paginaID=pagina;nodo->segmentoID=segmento;
				removerElemento(listaPaginasLRU,nodo);
			}
			list_iterate(listaPunteros,(void*)resetearMemoria);
			free(listaPunteros);
			cantidadDeSegmentos--;
			if(cantidadDeSegmentos == 0)
				tablaDeSegmentos = (Segmento**) realloc(tablaDeSegmentos, 1);
			else
				tablaDeSegmentos = (Segmento**) realloc(tablaDeSegmentos, cantidadDeSegmentos*sizeof(Segmento*));
			if(segmento!=cantidadDeSegmentos){
				segmentoDestruir(tablaDeSegmentos[segmento]);
				tablaDeSegmentos[segmento] = tablaDeSegmentos[cantidadDeSegmentos];
			}
			else
				segmentoDestruir(tablaDeSegmentos[segmento]);
			break;
		}
	}
	if(socketLFS!=1){
		tPaquete* mensaje = malloc(sizeof(tPaquete));
		mensaje->type = DROP;
		char* comando = string_from_format("DROP %s", tabla);
		strcpy(mensaje->payload,comando);
		mensaje->length = sizeof(mensaje->payload);
		enviarPaquete(socketLFS, mensaje,logger,"Ejecutar comando DROP desde Memoria.");
		liberarPaquete(mensaje);
		sem_wait(&loggerSemaforo);
		log_info(logger, "'DROP %s' ejecutado exitosamente", tabla);
		sem_post(&loggerSemaforo);
		free(comando);
	}
}

char* describeMemoriaTabla(char* tabla){
	char* retorno;
	if(socketLFS!=1){
		tPaquete* mensaje = malloc(sizeof(tPaquete));
		mensaje->type = DESCRIBE;
		char* comando = string_from_format("DESCRIBE %s", tabla);
		strcpy(mensaje->payload,comando);
		mensaje->length = sizeof(mensaje->payload);
		enviarPaquete(socketLFS, mensaje,logger,"Ejecutar comando DESCRIBE_TABLA desde Memoria.");
		sem_wait(&loggerSemaforo);
		log_info(logger, "'DESCRIBE %s' enviado a LFS", tabla);
		sem_post(&loggerSemaforo);
		liberarPaquete(mensaje);
		free(comando);
		char* sPayload;
		tMensaje tipo_mensaje;
		recibirPaquete(socketLFS,&tipo_mensaje,&sPayload,logger,"Cantidad de tablas");
		recibirPaquete(socketLFS,&tipo_mensaje,&sPayload,logger,"Metadata de la tabla desde LFS");
		sem_wait(&loggerSemaforo);
		log_info(logger, "'DESCRIBE %s' ejecutado exitosamente", tabla);
		sem_post(&loggerSemaforo);
		//printf("\n%s",sPayload);
		retorno = string_duplicate(sPayload);
		free(sPayload);
	}
	return retorno;
}

t_list* describeMemoria(){
	t_list* retorno = list_create();
	if(socketLFS!=1){
		tPaquete* mensaje = malloc(sizeof(tPaquete));
		mensaje->type = DESCRIBE;
		char* comando = string_from_format("DESCRIBE");
		strcpy(mensaje->payload,comando);
		mensaje->length = sizeof(mensaje->payload);
		enviarPaquete(socketLFS, mensaje,logger,"Ejecutar comando DESCRIBE desde Memoria.");
		liberarPaquete(mensaje);
		sem_wait(&loggerSemaforo);
		log_info(logger, "'DESCRIBE' enviado a LFS");
		sem_post(&loggerSemaforo);
		free(comando);
		char* sPayload;
		tMensaje tipo_mensaje;
		recibirPaquete(socketLFS,&tipo_mensaje,&sPayload,logger,"Cantidad de tablas en LFS");
		int cantidadTablas = atoi(sPayload);
		free(sPayload);
		char* metadata;
		for(int i=0;i<cantidadTablas; i++){
			recibirPaquete(socketLFS,&tipo_mensaje,&sPayload,logger,"Metadata de una tabla de LFS");
			//printf("\n%s",sPayload);
			metadata = string_duplicate(sPayload);
			list_add(retorno,metadata);
			free(sPayload);
		}
		sem_wait(&loggerSemaforo);
		log_info(logger, "'DESCRIBE' ejecutado exitosamente");
		sem_post(&loggerSemaforo);
	}
	return retorno;
}

void levantarMemoria(){
	tablaDeSegmentos = malloc(1);
	//maxValueSize = 20; //ESTO TIENE QUE VENIR DE LFS
	tamanioRealDeUnRegistro = sizeof(uint64_t) + sizeof(uint16_t) + maxValueSize ;
	cantidadDeRegistros = configuracion->TAM_MEM / tamanioRealDeUnRegistro;
	granMalloc = malloc(cantidadDeRegistros*tamanioRealDeUnRegistro);
	for(int i=0; i <cantidadDeRegistros; i++)
			//setTimestamp(granMalloc+i*tamanioRealDeUnRegistro,-1);
			vaciarMemoria();
	sem_wait(&loggerSemaforo);
	log_debug(logger, "Tamanio granMalloc: %d", malloc_usable_size(granMalloc));
	sem_post(&loggerSemaforo);
}
void resetearMemoria(void* punteroMemoria){
	memset(punteroMemoria,0,tamanioRealDeUnRegistro);
}
void vaciarMemoria(){
	memset(granMalloc,0,cantidadDeRegistros*tamanioRealDeUnRegistro);
}
uint16_t getKey(void* registro){
	uint16_t key = *(uint16_t*)registro;
	return key;
}
uint64_t getTimestamp(void* registro){
	uint64_t timestamp = *(uint64_t*)(registro+sizeof(uint16_t));
	return timestamp;
}
Registro* getRegistro(void* registro){
	Registro* reg = (Registro*) malloc(sizeof(Registro));
	reg->value = (char*) malloc(maxValueSize);
	reg->key = *(uint16_t*)registro;
	reg->timestamp = *(uint64_t*)(registro+sizeof(uint16_t));
	strcpy(reg->value,(char*)(registro+sizeof(uint16_t)+sizeof(uint64_t)));
	return reg;
}
void setValue(void* registro, char* value){
	memcpy(registro+sizeof(uint16_t)+sizeof(uint64_t),value,maxValueSize);
}
void setTimestamp(void* registro, uint64_t timestamp){
	memcpy(registro+sizeof(uint16_t),&timestamp,sizeof(uint64_t));
}
void setRegistro(void* registro, uint16_t key, uint64_t timestamp, char* value){
	memcpy(registro,&key,sizeof(uint16_t));
	memcpy(registro+sizeof(uint16_t),&timestamp,sizeof(int));
	memcpy(registro+sizeof(uint16_t)+sizeof(uint64_t),value,maxValueSize);
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
void asignarRegistroANuevoSegmento(char* tabla, uint16_t key, char* value, uint64_t timestamp, int nSegmento, int nRegistro, int flagModificado){
	cantidadDeSegmentos++;
	tablaDeSegmentos = (Segmento**) realloc(tablaDeSegmentos, cantidadDeSegmentos*sizeof(Segmento*));
	Pagina* paginaDelSegmento = paginaCrear(flagModificado, nRegistro);
	Pagina** tablaDePaginasDelSegmento = (Pagina**) malloc(sizeof(Pagina*));
	Segmento* segmento = segmentoCrear(tabla, tablaDePaginasDelSegmento);
	tablaDeSegmentos[nSegmento] = segmento;
	tablaDeSegmentos[nSegmento]->cantidadDePaginas = 1;
	strcpy(tablaDeSegmentos[nSegmento]->tabla, tabla);
	tablaDeSegmentos[nSegmento]->tablaDePaginas = tablaDePaginasDelSegmento;
	tablaDeSegmentos[nSegmento]->tablaDePaginas[tablaDeSegmentos[nSegmento]->cantidadDePaginas - 1] = paginaDelSegmento;
	setRegistro(granMalloc+nRegistro*tamanioRealDeUnRegistro,key,timestamp,value);
}
void asignarRegistroASegmentoExistente(uint16_t key, char* value, uint64_t timestamp, int nSegmento, int nPagina, int nRegistro, int flagModificado){
	Pagina* paginaDelSegmento = paginaCrear(flagModificado, nRegistro);
	tablaDeSegmentos[nSegmento]->cantidadDePaginas ++;
	tablaDeSegmentos[nSegmento]->tablaDePaginas = (Pagina**) realloc(tablaDeSegmentos[nSegmento]->tablaDePaginas, tablaDeSegmentos[nSegmento]->cantidadDePaginas*sizeof(Pagina*));
	tablaDeSegmentos[nSegmento]->tablaDePaginas[nPagina] = paginaDelSegmento;
	tablaDeSegmentos[nSegmento]->tablaDePaginas[nPagina]->frame = granMalloc + nRegistro*tamanioRealDeUnRegistro;
	setRegistro(granMalloc+nRegistro*tamanioRealDeUnRegistro,key,timestamp,value);
}
int buscarRegistroDisponible(){
	for(int i = 0; i < cantidadDeRegistros; i++){
		if (getTimestamp(granMalloc+i*tamanioRealDeUnRegistro) == 0) return i;
	}
	return -1;
}
int buscarRegistro(t_nodoLRU* nodo_reemplazo){
	uint16_t key = getKey(tablaDeSegmentos[nodo_reemplazo->segmentoID]->tablaDePaginas[nodo_reemplazo->paginaID]->frame);
	for(int i = 0; i < cantidadDeRegistros; i++){
		if (getKey(granMalloc+i*tamanioRealDeUnRegistro) == key){
			free(nodo_reemplazo);
			return i;
		}
	}
	free(nodo_reemplazo);
	return -1;
}

void journalMemoria(){
	char *instruccion;
	Registro *registro = malloc(sizeof(Registro));
	//for revisando todas las páginas
	sem_wait(&mutexMemoria);
	if(cantidadDeSegmentos != 0){
		for(int j=0; j<cantidadDeSegmentos; j++){
			for(int i=0; i<tablaDeSegmentos[j]->cantidadDePaginas; i++){
				if(tablaDeSegmentos[j]->tablaDePaginas[i]->modificado == 1){
					instruccion = malloc(15+sizeof(tablaDeSegmentos[j]->tabla)+sizeof(registro->key)+sizeof(registro->value)+sizeof(registro->timestamp));
					registro = getRegistro(tablaDeSegmentos[j]->tablaDePaginas[i]->frame);
					sprintf (instruccion, "INSERT %s %hd %s %llu \n", tablaDeSegmentos[j]->tabla, registro->key, registro->value, registro->timestamp);
					ejecutarInsertJournal(instruccion);
					free(instruccion);
				}
			}
			segmentoDestruir(tablaDeSegmentos[j]);
		}
		cantidadDeSegmentos = 0;
		tablaDeSegmentos = (Segmento**) realloc(tablaDeSegmentos, 1);
	}
	sem_post(&mutexMemoria);
	sem_wait(&loggerSemaforo);
	log_info(logger, "'JOURNAL' ejecutado exitosamente");
	sem_post(&loggerSemaforo);
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
    			sem_wait(&loggerSemaforo);
    			log_info(logger, "Journal automático ejecutando");
    			sem_post(&loggerSemaforo);
    			journalMemoria();
    			break;
    		}
        }
    }
}
