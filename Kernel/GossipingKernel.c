/*
 * GossipingKernel.c
 *
 *  Created on: 15 jul. 2019
 *      Author: utnso
 */

#include "Planificador.h"

int pideRetardoGossiping() {
	tPaquete* msjeEnviado = malloc(sizeof(tPaquete));
	msjeEnviado->type = HANDSHAKE;
	strcpy(msjeEnviado->payload, "Pedido de RETARDO_GOSSIPING a Memoria");
	msjeEnviado->length = sizeof(msjeEnviado->payload);
	enviarPaquete(socketMemoria, msjeEnviado, logger, "Kernel realiza pedido de Retardo de Gossiping a Memoria");
	liberarPaquete(msjeEnviado);

	tPaquete* msjeRecibido = malloc(sizeof(tPaquete));
	recv(socketMemoria, msjeRecibido, sizeof(tPaquete), 0);
	int retardoGossiping = atoi(msjeRecibido->payload);
	liberarPaquete(msjeRecibido);

	return retardoGossiping;
}

void armarNodoMemoria(TablaGossip* nodo) {
	// Cargo datos faltantes del nodo
	//nodo->socketMemoria = -1;
	nodo->criterioSC = 0;
	nodo->criterioSHC = 0;
	nodo->criterioEC = 0;
	// Si el nodo no está en listaGossiping, lo agrego. Sino, lo libero
	if (!nodoEstaEnLista(listaGossiping, nodo))
		list_add(listaGossiping, nodo);
	else
		free(nodo);
}

void pideListaGossiping(int socketMem) {
	tPaquete* msjeEnviado = malloc(sizeof(tPaquete));
	msjeEnviado->type = GOSSIPING;
	strcpy(msjeEnviado->payload, "Kernel pide Lista de Gossiping a Memoria");
	msjeEnviado->length = sizeof(msjeEnviado->payload);
	enviarPaquete(socketMem, msjeEnviado, logger, "Kernel realiza pedido de Lista de Gossiping a Memoria");
	liberarPaquete(msjeEnviado);

	int status;
	int cantElementosListaRecibida;

	recv(socketMem, &cantElementosListaRecibida, sizeof(int), 0);

	for (int i = 0; i < cantElementosListaRecibida; i++) {
		TablaGossip* nodoRecibido = malloc(sizeof(TablaGossip));
		status = recibirNodoYDeserializar(nodoRecibido, socketMem);
		if (status)
			armarNodoMemoria(nodoRecibido);
	}
}

void gossipingKernel() {
	// Pide a la Memoria conectada por Archivo de Configuración el Retardo de Gossiping
	int retardoGossiping = pideRetardoGossiping();
	// Pide a la memoria del archivo de configuración la Lista de Gossiping
	pideListaGossiping(socketMemoria);

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
