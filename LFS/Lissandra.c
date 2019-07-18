/*
 * Lissandra.c
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */
#include "LFS.h"
void configurar(ConfiguracionLFS* configuracion) {

	char* campos[] = {
					   "PUERTO_ESCUCHA",
					   "PUNTO_MONTAJE",
					   "RETARDO",
					   "TAMANIO_VALUE",
					   "TIEMPO_DUMP"
					 };

	t_config* archivoConfig = archivoConfigCrear(RUTA_CONFIG, campos);

	//Relleno los campos con la info del archivo

	//strcpy(configuracion->PUERTO, archivoConfigSacarStringDe(archivoConfig, "PUERTO"));
	configuracion->PUERTO_ESCUCHA = archivoConfigSacarIntDe(archivoConfig, "PUERTO_ESCUCHA");
	char* montaje = string_duplicate(archivoConfigSacarStringDe(archivoConfig, "PUNTO_MONTAJE"));
	if(!string_ends_with(montaje, "/"))
		string_append(&montaje, "/");
	strcpy(configuracion->PUNTO_MONTAJE, montaje);
	free(montaje);
	configuracion->RETARDO = archivoConfigSacarLongDe(archivoConfig, "RETARDO");
	configuracion->TAMANIO_VALUE = archivoConfigSacarIntDe(archivoConfig, "TAMANIO_VALUE");
	configuracion->TIEMPO_DUMP = archivoConfigSacarLongDe(archivoConfig, "TIEMPO_DUMP");

	archivoConfigDestruir(archivoConfig);
}

void cambiosConfigLFS(){
	while(1){
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		if(configModificadoSilencioso()){
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
			sleep(1);
			char* campos[] = {
						   "PUERTO_ESCUCHA",
						   "PUNTO_MONTAJE",
						   "RETARDO",
						   "TAMANIO_VALUE",
						   "TIEMPO_DUMP"
						 };
			t_config* archivoConfig = archivoConfigCrear(RUTA_CONFIG, campos);
			long retardoNuevo = archivoConfigSacarLongDe(archivoConfig, "RETARDO");
			long dumpNuevo = archivoConfigSacarLongDe(archivoConfig, "TIEMPO_DUMP");
			sem_wait(&configSemaforo);
			if(configuracion->RETARDO != retardoNuevo){
				sem_wait(&loggerSemaforo);
				log_error(logger, "Retardo cambiado %ld -> %ld", configuracion->RETARDO, retardoNuevo);
				sem_post(&loggerSemaforo);
				configuracion->RETARDO = retardoNuevo;
			}
			if(configuracion->TIEMPO_DUMP != dumpNuevo){
				sem_wait(&loggerSemaforo);
				log_error(logger, "Tiempo dump cambiado %ld -> %ld", configuracion->TIEMPO_DUMP, dumpNuevo);
				sem_post(&loggerSemaforo);
				configuracion->TIEMPO_DUMP = dumpNuevo;
			}
			sem_post(&configSemaforo);
			archivoConfigDestruir(archivoConfig);
		}
	}
}
void levantarLFS(){
	tablasLFS = list_create();
	sem_init(&memtableSemaforo, 1, 1);
	sem_init(&dumpSemaforo, 1, 1);
	sem_init(&loggerSemaforo, 1, 1);
	logger = log_create(logFile, "LFS", true, LOG_LEVEL_INFO);
	configuracion = malloc(sizeof(ConfiguracionLFS));
	configurar(configuracion);
	levantarFileSystem();

	log_info(logger, "Modulo LFS iniciado");
}
void destruirLFS(){
	sem_wait(&dumpSemaforo);
	pthread_cancel(hiloDump);
	destruirFileSystem();
	sem_wait(&configSemaforo);
	pthread_cancel(hiloConfig);
	sem_wait(&memtableSemaforo);
	list_destroy_and_destroy_elements(tablasLFS, (void*) tablaDestruir);
	log_info(logger, "Modulo LFS cerrado");
	log_destroy(logger);
	free(configuracion);
	close(socketEscucha);
	exit(0);
}

