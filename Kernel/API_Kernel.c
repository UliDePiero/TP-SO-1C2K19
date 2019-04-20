/*
 * API_Kernel.c
 *
 *  Created on: 17 abr. 2019
 *      Author: utnso
 */
#include "API_Kernel.h"

int main(void){
	char *linea;
	linea = readline(">");

	while(1){
		char** comando = string_n_split(string_trim(linea), 6, " ");
		if(string_starts_with("SELECT ", comando[0])){
			if(comandoValido(3, comando)){

			}
		}
		else if(string_starts_with("INSERT ",comando[0])){
			if(comandoValido(4, comando)){

			}
		}
		else if(string_starts_with("CREATE ",comando[0])){
			if(comandoValido(5, comando)){

			}
		}
		else if(string_starts_with("DESCRIBE ",comando[0]) ){
			if(comandoValido(2, comando)){

			}
		}
		else if(string_starts_with("DROP ",comando[0])){
			if(comandoValido(2, comando)){

			}
		}
		else if(string_starts_with("JOURNAL ",comando[0])){
			if(comandoValido(1, comando)){

			}
		}
		else if(string_starts_with("ADD ",comando[0])){
			if(comandoValido(5, comando)){

			}
		}
		else if(string_starts_with("RUN ",comando[0])){
			if(comandoValido(2, comando)){

			}
		}
		else if(string_starts_with("METRICS ",comando[0])){
			if(comandoValido(1, comando)){

			}
		}
		else{
			informarComandoInvalido();
		}
		free(comando);
		free(linea);
		linea = readline(">");
	}
}


int comandoValido(int inputs, char** comando){
	int valido = 1;
	for(int i = 1; i<inputs; i++){
		if(string_is_empty(comando[i])){
			valido = 0;
		}
	}
	if(!string_is_empty(comando[inputs])){
		valido = 0;
	}

	if(!valido){
		informarComandoInvalido();
	}
	return valido;
}

void informarComandoInvalido(){
	printf("ERROR: Comando no valido");
}
