/*
 * FileSystem.c
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */

#include "FileSystem.h"

void levantarFileSystem(){
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
	//list_destroy_and_destroy_elements(tablasLFS, (void*) tablaDestruir);
}

//MAINS DE TESTS
int main3(){
	configuracion = malloc(sizeof(ConfiguracionLFS));
	configurar(configuracion);

	//levantarFileSystem();
	/*createFS("A", 1, 5, 123);
	createFS("A", 1, 5, 123);
	createFS("B", 1, 2, 123);

	Tabla *a = list_get(tablasLFS, 0);
	printf("tabla: %s particiones=%d\n", a->nombreTabla, a->metadata->particiones);
	a = list_get(tablasLFS, 1);
	printf("tabla: %s particiones=%d\n", a->nombreTabla, a->metadata->particiones);*/

	/*printf("tamanioBloque: %d cantidadBloques: %d magicNumber: %s\n", metadata->tamanioBloque, metadata->cantidadBloques, metadata->magicNumber);
	puts("BITMAP:");
	printf("0: %d   1: %d\n", bitarray_test_bit(bitmap, 0), bitarray_test_bit(bitmap, 1));
	setBloqueOcupado(0);
	printf("0: %d   1: %d\n", bitarray_test_bit(bitmap, 0), bitarray_test_bit(bitmap, 1));*/

	//crearBitmap(metadata->cantidadBloques % metadata->tamanioBloque);
	//void mostrarBitmap();

	//destruirFileSystem();
	puts("destruyo");
	return 0;
}
int main7(){
	configuracion = malloc(sizeof(ConfiguracionLFS));
	configurar(configuracion);

	char *pathMetadata = "/home/utnso/workspace/tp-2019-1c-GGWP/LFS/Metadata";
	obtenerMetadata(pathMetadata);

	obtenerBitmap(pathMetadata);

	mostrarBitmap();

	free(metadata);
	bitarray_destroy(bitmap);
	free(configuracion);
	puts("FIN");
	return 0;
}
int main9(){
	configuracion = malloc(sizeof(ConfiguracionLFS));
	configurar(configuracion);
	levantarFileSystem();

	/*bitarray_set_bit(bitmap, 5191);
	bitarray_set_bit(bitmap, 5189);
	bitarray_set_bit(bitmap, 5185);
	bitarray_set_bit(bitmap, 1);
	limpiarBitmap();
	mostrarBitmap();
	guardarBitmap();*/

	/*createFS("A", 1, 5, 123);
	createFS("A", 1, 5, 123);
	createFS("B", 1, 2, 123);
	createFS("C", 1, 3, 123);*/

	//mostrarTablas();

	/*insertLFS("A", 1, "SC", 1);
	insertLFS("A", 2, "SC", 2);
	insertLFS("A", 5, "SC", 5);
	insertLFS("A", 98, "SC", 98);
	mostrarRegistros("A");*/

	destruirFileSystem();
	free(configuracion);
	puts("fin");
	return 0;
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
	size_t sizeArchivo = bytesArchivo(bitmapArchivo);
	bitarray = malloc(sizeArchivo);
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
size_t bytesArchivo(FILE* Archivo){
	//El archivo debe estar abierto con open()
	struct stat st;
	fstat(Archivo, &st);
	return st.st_size;
}
int cantidadBloques(char** bloquesArray){
	int cantidad = 0;
	for(int i = 0; i<metadata->cantidadBloques; i++){
		if(bloquesArray[i])
			cantidad++;
		else
			break;
	}
	return cantidad;
}
char* encontrarRegistroParticion(char* pathParticion, uint16_t key){
	char* value = malloc(configuracion->TAMANIO_VALUE+1);
	int timestamp = 0;
	char* camposMetadatas[] = {
							   "SIZE",
							   "BLOCKS"
							 };
	t_config* particionData = archivoConfigCrear(pathParticion, camposMetadatas);
	int bloques = cantidadBloques(archivoConfigSacarArrayDe(particionData, "BLOCKS"));
	free(particionData);
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
}
int proximoPuntoYComa(char* array){
	for(int i = 0; i<sizeof(array); i++)
		if(array[0] == ';')
			return i;
	return -1;
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
	for(int i=0; i < (bitmap->size); i++){
		if(bitarray_test_bit(bitmap,i)==0)
			cantidad++;
	}
	return cantidad;
}
void limpiarBitmap(){
	for(int i = 0; i<metadata->cantidadBloques; i++)
		bitarray_clean_bit(bitmap, i);
}
void setBloqueOcupado(int index){
	bitarray_set_bit(bitmap, index);
	guardarBitmap();
}
void setBloqueLibre(int index){
	bitarray_clean_bit(bitmap, index);
	guardarBitmap();
}
void guardarBitmap(){
	char *puntoMontaje= string_from_format("%s", configuracion->PUNTO_MONTAJE);
	char *bitmapFile = string_from_format("%s/Metadata/Bitmap.bin", puntoMontaje);
	int Bytes = metadata->cantidadBloques/8;
	if(metadata->cantidadBloques%8 != 0)
		Bytes++;

	FILE *bitmapArchivo = fopen(bitmapFile, "w+b");
	fwrite(bitmap->bitarray, Bytes, 1, bitmapArchivo);

	fclose(bitmapArchivo);
	free(puntoMontaje);
	free(bitmapFile);
}
int proximoBloqueLibre(){
	for(int i=0;i<(bitmap->size);i++){
		if(bitarray_test_bit(bitmap,i)==0)
			return i;
	}
	return -1;
}
void mostrarBitmap(){
	for(int i = 0; i < (bitmap->size); i++){
		if(i%64==0 && i!=0){
			printf("-----\n");
		}
		printf("%d: %d\n", i, bitarray_test_bit(bitmap, i));
	}
	printf("\n");
}


//Comandos
void createFS(char* nombreTabla, char* consistencia, int particiones, long tiempoCompactacion){
	char *pathTabla = string_from_format("%s/Tables/%s", configuracion->PUNTO_MONTAJE, nombreTabla);
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
		bitarray_set_bit(bitmap, bloqueLibre);
		particionPath = string_from_format("%s/%d.bin", pathTabla, i);
		nuevoArchivo = fopen(particionPath, "w+");
		fprintf(nuevoArchivo, "SIZE=%d\n", 0);
		fprintf(nuevoArchivo, "BLOCKS=[%d]\n", bloqueLibre);

		fclose(nuevoArchivo);
		free(particionPath);
	}
	guardarBitmap();
	free(pathTabla);

	list_add(tablasLFS, crearTabla(nombreTabla, consistencia, particiones, tiempoCompactacion));
}
char* selectFS(char* tabla, int particion, uint16_t key){
	char* pathTabla = string_from_format("%sTables/%s", configuracion->PUNTO_MONTAJE, tabla);
	char* pathParticion = string_from_format("/%d.bin", particion);
	//TODO: habria que recorrer los temporales tambien
	char* value = encontrarRegistroParticion(pathParticion, key);
	free(pathParticion);

	if(value){
		//break;
	}

	//TODO: recorrer la particion y encontrar el value

	return NULL;
	free(pathTabla);
}

