

#ifndef PROTOCOLO_H_
#define PROTOCOLO_H_

#include <stdlib.h>
#include <stdint.h> //para los "int8_t" "int16_t" "int32_t"
#include <string.h>

#define MAX_BUFFER 1024


typedef struct {
	int8_t  type;
	int16_t length;
} __attribute__ ((__packed__)) tHeader;

typedef struct {
	int8_t  type;
	int16_t length;
	char    payload[MAX_BUFFER];
} __attribute__ ((__packed__)) tPaquete;


typedef enum {

	/* Mensajes comunes */
	DESCONEXION,
	NO_SE_OBTIENE_RESPUESTA,
	NADA,
	HANDSHAKE,
	HANDSHAKE_MEM,
	SELECT,
	INSERT,
	CREATE,
	DESCRIBE,
	DESCRIBE_TABLA,
	DROP,
	JOURNAL,
	ADD,
	RUN,
	METRICS,
	SC,
	HC,
	EC,
	GOSSIPING,
	GOSSIPING_RECIBE,
	EXIT,
	ERROR_EN_COMANDO
} tMensaje;

/*
 * Aca se definen los payloads que se van a mandar en los paquetes
 */

typedef struct {
	uint32_t timestamp;
	uint16_t  key;
	char * value;
}tRegistro;




#endif /* PROTOCOLO_H_ */
