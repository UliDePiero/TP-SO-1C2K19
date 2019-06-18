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
	strcpy(configuracion->PUNTO_MONTAJE, archivoConfigSacarStringDe(archivoConfig, "PUNTO_MONTAJE"));
	configuracion->RETARDO = archivoConfigSacarIntDe(archivoConfig, "RETARDO");
	configuracion->TAMANIO_VALUE = archivoConfigSacarIntDe(archivoConfig, "TAMANIO_VALUE");
	configuracion->TIEMPO_DUMP = archivoConfigSacarIntDe(archivoConfig, "TIEMPO_DUMP");

	archivoConfigDestruir(archivoConfig);
}

void cambiosConfigLFS(){
	if(configModificado()){
		t_config* archivoConfig = config_create(RUTA_CONFIG);
		configuracion->RETARDO = archivoConfigSacarIntDe(archivoConfig, "RETARDO");
		configuracion->TIEMPO_DUMP = archivoConfigSacarIntDe(archivoConfig, "TIEMPO_DUMP");
		archivoConfigDestruir(archivoConfig);
	}
}
void destruirLFS(){
	sem_wait(&dumpSemaforo);
	//TODO: pthread_cancel(hiloDump);
	destruirFileSystem();
	sem_wait(&configSemaforo);
	free(configuracion);
	sem_wait(&memtableSemaforo);
	list_destroy_and_destroy_elements(tablasLFS, (void*) tablaDestruir);
}

int main(){
	tablasLFS = list_create();
	sem_init(&memtableSemaforo, 1, 1);
	sem_init(&dumpSemaforo, 1, 1);
	logger = log_create(logFile, "LFS",true, LOG_LEVEL_INFO);
	configuracion = malloc(sizeof(ConfiguracionLFS));
	configurar(configuracion);
	levantarFileSystem();


	//servidor
	//FUNCIONES SOCKETS (Usar dependiendo de la biblioteca que usemos)

	/*socketEscucha= levantarServidorIPautomatica(configuracion->PUERTO_ESCUCHA, BACKLOG); //BACKLOG es la cantidad de clientes que pueden conectarse a este servidor
	socketActivo = aceptarComunicaciones(socketEscucha);*/

	fd_set setSocketsOrquestador;
	FD_ZERO(&setSocketsOrquestador);

	// Inicializacion de sockets y actualizacion del log

	socketEscucha = crearSocketEscucha(configuracion->PUERTO_ESCUCHA, logger);

	//crearHiloIndependiente(&hiloCompactador,(void*)compactacion, NULL, "LFS");
	//pthread_create(&hiloCompactador, NULL, (void*)compactacion, NULL);
	//pthread_join(hiloCompactador, NULL);

	crearHiloIndependiente(&hiloAPI,(void*)API_LFS, NULL, "LFS API");
	crearHiloIndependiente(&hiloDump,(void*)dumpLFS, NULL, "LFS Dump");

	//ESTO TIENE QUE IR EN UN HILO APARTE PARA QUE QUEDE EN LOOP  ???
	FD_SET(socketEscucha, &setSocketsOrquestador);
	maxSock = socketEscucha;
	tMensaje tipoMensaje;
	char * sPayload;
	while (tipoMensaje != DESCONEXION) {

		puts("Escuchando");
		socketActivo = getConnection(&setSocketsOrquestador, &maxSock, socketEscucha, &tipoMensaje, &sPayload, logger);
		printf("Socket comunicacion: %d \n", socketActivo);//CORREGIR getConnection
		if (socketActivo != -1) {

			switch (tipoMensaje) {
				char** comando;
				case CREATE:
					printf("\nRecibi CREATE\n");
					comando = string_n_split(sPayload, 5, " ");
					if(comandoValido(5, comando))
						createLFS(comando[1], comando[2], atoi(comando[3]), atoi(comando[4]));
					break;
				case DROP:
					printf("\nRecibi DROP\n");
					comando = string_n_split(sPayload, 5, " ");
					if(comandoValido(2, comando))
						dropLFS(comando[1]);
					break;
				case DESCONEXION:
					printf("\nSe desconecto un cliente, socket: %d\n", socketActivo);
					destruirLFS();
					break;
				default:
					printf("Tipo de mensaje desconocido \n");
					break;
			}
		}

	}

}

