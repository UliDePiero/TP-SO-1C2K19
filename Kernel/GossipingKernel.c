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
	enviarPaquete(socketMemoria, msjeEnviado, logger,
			"Kernel realiza pedido de Retardo de Gossiping a Memoria");
	liberarPaquete(msjeEnviado);

	//tPaquete* msjeRecibido = malloc(sizeof(tPaquete));
	char* msjeRecibido;
	tMensaje tipo_mensaje;
	recibirPaquete(socketMemoria,&tipo_mensaje,&msjeRecibido,logger,"Respuesta de MEMORIA");
	//recv(socketMemoria, msjeRecibido, sizeof(tPaquete), 0);
	int retardoGossiping = atoi(msjeRecibido);
	if(msjeRecibido != NULL) free(msjeRecibido);
	sem_post(&gossip);
	return retardoGossiping;
}

void armarNodoMemoria(TablaGossip* nodo) {
	// Cargo datos faltantes del nodo
	if(!string_equals_ignore_case(nodo->IPMemoria, configuracion->IP_MEMORIA) || nodo->puertoMemoria != configuracion->PUERTO_MEMORIA)
		nodo->socketMemoria = 1;
	else
		nodo->socketMemoria = socketMemoria;
	nodo->criterioSC = 0;
	nodo->criterioSHC = 0;
	nodo->criterioEC = 0;
	// Si el nodo no está en listaGossiping, lo agrego, me conecto y creo hilo de respuestas
	if (!nodoEstaEnLista(listaGossiping, nodo)) {
		list_add(listaGossiping, nodo);
		//if(nodo->socketMemoria == 1)
		conectarConNuevaMemoria(nodo);
	} else
		free(nodo); // Si el nodo ya se encontraba en listaGossiping, lo libero
}

void pideListaGossiping_1(int socketMem) {
	tPaquete* msjeEnviado = malloc(sizeof(tPaquete));
	msjeEnviado->type = GOSSIPING;
	strcpy(msjeEnviado->payload, "Kernel pide Lista de Gossiping a Memoria");
	msjeEnviado->length = sizeof(msjeEnviado->payload);
	enviarPaquete(socketMem, msjeEnviado, logger,
			"Kernel realiza pedido de Lista de Gossiping a Memoria");
	liberarPaquete(msjeEnviado);

	int status;
	int cantElementosListaRecibida;

	tMensaje tipoMensaje;
	char * sPayload;
	//recv(socketMem, msjeRecibido, sizeof(tPaquete), 0);
	recibirPaquete(socketMem,&tipoMensaje,&sPayload,logger,"Recibo cantidad de listaGossiping");
	cantElementosListaRecibida = atoi(sPayload);
	if(sPayload)free(sPayload);

	for (int i = 0; i < cantElementosListaRecibida; i++) {
		TablaGossip* nodoRecibido = malloc(sizeof(TablaGossip));
		status = recibirNodoYDeserializar(nodoRecibido, socketMem);
		if (status)
			armarNodoMemoria(nodoRecibido);
	}
	sem_post(&gossip);
}

int pideListaGossiping(int socketMem) {
	tPaquete* msjeEnviado = malloc(sizeof(tPaquete));
	msjeEnviado->type = GOSSIPING;
	strcpy(msjeEnviado->payload, "Kernel pide Lista de Gossiping a Memoria");
	msjeEnviado->length = sizeof(msjeEnviado->payload);
	if(enviarPaquete(socketMem, msjeEnviado, logger, "Kernel realiza pedido de Lista de Gossiping a Memoria")==-1){
		liberarPaquete(msjeEnviado);
		return 0;
	}else
		return 1;
/*
	int status;
	int cantElementosListaRecibida;

	recv(socketMem, &cantElementosListaRecibida, sizeof(int), 0);

	for (int i = 0; i < cantElementosListaRecibida; i++) {
		TablaGossip* nodoRecibido = malloc(sizeof(TablaGossip));
		status = recibirNodoYDeserializar(nodoRecibido, socketMem);
		if (status)
			armarNodoMemoria(nodoRecibido);
	}*/
}

