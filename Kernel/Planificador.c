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

void cambiosConfigKernel() {
	if (configModificado()) {
		t_config* archivoConfig = config_create(RUTA_CONFIG);
		configuracion->QUANTUM = archivoConfigSacarIntDe(archivoConfig, "QUANTUM");
		configuracion->METADATA_REFRESH = archivoConfigSacarIntDe(archivoConfig, "METADATA_REFRESH");
		configuracion->SLEEP_EJECUCION = archivoConfigSacarIntDe(archivoConfig, "SLEEP_EJECUCION");
		archivoConfigDestruir(archivoConfig);
	}
}

int main()
{
	logger = log_create(logFile, "Planificador",true, LOG_LEVEL_INFO);
	configuracion = malloc(sizeof(ConfiguracionKernel));
	configurar(configuracion);
	New = queue_create();
	Ready = queue_create();
	Exec = queue_create();
	Exit = queue_create();
	ListaLQL = list_create();
	IDLQL = 0;
	sem_init(&semContadorLQL,0,0);
	sem_init(&semMultiprocesamiento,0,configuracion->MULTIPROCESAMIENTO);
	sem_init(&semEjecutarLQL,0,0);

	//FUNCIONES SOCKETS (Usar dependiendo de la biblioteca que usemos)

	// cliente
	//int socketMEMORIA = conectarAUnServidor(configuracion->IP_MEMORIA, configuracion->PUERTO_MEMORIA);
	socketMemoria = connectToServer(configuracion->IP_MEMORIA, configuracion->PUERTO_MEMORIA, logger);
	free(configuracion);
	//crearHiloIndependiente(&hiloAPI,(void*)API_Kernel, NULL, "Kernel");
	crearHilo(&hiloAPI,(void*)API_Kernel, NULL, "Kernel");
	crearHiloIndependiente(&hiloPlanificacion, (void*)planificacion, NULL, "Kernel");

	joinearHilo(hiloAPI,NULL,"Kernel");
	desconectarseDe(socketMemoria);
}
void planificacion(){
	int retornoRUN;
	while(1){
		sem_wait(&semMultiprocesamiento);
		sem_wait(&semContadorLQL);
		moverLQL(Ready,Exec);
		LQLEnEjecucion++;
		LQL = queue_peek(Exec);
		if(LQL->FlagIncializado == 0)
			LQL->FlagIncializado = 1;
		else{
			sem_wait(&semEjecutarLQL);
			retornoRUN = ejecutarRun(LQL->Instruccion, LQL->requestEjecutadas);
			if(retornoRUN == -1){
				actualizarRequestEjecutadas();
				moverLQL(Exec,Ready);
			}
			else
				moverLQL(Exec,Exit);
			sem_post(&semMultiprocesamiento);
		}
		sem_post(&semEjecutarLQL);
	}
}
void cargarNuevoLQL(char* ScriptLQL) {
	EstructuraLQL* NuevoLQL = malloc(sizeof(EstructuraLQL)); //FALTA FREE
	queue_push(New, NuevoLQL);
	NuevoLQL->FlagIncializado = 0;
	NuevoLQL->requestEjecutadas = 0;
	NuevoLQL->ID = IDLQL++;
	strcpy(NuevoLQL->Instruccion, ScriptLQL);
	//list_add(ListaLQL, NuevoLQL); //creo que no es necesaria
	queue_push(Ready, NuevoLQL);
	sem_post(&semContadorLQL);
}
void moverLQL(t_queue *colaOrigen, t_queue *colaDestino){
	/*
	EstructuraLQL* LQL;
	EstructuraLQL* LQL_Elegido = list_find(ListaLQL, (void*) (LQL->ID == ID)); //puede romper duramente
	*/
	queue_push(colaDestino, queue_pop(colaOrigen));
}
void actualizarRequestEjecutadas(){
	LQL = queue_peek(Exec);
	LQL->requestEjecutadas += configuracion->QUANTUM;
}

void crearListasDeCriteriosMemorias() {
	memoriaSC = list_create();
	memoriasSHC = list_create();
	memoriasEC = list_create();
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
		list_add(memoriaSC, (void*) nroMemoria); // Asignar la memoria al criterio
		// Agregar el criterio en la Tabla de Gossip de la memoria:
		nodoMemoria = buscarNodoMemoria(nroMemoria); // Buscar nodo correspondiente a la memoria en cuestión
		nodoMemoria->criterioSC = 1; // Como la memoria está asignada al criterio SC, ponemos este campo en 1
		printf("Se asignó la memoria %d al criterio SC \n", nroMemoria);
	} else {
		// Si ya hay una memoria asignada al criterio SC, informarlo y no hacer nada más
		printf("Ya existe una memoria asignada al criterio SC \n");
	}
}

void asociarACriterioEC(int nroMemoria) {
	// Las memorias se pueden asignar a este criterio sin restricción alguna
	list_add(memoriasEC, (void*) nroMemoria);
	nodoMemoria = buscarNodoMemoria(nroMemoria);
	nodoMemoria->criterioEC = 1;
	printf("Se asignó la memoria %d al criterio EC \n", nroMemoria);
}

void desasociarDeCriterioSC(int nroMemoria) { // TODO: REVER TODAS LAS FUNCIONES PARA DESASOCIAR
	if (memoriaSC->elements_count == 1) {
		list_remove(memoriaSC, 0); // Desasignar memoria del criterio (Como es una sola, va a estar en la primer posición de la lista)
		// Eliminar el criterio en la Tabla de Gossip de la memoria:
		nodoMemoria = buscarNodoMemoria(nroMemoria); // Buscar nodo correspondiente a la memoria en cuestión
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

void desasociarDeCriterioEC(int nroMemoria) { // TODO: REVER TODAS LAS FUNCIONES PARA DESASOCIAR
	if (memoriasEC->elements_count > 0) {
		int indice = buscarMemoriaEnListaCriterio(nroMemoria, memoriasEC);
		if (indice >= 0) {
			list_remove(memoriasEC, indice);
			nodoMemoria = buscarNodoMemoria(nroMemoria);
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

int generarNumeroRandom(int nroMax) {
	srand(time(NULL));
	int nroRandom = rand() % nroMax + 1;

	return nroRandom;
}

TablaGossip* elegirMemoriaCriterioEC() {
	if (memoriasEC->elements_count > 0) {
		// Generar un número random entre 1 y cantidad de memorias asignadas al criterio EC
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
