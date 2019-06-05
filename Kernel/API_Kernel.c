/*
 * API_Kernel.c
 *
 *  Created on: 17 abr. 2019
 *      Author: utnso
 */
#include "API_Kernel.h"

void API_Kernel(void){
	char* linea;
	char* linea2;
	int retornoRUN;

	linea = readline(">");

	while(strncmp("EXIT", linea, 5)){
		linea2 = malloc(strlen(linea)+1);
		strcpy(linea2, linea);
		switch(parser(linea)){
			case SELECT:
				cargarNuevoLQL(linea2);
				sem_wait(&semEjecutarLQL);
				ejecutarSelect(linea2);
				moverLQL(Exec,Exit);
				LQL = queue_peek(Exit);
				printf("\nLQL en Exit: %s\n", LQL->Instruccion);
				free(queue_pop(Exit));
				sem_post(&semMultiprocesamiento);
				break;
			case INSERT:
				cargarNuevoLQL(linea2);
				sem_wait(&semEjecutarLQL);
				ejecutarInsert(linea2);
				moverLQL(Exec,Exit);
				LQL = queue_peek(Exit);
				printf("\nLQL en Exit: %s\n", LQL->Instruccion);
				free(queue_pop(Exit));
				sem_post(&semMultiprocesamiento);
				break;
			case CREATE:
				cargarNuevoLQL(linea2);
				sem_wait(&semEjecutarLQL);
				ejecutarCreate(linea2);
				moverLQL(Exec,Exit);
				LQL = queue_peek(Exit);
				printf("\nLQL en Exit: %s\n", LQL->Instruccion);
				free(queue_pop(Exit));
				sem_post(&semMultiprocesamiento);
				break;
			case DESCRIBE:
				cargarNuevoLQL(linea2);
				sem_wait(&semEjecutarLQL);
				ejecutarDescribe(linea2);
				moverLQL(Exec,Exit);
				LQL = queue_peek(Exit);
				printf("\nLQL en Exit: %s\n", LQL->Instruccion);
				free(queue_pop(Exit));
				sem_post(&semMultiprocesamiento);
				break;
			case DROP:
				cargarNuevoLQL(linea2);
				sem_wait(&semEjecutarLQL);
				ejecutarDrop(linea2);
				moverLQL(Exec,Exit);
				LQL = queue_peek(Exit);
				printf("\nLQL en Exit: %s\n", LQL->Instruccion);
				free(queue_pop(Exit));
				sem_post(&semMultiprocesamiento);
				break;
			case JOURNAL:
				cargarNuevoLQL(linea2);
				sem_wait(&semEjecutarLQL);
				ejecutarJournal(linea2);
				moverLQL(Exec,Exit);
				LQL = queue_peek(Exit);
				printf("\nLQL en Exit: %s\n", LQL->Instruccion);
				free(queue_pop(Exit));
				sem_post(&semMultiprocesamiento);
				break;
			case ADD:
				cargarNuevoLQL(linea2);
				sem_wait(&semEjecutarLQL);
				ejecutarAdd(linea2);
				moverLQL(Exec,Exit);
				LQL = queue_peek(Exit);
				printf("\nLQL en Exit: %s\n", LQL->Instruccion);
				free(queue_pop(Exit));
				sem_post(&semMultiprocesamiento);
				break;
			case RUN:
				cargarNuevoLQL(linea2);
				sem_wait(&semEjecutarLQL);
				retornoRUN = ejecutarRun(linea2, 0);
				if(retornoRUN == -1){
					actualizarRequestEjecutadas();
					moverLQL(Exec,Ready);
					LQL = queue_peek(Ready);
					printf("\nLQL en Ready: %s\n", LQL->Instruccion);
					sem_post(&semContadorLQL);
				}
				else{
					moverLQL(Exec,Exit);
					LQL = queue_peek(Exit);
					printf("\nLQL en Exit: %s\n", LQL->Instruccion);
					free(queue_pop(Exit));
				}
				printf("\nRetorno de la ejecucion: %d\n",retornoRUN);
				sem_post(&semMultiprocesamiento);
				break;
			case METRICS:
				// Comando METRICS
			case -1:
				informarComandoInvalido();
				break;
			default:
				printf("Ingrese un comando \n");
				break;
		}
		free(linea2);
		linea = readline(">");
	}
	free(linea);
}

