/*
 * FileSystem.c
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */

#include "FileSystem.h"

void levantarFileSystem(){
	sem_init(&bitmapSemaforoFILE, 1, 1);
	sem_init(&bitmapSemaforo, 1, 1);
	sem_init(&configSemaforo, 1, 1);
	sem_init(&metadataSemaforo, 1, 1);
	char *puntoMontaje = string_duplicate(configuracion->PUNTO_MONTAJE);
	//Creo el punto de montaje en caso que no exista
	mkdir(puntoMontaje, 0777);

	//Carpeta Metadata
	char *pathMetadata = string_from_format("%sMetadata", puntoMontaje);
	mkdir(pathMetadata, 0777);
	obtenerMetadata(pathMetadata);
	obtenerBitmap(pathMetadata);
	free(pathMetadata);

	//Carpeta Tables
	obtenerTablas(puntoMontaje);

	//Carpeta Bloques
	char *bloquesPath = string_from_format("%sBloques", puntoMontaje);
	mkdir(bloquesPath, 0777);
	free(bloquesPath);

	free(puntoMontaje);
}
void destruirFileSystem(){
	//free(metadata->magicNumber);
	sem_wait(&bitmapSemaforo);
	free(bitarray);
	sem_wait(&metadataSemaforo);
	free(metadata);
	bitarray_destroy(bitmap);
}

void obtenerMetadata(char* pathMetadata){
	char *metadataFile = string_from_format("%s/Metadata.bin", pathMetadata);
	if( access( metadataFile, F_OK ) != -1 ) {
		//Metadata ya existe
		char* camposMetadatas[] = {
						   "BLOCK_SIZE",
						   "BLOCKS",
						   "MAGIC_NUMBER"
						 };
		t_config* archivoConfig = archivoConfigCrear(metadataFile, camposMetadatas);
		metadata = malloc(sizeof(Metadata));
		metadata->tamanioBloque = archivoConfigSacarIntDe(archivoConfig, "BLOCK_SIZE");
		metadata->magicNumber = archivoConfigSacarStringDe(archivoConfig, "MAGIC_NUMBER");
		metadata->cantidadBloques = archivoConfigSacarIntDe(archivoConfig, "BLOCKS");
		archivoConfigDestruir(archivoConfig);
	} else {
		//Metadata no existe
		FILE* Archivo = fopen(metadataFile, "w+");
		fprintf(Archivo, "BLOCK_SIZE=%d\n", 64);
		fprintf(Archivo, "BLOCKS=%d\n", 5192);
		fprintf(Archivo, "MAGIC_NUMBER=LISSANDRA\n");
		fclose(Archivo);

		metadata = malloc(sizeof(Metadata));
		metadata->tamanioBloque = 64;
		metadata->magicNumber = "MAGIC_NUMBER=LISSANDRA";
		metadata->cantidadBloques = 5192;
	}
	free(metadataFile);
}
void obtenerBitmap(char* pathMetadata){
	char *bitmapFile = string_from_format("%s/Bitmap.bin", pathMetadata);
	if( access( bitmapFile, F_OK ) == -1 ){
		crearBitmap(bitmapFile);
	}
	FILE *bitmapArchivo = fopen(bitmapFile, "r+b");
	fseek(bitmapArchivo, 0 , SEEK_END);
	long fileSize = ftell(bitmapArchivo);
  	fseek(bitmapArchivo, 0 , SEEK_SET);
	bitarray = malloc(fileSize);
	int Bytes = metadata->cantidadBloques/8;
	if(metadata->cantidadBloques%8 != 0)
		Bytes++;


	fread(bitarray, 1, Bytes, bitmapArchivo);
	fclose(bitmapArchivo);

	bitmap = bitarray_create_with_mode(bitarray, metadata->cantidadBloques, MSB_FIRST);

	free(bitmapFile);
}
void obtenerTablas(char* puntoMontaje){
	char *tablesPath = string_from_format("%sTables", puntoMontaje);
	mkdir(tablesPath, 0777);

	//Me fijo en la carpeta Tables por si ya hay tables en el filesystem y las agrego a la memtable
	char* camposMetadatas[] = {
					   "CONSISTENCY",
					   "PARTITIONS",
					   "COMPACTION_TIME"
					 };
	struct dirent *dir;
	DIR *tables = opendir(tablesPath);
	if (tables)
	{
		while ((dir = readdir(tables)) != NULL)
		{
			if(strcmp(dir->d_name, ".") && strcmp(dir->d_name, "..")){
				char *tablaPath = string_from_format("%sTables/%s", puntoMontaje, dir->d_name);
				char *tablaMetadataPath = string_from_format("%s/Metadata", tablaPath);
				t_config* archivoConfig = archivoConfigCrear(tablaMetadataPath, camposMetadatas);
				int particiones = archivoConfigSacarIntDe(archivoConfig, "PARTITIONS");
				int tiempoComp = archivoConfigSacarIntDe(archivoConfig, "COMPACTION_TIME");
				list_add(tablasLFS, crearTabla(dir->d_name, archivoConfigSacarStringDe(archivoConfig, "CONSISTENCY"), particiones, tiempoComp));
				free(tablaPath);
				free(tablaMetadataPath);
				archivoConfigDestruir(archivoConfig);
			}
		}
		closedir(tables);
	}
	free(tablesPath);
}

