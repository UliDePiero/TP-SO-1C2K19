/*
 * GossipingMemoria.c
 *
 *  Created on: 15 jul. 2019
 *      Author: utnso
 */

#include "Memoria.h"
void eliminaMemoriaDeListaGossipingSERVER (int socketMem){
	t_link_element* nodoActual = listaGossiping->head;
	t_link_element* nodoAnterior = NULL;
	TablaGossip* nodoAux;

	if (nodoActual)
		nodoAux = nodoActual->data;
	while (nodoActual && nodoAux->socketMemoria != socketMem) {
		nodoAnterior = nodoActual;
		nodoActual = nodoActual->next;
		if (nodoActual)
			nodoAux = nodoActual->data;
	}
	if (nodoActual && nodoAux->socketMemoria == socketMem) {
		if (!nodoAnterior)
			listaGossiping->head = nodoActual->next;
		else
			nodoAnterior->next = nodoActual->next;
		listaGossiping->elements_count--;
		sem_wait(&loggerSemaforo);
		log_debug(logger, "La Memoria %d se desconectó", nodoAux->IDMemoria);
		sem_post(&loggerSemaforo);
		free(nodoAux);
		free(nodoActual);
	}
}
void eliminaMemoriaDeListaGossiping(int socketMem) {
	t_link_element* nodoActual = listaGossiping->head;
	t_link_element* nodoAnterior = NULL;
	TablaGossip* nodoAux;
	int i;
	for (i = 0; i < BACKLOG; i++) {
		//sem_wait(&loggerSemaforo);
		//log_warning(logger, "CLIENTE ID %d SOCKET %d ", cliente[i][1], cliente[i][0]);
		//sem_post(&loggerSemaforo);
		if(cliente[i][0]==socketMem)
			break;
	}
	if(i < BACKLOG){
		if (nodoActual)
			nodoAux = nodoActual->data;
		while (nodoActual && nodoAux->IDMemoria != cliente[i][1]) {
			nodoAnterior = nodoActual;
			nodoActual = nodoActual->next;
			if (nodoActual)
				nodoAux = nodoActual->data;
		}
		if (nodoActual && nodoAux->IDMemoria == cliente[i][1]) {
			if (!nodoAnterior)
				listaGossiping->head = nodoActual->next;
			else
				nodoAnterior->next = nodoActual->next;
			listaGossiping->elements_count--;
			sem_wait(&loggerSemaforo);
			log_debug(logger, "La Memoria %d se desconectó", nodoAux->IDMemoria);
			sem_post(&loggerSemaforo);
			cliente[i][0] = 0;
			cantidadClientes --;
			free(nodoAux);
			free(nodoActual);
		}
	}
}

void conectarConNuevaMemoria(TablaGossip* nodo, int seed_gos) {
	// Cada vez que se conoce una nueva Memoria del pool por Gossiping, hay que conectarse a ella
	nodo->socketMemoria = connectToServer(nodo->IPMemoria, nodo->puertoMemoria,
			logger);

	// Si la conexión no falló, crea un hilo para las respuestas de esa nueva Memoria
	if (nodo->socketMemoria != 1) {
		sem_wait(&loggerSemaforo);
		log_trace(logger, "Memoria se conectó correctamente a la Memoria %d",
				nodo->IDMemoria);
		sem_post(&loggerSemaforo);
		realizarHandshake(nodo->socketMemoria);
		if(seed_gos < CANT_MAX_SEEDS){
			socketSEED[seed_gos] = nodo->socketMemoria;
			seed++;
		}
	} else {
		sem_wait(&loggerSemaforo);
		log_error(logger, "Falló la conexión con la Memoria %d",
				nodo->IDMemoria);
		sem_post(&loggerSemaforo);
	}
}

void armarNodoMemoria(TablaGossip* nodo, int seed_gos) {
	// Cargo dato faltante del nodo
	nodo->socketMemoria = 1;

	for (int i = 0; i < CANT_MAX_SEEDS; i++) {
		if(configuracion->PUERTO_SEEDS[i] != 0){
			//log_warning(logger,"IPSEED: %s",configuracion->IP_SEEDS[i]);
			if (string_equals_ignore_case(nodo->IPMemoria,configuracion->IP_SEEDS[i]) && nodo->puertoMemoria == configuracion->PUERTO_SEEDS[i])
				nodo->socketMemoria = socketSEED[i];
		}
	}

	// Si el nodo no está en listaGossiping, lo agrego y me conecto
	if (!nodoEstaEnLista(listaGossiping, nodo)) {
		list_add(listaGossiping, nodo);
		if (nodo->socketMemoria == 1) //Si la memoria no es una SEED me conecto
			conectarConNuevaMemoria(nodo, seed_gos);
	} else
		free(nodo); // Si el nodo ya se encontraba en listaGossiping, lo libero
}

