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
	desconectarseDe(socketActivo);
	desconectarseDe(socketEscucha);
	destruirFileSystem();
	free(configuracion);
	list_destroy_and_destroy_elements(tablasLFS, (void*) tablaDestruir);
}

int main()
{
	tablasLFS = list_create();
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

	crearHiloIndependiente(&hiloAPI,(void*)API_LFS, NULL, "LFS");

	//ESTO TIENE QUE IR EN UN HILO APARTE PARA QUE QUEDE EN LOOP  ???
	FD_SET(socketEscucha, &setSocketsOrquestador);
	maxSock = socketEscucha;
	tMensaje tipoMensaje;
	char * sPayload;
	while (1) {

		puts("Escuchando");
		socketActivo = getConnection(&setSocketsOrquestador, &maxSock, socketEscucha, &tipoMensaje, &sPayload, logger);
		printf("Socket comunicacion: %d \n", socketActivo);//CORREGIR getConnection
		if (socketActivo != -1) {

			switch (tipoMensaje) {
							case CREATE:
								printf("\nRecibi CREATE\n");
								//funcion CREATE
								break;

							default:
								printf("Tipo de mensaje desconocido \n");
								break;

			}
		}

	}

	destruirLFS();
}
//MAIN DE TESTS
int main2(){
	tablasLFS = list_create();
	//logger = log_create(logFile, "LFS",true, LOG_LEVEL_INFO);
	configuracion = malloc(sizeof(ConfiguracionLFS));
	configurar(configuracion);
	levantarFileSystem();

	/*createFS("A", "SC", 5, 123);
	createFS("A", "SC", 5, 123);
	createFS("B", "SC", 2, 123);
	createFS("C", "SC", 3, 123);*/

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
	dump();
	destruirLFS();
	puts("TERMINE");
}


//--------------------------------------------------------//


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


//---------------------------------------------------------//


//--------------------------------------------------------//




void createLFS(char* nombreTabla, char* consistencia, int particiones, long tiempoCompactacion){
	if(strcmp(consistencia, "SC") && strcmp(consistencia, "SHC") && strcmp(consistencia, "EC")){
		perror("Consistencia invalida");
		return;
	}
	if(tablaEncontrar(nombreTabla)!=NULL){
		perror("Ya existe una tabla con ese nombre");
		return;
	}
	createFS(nombreTabla, consistencia, particiones, tiempoCompactacion);
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
	/*
	//int numeroTabla = 0;
	//MetadataLFS* metadataTabla;
	Tabla* tablaEncontrada = tablaEncontrar(nombreTabla);
	char* strParticion = NULL;
	RegistroLFS* registroEncontrado = NULL;
	char* direccion = NULL;
	//if(tablaEncontrar(nombreTabla)!=NULL){
	if(tablaEncontrada != NULL){
		//metadataTabla = tablaEncontrada->metadata;
		//Calcular cual es la partición que contiene dicho KEY.
		strParticion = itoa(key % tablaEncontrada->metadata->particiones, strParticion, 10);
		//Escanear la partición objetivo, todos los archivos temporales y la memoria temporal de dicha tabla (si existe) buscando la key deseada.
		//Faltaría buscar en archivos temporales y memtable
		strcpy (direccion,"/Tables/");
		strcat (direccion,nombreTabla);
		strcat (direccion,"/");
		strcat (direccion,strParticion);
		strcat (direccion, ".bin");
		FILE* particion = fopen(direccion, "r");
		while (!feof(particion)){
			char* buffer = NULL;
			RegistroLFS* registro = NULL;
			fread(buffer, 100, 1, particion);//Modificar el valor fijo de la cantidad de caracteres a leer
			registro->timestamp = atoi(strtok(buffer, ";"));
			str_to_uint16(strtok(NULL, ";"), &registro->key);
			registro->value = atoi(strtok(NULL, ";"));
			if (registro->key == key){
				if (registroEncontrado == NULL)
					registroEncontrado = registro;
				else{
					if (registro->timestamp > registroEncontrado->timestamp)
						registroEncontrado = registro;
				}
			}
		}
		return registroEncontrado;
		//Encontradas las entradas para dicha Key, se retorna el valor con el Timestamp más grande.
		//return NULL; //CAMBIAR RETURN
	}
	else{
		perror("No se encontro la tabla solicitada.");
		return NULL;
	}*/

	Tabla* tabla = tablaEncontrar(nombreTabla);
	if(!tabla){
		perror("Tabla no encontrada");
		return NULL;
	}

	RegistroLFS* registro = registroEncontrar(tabla, key);
	if(registro)
		return registro->value;
	else{
		return selectFS(tabla->nombreTabla, key%tabla->metadata->particiones, key);
	}
}

