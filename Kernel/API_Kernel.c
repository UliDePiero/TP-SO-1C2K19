/*
 * API_Kernel.c
 *
 *  Created on: 17 abr. 2019
 *      Author: utnso
 */
#include "API_Kernel.h"

void API_Kernel(void){
	int retornoRUN;
	char* line = NULL;
	char* instruccion_API = NULL;

	line = readline(">\n");

	while(strncmp("EXIT", line, 5)){
		instruccion_API = string_duplicate(line);
		switch(parser(line)){
			case SELECT:
				cargarNuevoLQL(instruccion_API);
				sem_wait(&semEjecutarLQL);
				ejecutarSelect(instruccion_API);
				moverLQL(Exec,Exit);
				LQL = queue_peek(Exit);
				//printf("\nLQL en Exit: %s\n", LQL->Instruccion);
				sem_wait(&loggerSemaforo);
				log_trace(logger, "LQL en Exit: %s", LQL->Instruccion);
				sem_post(&loggerSemaforo);
				free(queue_pop(Exit));
				sem_post(&semMultiprocesamiento);
				break;
			case INSERT:
				cargarNuevoLQL(instruccion_API);
				sem_wait(&semEjecutarLQL);
				ejecutarInsert(instruccion_API);
				moverLQL(Exec,Exit);
				LQL = queue_peek(Exit);
				//printf("\nLQL en Exit: %s\n", LQL->Instruccion);
				sem_wait(&loggerSemaforo);
				log_trace(logger, "LQL en Exit: %s", LQL->Instruccion);
				sem_post(&loggerSemaforo);
				free(queue_pop(Exit));
				sem_post(&semMultiprocesamiento);
				break;
			case CREATE:
				cargarNuevoLQL(instruccion_API);
				sem_wait(&semEjecutarLQL);
				ejecutarCreate(instruccion_API);
				moverLQL(Exec,Exit);
				LQL = queue_peek(Exit);
				//printf("\nLQL en Exit: %s\n", LQL->Instruccion);
				sem_wait(&loggerSemaforo);
				log_trace(logger, "LQL en Exit: %s", LQL->Instruccion);
				sem_post(&loggerSemaforo);
				free(queue_pop(Exit));
				sem_post(&semMultiprocesamiento);
				break;
			case DESCRIBE:
				cargarNuevoLQL(instruccion_API);
				sem_wait(&semEjecutarLQL);
				ejecutarDescribe(instruccion_API);
				moverLQL(Exec,Exit);
				LQL = queue_peek(Exit);
				//printf("\nLQL en Exit: %s\n", LQL->Instruccion);
				sem_wait(&loggerSemaforo);
				log_trace(logger, "LQL en Exit: %s", LQL->Instruccion);
				sem_post(&loggerSemaforo);
				free(queue_pop(Exit));
				sem_post(&semMultiprocesamiento);
				break;
			case DROP:
				cargarNuevoLQL(instruccion_API);
				sem_wait(&semEjecutarLQL);
				ejecutarDrop(instruccion_API);
				moverLQL(Exec,Exit);
				LQL = queue_peek(Exit);
				//printf("\nLQL en Exit: %s\n", LQL->Instruccion);
				sem_wait(&loggerSemaforo);
				log_trace(logger, "LQL en Exit: %s", LQL->Instruccion);
				sem_post(&loggerSemaforo);
				free(queue_pop(Exit));
				sem_post(&semMultiprocesamiento);
				break;
			case JOURNAL:
				cargarNuevoLQL(instruccion_API);
				sem_wait(&semEjecutarLQL);
				ejecutarJournal(instruccion_API);
				moverLQL(Exec,Exit);
				LQL = queue_peek(Exit);
				//printf("\nLQL en Exit: %s\n", LQL->Instruccion);
				sem_wait(&loggerSemaforo);
				log_trace(logger, "LQL en Exit: %s", LQL->Instruccion);
				sem_post(&loggerSemaforo);
				free(queue_pop(Exit));
				sem_post(&semMultiprocesamiento);
				break;
			case ADD:
				cargarNuevoLQL(instruccion_API);
				sem_wait(&semEjecutarLQL);
				ejecutarAdd(instruccion_API);
				moverLQL(Exec,Exit);
				LQL = queue_peek(Exit);
				//printf("\nLQL en Exit: %s\n", LQL->Instruccion);
				sem_wait(&loggerSemaforo);
				log_trace(logger, "LQL en Exit: %s", LQL->Instruccion);
				sem_post(&loggerSemaforo);
				free(queue_pop(Exit));
				sem_post(&semMultiprocesamiento);
				break;
			case RUN:
				cargarNuevoLQL(instruccion_API);
				sem_wait(&semEjecutarLQL);
				retornoRUN = ejecutarRun(instruccion_API, 0);
				if(retornoRUN == -1){
					actualizarRequestEjecutadas();
					moverLQL(Exec,Ready);
					LQL = queue_peek(Ready);
					//printf("\nLQL en Ready: %s\n", LQL->Instruccion);
					sem_wait(&loggerSemaforo);
					log_trace(logger, "LQL en Ready: %s", LQL->Instruccion);
					sem_post(&loggerSemaforo);
					sem_post(&semContadorLQL);
				}
				else{
					moverLQL(Exec,Exit);
					LQL = queue_peek(Exit);
					//printf("\nLQL en Exit: %s\n", LQL->Instruccion);
					sem_wait(&loggerSemaforo);
					log_trace(logger, "LQL en Exit: %s", LQL->Instruccion);
					sem_post(&loggerSemaforo);
					free(queue_pop(Exit));
				}
				//printf("\nRetorno de la ejecucion: %d\n",retornoRUN);
				sem_wait(&loggerSemaforo);
				log_debug(logger, "Retorno de la ejecucion: %d", retornoRUN);
				sem_post(&loggerSemaforo);
				sem_post(&semMultiprocesamiento);
				break;
			case METRICS:
				// Comando METRICS
				printf("Comando METRICS\n");
				break;
			case -1:
				informarComandoInvalido();
				break;
			default:
				printf("Ingrese un comando \n");
				break;
		}
		free(line);
		free(instruccion_API);
		line = readline(">\n");
	}
	free(line);
}

