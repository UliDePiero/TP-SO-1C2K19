/*
 * API_Kernel.c
 *
 *  Created on: 17 abr. 2019
 *      Author: utnso
 */
#include "API_Kernel.h"


int ejecutarInstruccion(char * instruccion){
	int opcion = 0;
	opcion = parser (instruccion);
	//opcion = leerlinea (instruccion); PROBAR CON ESTA OTRA OPCION
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
		case ADD:
			//ejecutarAdd(instruccion);
			break;

		default:
			//printf("Es un comentario o fin de linea \n");
			break;
	}
	return opcion;
}

int api_kernel(void){
	char* linea;
	linea = readline(">");
	while(1){
		leerLinea(linea);
		linea = readline(">");
	}

}
