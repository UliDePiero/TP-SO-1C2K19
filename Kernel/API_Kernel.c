/*
 * API_Kernel.c
 *
 *  Created on: 17 abr. 2019
 *      Author: utnso
 */
#include "API_Kernel.h"

//void API_Kernel(void){
main(){
	char* linea;
	char* linea2;

		linea = readline(">");
		linea2 = malloc(strlen(linea)+1);
		strcpy(linea2, linea);

		while(strncmp("EXIT", linea, 5)){
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
					//ejecutarAdd(linea2);
					break;
				case RUN:
					ejecutarRun(linea2);
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
			linea2 = malloc(strlen(linea)+1);
			strcpy(linea2, linea);
		}
		free(linea2);
		free(linea);
}
int ejecutarInstruccion(char * instruccion){
	int opcion = 0;
	opcion = parser (instruccion);
	//opcion = leerlinea (instruccion); PROBAR CON ESTA OTRA OPCION
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
		case JOURNAL:
			ejecutarJournal(instruccion);
			break;
		case ADD:
			//ejecutarAdd(instruccion);
			break;
		case RUN:
			ejecutarRun(instruccion);
			break;

		default:
			//printf("Es un comentario o fin de linea \n");
			break;
	}
	return opcion;
}
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
void ejecutarJournal(char* instruccion){
	puts("journal ejecutado");
	//char** comando ;
	//comando = string_n_split(instruccion, 2, " ");
	//if(comandoValido(1, comando))
		puts("comando valido");
}
/*void ejecutarAdd(char* instruccion){
	puts("add ejecutado");
	char** comando ;
	comando = string_n_split(instruccion, 2, " ");
	if(comandoValido(2, comando))
		puts("comando valido");
}*/
void ejecutarRun(char* instruccion){
	puts("run ejecutado");
	char** comando ;
	comando = string_n_split(instruccion, 2, " ");
	if(comandoValido(2, comando)){
		puts("comando valido");
		script = fopen(comando[1],"r");
		//script = fopen(PATH_SCRIPT,"r"); //para hacer pruebas
		if(script == NULL) {
			  perror("Error al abrir el script.");
			  return;
		}
		while(fgets(stringLQL, 100, script)){
			printf("%s\n",comando[1]);
		}
		fclose(script);
	}
}
