/*
 * Planificador.c
 *
 *  Created on: 17 abr. 2019
 *      Author: utnso
 */

#include "Planificador.h"
void configurar(ConfiguracionKernel* configuracion) {

	char* campos[] = { "IP_MEMORIA", "PUERTO_MEMORIA", "QUANTUM",
			"MULTIPROCESAMIENTO", "METADATA_REFRESH", "SLEEP_EJECUCION" };

	t_config* archivoConfig = archivoConfigCrear(RUTA_CONFIG, campos);

	//Relleno los campos con la info del archivo

	strcpy(configuracion->IP_MEMORIA,
			archivoConfigSacarStringDe(archivoConfig, "IP_MEMORIA"));
	//strcpy(configuracion->PUERTO_MEMORIA, archivoConfigSacarStringDe(archivoConfig, "PUERTO_MEMORIA"));
	configuracion->PUERTO_MEMORIA = archivoConfigSacarIntDe(archivoConfig,
			"PUERTO_MEMORIA");
	configuracion->QUANTUM = archivoConfigSacarIntDe(archivoConfig, "QUANTUM");
	configuracion->MULTIPROCESAMIENTO = archivoConfigSacarIntDe(archivoConfig,
			"MULTIPROCESAMIENTO");
	configuracion->METADATA_REFRESH = archivoConfigSacarIntDe(archivoConfig,
			"METADATA_REFRESH");
	configuracion->SLEEP_EJECUCION = archivoConfigSacarIntDe(archivoConfig,
			"SLEEP_EJECUCION");

	archivoConfigDestruir(archivoConfig);
}

void cambiosConfigKernel() {
	if (configModificado()) {
		t_config* archivoConfig = config_create(RUTA_CONFIG);
		configuracion->QUANTUM = archivoConfigSacarIntDe(archivoConfig,
				"QUANTUM");
		configuracion->METADATA_REFRESH = archivoConfigSacarIntDe(archivoConfig,
				"METADATA_REFRESH");
		configuracion->SLEEP_EJECUCION = archivoConfigSacarIntDe(archivoConfig,
				"SLEEP_EJECUCION");
		archivoConfigDestruir(archivoConfig);
	}
}

