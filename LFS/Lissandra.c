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

int main()
{
	tablasLFS = list_create();
	logger = log_create(logFile, "LFS",true, LOG_LEVEL_INFO);
	configuracion = malloc(sizeof(ConfiguracionLFS));
	configurar(configuracion);


	//levantar FS


	//servidor
	//FUNCIONES SOCKETS (Usar dependiendo de la biblioteca que usemos)

	/*socketEscucha= levantarServidorIPautomatica(configuracion->PUERTO_ESCUCHA, BACKLOG); //BACKLOG es la cantidad de clientes que pueden conectarse a este servidor
	socketActivo = aceptarComunicaciones(socketEscucha);*/

	fd_set setSocketsOrquestador;
	FD_ZERO(&setSocketsOrquestador);

	// Inicializacion de sockets y actualizacion del log

	socketEscucha = crearSocketEscucha(configuracion->PUERTO_ESCUCHA, logger);

	free(configuracion);

	//crearHiloIndependiente(&hiloCompactador,(void*)compactacion, NULL, "LFS");
	//pthread_create(&hiloCompactador, NULL, (void*)compactacion, NULL);
	//pthread_join(hiloCompactador, NULL);

	crearHilo(&hiloFileSystem,(void*)fileSystem, NULL, "LFS"); //LEVANTO FILESYSTEM
	joinearHilo(hiloFileSystem, NULL, "LFS");
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

	desconectarseDe(socketActivo);
	desconectarseDe(socketEscucha);
	list_destroy_and_destroy_elements(tablasLFS, (void*) tablaDestruir);
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





void insertLFS(char* nombreTabla, uint16_t key, int value, int timestamp){
	//int numeroTabla = 0;
	//MetadataLFS* metadataTabla;
	char* strParticion = NULL;
	char* direccion = NULL;
	char* registro = NULL;
	char* buffer = NULL;
	Tabla* tablaEncontrada = tablaEncontrar(nombreTabla);
	if(tablaEncontrada!=NULL)
		perror("No se encontro la tabla solicitada.");
	else{
		//metadataTabla = tablasLFS[numeroTabla]->metadata;
		//INSERT EN UN ARCHIVO .BIN DIRECTO (NO SE CONTEMPLA DUMPS NI TEMPORALES)
		strParticion = itoa(key % tablaEncontrada->metadata->particiones, strParticion, 10);
		strcpy (direccion,"/Tables/");
		strcat (direccion,nombreTabla);
		strcat (direccion,"/");
		strcat (direccion,strParticion);
		strcat (direccion, ".bin");
		FILE* particion = fopen(direccion, "w");
		strcpy (registro,itoa(timestamp, buffer, 10));
		strcat (registro,";");
		strcat (registro,itoa(key, buffer, 10));
		strcat (registro,";");
		strcat (registro, itoa(value, buffer, 10));
		fwrite(registro , 1 , sizeof(registro) , particion);
		//Verificar si existe en memoria una lista de datos a dumpear. De no existir, alocar dicha memoria.
		//Insertar en la memoria temporal del punto anterior una nueva entrada que contenga los datos enviados en la request.
		/*
		De esta manera, cada insert se realizará siempre sobre la porción de memoria temporal asignada a dicha tabla sin importarle
		si dentro de la misma ya existe la key. Esto es así, ya que al momento de obtener la misma se retornará el que tenga un Timestamp más
		reciente mientras que el proceso de Compactación (explicado en el Anexo I), posterior al proceso de dump, será el que se encargue
		de unificar dichas Keys dentro del archivo original.
		*/
		/*
		 Todo dato dentro de un archivo será persistido con el formato:
		 [TIMESTAMP];[KEY];[VALUE]
		 */
	}
}

RegistroLFS* selectLFS(char* nombreTabla, uint16_t key){
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
	}
}
void createLFS(char* nombreTabla, int consistencia, int particiones, long tiempoCompactacion){
	if(tablaEncontrar(nombreTabla)!=NULL){
		//Loguear error
		perror("Ya existe una tabla con ese nombre.");
		return;
	}
	else{
		char path[] = RUTA_TABLAS;
		strcat(path, nombreTabla);
		mkdir(path, 0777);

		char *particionPath = malloc(100);
		char *nombreParticion = malloc(6);
		strcpy(particionPath, path);
		strcat(particionPath, "/Metadata");
		FILE *nuevoArchivo = fopen(particionPath, "w+");
		fprintf(nuevoArchivo, "CONSISTENCY=%d\n", consistencia);
		fprintf(nuevoArchivo, "PARTITIONS=%d\n", particiones);
		fprintf(nuevoArchivo, "COMPACTION_TIME=LISSANDRA\n");
		fclose(nuevoArchivo);
		strcat(particionPath, "/Metadata/Bitmap.bin");
		nuevoArchivo = fopen(particionPath, "w+");
		fclose(nuevoArchivo);

		for(int i = 0; i<particiones; i++){
			strcpy(particionPath, path);
			sprintf(nombreParticion, "/%d.bin", i);
			strcat(particionPath, nombreParticion);
			nuevoArchivo = fopen(particionPath, "w+");
			fprintf(nuevoArchivo, "SIZE=%d\n", 0);
			fprintf(nuevoArchivo, "BLOCKS=[]\n");
			fclose(nuevoArchivo);
		}

		free(particionPath);
		free(nombreParticion);
		list_add(tablasLFS, crearTabla(nombreTabla, consistencia, particiones, tiempoCompactacion));
	}
}

//Funciones de estructuras
Tabla* crearTabla(char* nombreTabla, int consistencia, int particiones, long tiempoCompactacion){
	Tabla *nuevaTabla = malloc(sizeof(Tabla));
	nuevaTabla->nombreTabla = nombreTabla;
	MetadataLFS *metadata = malloc(sizeof(MetadataLFS));
	metadata->consistencia = consistencia;
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
RegistroLFS* RegistroLFSCrear(uint16_t key, int timestamp, int value){
	RegistroLFS *registro = malloc(sizeof(RegistroLFS));
	registro->key = key;
	registro->timestamp = timestamp;
	registro->value = value;
	return registro;
}
void RegistroLFSDestruir(RegistroLFS* registro){
	free(registro);
}
Tabla* tablaEncontrar(char* nombre){
	int _is_the_one(Tabla *t) {
		return string_equals_ignore_case(t->nombreTabla, nombre);
	}
	return list_find(tablasLFS, (void*) _is_the_one);
}
