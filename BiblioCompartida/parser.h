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
#include <ctype.h>

/* --------------------    Definición de Funciones    -------------------- */

//Devuelve la instruccion a parsear. Si retorna 0 es un comentario. Si retorna -1 es fin de archivo
int parser(char * instruccion);
int parserSinTrim(char * instruccion);
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
char** validarComando(char* instruccion, int inputs);
char** validarComandoInsert(char* instruccion);

int cadenaEsDigito(char* cadena);
int validacionStringsFijosAdd(char** comando);
int validacionStringCriterios(char* criterio);

void str_to_uint16(const char *str, uint16_t *res);
void swap(char *x, char *y);
char* reverse(char *buffer, int i, int j);
char* itoa(int value, char* buffer, int base);

//Retorna el tiempo actual en milisegundos
uint64_t getCurrentTime();

#endif /* PARSER_H_ */
