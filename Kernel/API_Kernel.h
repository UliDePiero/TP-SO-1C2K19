/*
 * API_Kernel.h
 *
 *  Created on: 17 abr. 2019
 *      Author: utnso
 */

#ifndef API_KERNEL_H_
#define API_KERNEL_H_


#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <openssl/md5.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/config.h>
#include <commons/log.h>
#include <arpa/inet.h>
#include <sockets.h>
#include <configuraciones.h>
#include <commons/string.h>
#include <parser.h>

int api_kernel(void);
int ejecutarInstruccion(char * instruccion);

#endif /* API_KERNEL_H_ */