int main() {
	logger = log_create(logFile, "Planificador", true, LOG_LEVEL_INFO);
	configuracion = malloc(sizeof(ConfiguracionKernel));
	configurar(configuracion);
	crearListasDeCriteriosMemorias(); // Creación de listas de criterios de consistencia
	listaGossiping = list_create(); // Creación de lista para Tabla de Gossip
	New = queue_create();
	Ready = queue_create();
	Exec = queue_create();
	Exit = queue_create();
	ListaLQL = list_create();
	IDLQL = 0;
	sem_init(&semContadorLQL, 0, 0);
	sem_init(&semMultiprocesamiento, 0, configuracion->MULTIPROCESAMIENTO);
	sem_init(&semEjecutarLQL, 0, 0);

	//FUNCIONES SOCKETS (Usar dependiendo de la biblioteca que usemos)

	// cliente
	//int socketMEMORIA = conectarAUnServidor(configuracion->IP_MEMORIA, configuracion->PUERTO_MEMORIA);
	socketMemoria = connectToServer(configuracion->IP_MEMORIA,
			configuracion->PUERTO_MEMORIA, logger);

	realizarHandshakeConMemoria(); // Creo que esto después va a quedar dentro de Gossiping

	free(configuracion);
	//crearHiloIndependiente(&hiloAPI,(void*)API_Kernel, NULL, "Kernel");
	crearHilo(&hiloAPI, (void*) API_Kernel, NULL, "Kernel");
	crearHiloIndependiente(&hiloPlanificacion, (void*) planificacion, NULL,
			"Kernel");

	joinearHilo(hiloAPI, NULL, "Kernel");
	desconectarseDe(socketMemoria);
}
void planificacion() {
	while (1) {
		sem_wait(&semMultiprocesamiento);
		sem_wait(&semContadorLQL);
		moverLQL(Ready, Exec);
		LQL = queue_peek(Exec);
		printf("\nLQL en Exec: %s\n", LQL->Instruccion);
		LQLEnEjecucion++;
		LQL = queue_peek(Exec);
		if (LQL->FlagIncializado == 0) {
			LQL->FlagIncializado = 1;
			sem_post(&semEjecutarLQL);
		} else {
			if (ejecutarRun(LQL->Instruccion, LQL->requestEjecutadas) == -1) {
				actualizarRequestEjecutadas();
				moverLQL(Exec, Ready);
				LQL = queue_peek(Ready);
				printf("\nLQL en Ready: %s\n", LQL->Instruccion);
				sem_post(&semContadorLQL);
			} else {
				moverLQL(Exec, Exit);
				free(queue_pop(Exit));
				printf("\n>");
			}
			sem_post(&semMultiprocesamiento);
		}
	}
}
void cargarNuevoLQL(char* ScriptLQL) {
	EstructuraLQL* NuevoLQL = malloc(sizeof(EstructuraLQL)); //FALTA FREE//FALTA FREE//FALTA FREE//FALTA FREE//FALTA FREE//FALTA FREE moverLQL(Exec,Exit);
	queue_push(New, NuevoLQL);
	NuevoLQL->FlagIncializado = 0;
	NuevoLQL->requestEjecutadas = 0;
	NuevoLQL->ID = IDLQL++;
	strcpy(NuevoLQL->Instruccion, ScriptLQL);
	printf("\nNuevo LQL: %s\n", NuevoLQL->Instruccion);
	//list_add(ListaLQL, NuevoLQL); //creo que no es necesaria
	queue_push(Ready, NuevoLQL);
	LQL = queue_peek(Ready);
	printf("\nLQL en Ready: %s\n", LQL->Instruccion);
	sem_post(&semContadorLQL);
}
void moverLQL(t_queue *colaOrigen, t_queue *colaDestino) {
	/*
	 EstructuraLQL* LQL;
	 EstructuraLQL* LQL_Elegido = list_find(ListaLQL, (void*) (LQL->ID == ID)); //puede romper duramente
	 */
	queue_push(colaDestino, queue_pop(colaOrigen));
}
void actualizarRequestEjecutadas() {
	LQL = queue_peek(Exec);
	LQL->requestEjecutadas += configuracion->QUANTUM;
}

void realizarHandshakeConMemoria() {
	tPaquete* msjeEnviado = malloc(sizeof(tPaquete));
	msjeEnviado->type = HANDSHAKE;
	strcpy(msjeEnviado->payload, "Kernel realiza handshake con Memoria");
	msjeEnviado->length = sizeof(msjeEnviado->payload);
	enviarPaquete(socketMemoria, msjeEnviado, logger, "Realizar Handshake entre Kernel y Memoria.");
	liberarPaquete(msjeEnviado);

	tPaquete* msjeRecibido = malloc(sizeof(tPaquete));
	recv(socketMemoria, msjeRecibido, sizeof(tPaquete), 0);
	armarNodoMemoria(atoi(msjeRecibido->payload));
	liberarPaquete(msjeRecibido);
}

void crearListasDeCriteriosMemorias() {
	memoriaSC = list_create();
	memoriasSHC = list_create();
	memoriasEC = list_create();
}

void armarNodoMemoria(int nroMemoria) {
	TablaGossip* nodoMem = malloc(sizeof(TablaGossip));
	// Cargo datos el nodo
	nodoMem->IDMemoria = nroMemoria;
	//nodoMem->IPMemoria = configuracion->IP_MEMORIA; // Cambiar por IP de la memoria a la que se conecta
	strcpy(nodoMem->IPMemoria, configuracion->IP_MEMORIA);
	nodoMem->puertoMemoria = configuracion->PUERTO_MEMORIA; // Cambiar por puerto de la memoria a la que se conecta
	nodoMem->criterioSC = 0;
	nodoMem->criterioSHC = 0;
	nodoMem->criterioEC = 0;
	// Agrego el nodo a listaGossiping
	list_add(listaGossiping, nodoMem);
}