void respuestas(void* socket_Mem){
	int socket_Memoria = *((int *) socket_Mem), conexion = 1;
	char *sPayload;
	char **tablas;
	char **tabla;
	int numeroTabla;
	tMensaje tipo_mensaje;
	while(conexion != 0){
		conexion=recibirPaquete(socket_Memoria,&tipo_mensaje,&sPayload,logger,"Respuesta de MEMORIA");
		if(conexion !=0)
		switch (tipo_mensaje) {
			case SELECT:
				//printf("\nValue: %s",sPayload);
				//puts("\n>");
				sem_wait(&loggerSemaforo);
				log_info(logger, "Value: %s",sPayload);
				sem_post(&loggerSemaforo);
				break;
			case INSERT:
				//printf("\nSe inserto corectamente en MEMORIA: %s",sPayload);
				//puts("\n>");
				sem_wait(&loggerSemaforo);
				log_info(logger, "Se inserto corectamente en MEMORIA: %s",sPayload);
				sem_post(&loggerSemaforo);
				break;
			case CREATE:
				//printf("\nSe ejecuto corectamente el comando CREATE %s en MEMORIA",sPayload);
				//puts("\n>");
				sem_wait(&loggerSemaforo);
				log_info(logger, "Se ejecuto corectamente el comando CREATE %s en MEMORIA",sPayload);
				sem_post(&loggerSemaforo);
				break;
			case DESCRIBE:
				tablas = string_n_split(sPayload, 100, ";");
				numeroTabla = 0;
				sem_wait(&loggerSemaforo);
				log_info(logger, "Se ejecuto corectamente el comando DESCRIBE en MEMORIA");
				sem_post(&loggerSemaforo);
				puts("\nDESCRIBE:");
				while(tablas[numeroTabla]!=NULL)
				{
					printf("\n%s",tablas[numeroTabla]);
					numeroTabla++;
				}
				if(numeroTabla>1)
				{
					numeroTabla = 0;
					while(tablas[numeroTabla]!=NULL)
					{
						list_clean_and_destroy_elements(listaTablas,(void*)limpiarListaTablas);
						tabla = string_n_split(tablas[numeroTabla], 4, ",");
						Metadata* metadata = malloc(sizeof(Metadata));
						strcpy(metadata->consistencia,tabla[1]);
						metadata->particiones = atoi(tabla[2]);
						metadata->tiempoCompactacion = atol(tabla[3]);
						Tabla* tab = malloc(sizeof(Tabla));
						tab->nombreTabla = malloc(sizeof(tabla[0]));
						strcpy(tab->nombreTabla,tabla[0]);
						tab->metadata = metadata;
						list_add(listaTablas,tab);
						numeroTabla++;

						for(int i = 0; i<4; i++)
							free(tabla[i]);
						free(tabla);
					}
				}
				else
				{
					tabla = string_n_split(tablas[0], 4, ",");
					Tabla* tab = encontrarTabla(tabla[0]);
					strcpy(tab->metadata->consistencia,tabla[1]);
					tab->metadata->particiones = atoi(tabla[2]);
					tab->metadata->tiempoCompactacion = atol(tabla[3]);
					for(int i = 0; i<4; i++)
						free(tabla[i]);
					free(tabla);
				}

				for(int i = 0; i<100; i++)
					free(tablas[i]);
				free(tablas);
				puts("\n>");
				break;
			case DROP:
				//printf("\nSe elimino la tabla %s de MEMORIA", sPayload);
				//puts("\n>");
				sem_wait(&loggerSemaforo);
				log_info(logger, "Se elimino la tabla %s de MEMORIA", sPayload);
				sem_post(&loggerSemaforo);
				break;
			case JOURNAL:
				if(atoi(sPayload)==1){
					//printf("\nJOURNAL de MEMORIA correctamente ejecutado.");
					sem_wait(&loggerSemaforo);
					log_info(logger, "JOURNAL de MEMORIA correctamente ejecutado");
					sem_post(&loggerSemaforo);
				}else{
					//printf("\nError en el JOURNAL de MEMORIA.");
					sem_wait(&loggerSemaforo);
					log_error(logger, "Error en el JOURNAL de MEMORIA");
					sem_post(&loggerSemaforo);
				}
					//puts("\n>");
				break;
			case ERROR_EN_COMANDO:
				//printf("\nHubo un error en la ejecucion del comando %s en MEMORIA", sPayload);
				//puts("\n>");
				sem_wait(&loggerSemaforo);
				log_error(logger, "Hubo un error en la ejecucion del comando %s en MEMORIA", sPayload);
				sem_post(&loggerSemaforo);
				break;

			default:
				//printf("Tipo de mensaje desconocido \n");
				//puts("\n>");
				sem_wait(&loggerSemaforo);
				log_error(logger, "El tipo de mensaje ingresado es desconocido");
				sem_post(&loggerSemaforo);
				break;
		}
		if(sPayload != NULL) free(sPayload);
	}
	// ACTUALIZAR LISTA GOSSIPING!!! (Eliminar elemento del socket que llamó al hilo)
	free(socket_Mem);
	if(listaGossiping->elements_count == 0)
		pthread_cancel(hiloAPI);
}
TablaGossip* elegirMemoriaRandom() {
	int indice, nroMemoria;
	if (listaGossiping->elements_count > 0) {
		if (memoriasEC->elements_count > 0) {
			indice = generarNumeroRandom(memoriasEC->elements_count);
			nroMemoria = (int) list_get(memoriasEC, indice);
			TablaGossip* memoriaElegida = buscarNodoMemoria(nroMemoria);
			return memoriaElegida;
		}
		if (memoriaSC->elements_count > 0) {
			indice = generarNumeroRandom(memoriaSC->elements_count);
			nroMemoria = (int) list_get(memoriaSC, indice);
			TablaGossip* memoriaElegida = buscarNodoMemoria(nroMemoria);
			return memoriaElegida;
		}
		/*if (memoriasSHC->elements_count > 0) {
			indice = generarNumeroRandom(memoriasSHC->elements_count);
			nroMemoria = (int) list_get(memoriasSHC, indice);
			TablaGossip* memoriaElegida = buscarNodoMemoria(nroMemoria);
			return memoriaElegida;
		}*/
	} else {
		sem_wait(&loggerSemaforo);
		log_error(logger, "No se pudo elegir una memoria: No existe ninguna memoria asignada a algun criterio");
		sem_post(&loggerSemaforo);
	}
	return NULL;
}
int elegirSocketMemoria(char* tabla, int key){
	Tabla* tab = encontrarTabla(tabla);
	if(tab != NULL){
		TablaGossip* mem;
		if(string_equals_ignore_case(tab->metadata->consistencia,"EC")){
			mem = elegirMemoriaCriterioEC();
			if(mem != NULL)
				return mem->socketMemoria;
			else
				return -1;
		}
		if(string_equals_ignore_case(tab->metadata->consistencia,"SC")){
			mem = elegirMemoriaCriterioSC();
			if(mem != NULL)
				return mem->socketMemoria;
			else
				return -1;
		}
		/*if(string_equals_ignore_case(tab->metadata->consistencia,"SHC")){
			mem = elegirMemoriaCriterioSHC(key);
			if(mem != NULL)
				return mem->socketMemoria;
			else
				return -1;
		}*/
	}
	return -2;
}
int elegirSocketMemoria_CREATE(char* criterio){
	TablaGossip* mem;
	if(string_equals_ignore_case(criterio,"EC")){
		mem = elegirMemoriaCriterioEC();
		if(mem != NULL)
			return mem->socketMemoria;
		else
			return -1;
	}
	if(string_equals_ignore_case(criterio,"SC")){
		mem = elegirMemoriaCriterioSC();
		if(mem != NULL)
			return mem->socketMemoria;
		else
			return -1;
	}
	/*if(string_equals_ignore_case(criterio,"SHC")){
		mem = elegirMemoriaCriterioSHC(key);
		if(mem != NULL)
			return mem->socketMemoria;
		else
			return -1;
	}*/

	sem_wait(&loggerSemaforo);
	log_error(logger, "Criterio desconocido, tabla no creada.");
	sem_post(&loggerSemaforo);

	return -2;
}
Tabla* encontrarTabla(char* nombreTabla){
	int encuentraTabla(Tabla* t) {
		return string_equals_ignore_case(t->nombreTabla, nombreTabla);
	}
	return list_find(listaTablas, (void*)encuentraTabla);
}
void limpiarListaTablas(Tabla* tabla){
	free(tabla->metadata);
	free(tabla->nombreTabla);
	free(tabla);
}

