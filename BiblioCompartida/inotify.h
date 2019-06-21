/*
 * inotify.h
 *
 *  Created on: 4 may. 2019
 *      Author: utnso
 */

#ifndef INOTIFY_H_
#define INOTIFY_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>

/* --------------------    Definición de Var. Globales    -------------------- */

// El tamaño de un evento es igual al tamaño de la estructura de inotify
// más el tamaño máximo de nombre de archivo que nosotros soportemos,
// en este caso el tamaño de nombre máximo que vamos a manejar es de 24
// caracteres. Esto es porque la estructura inotify_event tiene un array
// sin dimensión.
#define EVENT_SIZE  ( sizeof (struct inotify_event) + 24 )

// El tamaño del buffer es igual a la cantidad máxima de eventos simultáneos
// que quiero manejar por el tamaño de cada uno de los eventos. En este caso
// Puedo manejar hasta 1024 eventos simultáneos.
#define BUF_LEN     ( 1024 * EVENT_SIZE )


/* --------------------    Definición de Funciones    -------------------- */

int configModificado(void);
int configModificadoSilencioso(void);

#endif /* INOTIFY_H_ */
