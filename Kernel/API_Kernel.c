/*
 * API_Kernel.c
 *
 *  Created on: 17 abr. 2019
 *      Author: utnso
 */
#include "API_Kernel.h"



int api_kernel(void){
	char* linea;
	char** comando;
	linea = readline(">");
	while(1){
		string_trim(&linea);
		if(string_starts_with(linea, "SELECT ")){
			comando = string_n_split(linea, 3, " ");
			if(comandoValido(3, comando)){
				printf("comando valido\n");
			}
		}
		else if(string_starts_with(linea, "INSERT")){
			comando = string_n_split(linea, 4, " ");
			if(comandoValido(4, comando)){
				printf("comando valido\n");
			}
		}
		else if(string_starts_with(linea, "CREATE")){
			comando = string_n_split(linea, 5, " ");
			if(comandoValido(5, comando)){
				printf("comando valido\n");
			}
		}
		else if(string_starts_with(linea, "DESCRIBE") ){
			comando = string_n_split(linea, 2, " ");
			if(comandoValido(2, comando)){
				printf("comando valido\n");
			}
		}
		else if(string_starts_with(linea, "DROP")){
			comando = string_n_split(linea, 2, " ");
			if(comandoValido(2, comando)){
				printf("comando valido\n");
			}
		}
		else if(string_starts_with(linea, "JOURNAL")){
			if(comandoValido(1, comando)){
				printf("comando valido\n");
			}
		}
		else if(string_starts_with(linea, "ADD")){
			comando = string_n_split(linea, 5, " ");
			if(comandoValido(5, comando)){
				printf("comando valido\n");
			}
		}
		else if(string_starts_with(linea, "RUN")){
			comando = string_n_split(linea, 2, " ");
			if(comandoValido(2, comando)){
				printf("comando valido\n");
			}
		}
		else if(string_starts_with(linea, "METRICS")){
			if(comandoValido(1, comando)){
				printf("comando valido\n");
			}
		}
		else if(!string_is_empty(linea)){
			informarComandoInvalido();
		}
		//free(comando);
		free(linea);
		linea = readline(">");
	}
}


int comandoValido(int inputs, char** comando){
	/*printf("comando[0]: %s \n", comando[0]);
	printf("comando[1]: %s \n", comando[1]);
	printf("comando[2]: %s \n", comando[2]);
	printf("comando[3]: %s \n", comando[3]);*/
	int valido = 1;

	for(int i = 1; i<inputs; i++){
		if(string_is_empty(&comando[i])){
			printf("entro %d \n", i);
			valido = 0;
			break;
		}
	}
	if(valido){
		char** subComando = string_n_split(comando[inputs-1], 2, " ");
		if(!string_is_empty(&subComando[1])){
			valido = 0;
		}
		free(subComando);
	}

	if(!valido){
		informarComandoInvalido();
	}
	return valido;
}

void informarComandoInvalido(){
	printf("ERROR: Comando no valido \n");
}