int bytesArchivoPath(char* path){
    int fd;
    struct stat fileInfo = {0};
    fd=open(path, O_RDONLY);
    if (fd == -1)
    {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }
    if (fstat(fd, &fileInfo) == -1)
    {
        perror("Error getting the file size");
        return -1;
    }
	return fileInfo.st_size;
	close(fd);
}
int cantidadBloques(char** bloquesArray){
	int cantidad = 0;
	sem_wait(&metadataSemaforo);
	for(int i = 0; i<metadata->cantidadBloques; i++){
		if(bloquesArray[i])
			cantidad++;
		else
			break;
	}
	sem_post(&metadataSemaforo);
	return cantidad;
}
int proximoPuntoYComa(char* array){
	for(int i = 0; i<sizeof(array); i++)
		if(array[0] == ';')
			return i;
	return -1;
}
char* fileToArray(char* path){
	//Returns NULL if file doesn't exist
	char* buffer = NULL;
	FILE* infile = fopen(path, "r");
	if(infile != NULL){
		fseek(infile, 0L, SEEK_END);
		long numbytes = ftell(infile);
		fseek(infile, 0L, SEEK_SET);
		buffer = (char*)calloc(numbytes, sizeof(char)+1);
		fread(buffer, sizeof(char), numbytes, infile);
		fclose(infile);
	}
	return buffer;
}
int calcularBloques(long sizeTotal){
	sem_wait(&metadataSemaforo);
	int bloques = sizeTotal/metadata->tamanioBloque;
	if(sizeTotal%metadata->tamanioBloque)
		bloques++;
	sem_post(&metadataSemaforo);
	return bloques;
}

//----------------- FUNCIONES DE BITMAP -----------------//

