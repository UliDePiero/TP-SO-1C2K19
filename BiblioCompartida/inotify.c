/*
 * inotify.c
 *
 *  Created on: 4 may. 2019
 *      Author: utnso
 */

#include "inotify.h"

void inotifyConfig(){
	char buffer[BUF_LEN];

	// Al inicializar inotify, éste nos devuelve un descriptor de archivo
	int file_descriptor = inotify_init();
	if (file_descriptor < 0) {
		perror("inotify_init");
	}

	// Creamos un monitor sobre un path indicando qué eventos queremos escuchar
	int watch_descriptor = inotify_add_watch(file_descriptor, "../", IN_MODIFY);
	if(watch_descriptor < 0){
		perror("inotify_add_watch");
	}

	// El file descriptor creado por inotify, es el que recibe la información sobre los eventos ocurridos,
	// para leer esta información el descriptor se lee como si fuera un archivo común y corriente pero
	// la diferencia está en que lo que leemos no es el contenido de un archivo sino la información
	// referente a los eventos ocurridos

	int length = read(file_descriptor, buffer, BUF_LEN);
	if (length < 0) {
		perror("read");
	}

	int offset = 0;

	// Luego del read, buffer es un array de n posiciones donde cada posición contiene
	// un evento ( inotify_event ) junto con el nombre de éste.
	while (offset < length) {
		// El buffer es de tipo array de char, o array de bytes. Esto es porque como los
		// nombres pueden tener nombres más cortos que 24 caracteres el tamaño va a ser menor
		// a sizeof( struct inotify_event ) + 24.
		struct inotify_event *event = (struct inotify_event *) &buffer[offset];
		// El campo "len" nos indica la longitud del tamaño del nombre
		if (event->len) {
			// Dentro de "mask" tenemos el evento que ocurrió y sobre dónde ocurrió,
			// sea un archivo o un directorio
			if (event->mask & IN_MODIFY) {
				printf("The file %s was modified.\n", event->name);
				// ACTUALIZAR CONFIG DEL PROCESO CORRESPONDIENTE
			}
		}
		offset += sizeof (struct inotify_event) + event->len;
	}

	// Para finalizar, eliminamos el monitor y cerramos el descriptor de archivo
	inotify_rm_watch(file_descriptor, watch_descriptor);
	close(file_descriptor);
}
