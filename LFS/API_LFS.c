/*
 * API_LFS.c
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */
#include "API_LFS.h"

void API_LFS(){
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
			case -1:
				informarComandoInvalido();
				break;
			default:
				//printf("Es un comentario o fin de linea \n");
				break;
		}
		free(linea2);
		linea = readline(">");
	}
	free(linea);
}
/*int ejecutarInstruccion(char * instruccion){
	int opcion = 0;
	opcion = parser(instruccion);
	switch(opcion){
		case SELECT:
			ejecutarSelect(instruccion);
			break;
		case INSERT:
			ejecutarInsert(instruccion);
			break;
		case CREATE:
			ejecutarCreate(instruccion);
			break;
		case DESCRIBE:
			ejecutarDescribe(instruccion);
			break;
		case DROP:
			ejecutarDrop(instruccion);
			break;

		default:
			//printf("Es un comentario o fin de linea \n");
			break;
	}
	return opcion;
}*/
void ejecutarSelect(char* instruccion){
	puts("select ejecutado");
	char** comando ;
	comando = string_n_split(instruccion, 3, " ");
	if(comandoValido(3, comando))
		puts("comando valido");
}
void ejecutarInsert(char* instruccion){
	puts("insert ejecutado");
	char** comando ;
	comando = string_n_split(instruccion, 4, " ");
	if(comandoValido(4, comando))
		puts("comando valido");
}
void ejecutarCreate(char* instruccion){
	puts("create ejecutado");
	char** comando ;
	comando = string_n_split(instruccion, 5, " ");
	if(comandoValido(5, comando))
		puts("comando valido");
}
void ejecutarDescribe(char* instruccion){
	puts("describe ejecutado");
	char** comando ;
	comando = string_n_split(instruccion, 2, " ");
	if(comandoValido(2, comando))
		puts("comando valido");
}
void ejecutarDrop(char* instruccion){
	puts("drop ejecutado");
	char** comando ;
	comando = string_n_split(instruccion, 2, " ");
	if(comandoValido(2, comando))
		puts("comando valido");
}
