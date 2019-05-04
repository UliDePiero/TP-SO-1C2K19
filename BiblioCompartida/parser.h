/*
 * parser.h
 *
 *  Created on: 27 abr. 2019
 *      Author: utnso
 */

#ifndef PARSER_H_
#define PARSER_H_
#include "protocolo.h"
#include <commons/string.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//Devuelve la instruccion a parsear. Si retorna 0 es un comentario. Si retorna -1 es fin de archivo
int parser(char * instruccion);
//Genera un String hasta que encuentra un espacio, devuelve la posicion hasta donde escribio el String
int parserEspacio(char * instruccion, int i, char * buffer);
//Genera un String hasta que encuentra el fin de linea
void parserFinDeLinea(char * instruccion, int i, char * buffer);
//Devuelve la instruccion a parsear. Retorna -1 en error o comando invalido
//void leerLinea(char* linea);
//int leerLinea(char* linea);


int leerlineas(char* linea, int tipo);
//Retorna si el comando es valido
int comandoValido(int inputs, char** comando);
//Informa que el comando ingresado no es valido
void informarComandoInvalido();

#endif /* PARSER_H_ */