void crearBitmap(char* pathBitmap){
	char data[1] = { 0b00000000 };
	int Bytes = metadata->cantidadBloques/8;
	if(metadata->cantidadBloques%8 != 0)
		Bytes++;
	t_bitarray *miniBitmap = bitarray_create_with_mode(data, sizeof(data), MSB_FIRST);
	FILE *bitmapArchivo = fopen(pathBitmap, "w+b");
	for(int i = 0; i<Bytes; i++)
		fwrite(miniBitmap->bitarray, miniBitmap->size, 1, bitmapArchivo);
	bitarray_destroy(miniBitmap);
	fclose(bitmapArchivo);

	//printf("size archivo: %d bytes %d bits\n", bytesArchivo(pathBitmap), bytesArchivo(pathBitmap)*8);
}
int bloquesLibres(){
	int cantidad=0;
	sem_wait(&bitmapSemaforo);
	for(int i=0; i < (bitmap->size); i++){
		if(bitarray_test_bit(bitmap,i)==0)
			cantidad++;
	}
	sem_post(&bitmapSemaforo);
	return cantidad;
}
void limpiarBitmap(){
	sem_wait(&bitmapSemaforo);
	sem_wait(&metadataSemaforo);
	for(int i = 0; i<metadata->cantidadBloques; i++)
		bitarray_clean_bit(bitmap, i);
	sem_post(&metadataSemaforo);
	sem_post(&bitmapSemaforo);
}
void setBloqueOcupado(int index){
	sem_wait(&bitmapSemaforo);
	bitarray_set_bit(bitmap, index);
	sem_post(&bitmapSemaforo);
}
void setBloqueLibre(int index){
	sem_wait(&bitmapSemaforo);
	bitarray_clean_bit(bitmap, index);
	sem_post(&bitmapSemaforo);
}
void guardarBitmap(){
	sem_wait(&configSemaforo);
	char *puntoMontaje= string_duplicate(configuracion->PUNTO_MONTAJE);
	sem_post(&configSemaforo);
	char *bitmapFile = string_from_format("%sMetadata/Bitmap.bin", puntoMontaje);
	sem_wait(&metadataSemaforo);
	int Bytes = metadata->cantidadBloques/8;
	if(metadata->cantidadBloques%8 != 0)
		Bytes++;
	sem_post(&metadataSemaforo);

	sem_wait(&bitmapSemaforoFILE);
	FILE *bitmapArchivo = fopen(bitmapFile, "w+b");
	fwrite(bitmap->bitarray, Bytes, 1, bitmapArchivo);

	fclose(bitmapArchivo);
	sem_post(&bitmapSemaforoFILE);
	free(puntoMontaje);
	free(bitmapFile);
}
int proximoBloqueLibre(){
	sem_wait(&bitmapSemaforo);
	for(int i=0;i<(bitmap->size);i++){
		if(bitarray_test_bit(bitmap,i)==0){
			bitarray_set_bit(bitmap, i);
			sem_post(&bitmapSemaforo);
			return i;
		}
	}
	sem_post(&bitmapSemaforo);
	return -1;
}
void mostrarBitmap(){
	sem_wait(&bitmapSemaforo);
	for(int i = 0; i < (bitmap->size); i++){
		if(i%64==0 && i!=0){
			printf("-----\n");
		}
		printf("%d: %d\n", i, bitarray_test_bit(bitmap, i));
	}
	printf("\n");
	sem_post(&bitmapSemaforo);
}