void conectarConNuevaMemoria(TablaGossip* nodo) {
	// Cada vez que se conoce una nueva Memoria por Gossiping, Kernel se conecta a ella
	if(nodo->socketMemoria == 1) nodo->socketMemoria = connectToServer(nodo->IPMemoria, nodo->puertoMemoria, logger);
	// Si la conexión no falló, crea un hilo para las respuestas de esa nueva Memoria
	if (nodo->socketMemoria != 1) {
		sem_wait(&loggerSemaforo);
		log_info(logger, "Kernel se conectó correctamente a la Memoria %d", nodo->IDMemoria);
		sem_post(&loggerSemaforo);
		int *socket_m = malloc(sizeof(*socket_m));
		*socket_m = nodo->socketMemoria; //Solo cambia el socket de la memoria nueva
		crearHiloIndependiente(&hiloRespuestasRequest, (void*) respuestas, (void*) socket_m, "Kernel(Respuestas)");
		tPaquete* mensaje = malloc(sizeof(tPaquete));
		mensaje->type = DESCRIBE;
		strcpy(mensaje->payload,"DESCRIBE");
		mensaje->length = sizeof(mensaje->payload);
		int resultado = enviarPaquete(nodo->socketMemoria, mensaje,logger,"Ejecutar comando DESCRIBE desde Kernel.");
		liberarPaquete(mensaje);

		if (resultado > 0){
			sem_wait(&loggerSemaforo);
			log_info(logger, "'DESCRIBE' enviado exitosamente a Memoria");
			sem_post(&loggerSemaforo);
		}
	}
}

void gossipingKernel() {
	// Pide a la Memoria conectada por Archivo de Configuración el Retardo de Gossiping
	sem_wait(&gossip);
	int retardoGossiping = pideRetardoGossiping();

	sem_wait(&loggerSemaforo);
	log_debug(logger, "Retardo gossiping: %d",retardoGossiping);
	sem_post(&loggerSemaforo);

	sem_wait(&gossip);

	sem_wait(&loggerSemaforo);
	log_debug(logger, "Kernel hace Gossiping con Memoria");
	sem_post(&loggerSemaforo);

	// Pide a la memoria del archivo de configuración la Lista de Gossiping
	pideListaGossiping_1(socketMemoria);

	while (1) {
		usleep(retardoGossiping * 1000);

		//sem_wait(&loggerSemaforo);
		//log_debug(logger, "Kernel hace Gossiping con Memoria");
		//sem_post(&loggerSemaforo);
		if (listaGossiping->elements_count > 0) {
			// Hago Gossiping siempre con la primera Memoria que esté en listaGossiping (Si no se desconectó, va a ser la que tenemos en el Archivo de Configuración)
			TablaGossip* nodoTablaGossipAux = listaGossiping->head->data;
			if(nodoTablaGossipAux)
				if(pideListaGossiping(nodoTablaGossipAux->socketMemoria)==0)
					listaGossiping->head->data = listaGossiping->head->next;
		}
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

int recibirNodoYDeserializar(TablaGossip *nodo, int socketMem) {
	int status;

	status = recv(socketMem, &(nodo->IDMemoria), sizeof(nodo->IDMemoria), MSG_WAITALL);
	//printf("ID%d",nodo->IDMemoria);

	if (!status)
		return 0;

	status = recv(socketMem, nodo->IPMemoria, sizeof(nodo->IPMemoria), MSG_WAITALL);
	//printf("ip%s",nodo->IPMemoria);
	if (!status)
		return 0;
	status = recv(socketMem, &(nodo->puertoMemoria),
			sizeof(nodo->puertoMemoria), MSG_WAITALL);
	//printf("puerto%d",nodo->puertoMemoria);
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
