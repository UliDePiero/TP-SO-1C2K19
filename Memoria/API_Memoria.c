/*
 * API_Memoria.C
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */
#include "API_Memoria.h"

void* API_Memoria(){
	//char* linea;
	//char* linea2;
	char* line = NULL;
	char* instruccion_API = NULL;
	char* retorno;
	t_list* retornoLista;

	line = readline(">");

	while(strncmp("EXIT", line, 5)){
		//linea2 = malloc(strlen(linea)+1);
		//strcpy(linea2, linea);
		instruccion_API = string_duplicate(line);
		switch(parser(line)){
			case SELECT:
				retorno = ejecutarSelect(instruccion_API);
				if(retorno!=NULL) free(retorno);
				break;
			case INSERT:
				retorno = ejecutarInsert(instruccion_API);
				if(retorno!=NULL) free(retorno);
				break;
			case CREATE:
				retorno = ejecutarCreate(instruccion_API);
				if(retorno!=NULL) free(retorno);
				break;
			case DESCRIBE:
				retornoLista = ejecutarDescribe(instruccion_API);
				if(retornoLista)list_destroy(retornoLista);
				break;
			case DROP:
				retorno =ejecutarDrop(instruccion_API);
				if(retorno!=NULL) free(retorno);
				break;
			case JOURNAL:
				ejecutarJournal(instruccion_API);
				break;
			case -1:
				informarComandoInvalido();
				free(instruccion_API);
				break;
			default:
				printf("Ingrese un comando \n");
				free(instruccion_API);
				break;
		}
		free(line);
/*
		int s, p;
		for(s=0; s<cantidadDeSegmentos; s++)
			for(p=0; p<tablaDeSegmentos[s]->cantidadDePaginas; p++)
				printf("\n%s %d %llu", tablaDeSegmentos[s]->tabla,getKey(tablaDeSegmentos[s]->tablaDePaginas[p]->frame),getTimestamp(tablaDeSegmentos[s]->tablaDePaginas[p]->frame));
*/
		line = readline(">");
	}
	free(line);
	terminar();
	return (void*)1;
}
char* ejecutarSelect(char* instruccion){
	char* retorno = NULL;
	char** comando = validarComando(instruccion, 3);
	if(comando){
		Registro* registro = selectMemoria(comando[1], atoi(comando[2]));
		usleep(configuracion->RETARDO_MEM * 1000);
		if(registro != NULL)
		{
			//printf("value: %s\n", registro->value);
			retorno = string_duplicate(registro->value);
			free(registro->value);
			sem_wait(&loggerSemaforo);
			log_info(logger, "Resultado de '%s': %s ", instruccion, retorno);
			sem_post(&loggerSemaforo);
		}
		free(registro);
		for(int i = 0; i<3; i++)
			free(comando[i]);
		free(comando);
	}
	free(instruccion);
	return retorno;
}
char* ejecutarInsert(char* instruccion){
	char* retorno = NULL;
	char** comando = validarComandoInsert(instruccion);
	//char** comando = validarComando(instruccion, 4);
	if(comando){
		usleep(configuracion->RETARDO_MEM * 1000);
		insertMemoria(comando[1], atoi(comando[2]), comando[3], getCurrentTime(), 1);
		retorno = string_from_format("Tabla:%s Key:%d Value:%s",comando[1], atoi(comando[2]), comando[3]);
		for(int i = 0; i<4; i++)
			free(comando[i]);
		free(comando);
	}
	sem_wait(&loggerSemaforo);
	log_info(logger, "Resultado de '%s': %s ", instruccion, retorno);
	sem_post(&loggerSemaforo);
	free(instruccion);
	return retorno;
}
char* ejecutarCreate(char* instruccion){
	char* retorno = NULL;
	char** comando = validarComando(instruccion, 5);
	if(comando){
		tPaquete* mensaje = malloc(sizeof(tPaquete));
		mensaje->type = CREATE;
		strcpy(mensaje->payload,instruccion);
		mensaje->length = sizeof(mensaje->payload);
		enviarPaquete(socketLFS, mensaje,logger,"Ejecutar comando CREATE desde Memoria.");
		liberarPaquete(mensaje);
		sem_wait(&loggerSemaforo);
		log_trace(logger, "'%s' enviado a LFS", instruccion);
		sem_post(&loggerSemaforo);
		retorno = string_from_format("Tabla:%s Consistencia:%s Particiones:%d Tiempo compactacion:%d",comando[1], comando[2], atoi(comando[3]), atoi(comando[4]));
		for(int i = 0; i<5; i++)
			free(comando[i]);
		free(comando);
	}
	sem_wait(&loggerSemaforo);
	log_info(logger, "Resultado de '%s': %s ", instruccion, retorno);
	sem_post(&loggerSemaforo);
	free(instruccion);
	return retorno;
}
void muestraLista(char* elemento){
	printf("\nMetadata: %s\n", elemento);
}
t_list* ejecutarDescribe(char* instruccion){
	t_list* retorno;
	char** comando = string_n_split(instruccion, 2, " ");
	if(comando[1]){
		usleep(configuracion->RETARDO_MEM * 1000);
		retorno = list_create();
		char* metadata = describeMemoriaTabla(comando[1]);
		list_add(retorno, metadata);
		//printf("\nMetadata: %s\n", /*(char*)retorno->head->data*/metadata);
		free(comando[1]);
		sem_wait(&loggerSemaforo);
		log_info(logger, "Resultado de '%s': %s ", instruccion, metadata);
		sem_post(&loggerSemaforo);
	}else{
		usleep(configuracion->RETARDO_MEM * 1000);
		retorno = describeMemoria();
		//list_iterate(retorno,(void*)muestraLista);
	}
	free(comando[0]);
	free(comando);
	free(instruccion);
	return retorno;
}
char* ejecutarDrop(char* instruccion){
	char* retorno = NULL;
	//puts("drop ejecutado");
	char** comando = validarComando(instruccion, 2);
	if(comando){
		usleep(configuracion->RETARDO_MEM * 1000);
		dropMemoria(comando[1]);
		retorno = string_duplicate(comando[1]);
		for(int i = 0; i<2; i++)
			free(comando[i]);
		free(comando);
	}
	free(instruccion);
	return retorno;
}
int ejecutarJournal(char* instruccion){
	//puts("journal ejecutado");
	int retorno = 0;
	char** comando = validarComando(instruccion, 1);
	if(comando){
		usleep(configuracion->RETARDO_MEM * 1000);
		journalMemoria();
		free(comando[0]);
		free(comando);
		retorno = 1;
	}
	free(instruccion);
	return retorno;
}