int main(){
	levantarLFS();

	crearHiloIndependiente(&hiloConfig,(void*)cambiosConfigLFS, NULL, "LFS Config");
	crearHiloIndependiente(&hiloAPI,(void*)API_LFS, NULL, "LFS");
	crearHiloIndependiente(&hiloDump,(void*)dumpLFS, NULL, "LFS Dump");

	//servidor
	//FUNCIONES SOCKETS (Usar dependiendo de la biblioteca que usemos)

	fd_set setSocketsOrquestador;
	FD_ZERO(&setSocketsOrquestador);

	// Inicializacion de sockets y actualizacion del log

	socketEscucha = crearSocketEscucha(configuracion->PUERTO_ESCUCHA, logger);

	FD_SET(socketEscucha, &setSocketsOrquestador);
	maxSock = socketEscucha;
	tMensaje tipoMensaje;
	char * sPayload;
	tPaquete* mensaje;
	char* retorno;
	t_list* retornoLista;
	while (1) {

		puts("Escuchando");
		socketActivo = getConnection(&setSocketsOrquestador, &maxSock, socketEscucha, &tipoMensaje, &sPayload, logger);
		if (socketActivo != -1) {
			sem_wait(&loggerSemaforo);
			log_info(logger, "Comunicacion establecida en el socket %d", socketActivo);
			sem_post(&loggerSemaforo);
			switch (tipoMensaje) {
				char** comando;
				case SELECT:
					comando = validarComando(sPayload, 3);
					if(comando){
						retorno = selectLFS(comando[1], atoi(comando[2]));
						mensaje = malloc(sizeof(tPaquete));
						if(retorno != NULL)
						{
							mensaje->type = SELECT;
							strcpy(mensaje->payload,retorno);
							free(retorno);
						}
						else
						{
							mensaje->type = ERROR_EN_COMANDO;
							strcpy(mensaje->payload,"SELECT");
						}
						mensaje->length = sizeof(mensaje->payload);
						enviarPaquete(socketActivo, mensaje,logger,"Value del SELECT de LFS.");
						liberarPaquete(mensaje);
						for(int i = 0; i<3; i++)
							free(comando[i]);
						free(comando);
					}
					break;
				case INSERT:
					comando = validarComandoInsert(sPayload);
					if(comando){
						insertLFS(comando[1], atoi(comando[2]), comando[3], atoi(comando[4]));
						free(comando[4]);
						for(int i = 0; i<4; i++)
							free(comando[i]);
							free(comando);
						}
					break;
				case CREATE:
					comando = validarComando(sPayload, 5);
					if(comando){
						createLFS(comando[1], comando[2], atoi(comando[3]), atol(comando[4]));
						for(int i = 0; i<5; i++)
							free(comando[i]);
						free(comando);
					}
					break;
				case DESCRIBE:
					comando = string_n_split(sPayload, 2, " ");
					if(comando[1]){
						retornoLista = describeLFS(comando[1]);
						free(comando[1]);
					}else{
						retornoLista = describeLFS(NULL);
					}
					free(comando[0]);
					free(comando);
					mensaje = malloc(sizeof(tPaquete));
					if(retornoLista != NULL)
					{
						mensaje->type = DESCRIBE;
						itoa(retornoLista->elements_count,mensaje->payload,10);
						mensaje->length = sizeof(mensaje->payload);
						enviarPaquete(socketActivo, mensaje,logger,"Cantidad de tablas en LFS");
						liberarPaquete(mensaje);
						list_iterate(retornoLista,(void*)enviarMensajesDESCRIBE);
						list_destroy(retornoLista);
					}
					else
					{
						mensaje->type = ERROR_EN_COMANDO;
						strcpy(mensaje->payload,"0");
						mensaje->length = sizeof(mensaje->payload);
						enviarPaquete(socketActivo, mensaje,logger,"Cantidad de tablas en LFS");
						liberarPaquete(mensaje);
					}
					break;
				case DROP:
					comando = validarComando(sPayload, 2);
					if(comando){
						dropLFS(comando[1]);
						for(int i = 0; i<2; i++)
							free(comando[i]);
						free(comando);
					}
					break;
				case DESCONEXION:
					log_info(logger, "Se desconecto un cliente, socket: %d", socketActivo);
					break;
				case HANDSHAKE:
					log_info(logger, "Memoria y LFS realizan Handshake");
					tPaquete* mensaje = malloc(sizeof(tPaquete));
					mensaje->type = HANDSHAKE;
					strcpy(mensaje->payload, string_itoa(configuracion->TAMANIO_VALUE));
					mensaje->length = sizeof(mensaje->payload);
					enviarPaquete(socketActivo, mensaje, logger, "Maximo tamanio del value");
					liberarPaquete(mensaje);
					break;
				default:
					sem_wait(&loggerSemaforo);
					log_error(logger, "Comando desconocido \"%s\"", sPayload);
					sem_post(&loggerSemaforo);
					break;
			}
		}

	}
}
void enviarMensajesDESCRIBE(char* tabla){
	tPaquete* mensaje = malloc(sizeof(tPaquete));
	mensaje->type = DESCRIBE;
	strcpy(mensaje->payload,tabla);
	mensaje->length = sizeof(mensaje->payload);
	enviarPaquete(socketActivo, mensaje,logger,"Metadata de una tabla de LFS");
	liberarPaquete(mensaje);
}
//MAIN DE TESTS
int main2(){
	levantarLFS();
	crearHiloIndependiente(&hiloConfig,(void*)cambiosConfigLFS, NULL, "LFS config");
	crearHilo(&hiloAPI,(void*)API_LFS, NULL, "LFS API");
	crearHiloIndependiente(&hiloDump,(void*)dumpLFS, NULL, "LFS Dump");
	joinearHilo(hiloAPI, NULL, "LFS API");


	destruirLFS();
	puts("TERMINE");
	return 0;

}
int main1(){
	levantarLFS();
	crearHiloIndependiente(&hiloConfig,(void*)cambiosConfigLFS, NULL, "LFS config");
	crearHiloIndependiente(&hiloDump,(void*)dumpLFS, NULL, "LFS Dump");

	createLFS("A", "SC", 5, 50000);
	for(int i = 0; i<500; i++)
		insertLFS("A", i, "value 1", 0);
	sleep(60);
	for(int i = 0; i<500; i++)
		insertLFS("A", i, "value 2", 0);
	sleep(60);
	for(int i = 0; i<500; i++)
		insertLFS("A", i, "value 3", 0);
	sleep(60);

	destruirLFS();
	puts("TERMINE");
	return 0;
}