void createFS(char* nombreTabla, char* consistencia, int particiones, long tiempoCompactacion){
	sem_wait(&configSemaforo);
	char *puntoMontaje = string_duplicate(configuracion->PUNTO_MONTAJE);
	sem_post(&configSemaforo);
	char *pathTabla = string_from_format("%sTables/%s", puntoMontaje, nombreTabla);
	free(puntoMontaje);
	mkdir(pathTabla, 0777);

	//Creo la metadata
	char *metadataPath = string_from_format("%s/Metadata", pathTabla);
	FILE *nuevoArchivo = fopen(metadataPath, "w+");
	fprintf(nuevoArchivo, "CONSISTENCY=%s\n", consistencia);
	fprintf(nuevoArchivo, "PARTITIONS=%d\n", particiones);
	fprintf(nuevoArchivo, "COMPACTION_TIME=%ld\n", tiempoCompactacion);
	fclose(nuevoArchivo);

	//Creo las particiones
	char *particionPath;
	int bloqueLibre;
	int bloques[particiones];
	for(int i = 0; i<particiones; i++){
		bloqueLibre = proximoBloqueLibre();
		if(bloqueLibre == -1){
			for(int j = 0; j<i; j++){
				char* pathPart = string_from_format("%s/%d.bin", pathTabla, j);
				remove(pathPart);
				free(pathPart);
				setBloqueLibre(bloques[j]);
			}
			sem_wait(&loggerSemaforo);
			log_error(logger, "No hay bloques disponibles. No se puedo crear la tabla \"%s\"", nombreTabla);
			sem_post(&loggerSemaforo);
			guardarBitmap();
			free(metadataPath);
			free(pathTabla);
			return;
		}
		bloques[i] = i;
		particionPath = string_from_format("%s/%d.bin", pathTabla, i);
		nuevoArchivo = fopen(particionPath, "w+");
		fprintf(nuevoArchivo, "SIZE=%d\n", 0);
		fprintf(nuevoArchivo, "BLOCKS=[%d]\n", bloqueLibre);

		fclose(nuevoArchivo);
		free(particionPath);
	}
	guardarBitmap();
	free(pathTabla);
	free(metadataPath);

	Tabla* tabla = crearTabla(nombreTabla, consistencia, particiones, tiempoCompactacion);
	sem_wait(&memtableSemaforo);
	list_add(tablasLFS, tabla);
	sem_post(&memtableSemaforo);
	sem_wait(&loggerSemaforo);
	log_info(logger, "Create tabla \"%s\" consistencia %s particiones %d tiempo compactacion %ld", nombreTabla, consistencia, particiones, tiempoCompactacion);
	sem_post(&loggerSemaforo);
}
void selectFS(char* tabla, int particion, uint16_t key){
	sem_wait(&configSemaforo);
	char *puntoMontaje = string_duplicate(configuracion->PUNTO_MONTAJE);
	sem_post(&configSemaforo);
	char *pathBloques = string_from_format("%sBloques", puntoMontaje);
	Tabla* t = tablaEncontrar(tabla);
	if(!t){
		sem_wait(&loggerSemaforo);
		log_error(logger, "Tabla \"%s\" no encontrada", tabla);
		sem_post(&loggerSemaforo);
		return;
	}
	char* value = string_new();
	uint64_t timestampMayor = 0;
	char* pathTabla = string_from_format("%sTables/%s", puntoMontaje, tabla);
	char* pathParticion = string_from_format("%d.bin", particion);
	free(puntoMontaje);

	char* camposMetadatas[] = {
							   "SIZE",
							   "BLOCKS"
							 };
	struct dirent *dir;
	sem_wait(&t->semaforo);
	DIR *tables = opendir(pathTabla);
	if (tables){
		struct stat buffer;
		int status;
		while ((dir = readdir(tables)) != NULL){
			if(strcmp(dir->d_name, ".") && strcmp(dir->d_name, "..") && (string_ends_with(dir->d_name, ".temp") || string_ends_with(dir->d_name, ".tempc") || !strcmp(dir->d_name, pathParticion))){
				char* pathArchivo = string_from_format("%s/%s", pathTabla, dir->d_name);
				status = stat(pathArchivo, &buffer);
				if(status == 0 && buffer.st_size != 0) {
					t_config* archivoParticion = archivoConfigCrear(pathArchivo, camposMetadatas);
					unsigned int size = archivoConfigSacarIntDe(archivoParticion, "SIZE");
					if(size!=0){
						char* bloquesJuntos = string_new();
						char** bloques = archivoConfigSacarArrayDe(archivoParticion, "BLOCKS");
						for(int i = 0; bloques[i]!=NULL;i++){
							char* pathBloque = string_from_format("%s/%d.bin", pathBloques, atoi(bloques[i]));
							char* buffer = fileToArray(pathBloque);
							if(buffer){
								string_append(&bloquesJuntos, buffer);
								free(buffer);
							}
							free(pathBloque);
							free(bloques[i]);
						}
						free(bloques);
						RegistroLFS* registroValue = registroEncontrarArray(key, bloquesJuntos);
						free(bloquesJuntos);

						if(registroValue){
							if(registroValue->timestamp > timestampMayor || string_is_empty(value)){
								timestampMayor = registroValue->timestamp;
								free(value);
								value = string_from_format("%s", registroValue->value);
							}
							RegistroLFSDestruir(registroValue);
						}
					}
					archivoConfigDestruir(archivoParticion);
				}
				free(pathArchivo);
			}
		}
		sem_post(&t->semaforo);
		closedir(tables);
	}

	free(pathBloques);
	free(pathTabla);
	free(pathParticion);

	if(string_is_empty(value)){
		free(value);
		sem_wait(&loggerSemaforo);
		log_error(logger, "Key \"%hi\" no encontrado para la tabla \"%s\"", key, tabla);
		sem_post(&loggerSemaforo);
		return;
	}else{
		sem_wait(&loggerSemaforo);
		log_info(logger, "Select tabla \"%s\" key %hd value \"%s\"", tabla, key, value);
		sem_post(&loggerSemaforo);
		free(value);
		return;
	}
}
void dropFS(char* nombreTabla){
	sem_wait(&configSemaforo);
	char *puntoMontaje = string_duplicate(configuracion->PUNTO_MONTAJE);
	sem_post(&configSemaforo);
	char *pathTabla = string_from_format("%sTables/%s", puntoMontaje, nombreTabla);
	char *pathBloques = string_from_format("%sBloques", puntoMontaje);
	char *metadataPath = string_from_format("%s/Metadata", pathTabla);
	remove(metadataPath);
	free(metadataPath);
	free(puntoMontaje);

	char* camposParticion[] = {
					   "SIZE",
					   "BLOCKS"
					 };
	struct dirent *dir;
	DIR *particiones = opendir(pathTabla);
	if (particiones){
		while ((dir = readdir(particiones)) != NULL){
			if(strcmp(dir->d_name, ".") && strcmp(dir->d_name, "..")){
				char *particionPath = string_from_format("%s/%s", pathTabla, dir->d_name);
				t_config* archivoConfig = archivoConfigCrear(particionPath, camposParticion);
				if(archivoConfigSacarLongDe(archivoConfig, "SIZE")){
					char** blocks = archivoConfigSacarArrayDe(archivoConfig, "BLOCKS");
					for(int i=0;blocks[i]!=NULL;i++){
						int bloque = atoi(blocks[i]);
						char *pathBloque = string_from_format("%s/%d.bin", pathBloques, bloque);

						if(remove(pathBloque)){
							sem_wait(&loggerSemaforo);
							log_error(logger, "Error al borrar el bloque \"%s\"", pathBloque);
							sem_post(&loggerSemaforo);
						}
						setBloqueLibre(bloque);
						free(pathBloque);
						free(blocks[i]);
					}
					free(blocks);
				}
				archivoConfigDestruir(archivoConfig);
				if(remove(particionPath)){
					sem_wait(&loggerSemaforo);
					log_error(logger, "Error al borrar la particion \"%s\"", particionPath);
					sem_post(&loggerSemaforo);
				}
				free(particionPath);
			}
		}
		closedir(particiones);
	}
	rmdir(pathTabla);
	free(pathTabla);
	free(pathBloques);
	guardarBitmap();
}