TablaGossip* buscarNodoMemoria(int nroMemoria) {
	TablaGossip* nodoBuscado = malloc(sizeof(TablaGossip));
	t_link_element* nodoActual = listaGossiping->head;

	for (int i = 0; i < listaGossiping->elements_count; i++) {
		nodoBuscado = nodoActual->data; // Como entró al ciclo sé que nodoActual no es NULL

		if (nodoBuscado->IDMemoria == nroMemoria)
			return nodoBuscado; // Si encuentra memoria con ID igual a nroMemoria, retorna dicho nodo
		nodoActual = nodoActual->next;
	}

	return NULL; // Llegó al final de la lista y no encontró ninguna memoria con ID igual a nroMemoria
}

void asociarACriterioSC(int nroMemoria) {
	if (memoriaSC->elements_count == 0) {
		TablaGossip* nodoMemoria = buscarNodoMemoria(nroMemoria); // Buscar nodo correspondiente a la memoria en cuestión
		if (nodoMemoria) {
			list_add(memoriaSC, (void*) nroMemoria); // Asignar la memoria al criterio
			// Agregar el criterio en la Tabla de Gossip de la memoria:
			nodoMemoria->criterioSC = 1; // Como la memoria está asignada al criterio SC, ponemos este campo en 1
			printf("Se asignó la memoria %d al criterio SC \n", nroMemoria);
		} else
			printf("La memoria %d no se encuentra conectada\n", nroMemoria);
	} else {
		// Si ya hay una memoria asignada al criterio SC, informarlo y no hacer nada más
		printf("Ya existe una memoria asignada al criterio SC \n");
	}
}

void asociarACriterioSHC(int nroMemoria) {
	// Las memorias se pueden asignar a este criterio sin restricción alguna
	TablaGossip* nodoMemoria = buscarNodoMemoria(nroMemoria);
	if (nodoMemoria) {
		list_add(memoriasSHC, (void*) nroMemoria);
		nodoMemoria->criterioSHC = 1;
		printf("Se asignó la memoria %d al criterio SHC \n", nroMemoria);
		// Realizar un JOURNAL sobre todas las memorias asociadas al criterio, para garantizar que las keys se mantienen en las memorias correctas
		// TODO: Llamada a JOURNAL de memoria (pasándole la lista memoriasSHC)
	} else
		printf("La memoria %d no se encuentra conectada\n", nroMemoria);
}

void asociarACriterioEC(int nroMemoria) {
	// Las memorias se pueden asignar a este criterio sin restricción alguna
	TablaGossip* nodoMemoria = buscarNodoMemoria(nroMemoria);
	if (nodoMemoria) {
		list_add(memoriasEC, (void*) nroMemoria);
		nodoMemoria->criterioEC = 1;
		printf("Se asignó la memoria %d al criterio EC \n", nroMemoria);
	} else
		printf("La memoria %d no se encuentra conectada\n", nroMemoria);
}

void desasociarDeCriterioSC(int nroMemoria) { // TODO: REVER TODAS LAS FUNCIONES PARA DESASOCIAR
	if (memoriaSC->elements_count == 1) {
		list_remove(memoriaSC, 0); // Desasignar memoria del criterio (Como es una sola, va a estar en la primer posición de la lista)
		// Eliminar el criterio en la Tabla de Gossip de la memoria:
		TablaGossip* nodoMemoria = buscarNodoMemoria(nroMemoria); // Buscar nodo correspondiente a la memoria en cuestión
		nodoMemoria->criterioSC = 0; // Como la memoria ya no está asignada al criterio SC, ponemos este campo en 0
		printf("Se desasoció la memoria %d del criterio SC \n", nroMemoria);
	} else {
		// Si no existe memoria asignada al criterio, informarlo y no hacer nada más
		printf("No existe ninguna memoria asignada al criterio SC \n");
	}
}

int buscarMemoriaEnListaCriterio(int nroMemoria, t_list* listaMemoriasCriterio) {
	t_link_element* nodoActual = listaMemoriasCriterio->head;

	for (int i = 0; i < listaMemoriasCriterio->elements_count; i++) {
		if ((int) nodoActual->data == nroMemoria)
			return i;
		nodoActual = nodoActual->next;
	}
	return -1;
}