int recibeLista(int socketMem, int seed_gos) {
	int status;
	int cantElementosListaRecibida;

	tMensaje tipoMensaje;
	char * sPayload;
	//recv(socketMemoria, msjeRecibido, sizeof(tPaquete), 0);
	if(recibirPaquete(socketMem,&tipoMensaje,&sPayload,logger,"Recibo cantidad de listaGossiping") > 0){
		cantElementosListaRecibida = atoi(sPayload);
		if(sPayload)free(sPayload);

		for (int i = 0; i < cantElementosListaRecibida; i++) {
			TablaGossip* nodoRecibido = malloc(sizeof(TablaGossip));
			status = recibirNodoYDeserializar(nodoRecibido, socketMem);
			if (status)
				armarNodoMemoria(nodoRecibido, seed_gos);
		}
		return 1;
	}else
		return 0;
}

int pideListaGossiping(int socketMem, int seed_gos) {
	int retorno;
	tPaquete* msjeEnviado = malloc(sizeof(tPaquete));
	msjeEnviado->type = GOSSIPING;
	strcpy(msjeEnviado->payload, "Memoria pide Lista de Gossiping");
	msjeEnviado->length = sizeof(msjeEnviado->payload);
	if(enviarPaquete(socketMem, msjeEnviado, logger,"Memoria realiza pedido de Lista de Gossiping")==-1){
		liberarPaquete(msjeEnviado);
		retorno = -1;
	}
	else{
		liberarPaquete(msjeEnviado);
		retorno = recibeLista(socketMem, seed_gos);
	}

	return retorno;
}

void pideListaGossiping_2(int socketMem) {
	tPaquete* msjeEnviado = malloc(sizeof(tPaquete));
	msjeEnviado->type = GOSSIPING;
	strcpy(msjeEnviado->payload, "Memoria pide Lista de Gossiping");
	msjeEnviado->length = sizeof(msjeEnviado->payload);
	enviarPaquete(socketMem, msjeEnviado, logger,
			"Memoria realiza pedido de Lista de Gossiping");
	liberarPaquete(msjeEnviado);
}

void enviarListaGossiping(int socketEnvio) {
	sem_wait(&mutexMemoria);
	TablaGossip* nodoGossipAux = malloc(sizeof(TablaGossip));
	int tamanioPaquete = sizeof(nodoGossipAux->IDMemoria) + sizeof(nodoGossipAux->IPMemoria) + sizeof(nodoGossipAux->puertoMemoria);
	free(nodoGossipAux);
	char* paqueteSerializado;
	t_link_element* nodoAux = listaGossiping->head;
	tPaquete* msjeEnviado = malloc(sizeof(tPaquete));
	msjeEnviado->type = GOSSIPING_RECIBE;
	char* string_mensaje = string_itoa(listaGossiping->elements_count);
	strcpy(msjeEnviado->payload, string_mensaje);
	if(string_mensaje) free(string_mensaje);
	msjeEnviado->length = sizeof(msjeEnviado->payload);

	enviarPaquete(socketEnvio, msjeEnviado, logger, "Envio cantidad de elementos de listaGossipig");
	liberarPaquete(msjeEnviado);
	//send(socketEnvio, string_itoa(listaGossiping->elements_count), sizeof(int), 0);
	for (int i = 0; i < listaGossiping->elements_count; i++) {
		nodoGossipAux = nodoAux->data;
		paqueteSerializado = malloc(tamanioPaquete);
		serializarNodo(nodoGossipAux, paqueteSerializado);
		send(socketEnvio, paqueteSerializado, tamanioPaquete, 0);
		free(paqueteSerializado);
		nodoAux = nodoAux->next;
	}
	sem_post(&mutexMemoria);
}

