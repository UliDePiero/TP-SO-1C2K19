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
				sleep(configuracion->SLEEP_EJECUCION / 1000);
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
				//sem_wait(&loggerSemaforo);
				//log_debug(logger, "Retorno de la ejecucion: %d", retornoRUN);
				//sem_post(&loggerSemaforo);
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
	Tabla* tab;
	Metadata* metadata;
	int numeroTabla;
	int status;
	TablaGossip* nodoRecibido;
	tMensaje tipo_mensaje;
	while(conexion != 0){
		conexion=recibirPaquete(socket_Memoria,&tipo_mensaje,&sPayload,logger,"Respuesta de MEMORIA");
		if(conexion !=0){
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
				sem_wait(&mutexTablas);
				list_clean_and_destroy_elements(listaTablas,(void*)limpiarListaTablas);
				if(strcmp(sPayload," ") != 0){
					tablas = string_n_split(sPayload, 100, ";");
					numeroTabla = 0;
					while(tablas[numeroTabla]!=NULL)
					{
						tabla = string_n_split(tablas[numeroTabla], 4, ",");

						metadata = malloc(sizeof(Metadata));
						strcpy(metadata->consistencia,tabla[1]);
						metadata->particiones = atoi(tabla[2]);
						metadata->tiempoCompactacion = atol(tabla[3]);
						tab = malloc(sizeof(Tabla));
						tab->nombreTabla = malloc(sizeof(char)*string_length(tabla[0]));
						strcpy(tab->nombreTabla,tabla[0]);
						tab->metadata = metadata;
						list_add(listaTablas,tab);
						//sem_wait(&loggerSemaforo);
						//log_info(logger, "Tabla %s: consistencia %s particiones %d tiempo compactacion %ld", tab->nombreTabla, tab->metadata->consistencia, tab->metadata->particiones, tab->metadata->tiempoCompactacion);
						//sem_post(&loggerSemaforo);

						for(int i = 0; i<4; i++)
							free(tabla[i]);
						free(tabla);
						numeroTabla++;
					}
					for(int i = 0; i<numeroTabla; i++)
						free(tablas[i]);
					free(tablas);
				}
				sem_post(&mutexTablas);
				//puts("\n>");
				//sem_wait(&loggerSemaforo);
				//log_info(logger, "Se ejecuto corectamente el comando DESCRIBE en MEMORIA");
				//sem_post(&loggerSemaforo);
				break;
			case DESCRIBE_TABLA:
				if(strcmp(sPayload," ") != 0){
					tabla = string_n_split(sPayload, 4, ",");
					sem_wait(&mutexTablas);
					tab = encontrarTabla(tabla[0]);
					if(!tab){
							metadata = malloc(sizeof(Metadata));
							strcpy(metadata->consistencia,tabla[1]);
							metadata->particiones = atoi(tabla[2]);
							metadata->tiempoCompactacion = atol(tabla[3]);
							tab = malloc(sizeof(Tabla));
							tab->nombreTabla = malloc(sizeof(char)*string_length(tabla[0]));
							strcpy(tab->nombreTabla,tabla[0]);
							tab->metadata = metadata;
							list_add(listaTablas,tab);

							for(int i = 0; i<4; i++)
								free(tabla[i]);
							free(tabla);
					}
					sem_wait(&loggerSemaforo);
					log_info(logger, "Tabla %s: consistencia %s particiones %d tiempo compactacion %ld", tab->nombreTabla, tab->metadata->consistencia, tab->metadata->particiones, tab->metadata->tiempoCompactacion);
					sem_post(&loggerSemaforo);
				}
				sem_wait(&loggerSemaforo);
				log_info(logger, "Se ejecuto corectamente el comando DESCRIBE en MEMORIA");
				sem_post(&loggerSemaforo);
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
			case GOSSIPING_RECIBE:
				for (int i = 0; i < atoi(sPayload); i++) {
					nodoRecibido = malloc(sizeof(TablaGossip));
					status = recibirNodoYDeserializar(nodoRecibido, socket_Memoria);
					if (status)
						armarNodoMemoria(nodoRecibido);
				}
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
	}
	eliminaMemoriaDeListaGossiping(socket_Memoria); // La Memoria se desconectó, la elimino de listaGossiping
	free(socket_Mem);
	if(listaGossiping->elements_count == 0)
		pthread_cancel(hiloAPI);
}
TablaGossip* elegirMemoriaRandom() {
	int indice;
	if (listaGossiping->elements_count > 0) {
		/*if (memoriasEC->elements_count > 0) {
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
		if (memoriasSHC->elements_count > 0) {
			indice = generarNumeroRandom(memoriasSHC->elements_count);
			nroMemoria = (int) list_get(memoriasSHC, indice);
			TablaGossip* memoriaElegida = buscarNodoMemoria(nroMemoria);
			return memoriaElegida;
		}*/
		indice = generarNumeroRandom(listaGossiping->elements_count);
		TablaGossip* memoriaElegida = list_get(listaGossiping, indice);
		return memoriaElegida;
	} else {
		sem_wait(&loggerSemaforo);
		log_error(logger, "No se pudo elegir una memoria: No hay memorias conectadas");
		sem_post(&loggerSemaforo);
	}
	return NULL;
}
int elegirSocketMemoria(char* tabla, int key){
	sem_wait(&mutexTablas);
	Tabla* tab = encontrarTabla(tabla);
	//sem_post(&mutexTablas);
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
int eliminarTabla(char* nombreTabla){
	t_link_element* nodoActual = listaTablas->head;
	t_link_element* nodoAnterior = NULL;
	Tabla* nodoAux;

	if (nodoActual)
		nodoAux = nodoActual->data;
	while (nodoActual && !string_equals_ignore_case(nodoAux->nombreTabla, nombreTabla)) {
		nodoAnterior = nodoActual;
		nodoActual = nodoActual->next;
		if (nodoActual)
			nodoAux = nodoActual->data;
	}
	if (nodoActual && string_equals_ignore_case(nodoAux->nombreTabla, nombreTabla)) {
		if (!nodoAnterior)
			listaTablas->head = nodoActual->next;
		else
			nodoAnterior->next = nodoActual->next;
		listaTablas->elements_count--;
		free(nodoActual);
		free(nodoAux);
		return 1;
	}else
		return 0;
}
Tabla* encontrarTabla(char* nombreTabla){
	int encuentraTabla(Tabla* t) {
		//log_warning(logger2,"BUSCO :%s TENGO:%s",nombreTabla,t->nombreTabla);
		return string_equals_ignore_case(t->nombreTabla, nombreTabla);
	}
	Tabla* tabla = list_find(listaTablas, (void*)encuentraTabla);
	sem_post(&mutexTablas);
	return tabla;
}
void limpiarListaTablas(Tabla* tabla){
	if(tabla->metadata)free(tabla->metadata);
	if(tabla->nombreTabla)free(tabla->nombreTabla);
	if(tabla)free(tabla);
}

int ejecutarSelect(char* instruccion){
	sleep(configuracion->SLEEP_EJECUCION / 1000);
	int resultado = 0;
	char** comando = validarComando(instruccion, 3);
	if(comando){
		socketElegido = elegirSocketMemoria(comando[1],atoi(comando[2]));
		if(socketElegido != -1 && socketElegido != -2){
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
	char** comando = validarComandoInsert(instruccion);
	if(comando){
		socketElegido = elegirSocketMemoria(comando[1],atoi(comando[2]));
		//sem_wait(&loggerSemaforo);
		//log_warning(logger, "SOCKET ELEGIDO: %d\n", socketElegido);
		//sem_post(&loggerSemaforo);
		if(socketElegido != -1 && socketElegido != -2){
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
		if(socketElegido != -1 && socketElegido != -2){
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

				sem_wait(&mutexTablas);
				if(!encontrarTabla(comando[1])){
					Metadata* metadata = malloc(sizeof(Metadata));
					strcpy(metadata->consistencia,comando[2]);
					metadata->particiones = atoi(comando[3]);
					metadata->tiempoCompactacion = atol(comando[4]);
					Tabla* tab = malloc(sizeof(Tabla));
					tab->nombreTabla = malloc(sizeof(char)*string_length(comando[1]));
					strcpy(tab->nombreTabla,comando[1]);
					tab->metadata = metadata;
					list_add(listaTablas,tab);
					sem_wait(&loggerSemaforo);
					log_info(logger, "Nueva tabla %s: consistencia %s particiones %d tiempo compactacion %ld", tab->nombreTabla, tab->metadata->consistencia, tab->metadata->particiones, tab->metadata->tiempoCompactacion);
					sem_post(&loggerSemaforo);
				}
				//sem_post(&mutexTablas);
			}

			for(int i = 0; i<5; i++)
				free(comando[i]);
			free(comando);
		}
	}
}
void ejecutarDescribe(char* instruccion){
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
				//log_info(logger, "'%s' enviado exitosamente a Memoria", instruccion);
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
		if(socketElegido != -1 && socketElegido != -2){
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

				sem_wait(&mutexTablas);
				Tabla* tab = encontrarTabla(comando[1]);
				if(tab){
					if(eliminarTabla(tab->nombreTabla)){
						sem_wait(&loggerSemaforo);
						log_info(logger, "Tabla eliminada %s: ", comando[1]);
						sem_post(&loggerSemaforo);
					}
				}
				//sem_post(&mutexTablas);
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
	char *LQL_NoENTER;
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
		LQL_NoENTER = (char*)malloc(100);
		for(int i=0; i<requestEjecutadas; i++)
			fgets(LQL_NoENTER, 100, script);
		while(fgets(LQL_NoENTER, 100, script)!=NULL){
			quantum--;
			if(quantum>=0){
				if(string_contains(LQL_NoENTER,"\n"))
					stringLQL = string_substring_until(LQL_NoENTER,string_length(LQL_NoENTER)-1);
				else
					stringLQL = string_duplicate(LQL_NoENTER);
				printf("\n%s\n",stringLQL);
				switch(parser(stringLQL)){
						case SELECT:
							/*cargarNuevoLQL(stringLQL);
							sem_wait(&semEjecutarLQL);
							ejecutarSelect(stringLQL);
							moverLQL(Exec,Exit);
							sem_post(&semMultiprocesamiento);*/
							if(ejecutarSelect(stringLQL)){
								free(LQL_NoENTER);
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
								free(LQL_NoENTER);
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
							sleep(configuracion->SLEEP_EJECUCION / 1000);
							ejecutarDescribe(stringLQL);
							break;
						case DROP:
							/*cargarNuevoLQL(stringLQL);
							sem_wait(&semEjecutarLQL);
							ejecutarDrop(stringLQL);
							moverLQL(Exec,Exit);
							sem_post(&semMultiprocesamiento);*/
							if(ejecutarDrop(stringLQL)){
								free(LQL_NoENTER);
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
							free(LQL_NoENTER);
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
				free(stringLQL);
			}
			else{
				free(LQL_NoENTER);
				fclose(script);
				for(int i = 0; i<2; i++)
					free(comando[i]);
				free(comando);
				return quantum;
			}
		}
		free(LQL_NoENTER);
		fclose(script);
		for(int i = 0; i<2; i++)
			free(comando[i]);
		free(comando);
		return quantum;
	}
	return -2;
}
