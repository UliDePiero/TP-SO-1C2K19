
#ifndef LIBSOCKETS_H_
#define LIBSOCKETS_H_

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>

#include "log.h"
#include "protocolo.h"


int crearSocketEscucha(int puerto, t_log* logger);

int enviarPaquete(int socketServidor, tPaquete* buffer, t_log* logger, char* info);

int recibirPaquete(int socketReceptor, tMensaje* tipoMensaje, char** payload, t_log* pLogger, char* sMensajeLogger);

void liberarPaquete(tPaquete* mensaje);

signed int getConnection(fd_set *master, int *maxSock, int sockListener, tMensaje *tipoMensaje, char** payload, t_log* logger);

signed int getConnectionTimeOut(fd_set *setSockets, int *maxSock, int sockListener, tMensaje *tipoMensaje, char** payload, struct timeval *timeout, t_log* logger);

//signed int multiplexar(fd_set *master, fd_set *temp, int *maxSock, tMensaje* tipoMensaje, char** buffer, t_log* logger);

signed int connectToServer(char *ip_server, int puerto, t_log *logger);

int desconectarseDe(int iSocket);

void crearHiloIndependiente(pthread_t* hilo, void *(*funcion) (void *), void *__restrict argumento, char* proceso);

void crearHilo(pthread_t* hilo, void *(*funcion) (void *), void *__restrict argumento, char* proceso);

void joinearHilo(pthread_t hilo, void **retorno, char* proceso);

#endif /* LIBSOCKETS_H_ */
