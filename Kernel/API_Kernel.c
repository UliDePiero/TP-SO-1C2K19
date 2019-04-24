/*
 * API_Kernel.c
 *
 *  Created on: 17 abr. 2019
 *      Author: utnso
 */
#include "API_Kernel.h"



int api_kernel(void){
	char* linea;
	linea = readline(">");
	while(1){
		leerLinea(linea);
		linea = readline(">");
	}
}

void leerLinea(char* linea){
	char** comando;

	string_trim(&linea);
	if(string_starts_with(linea, "SELECT ")){
		comando = string_n_split(linea, 3, " ");
		if(comandoValido(3, comando)){
			puts("comando valido");
		}
	}
	else if(string_starts_with(linea, "INSERT ")){
		comando = string_n_split(linea, 4, " ");
		if(comandoValido(4, comando)){
			puts("comando valido");
		}
	}
	else if(string_starts_with(linea, "CREATE ")){
		comando = string_n_split(linea, 5, " ");
		if(comandoValido(5, comando)){
			puts("comando valido");
		}
	}
	else if(string_starts_with(linea, "DESCRIBE ") ){
		comando = string_n_split(linea, 2, " ");
		if(comandoValido(2, comando)){
			puts("comando valido");
		}
	}
	else if(string_starts_with(linea, "DROP ")){
		comando = string_n_split(linea, 2, " ");
		if(comandoValido(2, comando)){
			puts("comando valido");
		}
	}
	else if(!strcmp(linea, "JOURNAL")){
		puts("comando valido");
	}
	else if(string_starts_with(linea, "ADD ")){
		comando = string_n_split(linea, 5, " ");
		if(comandoValido(5, comando)){
			puts("comando valido");
		}
	}
	else if(string_starts_with(linea, "RUN ")){
		comando = string_n_split(linea, 2, " ");
		if(comandoValido(2, comando)){
			puts("comando valido");
		}
	}
	else if(!strcmp(linea, "METRICS")){
		puts("comando valido");
	}
	else if(!string_is_empty(linea)){
		informarComandoInvalido();
	}

	//free(comando);
	free(linea);
}

int comandoValido(int inputs, char** comando){
	/*printf("comando[0]: %s \n", comando[0]);
	printf("comando[1]: %s \n", comando[1]);
	printf("comando[2]: %s \n", comando[2]);
	printf("comando[3]: %s \n", comando[3]);*/
	int valido = 1;

	for(int i = 1; i<inputs; i++){
		if(string_is_empty(&comando[i])){
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
	puts("ERROR: Comando no valido");
}