void dump(){
	t_list *listaAuxiliar = list_create();
	sem_wait(&memtableSemaforo);
	for(int t = 0; t<tablasLFS->elements_count; t++){
		Tabla* tabla = list_get(tablasLFS, t);
		list_add(listaAuxiliar, tabla);
	}
	sem_post(&memtableSemaforo);
	for(int t = 0; t<listaAuxiliar->elements_count; t++){
		Tabla* tabla = list_get(listaAuxiliar, t);
		sem_wait(&tabla->semaforo);
		if(tabla->registro->elements_count > 0){
			char* registrosComprimidos = string_new();
			for(int i = 0; i<tabla->registro->elements_count; i++){
				RegistroLFS* reg = list_get(tabla->registro, i);
				char* comprimido = comprimirRegistro(reg);
				string_append(&registrosComprimidos, comprimido);
				free(comprimido);
			}
			char* nombre = string_from_format("%s", tabla->nombreTabla);
			sem_post(&tabla->semaforo);
			if(string_length(registrosComprimidos) > 0)
				crearNuevosBloques(registrosComprimidos, nombre);
			free(nombre);
			free(registrosComprimidos);
		}else{
			sem_post(&tabla->semaforo);
		}
	}
	list_destroy(listaAuxiliar);
	limpiarMemtable();
}
void crearNuevosBloques(char* registrosComprimidos, char* nombre){
	sem_wait(&configSemaforo);
	char* puntoMontaje = string_duplicate(configuracion->PUNTO_MONTAJE);
	sem_post(&configSemaforo);
	long sizeTotal = string_length(registrosComprimidos);
	int bloques = calcularBloques(sizeTotal);
	int blocks[bloques];

	//Creo los bloques
	for(int i = 0; i<bloques; i++){
		int proxBloque = proximoBloqueLibre();
		if(proxBloque == -1){
			for(int j = 0; j<i; j++){
				char* pathBloque = string_from_format("%sBloques/%d.bin", puntoMontaje, blocks[j]);
				remove(pathBloque);
				free(pathBloque);
			}
			guardarBitmap();
			free(puntoMontaje);
			sem_wait(&loggerSemaforo);
			log_error(logger, "No hay bloques libres. No se pudo dumpear la tabla \"%s\"", nombre);
			sem_post(&loggerSemaforo);
			return;
		}
		blocks[i] = proxBloque;
		char* pathNuevo = string_from_format("%sBloques/%d.bin", puntoMontaje, proxBloque);
		FILE* nuevoBloque = fopen(pathNuevo, "w+");
		free(pathNuevo);

		sem_wait(&metadataSemaforo);
		unsigned long pos = i*metadata->tamanioBloque;
		char* substring = string_substring(registrosComprimidos, pos, metadata->tamanioBloque);
		sem_post(&metadataSemaforo);
		fprintf(nuevoBloque, "%s", substring);
		free(substring);

		fclose(nuevoBloque);
	}
	guardarBitmap();

	//Creo el .temp
	char* pathTable = string_from_format("%sTables/%s", puntoMontaje, nombre);
	struct dirent *dir;
	DIR *tables = opendir(pathTable);
	int tempNum = 0;
	if (tables){
		while ((dir = readdir(tables)) != NULL){
			if(strcmp(dir->d_name, ".") && strcmp(dir->d_name, "..") && string_ends_with(dir->d_name, ".temp")){
				tempNum++;
			}
		}
	}
	closedir(tables);
	char* pathNuevo = string_from_format("%s/%d.temp", pathTable, tempNum);
	FILE* nuevoTemporal = fopen(pathNuevo, "w+");
	free(pathNuevo);
	fprintf(nuevoTemporal, "SIZE=%ld\n", sizeTotal);
	fprintf(nuevoTemporal, "BLOCKS=[%d", blocks[0]);
	for(int b = 1; b<bloques; b++)
		fprintf(nuevoTemporal, ",%d", blocks[b]);
	fprintf(nuevoTemporal, "]\n");
	fclose(nuevoTemporal);
	free(puntoMontaje);
	free(pathTable);
}

