/*
 * API_Kernel.c
 *
 *  Created on: 17 abr. 2019
 *      Author: utnso
 */
#include "API_Kernel.h"

void API_Kernel(void){
//int main(){
	char* linea;
	char* linea2;

	linea = readline(">");

	while(strncmp("EXIT", linea, 5)){
		linea2 = malloc(strlen(linea)+1);
		strcpy(linea2, linea);
		switch(parser(linea)){
			case SELECT:
				ejecutarSelect(linea2);
				break;
			case INSERT:
				ejecutarInsert(linea2);
				break;
			case CREATE:
				ejecutarCreate(linea2);
				break;
			case DESCRIBE:
				ejecutarDescribe(linea2);
				break;
			case DROP:
				ejecutarDrop(linea2);
				break;
			case JOURNAL:
				ejecutarJournal(linea2);
				break;
			case ADD:
				ejecutarAdd(linea2);
				break;
			case RUN:
				ejecutarRun(linea2);
				break;
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
void ejecutarAdd(char* instruccion){
	puts("add ejecutado");
	char** comando ;
	comando = string_n_split(instruccion, 2, " ");
	if(comandoValido(2, comando)){
		puts("comando valido");
		tPaquete* mensaje = malloc(sizeof(tPaquete));
		mensaje->type = ADD;
		strcpy(mensaje->payload,instruccion);
		mensaje->length = sizeof(mensaje->payload);
		enviarPaquete(socketMemoria, mensaje,logger,"Ejecutar comando ADD desde Kernel.");
		liberarPaquete(mensaje);
	}
}
void ejecutarRun(char* instruccion){
	puts("run ejecutado");
	char** comando ;
	FILE *script;
	char *stringLQL;

	comando = string_n_split(instruccion, 2, " ");
	if(comandoValido(2, comando)){
		puts("comando valido");
		script = fopen(comando[1],"r");
		//script = fopen(PATH_SCRIPT,"r"); //para hacer pruebas
		if(script == NULL) {
			  perror("Error al abrir el script.");
			  return;
		}
		stringLQL = (char*)malloc(100);
		while(fgets(stringLQL, 100, script)!=NULL){
			printf("\n%s\n",stringLQL);
			switch(parserSinTrim(stringLQL)){
					case SELECT:
						ejecutarSelect(stringLQL);
						break;
					case INSERT:
						ejecutarInsert(stringLQL);
						break;
					case CREATE:
						ejecutarCreate(stringLQL);
						break;
					case DESCRIBE:
						ejecutarDescribe(stringLQL);
						break;
					case DROP:
						ejecutarDrop(stringLQL);
						break;
					case JOURNAL:
						ejecutarJournal(stringLQL);
						break;
					case ADD:
						ejecutarAdd(stringLQL);
						break;
					case -1:
						informarComandoInvalido();
						break;
					default:
						printf("Es un comentario o fin de linea \n");
						break;
				}
		}
		free(stringLQL);
		fclose(script);
	}
}