//MAIN DE TESTS
int main98(){
	tablasLFS = list_create();
	//logger = log_create(logFile, "LFS",true, LOG_LEVEL_INFO);
	configuracion = malloc(sizeof(ConfiguracionLFS));
	configurar(configuracion);
	levantarFileSystem();

	createFS("A", "SC", 5, 123);
	createFS("A", "SC", 5, 123);
	createFS("B", "SC", 2, 123);
	createFS("C", "SC", 3, 123);

	/*char* value = selectLFS("A", 1);
	printf("value:%s key:1\n", value);
	value = selectLFS("A", 19);
	printf("value:%s key:19\n", value);
	free(value);
	value = selectLFS("A", 200);
	printf("value:%s key:200\n", value);
	free(value);*/

	//crearHilo(&hiloAPI,(void*)API_LFS, NULL, "LFS"); //LEVANTO API
	//joinearHilo(hiloAPI, NULL, "LFS");

	//limpiarMemtable();
	//mostrarRegistros("A");
	//mostrarRegistros("B");

	/*Tabla* t = list_get(tablasLFS, 1);
	char* registrosComprimidos = string_new();
	for(int i = 0; i<t->registro->elements_count; i++){
		RegistroLFS* reg = list_get(t->registro, i);
		char* comprimido = comprimirRegistro(reg);
		string_append(&registrosComprimidos, comprimido);
		free(comprimido);
	}
	printf("Registros: \n%s", registrosComprimidos);
	free(registrosComprimidos);*/

	for(int i = 0; i<20; i++){
		insertLFS("A", i, "value", 1);
	}
	//dump();
	destruirLFS();
	puts("TERMINE");
	return 0;
}
int main99(){
	tablasLFS = list_create();
	sem_init(&memtableSemaforo, 1, 1);
	sem_init(&dumpSemaforo, 1, 1);
	configuracion = malloc(sizeof(ConfiguracionLFS));
	configurar(configuracion);
	levantarFileSystem();
	crearHilo(&hiloAPI,(void*)API_LFS, NULL, "LFS API");
	//crearHiloIndependiente(&hiloDump,(void*)dumpLFS, NULL, "LFS Dump");
	joinearHilo(hiloAPI, NULL, "LFS API");

	/*createLFS("A", "SC", 5, 5000);
	insertLFS("A", 1, "value1", 1);
	insertLFS("A", 2, "value2", 1);
	insertLFS("A", 3, "value3", 1);
	insertLFS("A", 4, "value4", 1);

	sleep(10);
	dropLFS("A");
	sleep(9);*/

	destruirLFS();
	puts("TERMINE");
	return 0;
}

