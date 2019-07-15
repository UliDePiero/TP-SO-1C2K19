/*
 * GossipingMemoria.c
 *
 *  Created on: 15 jul. 2019
 *      Author: utnso
 */

#include "Memoria.h"

void serializarNodo(TablaGossip* nodo, char* paqueteSerializado) {
	int offset = 0;

	memcpy(paqueteSerializado + offset, &(nodo->IDMemoria), sizeof(nodo->IDMemoria));
	offset += sizeof(nodo->IDMemoria);

	memcpy(paqueteSerializado + offset, nodo->IPMemoria, sizeof(nodo->IPMemoria));
	offset += sizeof(nodo->IPMemoria);

	memcpy(paqueteSerializado + offset, &(nodo->puertoMemoria), sizeof(nodo->puertoMemoria));
	offset += sizeof(nodo->puertoMemoria);

	memcpy(paqueteSerializado + offset, &(nodo->socketMemoria), sizeof(nodo->socketMemoria));
}

int recibirNodoYDeserializar(TablaGossip *nodo, int socketMem) {
	int status;

	status = recv(socketMem, &(nodo->IDMemoria), sizeof(nodo->IDMemoria), 0);
	if (!status)
		return 0;

	status = recv(socketMem, nodo->IPMemoria, sizeof(nodo->IPMemoria), 0);
	if (!status)
		return 0;

	status = recv(socketMem, &(nodo->puertoMemoria), sizeof(nodo->puertoMemoria), 0);
	if (!status)
		return 0;

	status = recv(socketMem, &(nodo->socketMemoria), sizeof(nodo->socketMemoria), 0);
	if (!status)
		return 0;

	return status;
}

int nodoEstaEnLista(t_list* lista, TablaGossip* nodo) {
	t_link_element* nodoActual = lista->head;
	TablaGossip* nodoAux;

	if (nodoActual)
		nodoAux = nodoActual->data;

	while (nodoActual && nodoAux->IDMemoria != nodo->IDMemoria) {
		nodoActual = nodoActual->next;
		if (nodoActual)
			nodoAux = nodoActual->data;
	}
	if (nodoActual)
		return 1;
	else
		return 0;
}
