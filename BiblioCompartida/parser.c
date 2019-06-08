/*
 * parser.c
 *
 *  Created on: 27 abr. 2019
 *      Author: utnso
 */
#include "parser.h"

void parserFinDeLinea(char * instruccion, int i, char * buffer) {
	int j = 0;
	while (instruccion[i] != '\0') {
		buffer[j] = instruccion[i];
		i++;
		j++;
	}
	buffer[j] = '\0';
}

int parserEspacio(char * instruccion, int i, char * buffer) {
	int j = 0;
	while (instruccion[i] != ' ') {
		buffer[j] = instruccion[i];
		i++;
		j++;
	}
	buffer[j] = '\0';
	return i;
}

/*int parser(char * instruccion){
 char l1, l2;
 l1 = instruccion[0];
 if (l1 != '#' && l1 != ' '){
 //parserEspacio(instruccion, 0, buffer);
 // printf("Palabra reservada: %s \n", buffer);
 l2 = instruccion[1];
 if(l1 == 'S')
 return SELECT;
 if(l1 == 'I')
 return INSERT;
 if(l1 == 'C')
 return CREATE;
 if(l1 == 'D' && l2 == 'E')
 return DESCRIBE;
 if(l1 == 'D' && l2 == 'R')
 return DROP;
 if(l1 == 'J')
 return JOURNAL;
 if(l1 == 'A')
 return ADD;
 if(l1 == 'R')
 return RUN;
 if(l1 == 'M')
 return METRICS;
 }else if (l1 == '#')
 return 0;
 return -1;
 }*/
int parser(char* linea) {
	string_trim(&linea);
	if (string_starts_with(linea, "SELECT ")) //if(!strncmp("SELECT ", linea, 7))
		return SELECT;
	if (string_starts_with(linea, "INSERT ")) //if(!strncmp("INSERT ", linea, 7))
		return INSERT;
	if (string_starts_with(linea, "CREATE ")) //if(!strncmp("CREATE ", linea, 7))
		return CREATE;
	if (string_starts_with(linea, "DESCRIBE ")) //if(!strncmp("DESCRIBE ", linea, 9)) // TODO: Admitir DESCRIBE y DESCRIBE [TABLA]
		return DESCRIBE;
	if (string_starts_with(linea, "DROP ")) //if(!strncmp("DROP ", linea, 5))
		return DROP;
	if (!strcmp("JOURNAL", linea))
		return JOURNAL;
	if (string_starts_with(linea, "ADD ")) //if(!strncmp("ADD ", linea, 4))
		return ADD;
	if (string_starts_with(linea, "RUN ")) //if(!strncmp("RUN ", linea, 4))
		return RUN;
	if (!strcmp("METRICS", linea))
		return METRICS;
	if (string_is_empty(linea))
		return 0;
	return -1;
}
int parserSinTrim(char* linea) {
	if (!strncmp("SELECT ", linea, 7))
		return SELECT;
	if (!strncmp("INSERT ", linea, 7))
		return INSERT;
	if (!strncmp("CREATE ", linea, 7))
		return CREATE;
	if (!strncmp("DESCRIBE ", linea, 9))
		return DESCRIBE;
	if (!strncmp("DROP ", linea, 5))
		return DROP;
	if (!strcmp("JOURNAL", linea))
		return JOURNAL;
	if (!strncmp("ADD ", linea, 4))
		return ADD;
	if (!strncmp("RUN ", linea, 4))
		return RUN;
	if (!strcmp("METRICS", linea))
		return METRICS;
	if (string_is_empty(linea))
		return 0;
	return -1;
}
int leerLinea(char* linea) {
	char** comando;

	string_trim(&linea);
	if (string_starts_with(linea, "SELECT ")) {
		comando = string_n_split(linea, 3, " ");
		if (comandoValido(3, comando)) {
			puts("comando valido");
			return SELECT;
		}
	} else if (string_starts_with(linea, "INSERT ")) {
		comando = string_n_split(linea, 4, " ");
		if (comandoValido(4, comando)) {
			puts("comando valido");
			return INSERT;
		}
	} else if (string_starts_with(linea, "CREATE ")) {
		comando = string_n_split(linea, 5, " ");
		if (comandoValido(5, comando)) {
			puts("comando valido");
			return CREATE;
		}
	} else if (string_starts_with(linea, "DESCRIBE ")) {
		comando = string_n_split(linea, 2, " ");
		if (comandoValido(2, comando)) {
			puts("comando valido");
			return DESCRIBE;
		}
	} else if (string_starts_with(linea, "DROP ")) {
		comando = string_n_split(linea, 2, " ");
		if (comandoValido(2, comando)) {
			puts("comando valido");
			return DROP;
		}
	} else if (!strcmp(linea, "JOURNAL")) {
		puts("comando valido");
		return JOURNAL;
	} else if (string_starts_with(linea, "ADD ")) {
		comando = string_n_split(linea, 5, " ");
		if (comandoValido(5, comando)) {
			puts("comando valido");
			return ADD;
		}
	} else if (string_starts_with(linea, "RUN ")) {
		comando = string_n_split(linea, 2, " ");
		if (comandoValido(2, comando)) {
			puts("comando valido");
			return RUN;
		}
	} else if (!strcmp(linea, "METRICS")) {
		puts("comando valido");
		return METRICS;
	} else if (!string_is_empty(linea)) {
		informarComandoInvalido();
		return -1;
	}

	//free(comando);
	free(linea);
	return -1;
}

int comandoValido(int inputs, char** comando) {
	int valido = 1;
	for (int i = 1; i < inputs; i++) {
		if (string_is_empty(comando[i])) {
			valido = 0;
			break;
		}
	}
	/*if (valido) {
		char** subComando = string_n_split(comando[inputs - 1], 2, " ");
		if (!string_is_empty(&subComando[1])) {
			valido = 0;
		}
		free(subComando);
	}*/
	if (!valido) {
		informarComandoInvalido();
	}
	return valido;
}
void informarComandoInvalido() {
	printf("\nERROR: Comando no valido\n");
	return;
}

int cadenaEsDigito(char* cadena) {
	int esDigito = 1;

	for (int i = 0; i < string_length(cadena); i++) {
		if (!isdigit(cadena[i]))
			esDigito = 0;
	}

	return esDigito;
}

int validacionStringsFijosAdd(char** comando) {
	if (!strcmp("ADD", comando[0]) && !strcmp("MEMORY", comando[1])
			&& !strcmp("TO", comando[3]))
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