//Funciones de estructuras
Tabla* crearTabla(char* nombreTabla, char* consistencia, int particiones, long tiempoCompactacion){
	Tabla *nuevaTabla = malloc(sizeof(Tabla));
	nuevaTabla->nombreTabla = nombreTabla;
	MetadataLFS *metadata = malloc(sizeof(MetadataLFS));
	strcpy(metadata->consistencia, consistencia);
	metadata->particiones = particiones;
	metadata->tiempoCompactacion = tiempoCompactacion;
	nuevaTabla->metadata = metadata;
	nuevaTabla->registro = list_create();

	return nuevaTabla;
}
void tablaDestruir(Tabla* tabla){
	free(tabla->metadata);
	list_destroy_and_destroy_elements(tabla->registro, (void*) RegistroLFSDestruir);
	free(tabla);
}
void mostrarTablas(){
	Tabla *t;
	for(int i = 0; i<tablasLFS->elements_count; i++){
		t = list_get(tablasLFS, i);
		printf("tabla: %s particiones=%d\n", t->nombreTabla, t->metadata->particiones);
	}
}
Tabla* tablaEncontrar(char* nombre){
	int _is_the_one(Tabla *t) {
		return string_equals_ignore_case(t->nombreTabla, nombre);
	}
	return list_find(tablasLFS, (void*) _is_the_one);
}
RegistroLFS* RegistroLFSCrear(uint16_t key, int timestamp, char* value){
	RegistroLFS *registro = malloc(sizeof(RegistroLFS));
	registro->key = key;
	registro->timestamp = timestamp;
	registro->value = value;
	return registro;
}
void RegistroLFSDestruir(RegistroLFS* registro){
	free(registro);
}
RegistroLFS* registroEncontrar(Tabla* tabla, uint16_t key){
	//RETORNA NULL SI EL REGISTRO NO EXISTE EN LA MEMTABLE
	RegistroLFS* registro;
	if(tabla->registro->elements_count > 0){
		for(int i = 0; i < tabla->registro->elements_count; i++){
			RegistroLFS* registroEncontrado = list_get(tabla->registro, i);
			if(registroEncontrado->key == key && (!registro || registroEncontrado->timestamp > registro->timestamp))
				registro = registroEncontrado;
		}
	}

	return registro;
}
char* comprimirRegistro(RegistroLFS* reg){
	char* comprimido = malloc(sizeof(reg->key)+sizeof(reg->timestamp)+sizeof(reg->value)+5);
	sprintf(comprimido, "%d;%hd;%s\n", reg->timestamp, reg->key, reg->value);
	return comprimido;
}
void mostrarRegistros(char* nombre){
	Tabla* tabla = tablaEncontrar(nombre);
	printf("Tabla %s (%d)\n", nombre, tabla->registro->elements_count);
	for(int i = 0; i < tabla->registro->elements_count; i++){
		RegistroLFS* registro = list_get(tabla->registro, i);
		printf("key: %hd timestamp: %d value: %s\n", registro->key, registro->timestamp, registro->value);
	}
}
void limpiarMemtable(){
	for(int j = 0; j<tablasLFS->elements_count; j++){
		Tabla* t = list_get(tablasLFS, j);
		list_destroy_and_destroy_elements(t->registro, (void*) RegistroLFSDestruir);
		t->registro = list_create();
	}
}