void ejecutarSelect(char* instruccion){
	puts("select ejecutado");
	char** comando ;
	comando = string_n_split(instruccion, 3, " ");
	if(comandoValido(3, comando)){
		puts("comando valido");
		tPaquete* mensaje = malloc(sizeof(tPaquete));
		mensaje->type = SELECT;
		strcpy(mensaje->payload,instruccion);
		mensaje->length = sizeof(mensaje->payload);
		enviarPaquete(socketMemoria, mensaje,logger,"Ejecutar comando SELECT desde Kernel.");
		liberarPaquete(mensaje);
	}

}
void ejecutarInsert(char* instruccion){
	puts("insert ejecutado");
	char** comando ;
	comando = string_n_split(instruccion, 4, " ");
	if(comandoValido(4, comando)){
		puts("comando valido");
		tPaquete* mensaje = malloc(sizeof(tPaquete));
		mensaje->type = INSERT;
		strcpy(mensaje->payload,instruccion);
		mensaje->length = sizeof(mensaje->payload);
		enviarPaquete(socketMemoria, mensaje,logger,"Ejecutar comando INSERT desde Kernel.");
		liberarPaquete(mensaje);
	}
}
void ejecutarCreate(char* instruccion){
	puts("create ejecutado");
	char** comando ;
	comando = string_n_split(instruccion, 5, " ");
	if(comandoValido(5, comando)){
		puts("comando valido");
		tPaquete* mensaje = malloc(sizeof(tPaquete));
		mensaje->type = CREATE;
		strcpy(mensaje->payload,instruccion);
		mensaje->length = sizeof(mensaje->payload);
		enviarPaquete(socketMemoria, mensaje,logger,"Ejecutar comando CREATE desde Kernel.");
		liberarPaquete(mensaje);
	}
}
void ejecutarDescribe(char* instruccion){
	puts("describe ejecutado");
	char** comando ;
	comando = string_n_split(instruccion, 2, " ");
	if(comandoValido(2, comando)){
		puts("comando valido");
		tPaquete* mensaje = malloc(sizeof(tPaquete));
		mensaje->type = DESCRIBE;
		strcpy(mensaje->payload,instruccion);
		mensaje->length = sizeof(mensaje->payload);
		enviarPaquete(socketMemoria, mensaje,logger,"Ejecutar comando DESCRIBE desde Kernel.");
		liberarPaquete(mensaje);

	}
}
void ejecutarDrop(char* instruccion){
	puts("drop ejecutado");
	char** comando ;
	comando = string_n_split(instruccion, 2, " ");
	if(comandoValido(2, comando)){
		puts("comando valido");
		tPaquete* mensaje = malloc(sizeof(tPaquete));
		mensaje->type = DROP;
		strcpy(mensaje->payload,instruccion);
		mensaje->length = sizeof(mensaje->payload);
		enviarPaquete(socketMemoria, mensaje,logger,"Ejecutar comando DROP desde Kernel.");
		liberarPaquete(mensaje);
	}
}
void ejecutarJournal(char* instruccion){
	puts("journal ejecutado");
	char** comando ;
	comando = string_n_split(instruccion, 1, " ");
	if(comandoValido(1, comando)){
		puts("comando valido");
		tPaquete* mensaje = malloc(sizeof(tPaquete));
		mensaje->type = JOURNAL;
		strcpy(mensaje->payload,instruccion);
		mensaje->length = sizeof(mensaje->payload);
		enviarPaquete(socketMemoria, mensaje,logger,"Ejecutar comando JOURNAL desde Kernel.");
		liberarPaquete(mensaje);
	}
}

int cadenaEsDigito(char* cadena) {
	int esDigito = 1;

	for (int i = 0; i < strlen(cadena); i++) {
		if (!isdigit(cadena[i]))
			esDigito = 0;
	}

	return esDigito;
}

int validacionStringsFijosAdd(char** comando) {
	if (!strcmp("ADD", comando[0]) && !strcmp("MEMORY", comando[1]) && !strcmp("TO", comando[3]))
		return 1;
	else
		return 0;
}

int validacionStringCriterios(char* criterio) {
	if (!strcmp("SC", criterio) || !strcmp("SHC", criterio)
			|| !strcmp("EC", criterio))
		return 1;
	else {
		printf("Criterio inválido. Los criterios válidos son: SC, SHC y EC\n");
		return 0;
	}
}

void ejecutarAdd(char* instruccion) {
	puts("add ejecutado");
	char** comando;
	comando = string_n_split(instruccion, 5, " ");
	if (comandoValido(5, comando)) {
		puts("comando valido");
		if (validacionStringsFijosAdd(comando) && cadenaEsDigito(comando[2]) && validacionStringCriterios(comando[4])) {
			printf("Comando ADD ejecutado correctamente\n");
			if (!strcmp("SC", comando[4])) {
				printf("Criterio SC\n"); //asociarACriterioSC(comando[2]);
			} else if (!strcmp("SHC", comando[4])) {
				printf("Criterio SHC\n");
			} else if (!strcmp("EC", comando[4])) {
				printf("Criterio EC\n");
			}
		} else
			printf("Error en el comando ADD. La sintaxis correcta es: ADD MEMORY [NÚMERO] TO [CRITERIO]\n");
		/*
		tPaquete* mensaje = malloc(sizeof(tPaquete));
		mensaje->type = ADD;
		strcpy(mensaje->payload,instruccion);
		mensaje->length = sizeof(mensaje->payload);
		enviarPaquete(socketMemoria, mensaje,logger,"Ejecutar comando ADD desde Kernel.");
		liberarPaquete(mensaje);*/
	}
}
int ejecutarRun(char* instruccion, int requestEjecutadas){
	puts("run ejecutado");
	char** comando ;
	FILE *script;
	char *stringLQL;
	int quantum = configuracion->QUANTUM;

	comando = string_n_split(instruccion, 2, " ");
	if(comandoValido(2, comando)){
		puts("comando valido");
		script = fopen(comando[1],"r");
		//script = fopen(PATH_SCRIPT,"r"); //para hacer pruebas
		if(script == NULL) {
			  perror("Error al abrir el script.");
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
							ejecutarSelect(stringLQL);
							break;
						case INSERT:
							/*cargarNuevoLQL(stringLQL);
							sem_wait(&semEjecutarLQL);
							ejecutarInsert(stringLQL);
							moverLQL(Exec,Exit);
							sem_post(&semMultiprocesamiento);*/
							ejecutarInsert(stringLQL);
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
							ejecutarDrop(stringLQL);
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
				return quantum;
			}
		}
		free(stringLQL);
		fclose(script);
		return quantum;
	}
	return -2;
}
