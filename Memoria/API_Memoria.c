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

	line = readline(">");

	while(strncmp("EXIT", line, 5)){
		//linea2 = malloc(strlen(linea)+1);
		//strcpy(linea2, linea);
		instruccion_API = string_duplicate(line);
		switch(parser(line)){
			case SELECT:
				ejecutarSelect(instruccion_API);
				break;
			case INSERT:
				ejecutarInsert(instruccion_API);
				break;
			case CREATE:
				ejecutarCreate(instruccion_API);
				break;
			case DESCRIBE:
				ejecutarDescribe(instruccion_API);
				break;
			case DROP:
				ejecutarDrop(instruccion_API);
				break;
			case JOURNAL:
				ejecutarJournal(instruccion_API);
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
		line = readline(">");
	}
	free(line);
	terminar();
	return (void*)1;
}
void ejecutarSelect(char* instruccion){
	char** comando = validarComando(instruccion, 3);
	if(comando){
		Registro* registro = selectMemoria(comando[1], atoi(comando[2]));
		sleep(configuracion->RETARDO_MEM / 1000);
		if(registro != NULL) printf("value: %s\n", registro->value);
		free(registro);
		for(int i = 0; i<3; i++)
			free(comando[i]);
		free(comando);
	}
}
void ejecutarInsert(char* instruccion){
	char** comando = validarComando(instruccion, 4);
	if(comando){
		sleep(configuracion->RETARDO_MEM / 1000);
		insertMemoria(comando[1], atoi(comando[2]), comando[3], getCurrentTime());
		for(int i = 0; i<4; i++)
			free(comando[i]);
		free(comando);
	}
}
void ejecutarCreate(char* instruccion){
	char** comando = validarComando(instruccion, 5);
	if(comando){
		tPaquete* mensaje = malloc(sizeof(tPaquete));
		mensaje->type = CREATE;
		strcpy(mensaje->payload,instruccion);
		mensaje->length = sizeof(mensaje->payload);
		enviarPaquete(socketLFS, mensaje,logger,"Ejecutar comando CREATE desde Memoria.");
		liberarPaquete(mensaje);
		for(int i = 0; i<5; i++)
			free(comando[i]);
		free(comando);
	}
}
void ejecutarDescribe(char* instruccion){
	puts("describe ejecutado");
	char** comando = validarComando(instruccion, 2);
	if(comando){
		puts("comando valido");
		for(int i = 0; i<2; i++)
			free(comando[i]);
		free(comando);
	}
}
void ejecutarDrop(char* instruccion){
	puts("drop ejecutado");
	char** comando = validarComando(instruccion, 2);
	if(comando){
		puts("comando valido");
		for(int i = 0; i<2; i++)
			free(comando[i]);
		free(comando);
	}
}
void ejecutarJournal(char* instruccion){
	puts("journal ejecutado");
	char** comando = validarComando(instruccion, 1);
	if(comando){
		sleep(configuracion->RETARDO_MEM / 1000);
		journalMemoria();
		free(comando[0]);
		free(comando);
	}
}
