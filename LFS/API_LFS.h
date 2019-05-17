/*
 * API_LFS.h
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */

#ifndef API_LFS_H_
#define API_LFS_H_

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
#include <time.h>
#include "LFS.h"

void API_LFS();
int ejecutarInstruccion(char * instruccion);
void ejecutarSelect(char*);
void ejecutarInsert(char*);
void ejecutarCreate(char*);
void ejecutarDescribe(char*);
void ejecutarDrop(char*);

#endif /* API_LFS_H_ */
