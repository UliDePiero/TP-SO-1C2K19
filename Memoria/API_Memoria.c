/*
 * API_Memoria.C
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */
#include "API_Memoria.h"

void API_Memoria(){
	char* linea;

	linea = readline(">");

	while(strncmp("EXIT", linea, 5)){
		ejecutarInstruccion(linea);
		//free(linea);
		linea = readline(">");

	}
	//free(linea);
}
int ejecutarInstruccion(char * instruccion){
	int opcion = 0;
	opcion = parser (instruccion);
	switch(opcion){
		case SELECT:
			//ejecutarSelect(instruccion);
			break;
		case INSERT:
			//ejecutarInsert(instruccion);
			break;
		case CREATE:
			//ejecutarCreate(instruccion);
			break;
		case DESCRIBE:
			//ejecutarDescribe(instruccion);
			break;
		case DROP:
			//ejecutarDrop(instruccion);
			break;
		case JOURNAL:
			//ejecutarJournal(instruccion);
			break;

		default:
			//printf("Es un comentario o fin de linea \n");
			break;
	}
	return opcion;
}
