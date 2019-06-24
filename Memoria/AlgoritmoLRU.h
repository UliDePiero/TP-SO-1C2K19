/*
 * AlgoritmoLRU.h
 *
 *  Created on: 24 jun. 2019
 *      Author: utnso
 */

#ifndef ALGORITMOLRU_H_
#define ALGORITMOLRU_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>

/* --------------------    Definición de Estructuras    -------------------- */

#define CANT_MAX_PAGINAS_EN_LISTA 3

typedef struct {
	int segmentoID;
	int paginaID;
	int modificado;
} t_nodoLRU;

/* --------------------    Definición de Var. Globales    -------------------- */

t_list* listaPaginasLRU;

/* --------------------    Definición de Funciones    -------------------- */

#endif /* ALGORITMOLRU_H_ */
