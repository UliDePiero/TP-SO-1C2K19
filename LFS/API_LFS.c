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

void ejecutarSelect(char* instruccion){
	puts("select ejecutado");
	char** comando ;
	RegistroLFS* registro;
	comando = string_n_split(instruccion, 3, " ");
	if(comandoValido(3, comando)){
		registro = selectLFS(comando[1], atoi(comando[2]));
		printf("value: %d\n", registro->value);
	}
}
void ejecutarInsert(char* instruccion){
	puts("insert ejecutado");
	char** comando ;
	comando = string_n_split(instruccion, 5, " ");
	if(comandoValido(4, comando))
		insertLFS(comando[1], atoi(comando[2]), atoi(comando[3]), (int)time(NULL));
	else
		insertLFS(comando[1], atoi(comando[2]), atoi(comando[3]), atoi(comando[4]));
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