//----------------- FUNCIONES DE ESTRUCTURAS -----------------//

Tabla* crearTabla(char* nombreTabla, char* consistencia, int particiones, long tiempoCompactacion){
	Tabla *nuevaTabla = malloc(sizeof(Tabla));
	nuevaTabla->nombreTabla = string_from_format("%s", nombreTabla);
	MetadataLFS *metadata = malloc(sizeof(MetadataLFS));
	strcpy(metadata->consistencia, consistencia);
	metadata->particiones = particiones;
	metadata->tiempoCompactacion = tiempoCompactacion;
	nuevaTabla->metadata = metadata;
	nuevaTabla->registro = list_create();
	sem_init(&nuevaTabla->semaforo, 1, 1);
	pthread_create(&nuevaTabla->hiloCompactacion, NULL, (void*)compactacion, (void*)nuevaTabla->nombreTabla);
	pthread_detach(nuevaTabla->hiloCompactacion);

	return nuevaTabla;
}
void tablaDestruir(Tabla* tabla){
	pthread_cancel(tabla->hiloCompactacion);
	sem_wait(&tabla->semaforo);
	free(tabla->metadata);
	free(tabla->nombreTabla);
	list_destroy_and_destroy_elements(tabla->registro, (void*) RegistroLFSDestruir);
	free(tabla);
}
void tablaRemover(char* nombreTabla){
	sem_wait(&memtableSemaforo);
	for(int i = 0; i<tablasLFS->elements_count; i++){
		Tabla* t = list_get(tablasLFS, i);
		if(string_equals_ignore_case(t->nombreTabla, nombreTabla)){
			list_remove_and_destroy_element(tablasLFS, i, (void*)tablaDestruir);
			break;
		}
	}
	sem_post(&memtableSemaforo);
}
void mostrarTablas(){
	Tabla *t;
	sem_wait(&memtableSemaforo);
	for(int i = 0; i<tablasLFS->elements_count; i++){
		t = list_get(tablasLFS, i);
		sem_wait(&t->semaforo);
		printf("tabla: %s particiones=%d\n", t->nombreTabla, t->metadata->particiones);
		sem_post(&t->semaforo);
	}
	sem_post(&memtableSemaforo);
}
Tabla* tablaEncontrar(char* nombre){
	int _is_the_one(Tabla *t) {
		return string_equals_ignore_case(t->nombreTabla, nombre);
	}
	sem_wait(&memtableSemaforo);
	Tabla* tabla = list_find(tablasLFS, (void*) _is_the_one);
	sem_post(&memtableSemaforo);
	return tabla;
}
RegistroLFS* RegistroLFSCrear(uint16_t key, uint64_t timestamp, char* value){
	RegistroLFS *registro = malloc(sizeof(RegistroLFS));
	registro->key = key;
	registro->timestamp = timestamp;
	registro->value = string_duplicate(value);
	return registro;
}
void RegistroLFSDestruir(RegistroLFS* registro){
	free(registro->value);
	free(registro);
}
RegistroLFS* registroEncontrar(Tabla* tabla, uint16_t key){
	//RETORNA NULL SI EL REGISTRO NO EXISTE EN LA MEMTABLE
	RegistroLFS* registro = NULL;
	sem_wait(&tabla->semaforo);
	if(tabla->registro->elements_count > 0){
		for(int i = 0; i < tabla->registro->elements_count; i++){
			RegistroLFS* registroEncontrado = list_get(tabla->registro, i);
			if(registroEncontrado->key == key && (!registro || registroEncontrado->timestamp > registro->timestamp))
				registro = registroEncontrado;
		}
	}
	sem_post(&tabla->semaforo);
	return registro;
}
RegistroLFS* registroEncontrarArray(uint16_t key, char* array){
	char* value = string_new();
	uint64_t timestampMayor = 0;
	int numero = 0;
	char* timestampArray = string_new();
	uint64_t timestampNuevo = 0;
	char* keyArray = string_new();
	char* valueNuevo = string_new();

	for(int i = 0; i!=string_length(array); i++){
		if(array[i]=='\n'){
			timestampNuevo = strtoull(timestampArray, NULL, 0);
			if(atoi(keyArray) == key && timestampNuevo > timestampMayor){
				timestampMayor = timestampNuevo;
				free(value);
				value = string_from_format("%s", valueNuevo);
			}
			free(timestampArray);
			timestampArray = string_new();
			free(keyArray);
			keyArray = string_new();
			free(valueNuevo);
			valueNuevo = string_new();
			numero = 0;
		}
		else{
			if(array[i]==';')
				numero++;
			else switch(numero){
					case 0:;
						char* a = string_duplicate(timestampArray);
						free(timestampArray);
						timestampArray = string_from_format("%s%c", a, array[i]);
						free(a);
						break;
					case 1:;
						char* b = string_duplicate(keyArray);
						free(keyArray);
						keyArray = string_from_format("%s%c", b, array[i]);
						free(b);
						break;
					case 2:;
						char* c = string_duplicate(valueNuevo);
						free(valueNuevo);
						valueNuevo = string_from_format("%s%c", c, array[i]);
						free(c);
						break;
				}
		}
	}

	free(valueNuevo);
	free(keyArray);
	free(timestampArray);


	if(value){
		RegistroLFS* reg = RegistroLFSCrear(key, timestampMayor, value);
		free(value);
		return reg;
	}
	else{
		return NULL;
	}
}
RegistroLFS* registroEncontrarLista(t_list* lista, uint16_t key){
	//RETORNA EL PRIMER REGISTRO CON ESA KEY DE LA LISTA
	//RETORNA NULL SI LA KEY NO EXISTE EN LA LISTA
	RegistroLFS* registro = NULL;
	for(int i = 0; i < lista->elements_count; i++){
		RegistroLFS* registroEncontrado = list_get(lista, i);
		if(registroEncontrado->key == key){
			registro = registroEncontrado;
			break;
		}
	}
	return registro;
}
t_list* encontrarRegistros(t_list* lista, int particiones, int particion){
	//RETORNA UNA LISTA CON LOS REGISTROS QUE PERTENECEN A LA PARTICION
	//RETORNA NULL SI NO HAY REGISTROS
	t_list* ret = list_create();
	for(int i = 0; i<lista->elements_count; i++){
		RegistroLFS* reg = list_get(lista, i);
		if(reg->key % particiones == particion){
			list_add(ret, reg);
		}
	}
	if(list_is_empty(ret)){
		list_destroy(ret);
		return NULL;
	}

	return ret;
}
char* encontrarYComprimirRegistros(t_list* lista, int particiones, int particion){
	//RETORNA LOS REGISTROS COMPRIMIDOS QUE PERTENECEN A LA PARTICION
	//RETORNA NULL SI NO HAY REGISTROS
	char* ret = string_new();
	for(int i = 0; i<lista->elements_count; i++){
		RegistroLFS* reg = list_get(lista, i);
		if(reg->key % particiones == particion){
			char* comprimido = comprimirRegistro(reg);
			string_append(&ret, comprimido);
			free(comprimido);
		}
	}

	if(string_is_empty(ret)){
		free(ret);
		return NULL;
	}

	return ret;
}
void agregarRegistros(t_list* registros, char* array){
	int numero = 0;
	char* value = string_new();
	char* timestampArray = string_new();
	char* keyArray = string_new();

	for(int i = 0; i!=string_length(array); i++){
		if(array[i]=='\n'){
			RegistroLFS* reg = registroEncontrarLista(registros, atoi(keyArray));
			if(reg){
				uint64_t timestamp = 0;
				sscanf(timestampArray,"%" SCNu64, &timestamp);
				if(timestamp > reg->timestamp){
					reg->timestamp = timestamp;
					free(reg->value);
					reg->value = string_duplicate(value);
				}
			}else{
				list_add(registros, RegistroLFSCrear(atoi(keyArray), atoi(timestampArray), value));
			}
			free(timestampArray);
			timestampArray = string_new();
			free(keyArray);
			keyArray = string_new();
			free(value);
			value = string_new();
			numero = 0;
		}
		else{
			if(array[i]==';')
				numero++;
			else switch(numero){
					case 0:;
						char* a = string_duplicate(timestampArray);
						free(timestampArray);
						timestampArray = string_from_format("%s%c", a, array[i]);
						free(a);
						break;
					case 1:;
						char* b = string_duplicate(keyArray);
						free(keyArray);
						keyArray = string_from_format("%s%c", b, array[i]);
						free(b);
						break;
					case 2:;
						char* c = string_duplicate(value);
						free(value);
						value = string_from_format("%s%c", c, array[i]);
						free(c);
						break;
				}
		}
	}

	free(value);
	free(keyArray);
	free(timestampArray);
}
int cantDigitos(uint64_t numero){
    int digitos = 0;
    if (numero < 0)
    	digitos = 1;
    while (numero) {
        numero /= 10;
        digitos++;
    }
    return digitos;
}
char* comprimirRegistro(RegistroLFS* reg){
	char* comprimido;
	comprimido = (char*) malloc(cantDigitos(reg->timestamp) + cantDigitos(reg->key) + string_length(reg->value) + 5);
	sprintf(comprimido, "%" PRIu64 ";%hd;%s\n", reg->timestamp, reg->key, reg->value);
	return comprimido;
}
void mostrarRegistros(char* nombre){
	Tabla* tabla = tablaEncontrar(nombre);
	sem_wait(&tabla->semaforo);
	printf("Tabla %s (%d)\n", nombre, tabla->registro->elements_count);
	for(int i = 0; i < tabla->registro->elements_count; i++){
		RegistroLFS* registro = list_get(tabla->registro, i);
		printf("key: %hd timestamp: %" PRIu64 " value: %s\n", registro->key, registro->timestamp, registro->value);
	}
	sem_post(&tabla->semaforo);
}
void limpiarMemtable(){
	sem_wait(&memtableSemaforo);
	for(int j = 0; j<tablasLFS->elements_count; j++){
		Tabla* t = list_get(tablasLFS, j);
		limpiarTablaMemtable(t);
	}
	sem_post(&memtableSemaforo);
}
void limpiarTablaMemtable(Tabla* tabla){
	sem_wait(&tabla->semaforo);
	list_destroy_and_destroy_elements(tabla->registro, (void*) RegistroLFSDestruir);
	tabla->registro = list_create();
	sem_post(&tabla->semaforo);
}


