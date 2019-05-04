/*
 * parser.h
 *
 *  Created on: 27 abr. 2019
 *      Author: utnso
 */

#ifndef PARSER_H_
#define PARSER_H_
#include "protocolo.h"

//Devuelve la instruccion a parsear. Si retorna 0 es un comentario. Si retorna -1 es fin de archivo
int parser(char * instruccion);
//Genera un String hasta que encuentra un espacio, devuelve la posicion hasta donde escribio el String
int parserEspacio(char * instruccion, int i, char * buffer);
//Genera un String hasta que encuentra el fin de linea
void parserFinDeLinea(char * instruccion, int i, char * buffer);
//Devuelve la instruccion a parsear. Retorna -1 en error o comando invalido
//void leerLinea(char* linea);
int leerLinea(char* linea);
int comandoValido(int inputs, char** comando);
void informarComandoInvalido();

#endif /* PARSER_H_ */
