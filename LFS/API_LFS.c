/*
 * API_LFS.c
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */


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

		default:
			//printf("Es un comentario o fin de linea \n");
			break;
	}
	return opcion;
}
