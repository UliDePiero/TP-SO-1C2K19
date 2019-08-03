/*
 * AlgoritmoLRU.c
 *
 *  Created on: 24 jun. 2019
 *      Author: utnso
 */

#include "AlgoritmoLRU.h"

/* --------------------    Definición de Funciones    -------------------- */

int memoriaEstaFull(t_list* lista) {
	t_link_element* nodo = lista->head;
	t_nodoLRU* nodoLRUAux;

	if (lista->elements_count == cantidadDeRegistros) {
		while (nodo) {
			nodoLRUAux = nodo->data;
			if (nodoLRUAux->modificado == 0)
				return 0; // Por lo menos hay un nodo sin modificar, entonces la memoria no está en estado FULL
			nodo = nodo->next;
		}
	} else
		// Cantidad de elementos de la lista < Cantidad máxima de registros
		return 0; // Tengo espacio restante para agregar más Páginas

	return 1; // La lista está llena y todos sus nodos fueron modificados
}

void mostrarlistaPaginasLRU(t_list* lista) { // SÓLO PARA PRUEBAS
	t_link_element* nodo = lista->head;
	t_nodoLRU* nodoLRUAux;

	if (lista->head == NULL)
		printf("Lista LRU vacía\n");
	else {
		printf("Lista LRU:\n");
		while (nodo) {
			nodoLRUAux = nodo->data;
			printf("SegmentoID: %d - PáginaID: %d - Flag Modificado: %d \n",
					nodoLRUAux->segmentoID, nodoLRUAux->paginaID,
					nodoLRUAux->modificado);
			nodo = nodo->next;
		}
		printf("\n");
	}
}

void encolarNuevaPagina(t_list* lista, t_nodoLRU* nodo) {
	// Añadimos el nuevo nodo al final de la lista
	list_add(lista, nodo);
}

void encolarPaginaExistente(t_list* lista, t_link_element* nodo) {
	t_link_element* nodoActual = nodo->next; // Parto desde el nodo en el que me encontraba

	while (nodoActual->next) // Ciclo hasta que llega al último nodo
		nodoActual = nodoActual->next;

	nodoActual->next = nodo;
	nodo->next = NULL;
}

void removerElemento(t_list* lista, t_nodoLRU* nodo) {
	t_link_element* nodoActual = lista->head;
	t_link_element* nodoAnterior = NULL;
	t_nodoLRU* nodoLRUAux;

	if (nodoActual)
		nodoLRUAux = nodoActual->data;
	while (nodoActual && (nodoLRUAux->segmentoID != nodo->segmentoID || nodoLRUAux->paginaID != nodo->paginaID))
	{
		nodoAnterior = nodoActual;
		nodoActual = nodoActual->next;
		if (nodoActual)
			nodoLRUAux = nodoActual->data;
	}
	if (nodoActual && (nodoLRUAux->segmentoID == nodo->segmentoID && nodoLRUAux->paginaID == nodo->paginaID)) {
		if (!nodoAnterior)
			lista->head = nodoActual->next;
		else
			nodoAnterior->next = nodoActual->next;
		lista->elements_count--;
		free(nodoActual);
	}
	free(nodo);
}

t_nodoLRU* desencolarPrimerElementoNoModificado(t_list *lista) {
	t_link_element* nodoActual = lista->head;
	t_link_element* nodoAnterior = NULL;
	t_nodoLRU* nodoLRUAux;

	if (nodoActual) {
		nodoLRUAux = nodoActual->data;
		while (nodoActual && nodoLRUAux->modificado != 0) {
			nodoAnterior = nodoActual;
			nodoActual = nodoActual->next;
			if (nodoActual)
				nodoLRUAux = nodoActual->data;
		}

		if (nodoLRUAux->modificado == 0) {
			if (!nodoAnterior)
				lista->head = nodoActual->next;
			else
				nodoAnterior->next = nodoActual->next;
			lista->elements_count--;
			free(nodoActual);
			return nodoLRUAux;
		}
	}
	return NULL;
}

int estaEnListaDePaginas(t_list* lista, t_nodoLRU* nodo) {
	mostrarlistaPaginasLRU(listaPaginasLRU);
	//printf("BUSCO: seg %d pag %d", nodo->segmentoID, nodo->paginaID);
	t_link_element* nodoActual = lista->head;
	t_nodoLRU* nodoLRUAux;

	if (nodoActual)
		nodoLRUAux = nodoActual->data;

	while (nodoActual
			&& (nodoLRUAux->segmentoID != nodo->segmentoID
					|| nodoLRUAux->paginaID != nodo->paginaID)) {
		nodoActual = nodoActual->next;
		if (nodoActual)
			nodoLRUAux = nodoActual->data;
	}
	if (nodoActual)
		return 1;
	else
		return 0;
}

t_nodoLRU* insertarEnListaDePaginasLRU(t_list* lista, t_nodoLRU* nodo) {
	t_nodoLRU* nodoLRU;
	if (estaEnListaDePaginas(lista, nodo)) {
		//puts("ENCONTRE");
		// Busca la página y actualiza su posición en la lista
		t_link_element* nodoActual = lista->head;
		t_link_element* nodoAnterior = NULL;
		t_nodoLRU* nodoLRUAux;

		if (nodoActual)
			nodoLRUAux = nodoActual->data;
		while (nodoActual
				&& (nodoLRUAux->segmentoID != nodo->segmentoID
						|| nodoLRUAux->paginaID != nodo->paginaID)) {
			nodoAnterior = nodoActual;
			nodoActual = nodoActual->next;
			if (nodoActual)
				nodoLRUAux = nodoActual->data;
		}
		if (nodoLRUAux->segmentoID == nodo->segmentoID
				&& nodoLRUAux->paginaID == nodo->paginaID) {
			nodoLRUAux->modificado = nodo->modificado; // El Flag Modificado puede haber cambiado
			// Si nodoActual no está último en la lista reordeno, sino no hago nada
			if (nodoActual->next) {
				if (nodoAnterior)
					nodoAnterior->next = nodoActual->next;
				else
					lista->head = nodoActual->next;

				encolarPaginaExistente(lista, nodoActual);
			}
		}
	} else {
		if (lista->elements_count >= cantidadDeRegistros) {
			if (!memoriaEstaFull(lista)) {
				sem_wait(&loggerSemaforo);
				log_warning(logger, "LRU (llenita)");
				sem_post(&loggerSemaforo);
				nodoLRU = desencolarPrimerElementoNoModificado(lista);
				//encolarNuevaPagina(lista, nodo);
			}
			else{
				sem_wait(&loggerSemaforo);
				log_warning(logger, "Memoria FULL");
				sem_post(&loggerSemaforo);
				journalMemoria();
				vaciarMemoria();
				list_clean(lista);
				nodoLRU = malloc(sizeof(t_nodoLRU));
				nodoLRU->segmentoID = -1;
				//encolarNuevaPagina(lista, nodo);
			}
		} else
			encolarNuevaPagina(lista, nodo);
	}
	return nodoLRU;
}