int ejecutarSelect(char* instruccion){
	sleep(configuracion->SLEEP_EJECUCION / 1000);
	int resultado = 0;
	char** comando = validarComando(instruccion, 3);
	if(comando){
		socketElegido = elegirSocketMemoria(comando[1],atoi(comando[2]));
		if(socketElegido != -1){
			tPaquete* mensaje = malloc(sizeof(tPaquete));
			mensaje->type = SELECT;
			strcpy(mensaje->payload,instruccion);
			mensaje->length = sizeof(mensaje->payload);
			resultado = enviarPaquete(socketElegido, mensaje,logger,"Ejecutar comando SELECT desde Kernel.");
			liberarPaquete(mensaje);

			if (resultado > 0){
				sem_wait(&loggerSemaforo);
				log_info(logger, "'%s' enviado exitosamente a Memoria", instruccion);
				sem_post(&loggerSemaforo);
			}

			for(int i = 0; i<3; i++)
				free(comando[i]);
			free(comando);

			return EXIT_SUCCESS;
		}
		else if(socketElegido == -2){
				sem_wait(&loggerSemaforo);
				log_error(logger, "Tabla '%s' no encontrada dentro de la metadata conocida.", comando[1]);
				sem_post(&loggerSemaforo);

				for(int i = 0; i<3; i++)
					free(comando[i]);
				free(comando);
			 }
	}
	return EXIT_FAILURE;
}
int ejecutarInsert(char* instruccion){
	sleep(configuracion->SLEEP_EJECUCION / 1000);
	int resultado = 0;
	char** comando = validarComando(instruccion, 4);
	if(comando){
		socketElegido = elegirSocketMemoria(comando[1],atoi(comando[2]));
		if(socketElegido != -1){
			tPaquete* mensaje = malloc(sizeof(tPaquete));
			mensaje->type = INSERT;
			strcpy(mensaje->payload,instruccion);
			mensaje->length = sizeof(mensaje->payload);
			resultado = enviarPaquete(socketElegido, mensaje,logger,"Ejecutar comando INSERT desde Kernel.");
			liberarPaquete(mensaje);

			if (resultado > 0){
				sem_wait(&loggerSemaforo);
				log_info(logger, "'%s' enviado exitosamente a Memoria", instruccion);
				sem_post(&loggerSemaforo);
			}

			for(int i = 0; i<4; i++)
				free(comando[i]);
			free(comando);

			return EXIT_SUCCESS;
		}
		else if(socketElegido == -2){
				sem_wait(&loggerSemaforo);
				log_error(logger, "Tabla '%s' no encontrada dentro de la metadata conocida.", comando[1]);
				sem_post(&loggerSemaforo);

				for(int i = 0; i<4; i++)
					free(comando[i]);
				free(comando);
			 }
	}
	return EXIT_FAILURE;
}
void ejecutarCreate(char* instruccion){
	sleep(configuracion->SLEEP_EJECUCION / 1000);
	int resultado = 0;
	char** comando = validarComando(instruccion, 5);
	if(comando){
		socketElegido = elegirSocketMemoria_CREATE(comando[2]);
		if(socketElegido != -1){
			tPaquete* mensaje = malloc(sizeof(tPaquete));
			mensaje->type = CREATE;
			strcpy(mensaje->payload,instruccion);
			mensaje->length = sizeof(mensaje->payload);
			resultado = enviarPaquete(socketElegido, mensaje,logger,"Ejecutar comando CREATE desde Kernel.");
			liberarPaquete(mensaje);

			if (resultado > 0){
				sem_wait(&loggerSemaforo);
				log_info(logger, "'%s' enviado exitosamente a Memoria", instruccion);
				sem_post(&loggerSemaforo);
			}

			for(int i = 0; i<5; i++)
				free(comando[i]);
			free(comando);
		}
	}
}
void ejecutarDescribe(char* instruccion){
	sleep(configuracion->SLEEP_EJECUCION / 1000);
	int resultado = 0;
	char** comando = string_n_split(instruccion, 2, " ");
	if(comando){
		TablaGossip* mem = elegirMemoriaRandom();
		if(mem != NULL){
			tPaquete* mensaje = malloc(sizeof(tPaquete));
			mensaje->type = DESCRIBE;
			strcpy(mensaje->payload,instruccion);
			mensaje->length = sizeof(mensaje->payload);
			resultado = enviarPaquete(mem->socketMemoria, mensaje,logger,"Ejecutar comando DESCRIBE desde Kernel.");
			liberarPaquete(mensaje);

			if (resultado > 0){
				sem_wait(&loggerSemaforo);
				log_info(logger, "'%s' enviado exitosamente a Memoria", instruccion);
				sem_post(&loggerSemaforo);
			}

			for(int i = 0; i<2; i++)
				free(comando[i]);
			free(comando);
		}
	}
}
int ejecutarDrop(char* instruccion){
	sleep(configuracion->SLEEP_EJECUCION / 1000);
	int resultado = 0;
	char** comando = validarComando(instruccion, 2);
	if(comando){
		socketElegido = elegirSocketMemoria(comando[1],-1);
		if(socketElegido != -1){
			tPaquete* mensaje = malloc(sizeof(tPaquete));
			mensaje->type = DROP;
			strcpy(mensaje->payload,instruccion);
			mensaje->length = sizeof(mensaje->payload);
			resultado = enviarPaquete(socketElegido, mensaje,logger,"Ejecutar comando DROP desde Kernel.");
			liberarPaquete(mensaje);

			if (resultado > 0){
				sem_wait(&loggerSemaforo);
				log_info(logger, "'%s' enviado exitosamente a Memoria", instruccion);
				sem_post(&loggerSemaforo);
			}

			for(int i = 0; i<2; i++)
				free(comando[i]);
			free(comando);

			return EXIT_SUCCESS;
		}
		else if(socketElegido == -2){
				sem_wait(&loggerSemaforo);
				log_error(logger, "Tabla '%s' no encontrada dentro de la metadata conocida.", comando[1]);
				sem_post(&loggerSemaforo);

				for(int i = 0; i<2; i++)
					free(comando[i]);
				free(comando);
			 }
	}
	return EXIT_FAILURE;
}
void ejecutarJournal(char* instruccion){
	sleep(configuracion->SLEEP_EJECUCION / 1000);
	int resultado = 0, nroMemoria = -1;
	TablaGossip* memoriaElegida;
	char** comando = validarComando(instruccion, 1);
	if(comando){
		for(int a=0; a<memoriasEC->elements_count; a++){
			nroMemoria = (int) list_get(memoriasEC, a);
			memoriaElegida = buscarNodoMemoria(nroMemoria);

			tPaquete* mensaje = malloc(sizeof(tPaquete));
			mensaje->type = JOURNAL;
			strcpy(mensaje->payload,instruccion);
			mensaje->length = sizeof(mensaje->payload);
			resultado = enviarPaquete(memoriaElegida->socketMemoria, mensaje,logger,"Ejecutar comando JOURNAL desde Kernel.");
			liberarPaquete(mensaje);

			if (resultado > 0){
				sem_wait(&loggerSemaforo);
				log_info(logger, "'%s' enviado exitosamente a Memoria", instruccion);
				sem_post(&loggerSemaforo);
			}
		}
		for(int a=0; a<memoriaSC->elements_count; a++){
			nroMemoria = (int) list_get(memoriaSC, a);
			memoriaElegida = buscarNodoMemoria(nroMemoria);

			tPaquete* mensaje = malloc(sizeof(tPaquete));
			mensaje->type = JOURNAL;
			strcpy(mensaje->payload,instruccion);
			mensaje->length = sizeof(mensaje->payload);
			resultado = enviarPaquete(memoriaElegida->socketMemoria, mensaje,logger,"Ejecutar comando JOURNAL desde Kernel.");
			liberarPaquete(mensaje);

			if (resultado > 0){
				sem_wait(&loggerSemaforo);
				log_info(logger, "'%s' enviado exitosamente a Memoria", instruccion);
				sem_post(&loggerSemaforo);
			}
		}
		/*for(int a=0; a<memoriasSHC->elements_count; a++){
			nroMemoria = (int) list_get(memoriasSHC, a);
			memoriaElegida = buscarNodoMemoria(nroMemoria);

			tPaquete* mensaje = malloc(sizeof(tPaquete));
			mensaje->type = JOURNAL;
			strcpy(mensaje->payload,instruccion);
			mensaje->length = sizeof(mensaje->payload);
			resultado = enviarPaquete(memoriaElegida->socketMemoria, mensaje,logger,"Ejecutar comando JOURNAL desde Kernel.");
			liberarPaquete(mensaje);

			if (resultado > 0){
				sem_wait(&loggerSemaforo);
				log_info(logger, "'%s' enviado exitosamente a Memoria", instruccion);
				sem_post(&loggerSemaforo);
			}
		}*/
		free(comando[0]);
		free(comando);
	}
}