void createLFS(char* nombreTabla, char* consistencia, int particiones, long tiempoCompactacion){
	if(strcmp(consistencia, "SC") && strcmp(consistencia, "SHC") && strcmp(consistencia, "EC")){
		perror("Consistencia invalida");
		return;
	}
	sleep(configuracion->RETARDO / 1000);
	if(tablaEncontrar(nombreTabla)!=NULL){
		perror("Ya existe una tabla con ese nombre");
		return;
	}
	createFS(nombreTabla, consistencia, particiones, tiempoCompactacion);
	//printf("\nSe creo la tabla: %s\n",nombreTabla);
}
void insertLFS(char* nombreTabla, uint16_t key, char* value, int timestamp){
	if(sizeof(value) > configuracion->TAMANIO_VALUE){
		perror("Tamaño del value mayor que el permitido");
		return;
	}
	if(string_contains(value, ";")){
		perror("Value no puede contener ;");
		return;
	}
	sleep(configuracion->RETARDO / 1000);
	Tabla* t = tablaEncontrar(nombreTabla);
	if(!t){
		perror("Tabla no encontrada");
		return;
	}

	if(timestamp == 0)
		timestamp = (int)time(NULL);

	list_add(tablaEncontrar(nombreTabla)->registro, RegistroLFSCrear(key, timestamp, value));
}
char* selectLFS(char* nombreTabla, uint16_t key){
	//TODO: poner semaforos en select
	sleep(configuracion->RETARDO / 1000);
	Tabla* tabla = tablaEncontrar(nombreTabla);
	if(!tabla){
		perror("Tabla no encontrada");
		return NULL;
	}

	RegistroLFS* registro = registroEncontrar(tabla, key);
	if(registro){
		char* value = string_from_format("%s", registro->value);
		return value;
	}
	else{
		return selectFS(tabla->nombreTabla, key%tabla->metadata->particiones, key);
	}
}
void dropLFS(char* nombreTabla){
	sem_wait(&memtableSemaforo);
	for(int i = 0; i<tablasLFS->elements_count; i++){
		Tabla* t = list_get(tablasLFS, i);
		if(string_equals_ignore_case(t->nombreTabla, nombreTabla))
			list_remove_and_destroy_element(tablasLFS, i, (void*)tablaDestruir);
	}
	sem_post(&memtableSemaforo);
	dropFS(nombreTabla);
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
	pthread_create(&nuevaTabla->hiloCompactacion, NULL, (void*)compactar, nuevaTabla->nombreTabla);
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
RegistroLFS* RegistroLFSCrear(uint16_t key, int timestamp, char* value){
	RegistroLFS *registro = malloc(sizeof(RegistroLFS));
	registro->key = key;
	registro->timestamp = timestamp;
	registro->value = string_from_format("%s", value);
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
	//TODO: eliminar leaks
	char* value = string_new();
	int timestampMayor = 0;
	int numero = 0;
	char* timestampArray = string_new();
	char* keyArray = string_new();
	char* valueNuevo = string_new();

	for(int i = 0; i!=string_length(array); i++){
		if(array[i]=='\n'){
			if(atoi(keyArray) == key && atoi(timestampArray) > timestampMayor){
				timestampMayor = atoi(timestampArray);
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
					case 0:
						timestampArray = string_from_format("%s%c", timestampArray, array[i]);
						break;
					case 1:
						keyArray = string_from_format("%s%c", keyArray, array[i]);
						break;
					case 2:
						valueNuevo = string_from_format("%s%c", valueNuevo, array[i]);
						break;
				}
		}
	}
	/*if(atoi(keyArray) == key && atoi(timestampArray) > timestampMayor){
		timestampMayor = atoi(timestampArray);
		free(value);
		value = string_from_format("%s", valueNuevo);
	}*/

	free(valueNuevo);
	free(keyArray);
	free(timestampArray);


	if(string_is_empty(value)){
		free(value);
		return NULL;
	}
	else{
		RegistroLFS* reg = RegistroLFSCrear(key, timestampMayor, value);
		return reg;
	}
}
int cantDigitos(int numero){
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
	char * comprimido;
	comprimido = (char*) malloc(cantDigitos(reg->timestamp) + cantDigitos(reg->key) + string_length(reg->value) + 5) /** sizeof(char)*/;
	//char* comprimido = malloc(sizeof(reg->key)+sizeof(reg->timestamp)+sizeof(reg->value)+5);
	sprintf(comprimido, "%d;%hd;%s\n", reg->timestamp, reg->key, reg->value);
	return comprimido;
}
void mostrarRegistros(char* nombre){
	Tabla* tabla = tablaEncontrar(nombre);
	sem_wait(&tabla->semaforo);
	printf("Tabla %s (%d)\n", nombre, tabla->registro->elements_count);
	for(int i = 0; i < tabla->registro->elements_count; i++){
		RegistroLFS* registro = list_get(tabla->registro, i);
		printf("key: %hd timestamp: %d value: %s\n", registro->key, registro->timestamp, registro->value);
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

void dumpLFS(){
	sem_wait(&configSemaforo);
	unsigned int tiempo = configuracion->TIEMPO_DUMP/1000;
	sem_post(&configSemaforo);
	while(1){
		sleep(tiempo);
		sem_wait(&dumpSemaforo);
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		dump();
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		sem_post(&dumpSemaforo);
	}
}

void compactacion(char* nombreTabla){
	Tabla* t = tablaEncontrar(nombreTabla);
	int tiempo = t->metadata->tiempoCompactacion;
	while(1){
		sleep(tiempo);
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		compactar(nombreTabla);
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	}
}

//----------------- FUNCIONES AUXILIARES -----------------//

void str_to_uint16(const char *str, uint16_t *res){
  char *end;
  errno = 0;
  intmax_t val = strtoimax(str, &end, 10);
  /*if (errno == ERANGE || val < 0 || val > UINT16_MAX || end == str || *end != '\0')
    return false;*/
  *res = (uint16_t) val;
  //return true;
}
// inline function to swap two numbers
void swap(char *x, char *y) {
	char t = *x; *x = *y; *y = t;
}
// function to reverse buffer[i..j]
char* reverse(char *buffer, int i, int j)
{
	while (i < j)
		swap(&buffer[i++], &buffer[j--]);

	return buffer;
}
// Iterative function to implement itoa() function in C
char* itoa(int value, char* buffer, int base)
{
	// invalid input
	if (base < 2 || base > 32)
		return buffer;

	// consider absolute value of number
	int n = abs(value);

	int i = 0;
	while (n)
	{
		int r = n % base;

		if (r >= 10)
			buffer[i++] = 65 + (r - 10);
		else
			buffer[i++] = 48 + r;

		n = n / base;
	}

	// if number is 0
	if (i == 0)
		buffer[i++] = '0';

	// If base is 10 and value is negative, the resulting string
	// is preceded with a minus sign (-)
	// With any other base, value is always considered unsigned
	if (value < 0 && base == 10)
		buffer[i++] = '-';

	buffer[i] = '\0'; // null terminate string

	// reverse the string and return it
	return reverse(buffer, 0, i - 1);
}