void enviaLista(int socketMem) {
	tPaquete* msjeEnviado = malloc(sizeof(tPaquete));
	msjeEnviado->type = GOSSIPING_RECIBE;

	TablaGossip* nodoGossipAux;
	int tamanioPaquete = sizeof(nodoGossipAux->IDMemoria)
			+ sizeof(nodoGossipAux->IPMemoria)
			+ sizeof(nodoGossipAux->puertoMemoria);
	char* paqueteSerializado = malloc(tamanioPaquete);
	t_link_element* nodoAux = listaGossiping->head;

	//send(socketMem, string_itoa(listaGossiping->elements_count), sizeof(int), 0);

	strcpy(msjeEnviado->payload, string_itoa(listaGossiping->elements_count));
		msjeEnviado->length = sizeof(msjeEnviado->payload);
		enviarPaquete(socketMem, msjeEnviado, logger,
				"Memoria realiza envío de Lista de Gossiping");

	for (int i = 0; i < listaGossiping->elements_count; i++) {
		nodoGossipAux = nodoAux->data;

		serializarNodo(nodoGossipAux, paqueteSerializado);
		send(socketMem, paqueteSerializado, tamanioPaquete, 0);

		nodoAux = nodoAux->next;
	}
	free(paqueteSerializado);

	liberarPaquete(msjeEnviado);

}

void armarPropioNodo() {
	TablaGossip* nodoMem = malloc(sizeof(TablaGossip));

	nodoMem->IDMemoria = configuracion->MEMORY_NUMBER;
	strcpy(nodoMem->IPMemoria, configuracion->IP_PROPIA);
	nodoMem->puertoMemoria = configuracion->PUERTO;
	nodoMem->socketMemoria = 1;

	// Agrego al nodo correspondiente a la propia memoria en la primera posición de la listaGossiping de dicha memoria
	list_add(listaGossiping, nodoMem);
}

int nodoSocketEstaEnLista(int socketID) {
	if(socketID != 1){
		t_link_element* nodoActual = listaGossiping->head;
		TablaGossip* nodoAux;

		if (nodoActual)
			nodoAux = nodoActual->data;

		while (nodoActual && nodoAux->socketMemoria != socketID) {
			nodoActual = nodoActual->next;
			if (nodoActual)
				nodoAux = nodoActual->data;
		}
		if (nodoActual)
			return 1;
		else
			return 0;
	}else
		return 0;
}

void realizarHandshake(int socket){
	tPaquete* msjeEnviado = malloc(sizeof(tPaquete));
	msjeEnviado->type = HANDSHAKE_MEM;
	char* string_mensaje = string_itoa(configuracion->MEMORY_NUMBER);
	strcpy(msjeEnviado->payload, string_mensaje);
	if(string_mensaje) free(string_mensaje);
	msjeEnviado->length = sizeof(msjeEnviado->payload);

	enviarPaquete(socket, msjeEnviado, logger, "Envio mi Numero de Memoria");
	liberarPaquete(msjeEnviado);
}

void muestraListaGossip(TablaGossip* elemento){
	printf("MEMORIA: %d %s %d %d \n", elemento->IDMemoria, elemento->IPMemoria, elemento->puertoMemoria, elemento->socketMemoria);
}