void compactar(char* nombreTabla){
	Tabla* tabla = tablaEncontrar(nombreTabla);
	sem_wait(&configSemaforo);
	char *puntoMontaje = string_duplicate(configuracion->PUNTO_MONTAJE);
	sem_post(&configSemaforo);
	char *pathTabla = string_from_format("%sTables/%s", puntoMontaje, nombreTabla);
	char *pathBloques = string_from_format("%sBloques", puntoMontaje);
	free(puntoMontaje);
	t_list* direcciones = list_create();

	//RENOMBRAR .temp A .tempc
	struct dirent *dirTemp;
	DIR *archivos = opendir(pathTabla);
	if (archivos){
		while ((dirTemp = readdir(archivos)) != NULL){
			if(strcmp(dirTemp->d_name, ".") && strcmp(dirTemp->d_name, "..") && strcmp(dirTemp->d_name, "Metadata")){
				if(string_ends_with(dirTemp->d_name, ".temp")){
					char *pathParticion = string_from_format("%s/%s", pathTabla, dirTemp->d_name);
					char *nuevoNombre = string_from_format("%s/%sc", pathTabla, dirTemp->d_name);
					if(rename(pathParticion, nuevoNombre)){
						sem_wait(&loggerSemaforo);
						log_error(logger, "Error al renombrar la particion %s de la tabla \"%s\"", dirTemp->d_name, nombreTabla);
						sem_post(&loggerSemaforo);
					}
					free(pathParticion);
					list_add(direcciones, nuevoNombre);
				}else if(string_ends_with(dirTemp->d_name, ".bin")){
					list_add(direcciones, string_from_format("%s/%s", pathTabla, dirTemp->d_name));
				}
			}
		}
		closedir(archivos);
		sem_post(&tabla->semaforo);
	}

	//OBTENER REGISTROS
	char* camposParticion[] = {
						   "SIZE",
						   "BLOCKS"
						 };
	t_list* registros = list_create();
	for(int i = 0; i<direcciones->elements_count; i++){
		struct stat buffer;
		int status;
		char* pathArchivo = list_get(direcciones, i);
		status = stat(pathArchivo, &buffer);
		if(status == 0 && buffer.st_size != 0){
			t_config* archivoParticion = archivoConfigCrear(pathArchivo, camposParticion);
			unsigned int size = archivoConfigSacarIntDe(archivoParticion, "SIZE");
			if(size!=0){
				char* bloquesJuntos = string_new();
				char** bloques = archivoConfigSacarArrayDe(archivoParticion, "BLOCKS");
				for(int i = 0; bloques[i]!=NULL;i++){
					char* pathBloque = string_from_format("%s/%d.bin", pathBloques, atoi(bloques[i]));
					char* buffer = fileToArray(pathBloque);
					if(buffer){
						string_append(&bloquesJuntos, buffer);
						free(buffer);
					}
					free(pathBloque);
					free(bloques[i]);
				}
				free(bloques);
				agregarRegistros(registros, bloquesJuntos);
				free(bloquesJuntos);
			}
			archivoConfigDestruir(archivoParticion);
		}
	}

	if(list_is_empty(registros)){
		list_destroy(registros);
		list_destroy_and_destroy_elements(direcciones, (void*) free);
		free(pathBloques);
		free(pathTabla);
		return;
	}

	//BORRAR .bin Y .tempc
	clock_t t = clock();
	sem_wait(&tabla->semaforo);
	for(int i = 0; i<direcciones->elements_count; i++){
		char *particionPath = list_get(direcciones, i);
		t_config* archivoConfig = archivoConfigCrear(particionPath, camposParticion);
		if(archivoConfigSacarLongDe(archivoConfig, "SIZE") > 0){
			char** blocks = archivoConfigSacarArrayDe(archivoConfig, "BLOCKS");
			for(int i=0;blocks[i]!=NULL;i++){
				int bloque = atoi(blocks[i]);
				char *pathBloque = string_from_format("%s/%d.bin", pathBloques, bloque);

				if(remove(pathBloque)){
					sem_wait(&loggerSemaforo);
					log_error(logger, "Error al borrar el bloque \"%s\"", pathBloque);
					sem_post(&loggerSemaforo);
				}
				setBloqueLibre(bloque);
				free(pathBloque);
				free(blocks[i]);
			}
			free(blocks);
		}
		archivoConfigDestruir(archivoConfig);
		if(remove(particionPath)){
			sem_wait(&loggerSemaforo);
			log_error(logger, "Error al borrar la particion \"%s\"", particionPath);
			sem_post(&loggerSemaforo);
		}
		guardarBitmap();
	}
	//GUARDAR LOS REGISTROS EN NUEVOS .BIN
	for(int i = 0; i<tabla->metadata->particiones; i++){
		char* registrosParticion = encontrarYComprimirRegistros(registros, tabla->metadata->particiones, i);
		char* particionPath = string_from_format("%s/%d.bin", pathTabla, i);
		if(registrosParticion){
			crearNuevosBloquesCompactacion(registrosParticion, particionPath);
			free(registrosParticion);
		}else{
			FILE* nuevoArchivo = fopen(particionPath, "w+");
			int bloqueLibre = proximoBloqueLibre();
			if(bloqueLibre == -1){
				sem_wait(&loggerSemaforo);
				log_error(logger, "No hay bloques libres al compactar la tabla \"%s\"", nombreTabla);
				sem_post(&loggerSemaforo);
				break;
			}
			fprintf(nuevoArchivo, "SIZE=%d\n", 0);
			fprintf(nuevoArchivo, "BLOCKS=[%d]\n", bloqueLibre);
			fclose(nuevoArchivo);
		}
		free(particionPath);
		guardarBitmap();
	}
	sem_post(&tabla->semaforo);
	t = clock() - t;
	sem_wait(&loggerSemaforo);
	log_info(logger, "Tabla \"%s\" compactada. Tiempo bloqueada: %f", nombreTabla, ((double)t)/CLOCKS_PER_SEC);
	sem_post(&loggerSemaforo);
	free(pathBloques);
	free(pathTabla);
	list_destroy_and_destroy_elements(registros, (void*) RegistroLFSDestruir);
	list_destroy_and_destroy_elements(direcciones, (void*) free);
}
void crearNuevosBloquesCompactacion(char* registrosComprimidos, char* particionPath){
	long sizeTotal = string_length(registrosComprimidos);
	int bloques = calcularBloques(sizeTotal);
	int blocks[bloques];

	//Creo los bloques
	for(int i = 0; i<bloques; i++){
		int proxBloque = proximoBloqueLibre();
		if(proxBloque == -1){
			sem_wait(&loggerSemaforo);
			log_error(logger, "No hay bloques libres");
			sem_post(&loggerSemaforo);
			break;
		}
		blocks[i] = proxBloque;
		sem_wait(&configSemaforo);
		char *puntoMontaje = string_duplicate(configuracion->PUNTO_MONTAJE);
		sem_post(&configSemaforo);
		char* pathNuevo = string_from_format("%sBloques/%d.bin", puntoMontaje, proxBloque);
		FILE* nuevoBloque = fopen(pathNuevo, "w+");
		free(pathNuevo);
		free(puntoMontaje);

		sem_wait(&metadataSemaforo);
		unsigned long pos = i*metadata->tamanioBloque;
		char* substring = string_substring(registrosComprimidos, pos, metadata->tamanioBloque);
		sem_post(&metadataSemaforo);
		fprintf(nuevoBloque, "%s", substring);
		free(substring);

		fclose(nuevoBloque);
	}

	//Creo el .bin
	FILE* nuevoArchivo = fopen(particionPath, "w+");
	fprintf(nuevoArchivo, "SIZE=%ld\n", sizeTotal);
	fprintf(nuevoArchivo, "BLOCKS=[%d", blocks[0]);
	for(int b = 1; b<bloques; b++)
		fprintf(nuevoArchivo, ",%d", blocks[b]);
	fprintf(nuevoArchivo, "]\n");
	fclose(nuevoArchivo);
}

