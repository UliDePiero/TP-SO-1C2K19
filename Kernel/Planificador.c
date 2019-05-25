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


