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
				free(instruccion_API);
				break;
			default:
				printf("Ingrese un comando \n");
				free(instruccion_API);
				break;
		}
		free(line);
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
		sleep(configuracion->RETARDO_MEM / 1000);
		if(registro != NULL)
		{
			printf("value: %s\n", registro->value);
			retorno = string_duplicate(registro->value);
			free(registro->value);
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
	char** comando = validarComando(instruccion, 4);
	if(comando){
		sleep(configuracion->RETARDO_MEM / 1000);
		insertMemoria(comando[1], atoi(comando[2]), comando[3], getCurrentTime());
		retorno = string_from_format("Tabla:%s Key:%d Value:%s",comando[1], atoi(comando[2]), comando[3]);
		for(int i = 0; i<4; i++)
			free(comando[i]);
		free(comando);
	}
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
		retorno = string_from_format("Tabla:%s Consistencia:%s Particiones:%d Tiempo compactacion:%d",comando[1], comando[2], atoi(comando[3]), atoi(comando[4]));
		for(int i = 0; i<5; i++)
			free(comando[i]);
		free(comando);
	}
	free(instruccion);
	return retorno;
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
char* ejecutarDrop(char* instruccion){
	char* retorno = NULL;
	puts("drop ejecutado");
	char** comando = validarComando(instruccion, 2);
	if(comando){
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
	puts("journal ejecutado");
	int retorno = 0;
	char** comando = validarComando(instruccion, 1);
	if(comando){
		sleep(configuracion->RETARDO_MEM / 1000);
		journalMemoria();
		free(comando[0]);
		free(comando);
		retorno = 1;
	}
	free(instruccion);
	return retorno;
}