void desasociarDeCriterioSHC(int nroMemoria) { // TODO: REVER TODAS LAS FUNCIONES PARA DESASOCIAR
	if (memoriasSHC->elements_count > 0) {
		int indice = buscarMemoriaEnListaCriterio(nroMemoria, memoriasSHC);
		if (indice >= 0) {
			list_remove(memoriasSHC, indice);
			TablaGossip* nodoMemoria = buscarNodoMemoria(nroMemoria);
			nodoMemoria->criterioSHC = 0;
			printf("Se desasoció la memoria %d del criterio SHC \n",
					nroMemoria);
			// Realizar un JOURNAL sobre todas las memorias asociadas al criterio, para garantizar que las keys se mantienen en las memorias correctas
			// TODO: Llamada a JOURNAL de memoria (pasándole la lista memoriasSHC)
		} else {
			// Si no existe memoria asignada al criterio, informarlo y no hacer nada más
			printf("No existe ninguna memoria asignada al criterio SHC \n");
		}
	}
}

void desasociarDeCriterioEC(int nroMemoria) { // TODO: REVER TODAS LAS FUNCIONES PARA DESASOCIAR
	if (memoriasEC->elements_count > 0) {
		int indice = buscarMemoriaEnListaCriterio(nroMemoria, memoriasEC);
		if (indice >= 0) {
			list_remove(memoriasEC, indice);
			TablaGossip* nodoMemoria = buscarNodoMemoria(nroMemoria);
			nodoMemoria->criterioEC = 0;
			printf("Se desasoció la memoria %d del criterio EC \n", nroMemoria);
		}
	} else {
		// Si no existe memoria asignada al criterio, informarlo y no hacer nada más
		printf("No existe ninguna memoria asignada al criterio EC \n");
	}
}

TablaGossip* elegirMemoriaCriterioSC() {
	if (memoriaSC->elements_count == 1) {
		int nroMemoria = (int) memoriaSC->head->data; // Buscar número de memoria en lista de memoriaSC
		TablaGossip* memoriaElegida = buscarNodoMemoria(nroMemoria);
		return memoriaElegida;
	} else {
		printf("No existe ninguna memoria asignada al criterio SC \n");
	}
	return NULL;
}

int funcionHash(int key, int cantMaxMemoriasSHC) {
	int nroHash = key % cantMaxMemoriasSHC;
	return nroHash;
}

TablaGossip* elegirMemoriaCriterioSHC(int key) {
	if (memoriasSHC->elements_count > 0) {
		// Mediante la función de Hash, asignar una memoria a una determinada key
		int indice = funcionHash(key, memoriasSHC->elements_count);
		// Con el índice elijo la memoria de la lista de memoriasSHC
		int nroMemoria = (int) list_get(memoriasSHC, indice);
		// Con nroMemoria busco el nodo en la lista de TablaGossip y lo retorno
		TablaGossip* memoriaElegida = buscarNodoMemoria(nroMemoria);
		return memoriaElegida;
	} else {
		printf("No existe ninguna memoria asignada al criterio SHC \n");
	}
	return NULL;
}

int generarNumeroRandom(int nroMax) {
	srand(time(NULL));
	int nroRandom = rand() % nroMax;

	return nroRandom;
}

TablaGossip* elegirMemoriaCriterioEC() {
	if (memoriasEC->elements_count > 0) {
		// Generar un número random entre 0 y cantidad de memorias asignadas al criterio EC - 1
		int indice = generarNumeroRandom(memoriasEC->elements_count);
		// Con el índice elijo la memoria de la lista de memoriasEC
		int nroMemoria = (int) list_get(memoriasEC, indice);
		// Con nroMemoria busco el nodo en la lista de TablaGossip y lo retorno
		TablaGossip* memoriaElegida = buscarNodoMemoria(nroMemoria);
		return memoriaElegida;
	} else {
		printf("No existe ninguna memoria asignada al criterio EC \n");
	}
	return NULL;
}
