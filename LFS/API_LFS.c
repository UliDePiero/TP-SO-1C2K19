/*
 * API_LFS.c
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */
#include "API_LFS.h"

void* API_LFS(){
	line = readline(">");

	while(strncmp("EXIT", line, 5)){
		instruccion = string_from_format("%s", line);
		switch(parser(line)){
			case SELECT:
				ejecutarSelect();
				break;
			case INSERT:
				ejecutarInsert();
				break;
			case CREATE:
				ejecutarCreate();
				break;
			case DESCRIBE:
				ejecutarDescribe();
				break;
			case DROP:
				ejecutarDrop();
				break;
			case -1:
				informarComandoInvalido();
				break;
			default:
				//printf("Es un comentario o fin de linea \n");
				break;
		}
		free(line);
		free(instruccion);
		line = readline(">");
	}
	free(line);
	destruirLFS();
	return (void*)1;
}

void ejecutarSelect(){
	char** comando = validarComando(line, 3); //*************************************POR QUE LINE Y NO INSTRUCCION?*************************************
	if(comando){
		selectLFS(comando[1], atoi(comando[2]));
		for(int i = 0; i<3; i++)
			free(comando[i]);
		free(comando);
	}
}
void ejecutarInsert(){
	char** comando = validarComandoInsert(instruccion);
	if(comando){
		if(!comando[4]){
			insertLFS(comando[1], atoi(comando[2]), comando[3], getCurrentTime());
		}else{
			insertLFS(comando[1], atoi(comando[2]), comando[3], atoi(comando[4]));
			free(comando[4]);
		}
		for(int i = 0; i<4; i++)
			free(comando[i]);
		free(comando);
	}
}
void ejecutarCreate(){
	char** comando = validarComando(instruccion, 5);
	if(comando){
		createLFS(comando[1], comando[2], atoi(comando[3]), atol(comando[4]));
		for(int i = 0; i<5; i++)
			free(comando[i]);
		free(comando);
	}
}
void ejecutarDescribe(){
	char** comando = string_n_split(instruccion, 2, " ");
	if(comando[1]){
		describeLFS(comando[1]);
		free(comando[1]);
	}else{
		describeLFS(NULL);
	}
	free(comando[0]);
	free(comando);
}
void ejecutarDrop(){
	char** comando = validarComando(instruccion, 2);
	if(comando){
		dropLFS(comando[1]);
		for(int i = 0; i<2; i++)
			free(comando[i]);
		free(comando);
	}
}