void* gossipingMemoria() {
	armarPropioNodo();
	// Memoria intercambia listaGossiping con todos sus seeds
	int seed_gos = 0;
	while (configuracion->PUERTO_SEEDS[seed_gos] != 0 && seed_gos < CANT_MAX_SEEDS) {

		//sem_wait(&loggerSemaforo);
		//log_trace(logger, "Memoria hace Gossiping con su seed en la IP: %s y Puerto: %d", configuracion->IP_SEEDS[seed_gos], configuracion->PUERTO_SEEDS[seed_gos]);
		//sem_post(&loggerSemaforo);
		socketSEED[seed_gos] = connectToServer(configuracion->IP_SEEDS[seed_gos], configuracion->PUERTO_SEEDS[seed_gos], logger);

		if(socketSEED[seed_gos] == 1) {
			sem_wait(&loggerSemaforo);
			log_error(logger, "SEED no conectada");
			sem_post(&loggerSemaforo);
		}else {
			realizarHandshake(socketSEED[seed_gos]);
			//conectateconmigo(socketSEED[seed_gos]);
			pideListaGossiping(socketSEED[seed_gos],seed_gos);
			//enviaLista(socketSEED[seed_gos]);
			enviarListaGossiping(socketSEED[seed_gos]);
		}

		seed_gos++;
	}

	sem_post(&gossipMemoria);
	sem_post(&gossipMemoria);

	while (1) {
		sleep(configuracion->RETARDO_GOSSIPING / 1000);
		list_iterate(listaGossiping,(void*)muestraListaGossip);

		//int hizoGossipingConSeed = 0;

		seed_gos = 0;
		while (configuracion->PUERTO_SEEDS[seed_gos] != 0 && seed_gos < CANT_MAX_SEEDS) {
			sem_wait(&loggerSemaforo);
			log_trace(logger, "Memoria hace Gossiping con su seed en la IP: %s y Puerto: %d", configuracion->IP_SEEDS[seed_gos], configuracion->PUERTO_SEEDS[seed_gos]);
			sem_post(&loggerSemaforo);
			if (nodoSocketEstaEnLista(socketSEED[seed_gos]) && socketSEED[seed_gos] != 1) {
				//sem_wait(&loggerSemaforo);
				//log_warning(logger, "ESTA CONECTADA %d", socketSEED[seed_gos]);
				//sem_post(&loggerSemaforo);
				if(pideListaGossiping(socketSEED[seed_gos],seed_gos) > 0)
				//pideListaGossiping_2(socketSEED[seed_gos]);
				//enviaLista(socketSEED[seed_gos]);
					enviarListaGossiping(socketSEED[seed_gos]);
				//hizoGossipingConSeed = 1;
				else
					eliminaMemoriaDeListaGossipingSERVER(socketSEED[seed_gos]);

			}else{
				//sem_wait(&loggerSemaforo);
				//log_error(logger, "ESTA DESCONECTADA");
				//sem_post(&loggerSemaforo);
				socketSEED[seed_gos] = connectToServer(configuracion->IP_SEEDS[seed_gos], configuracion->PUERTO_SEEDS[seed_gos], logger);
				if(socketSEED[seed_gos] == 1) {
					sem_wait(&loggerSemaforo);
					log_error(logger, "SEED no conectada");
					sem_post(&loggerSemaforo);
				}else {
					realizarHandshake(socketSEED[seed_gos]);
					//sem_wait(&loggerSemaforo);
					//log_error(logger, "AHORA ESTA CONECTADA");
					//sem_post(&loggerSemaforo);
					//conectateconmigo(socketSEED[seed_gos]);
					pideListaGossiping(socketSEED[seed_gos],seed_gos);
					//pideListaGossiping_2(socketSEED[seed_gos]);
					//enviaLista(socketSEED[seed_gos]);
					enviarListaGossiping(socketSEED[seed_gos]);
					//hizoGossipingConSeed = 1;
				}
			}
			seed_gos++;
		}
		//En el TP no encuentro donde se menciona hacer GOSSIPING con una memoria que no es seed de esta

		// Si no pudo hacer Gossiping con ningún seed, hago con la segunda Memoria de la listaGossiping, si existe (Porque la primera va a ser sí misma)
		/*if (hizoGossipingConSeed == 0) {
			if (listaGossiping->elements_count > 1) { // Hay alguna otra memoria además de la actual
				TablaGossip* nodoTablaGossipAux = listaGossiping->head->next->data;
				sem_wait(&loggerSemaforo);
				log_trace(logger, "Memoria hace Gossiping con la Memoria %d", nodoTablaGossipAux->IDMemoria);
				sem_post(&loggerSemaforo);
				pideListaGossiping_2(nodoTablaGossipAux->socketMemoria);
				//enviaLista(nodoTablaGossipAux->socketMemoria);
				enviarListaGossiping(nodoTablaGossipAux->socketMemoria);
			}
		}*/
	}
}

void serializarNodo(TablaGossip* nodo, char* paqueteSerializado) {
	int offset = 0;

	memcpy(paqueteSerializado + offset, &(nodo->IDMemoria),
			sizeof(nodo->IDMemoria));
	offset += sizeof(nodo->IDMemoria);

	memcpy(paqueteSerializado + offset, nodo->IPMemoria,
			sizeof(nodo->IPMemoria));
	offset += sizeof(nodo->IPMemoria);

	memcpy(paqueteSerializado + offset, &(nodo->puertoMemoria),
			sizeof(nodo->puertoMemoria));
}

int recibirNodoYDeserializar(TablaGossip *nodo, int socketMem) {
	int status;

	status = recv(socketMem, &(nodo->IDMemoria), sizeof(nodo->IDMemoria), 0);
	if (!status)
		return 0;

	status = recv(socketMem, nodo->IPMemoria, sizeof(nodo->IPMemoria), 0);
	if (!status)
		return 0;

	status = recv(socketMem, &(nodo->puertoMemoria),
			sizeof(nodo->puertoMemoria), 0);
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