void dump(){
	for(int t = 0; t<tablasLFS->elements_count; t++){
		Tabla* tabla = list_get(tablasLFS, t);
		if(tabla->registro->elements_count > 0){
			char* registrosComprimidos = string_new();
			for(int i = 0; i<tabla->registro->elements_count; i++){
				RegistroLFS* reg = list_get(tabla->registro, i);
				char* comprimido = comprimirRegistro(reg);
				string_append(&registrosComprimidos, comprimido);
				free(comprimido);
			}
			if(string_length(registrosComprimidos) > 0){
				crearNuevosBloques(registrosComprimidos, tabla->nombreTabla);
			}
			free(registrosComprimidos);
		}
	}
	limpiarMemtable();
}
void crearNuevosBloques(char* registrosComprimidos, char* nombre){
	int sizeTotal = string_length(registrosComprimidos);
	int bloques = sizeTotal/metadata->tamanioBloque;
	if(sizeTotal%metadata->tamanioBloque)
		bloques++;
	int blocks[bloques];

	//Creo los bloques
	for(int i = 0; i<bloques; i++){
		int proxBloque = proximoBloqueLibre();
		if(proxBloque == -1){
			perror("No hay bloques libres");
			break;
		}
		blocks[i] = proxBloque;
		char* pathNuevo = string_from_format("%sBloques/%d.bin", configuracion->PUNTO_MONTAJE, proxBloque);
		bitarray_set_bit(bitmap, proxBloque);
		FILE* nuevoBloque = fopen(pathNuevo, "w+");
		free(pathNuevo);

		unsigned long pos = i*metadata->tamanioBloque;
		char* substring = string_substring(registrosComprimidos, pos, metadata->tamanioBloque);
		fprintf(nuevoBloque, "%s", substring);
		free(substring);

		fclose(nuevoBloque);
	}
	guardarBitmap();

	//Creo el .temp
	char* pathTable = string_from_format("%sTables/%s", configuracion->PUNTO_MONTAJE, nombre);
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
	free(registrosComprimidos);
}