//------------------------ COMANDOS --------------------------------//

void createLFS(char* nombreTabla, char* consistencia, int particiones, long tiempoCompactacion){
	if(strcmp(consistencia, "SC") && strcmp(consistencia, "SHC") && strcmp(consistencia, "EC")){
		sem_wait(&loggerSemaforo);
		log_error(logger, "Consistencia invalida \"%s\" al crear la tabla \"%s\"", consistencia, nombreTabla);
		sem_post(&loggerSemaforo);
		return;
	}
	if(!particiones){
		sem_wait(&loggerSemaforo);
		log_error(logger, "Error al crear la tabla \"%s\" con 0 particiones", consistencia, nombreTabla);
		sem_post(&loggerSemaforo);
		return;
	}
	if(!tiempoCompactacion){
		sem_wait(&loggerSemaforo);
		log_error(logger, "Error al crear la tabla \"%s\" con tiempo de compactacion = 0", consistencia, nombreTabla);
		sem_post(&loggerSemaforo);
		return;
	}
	sleep(configuracion->RETARDO / 1000);
	if(tablaEncontrar(nombreTabla)!=NULL){
		sem_wait(&loggerSemaforo);
		log_error(logger, "Ya existe una tabla con el nombre \"%s\"", nombreTabla);
		sem_post(&loggerSemaforo);
		return;
	}
	createFS(nombreTabla, consistencia, particiones, tiempoCompactacion);
}
void insertLFS(char* nombreTabla, uint16_t key, char* value, uint64_t timestamp){
	if(sizeof(value) > configuracion->TAMANIO_VALUE){
		sem_wait(&loggerSemaforo);
		log_error(logger, "Tamaño del value mayor que el permitido");
		sem_post(&loggerSemaforo);
		return;
	}
	if(string_contains(value, ";")){
		sem_wait(&loggerSemaforo);
		log_error(logger, "Value no puede contener ;");
		sem_post(&loggerSemaforo);
		return;
	}
	sleep(configuracion->RETARDO / 1000);
	Tabla* t = tablaEncontrar(nombreTabla);
	if(!t){
		sem_wait(&loggerSemaforo);
		log_error(logger, "Tabla \"%s\" no encontrada", nombreTabla);
		sem_post(&loggerSemaforo);
		return;
	}

	if(!timestamp)
		timestamp = getCurrentTime();

	list_add(tablaEncontrar(nombreTabla)->registro, RegistroLFSCrear(key, timestamp, value));
	sem_wait(&loggerSemaforo);
	log_info(logger, "Insertado registro tabla \"%s\" key %hd value \"%s\"", nombreTabla, key, value);
	sem_post(&loggerSemaforo);
}
char* selectLFS(char* nombreTabla, uint16_t key){
	sleep(configuracion->RETARDO / 1000);
	Tabla* tabla = tablaEncontrar(nombreTabla);
	if(!tabla){
		sem_wait(&loggerSemaforo);
		log_error(logger, "Tabla \"%s\" no encontrada", nombreTabla);
		sem_post(&loggerSemaforo);
		return NULL;
	}

	RegistroLFS* registro = registroEncontrar(tabla, key);
	if(registro){
		char* value = string_from_format("%s", registro->value);
		sem_wait(&loggerSemaforo);
		log_info(logger, "Select tabla \"%s\" key %hd value \"%s\"", nombreTabla, key, value);
		sem_post(&loggerSemaforo);
		//free(value);
		return value;
	}
	else{
		return selectFS(tabla->nombreTabla, key%tabla->metadata->particiones, key);
	}
}
t_list* describeLFS(char* nombreTabla){
	sleep(configuracion->RETARDO / 1000);
	t_list* listaTablas = list_create();
	char* tabla_s;
	if(nombreTabla){
		Tabla* tabla = tablaEncontrar(nombreTabla);
		if(tabla){
			sem_wait(&tabla->semaforo);
			sem_wait(&loggerSemaforo);
			log_info(logger, "Describe Tabla %s: consistencia %s particiones %d tiempo compactacion %ld", nombreTabla, tabla->metadata->consistencia, tabla->metadata->particiones, tabla->metadata->tiempoCompactacion);
			sem_post(&loggerSemaforo);
			tabla_s = string_from_format("%s,%s,%d,%ld;",nombreTabla, tabla->metadata->consistencia, tabla->metadata->particiones, tabla->metadata->tiempoCompactacion);
			list_add(listaTablas,tabla_s);
			sem_post(&tabla->semaforo);
		}else{
			sem_wait(&loggerSemaforo);
			log_error(logger, "Tabla \"%s\" no encontrada", nombreTabla);
			sem_post(&loggerSemaforo);
		}
	}else{
		sem_wait(&memtableSemaforo);
		for(int i = 0; i<tablasLFS->elements_count; i++){
			Tabla* tabla = list_get(tablasLFS, i);
			sem_wait(&tabla->semaforo);
			sem_wait(&loggerSemaforo);
			log_info(logger, "Describe Tabla %s: consistencia %s particiones %d tiempo compactacion %ld", tabla->nombreTabla, tabla->metadata->consistencia, tabla->metadata->particiones, tabla->metadata->tiempoCompactacion);
			tabla_s = string_from_format("%s,%s,%d,%ld;",nombreTabla, tabla->metadata->consistencia, tabla->metadata->particiones, tabla->metadata->tiempoCompactacion);
			list_add(listaTablas,tabla_s);
			sem_post(&loggerSemaforo);
			sem_post(&tabla->semaforo);
		}
		sem_post(&memtableSemaforo);
	}
	return listaTablas;
}
void dropLFS(char* nombreTabla){
	sleep(configuracion->RETARDO / 1000);
	Tabla* tabla = tablaEncontrar(nombreTabla);
	if(!tabla){
		sem_wait(&loggerSemaforo);
		log_error(logger, "Tabla \"%s\" no encontrada", nombreTabla);
		sem_post(&loggerSemaforo);
		return;
	}
	tablaRemover(nombreTabla);
	dropFS(nombreTabla);
	sem_wait(&loggerSemaforo);
	log_info(logger, "Drop tabla \"%s\"", nombreTabla);
	sem_post(&loggerSemaforo);
}

void dumpLFS(){
	long tiempo;
	while(1){
		sem_wait(&configSemaforo);
		tiempo = configuracion->TIEMPO_DUMP/1000;
		sem_post(&configSemaforo);
		//printf("Dump : %hd\n", tiempo);
		sleep(tiempo);
		sem_wait(&dumpSemaforo);
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		dump();
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		sem_post(&dumpSemaforo);
	}
}

void compactacion(char* nombreTabla){
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	Tabla* t = tablaEncontrar(nombreTabla);
	long tiempo = t->metadata->tiempoCompactacion/1000;
	while(1){
		sleep(tiempo);
		sem_wait(&t->semaforo);
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		compactar(nombreTabla);
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	}
}