void ejecutarAdd(char* instruccion) {
	sleep(configuracion->SLEEP_EJECUCION / 1000);
	char** comando = validarComando(instruccion, 5);
	if (comando){
		if (validacionStringsFijosAdd(comando) && cadenaEsDigito(comando[2]) && validacionStringCriterios(comando[4])) {
			sem_wait(&loggerSemaforo);
			log_info(logger, "'%s' ejecutado correctamente", instruccion);
			sem_post(&loggerSemaforo);
			int num = atoi(comando[2]);
			if (!strcmp("SC", comando[4])) {
				asociarACriterioSC(num);
			} else if (!strcmp("SHC", comando[4])) {
				asociarACriterioSHC(num);
			} else if (!strcmp("EC", comando[4])) {
				asociarACriterioEC(num);
			}
		} else{
			sem_wait(&loggerSemaforo);
			log_error(logger, "Error en el comando ADD. La sintaxis correcta es: ADD MEMORY [NÚMERO] TO [CRITERIO]");
			sem_post(&loggerSemaforo);
		}

		for(int i = 0; i<5; i++)
			free(comando[i]);
		free(comando);
	}
}
int ejecutarRun(char* instruccion, int requestEjecutadas){
	sleep(configuracion->SLEEP_EJECUCION / 1000);
	FILE *script;
	char *stringLQL;
	int quantum = configuracion->QUANTUM;

	char** comando = validarComando(instruccion, 2);
	if(comando){
		script = fopen(comando[1],"r");
		//script = fopen(PATH_SCRIPT,"r"); //para hacer pruebas
		if(script == NULL) {
			  sem_wait(&loggerSemaforo);
			  log_error(logger, "Error al abrir el script.");
			  sem_post(&loggerSemaforo);
			  for(int i = 0; i<2; i++)
				  free(comando[i]);
			  free(comando);
			  return -2;
		}
		stringLQL = (char*)malloc(100);
		for(int i=0; i<requestEjecutadas; i++)
			fgets(stringLQL, 100, script);
		while(fgets(stringLQL, 100, script)!=NULL){
			quantum--;
			if(quantum>=0){
				printf("\n%s\n",stringLQL);
				switch(parserSinTrim(stringLQL)){
						case SELECT:
							/*cargarNuevoLQL(stringLQL);
							sem_wait(&semEjecutarLQL);
							ejecutarSelect(stringLQL);
							moverLQL(Exec,Exit);
							sem_post(&semMultiprocesamiento);*/
							if(ejecutarSelect(stringLQL)){
								free(stringLQL);
								fclose(script);
								for(int i = 0; i<2; i++)
									free(comando[i]);
								free(comando);
								return -2;
							}
							break;
						case INSERT:
							/*cargarNuevoLQL(stringLQL);
							sem_wait(&semEjecutarLQL);
							ejecutarInsert(stringLQL);
							moverLQL(Exec,Exit);
							sem_post(&semMultiprocesamiento);*/
							if(ejecutarInsert(stringLQL)){
								free(stringLQL);
								fclose(script);
								for(int i = 0; i<2; i++)
									free(comando[i]);
								free(comando);
								return -2;
							}
							break;
						case CREATE:
							/*cargarNuevoLQL(stringLQL);
							sem_wait(&semEjecutarLQL);
							ejecutarCreate(stringLQL);
							moverLQL(Exec,Exit);
							sem_post(&semMultiprocesamiento);*/
							ejecutarCreate(stringLQL);
							break;
						case DESCRIBE:
							/*cargarNuevoLQL(stringLQL);
							sem_wait(&semEjecutarLQL);
							ejecutarDescribe(stringLQL);
							moverLQL(Exec,Exit);
							sem_post(&semMultiprocesamiento);*/
							ejecutarDescribe(stringLQL);
							break;
						case DROP:
							/*cargarNuevoLQL(stringLQL);
							sem_wait(&semEjecutarLQL);
							ejecutarDrop(stringLQL);
							moverLQL(Exec,Exit);
							sem_post(&semMultiprocesamiento);*/
							if(ejecutarDrop(stringLQL)){
								free(stringLQL);
								fclose(script);
								for(int i = 0; i<2; i++)
									free(comando[i]);
								free(comando);
								return -2;
							}
							break;
						case JOURNAL:
							/*cargarNuevoLQL(stringLQL);
							sem_wait(&semEjecutarLQL);
							ejecutarJournal(stringLQL);
							moverLQL(Exec,Exit);
							sem_post(&semMultiprocesamiento);*/
							ejecutarJournal(stringLQL);
							break;
						case ADD:
							/*cargarNuevoLQL(stringLQL);
							sem_wait(&semEjecutarLQL);
							ejecutarAdd(stringLQL);
							moverLQL(Exec,Exit);
							sem_post(&semMultiprocesamiento);*/
							ejecutarAdd(stringLQL);
							break;
						case -1:
							informarComandoInvalido();
							free(stringLQL);
							fclose(script);
							for(int i = 0; i<2; i++)
								free(comando[i]);
							free(comando);
							return -2;
							break;//creo que es innecesario pero por las dudas lo dejo
						default:
							printf("Fin de linea \n");
							break;
					}
			}
			else{
				free(stringLQL);
				fclose(script);
				for(int i = 0; i<2; i++)
					free(comando[i]);
				free(comando);
				return quantum;
			}
		}
		free(stringLQL);
		fclose(script);
		for(int i = 0; i<2; i++)
			free(comando[i]);
		free(comando);
		return quantum;
	}
	return -2;
}
