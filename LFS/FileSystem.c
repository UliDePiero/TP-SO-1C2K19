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
	sem_init(&compactacionSemaforo, 1, 1);
	char *puntoMontaje = string_from_format("%s", configuracion->PUNTO_MONTAJE);

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
	char *bloquesPath = string_from_format("%s/Bloques", puntoMontaje);
	mkdir(bloquesPath, 0777);
	free(bloquesPath);

	free(puntoMontaje);
}
void destruirFileSystem(){
	//free(metadata->magicNumber);
	free(bitarray);
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
	char *tablesPath = string_from_format("%s/Tables", puntoMontaje);
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
				char *tablaPath = string_from_format("%s/Tables/%s", puntoMontaje, dir->d_name);
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
/*char* encontrarRegistroParticion(char* pathParticion, uint16_t key){
	char* value = malloc(configuracion->TAMANIO_VALUE+1);
	int timestamp = 0;
	char* camposMetadatas[] = {
							   "SIZE",
							   "BLOCKS"
							 };
	t_config* particionData = archivoConfigCrear(pathParticion, camposMetadatas);
	int bloques = cantidadBloques(archivoConfigSacarArrayDe(particionData, "BLOCKS"));
	free(particionData);
	if(bloques == 0)
		return NULL;

	char* bloque = malloc(metadata->tamanioBloque+1);
	char* pathBloque;

	for(int i = 0; i<bloques; i++){
		//TODO: recorrer los bloques y encontrar el value
		pathBloque = string_from_format("%sBloques/%d.bin", configuracion->PUNTO_MONTAJE, i);
		FILE* archivo = fopen("file.txt", "r+");
		free(pathBloque);
		fread(bloque, metadata->tamanioBloque+1, 1, archivo);
		fclose(archivo);
	}

	free(bloque);
	return value;
}*/
int proximoPuntoYComa(char* array){
	for(int i = 0; i<sizeof(array); i++)
		if(array[0] == ';')
			return i;
	return -1;
}
char* fileToArray(char* path){
	char* buffer;
	FILE* infile = fopen(path, "r");
	if(infile != NULL){
		fseek(infile, 0L, SEEK_END);
		long numbytes = ftell(infile);
		fseek(infile, 0L, SEEK_SET);
		buffer = (char*)calloc(numbytes, sizeof(char));
		fread(buffer, sizeof(char), numbytes, infile);
		fclose(infile);
	}
	return buffer;
}
int calcularBloques(int sizeTotal){
	sem_wait(&metadataSemaforo);
	int bloques = sizeTotal/metadata->tamanioBloque;
	if(sizeTotal%metadata->tamanioBloque)
		bloques++;
	sem_post(&metadataSemaforo);
	return bloques;
}

//Funciones de bitmap
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
	char *puntoMontaje= string_from_format("%s", configuracion->PUNTO_MONTAJE);
	sem_post(&configSemaforo);
	char *bitmapFile = string_from_format("%s/Metadata/Bitmap.bin", puntoMontaje);
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


//Comandos
void createFS(char* nombreTabla, char* consistencia, int particiones, long tiempoCompactacion){
	sem_wait(&configSemaforo);
	char *pathTabla = string_from_format("%s/Tables/%s", configuracion->PUNTO_MONTAJE, nombreTabla);
	sem_post(&configSemaforo);
	mkdir(pathTabla, 0777);

	//Creo la metadata
	char *metadataPath = string_from_format("%s/Metadata", pathTabla);
	FILE *nuevoArchivo = fopen(metadataPath, "w+");
	fprintf(nuevoArchivo, "CONSISTENCY=%s\n", consistencia);
	fprintf(nuevoArchivo, "PARTITIONS=%d\n", particiones);
	fprintf(nuevoArchivo, "COMPACTION_TIME=%ld\n", tiempoCompactacion);
	fclose(nuevoArchivo);
	free(metadataPath);

	//Creo las particiones
	char *particionPath;
	int bloqueLibre;
	for(int i = 0; i<particiones; i++){
		bloqueLibre = proximoBloqueLibre();
		setBloqueOcupado(bloqueLibre);
		particionPath = string_from_format("%s/%d.bin", pathTabla, i);
		nuevoArchivo = fopen(particionPath, "w+");
		fprintf(nuevoArchivo, "SIZE=%d\n", 0);
		fprintf(nuevoArchivo, "BLOCKS=[%d]\n", bloqueLibre);

		fclose(nuevoArchivo);
		free(particionPath);
	}
	guardarBitmap();
	free(pathTabla);

	Tabla* tabla = crearTabla(nombreTabla, consistencia, particiones, tiempoCompactacion);
	sem_wait(&memtableSemaforo);
	list_add(tablasLFS, tabla);
	sem_post(&memtableSemaforo);
}
char* selectFS(char* tabla, int particion, uint16_t key){
	char* value = string_new();
	int timestampMayor = 0;
	char* pathTabla = string_from_format("%sTables/%s", configuracion->PUNTO_MONTAJE, tabla);
	char* pathParticion = string_from_format("%d.bin", particion);

	struct dirent *dir;
	DIR *tables = opendir(pathTabla);
	if (tables)
	{
		struct stat buffer;
		int status;
		while ((dir = readdir(tables)) != NULL)
		{
			if(strcmp(dir->d_name, ".") && strcmp(dir->d_name, "..") && (string_ends_with(dir->d_name, ".temp") || string_ends_with(dir->d_name, ".tempc") || !strcmp(dir->d_name, pathParticion))){

				char* pathArchivo = string_from_format("%s/%s", pathTabla, dir->d_name);
				status = stat(pathArchivo, &buffer);
				if(status == 0 && buffer.st_size != 0) {
					char* camposMetadatas[] = {
											   "SIZE",
											   "BLOCKS"
											 };
					t_config* archivoParticion = archivoConfigCrear(pathArchivo, camposMetadatas);
					unsigned int size = archivoConfigSacarIntDe(archivoParticion, "SIZE");
					if(size!=0){
						char* bloquesJuntos = string_new();
						char** bloques = archivoConfigSacarArrayDe(archivoParticion, "BLOCKS");
						char* pathBloques = string_from_format("%sBloques", configuracion->PUNTO_MONTAJE);
						for(int i = 0; bloques[i]!=NULL;i++){
							char* pathBloque = string_from_format("%s/%d.bin", pathBloques, atoi(bloques[i]));
							char* buffer = fileToArray(pathBloque);
							bloquesJuntos = string_from_format("%s%s", bloquesJuntos, buffer);
							free(buffer);
							free(pathBloque);
						}
						free(pathBloques);

						RegistroLFS* registroValue = registroEncontrarArray(key, bloquesJuntos);
						free(bloquesJuntos);

						if(registroValue && (registroValue->timestamp > timestampMayor || string_is_empty(value))){
							timestampMayor = registroValue->timestamp;
							free(value);
							value = string_from_format("%s", registroValue->value);
							RegistroLFSDestruir(registroValue);
						}
					}
				}
			}
		}
		closedir(tables);
	}

	free(pathTabla);
	free(pathParticion);

	if(string_is_empty(value)){
		free(value);
		return NULL;
	} else {
		return value;
	}

/*


	char* value = encontrarRegistroParticion(pathParticion, key);
	free(pathParticion);

	if(value){
		//break;
	}

	//TODO: recorrer la particion y encontrar el value

	return NULL;
	free(pathTabla);*/
}
void dropFS(char* nombreTabla){
	sem_wait(&configSemaforo);
	char *pathTabla = string_from_format("%s/Tables/%s", configuracion->PUNTO_MONTAJE, nombreTabla);
	char *pathBloques = string_from_format("%s/Bloques", configuracion->PUNTO_MONTAJE);
	sem_post(&configSemaforo);
	char *metadataPath = string_from_format("%s/Metadata", pathTabla);
	remove(metadataPath);
	free(metadataPath);

	char* camposParticion[] = {
					   "SIZE",
					   "BLOCKS"
					 };
	sem_wait(&compactacionSemaforo);
	struct dirent *dir;
	DIR *particiones = opendir(pathTabla);
	if (particiones)
	{
		while ((dir = readdir(particiones)) != NULL){
			if(strcmp(dir->d_name, ".") && strcmp(dir->d_name, "..")){
				char *particionPath = string_from_format("%s/%s", pathTabla, dir->d_name);
				t_config* archivoConfig = archivoConfigCrear(particionPath, camposParticion);
				if(archivoConfigSacarLongDe(archivoConfig, "SIZE") > 0){
					char** blocks = archivoConfigSacarArrayDe(archivoConfig, "BLOCKS");
					for(int i=0;blocks[i]!=NULL;i++){
						int bloque = atoi(blocks[i]);
						char *pathBloque = string_from_format("%s/%d.bin", pathBloques, bloque);

						remove(pathBloque);
						setBloqueLibre(bloque);
						free(pathBloque);
						free(blocks[i]);
					}
					free(blocks);
				}
				archivoConfigDestruir(archivoConfig);
				remove(particionPath);
				free(particionPath);
			}
		}
		closedir(particiones);
	}
	sem_post(&compactacionSemaforo);
	rmdir(pathTabla);
	free(pathTabla);
	free(pathBloques);
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
	int sizeTotal = string_length(registrosComprimidos);
	int bloques = calcularBloques(sizeTotal);
	int blocks[bloques];

	//Creo los bloques
	for(int i = 0; i<bloques; i++){
		int proxBloque = proximoBloqueLibre();
		if(proxBloque == -1){
			perror("No hay bloques libres");
			break;
		}
		blocks[i] = proxBloque;
		sem_wait(&configSemaforo);
		char* pathNuevo = string_from_format("%sBloques/%d.bin", configuracion->PUNTO_MONTAJE, proxBloque);
		sem_post(&configSemaforo);
		bitarray_set_bit(bitmap, proxBloque);
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
	sem_wait(&configSemaforo);
	char* pathTable = string_from_format("%sTables/%s", configuracion->PUNTO_MONTAJE, nombre);
	sem_post(&configSemaforo);
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
	fprintf(nuevoTemporal, "SIZE=%d\n", sizeTotal);
	fprintf(nuevoTemporal, "BLOCKS=[%d", blocks[0]);
	for(int b = 1; b<bloques; b++)
		fprintf(nuevoTemporal, ",%d", blocks[b]);
	fprintf(nuevoTemporal, "]\n");
	fclose(nuevoTemporal);

	free(pathTable);
}

void compactar(char* nombreTabla){
	//TODO:
	sem_wait(&compactacionSemaforo);
	sem_post(&compactacionSemaforo);
}
