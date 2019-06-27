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

int memoriaEstaFull(t_list* lista);
void mostrarlistaPaginasLRU(t_list* lista);
void encolarNuevaPagina(t_list* lista, t_nodoLRU* nodo);
void encolarPaginaExistente(t_list* lista, t_link_element* nodo);
void desencolarPrimerElementoNoModificado(t_list *lista);
int estaEnListaDePaginas(t_list* lista, t_nodoLRU* nodo);
t_list* insertarEnListaDePaginasLRU(t_list* lista, t_nodoLRU* nodo);

#endif /* ALGORITMOLRU_H_ */
