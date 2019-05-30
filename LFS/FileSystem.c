/*
 * FileSystem.c
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */

#include "FileSystem.h"

void levantarFileSystem(){
	tablasLFS = list_create();
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
void fileSystem(){

}
void destruirFileSystem(){
	//free(metadata->magicNumber);
	free(bitarray);
	free(metadata);
	bitarray_destroy(bitmap);
	list_destroy_and_destroy_elements(tablasLFS, (void*) tablaDestruir);
}

//TODO: testear todas las cosas
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
}
int main9(){
	configuracion = malloc(sizeof(ConfiguracionLFS));
	configurar(configuracion);

	levantarFileSystem();
	bitarray_set_bit(bitmap, 5191);
	bitarray_set_bit(bitmap, 5189);
	bitarray_set_bit(bitmap, 5185);
	bitarray_set_bit(bitmap, 1);
	//limpiarBitmap();
	mostrarBitmap();
	destruirFileSystem();

	//free(configuracion);
	puts("fin");
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
				int tiempoComp = archivoConfigSacarIntDe(archivoConfig, "COMPACTION_TIME"); //TODO: giardar la consistencia como string
				list_add(tablasLFS, crearTabla(dir->d_name, archivoConfigSacarIntDe(archivoConfig, "CONSISTENCY"), particiones, tiempoComp));
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
	//TODO: checkear
	char *puntoMontaje= string_from_format("%s", configuracion->PUNTO_MONTAJE);
	char *bitmapFile = string_from_format("%s/Metadata/Bitmap.bin", puntoMontaje);

	FILE *bitmapArchivo = fopen(bitmapFile, "w+b");
	fwrite(bitmap->bitarray, bitmap->size, 1, bitmapArchivo);

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


///
void createFS(char* nombreTabla, int consistencia, int particiones, long tiempoCompactacion){
	if(tablaEncontrar(nombreTabla)!=NULL){
		//Loguear error
		perror("Ya existe una tabla con ese nombre.");
		return;
	}
	else{
		char *pathTabla = string_from_format("%s/Tables/%s", configuracion->PUNTO_MONTAJE, nombreTabla);
		mkdir(pathTabla, 0777);

		//Creo la metadata
		char *metadataPath = string_from_format("%s/Metadata", pathTabla);
		FILE *nuevoArchivo = fopen(metadataPath, "w+");
		fprintf(nuevoArchivo, "CONSISTENCY=%d\n", consistencia);
		fprintf(nuevoArchivo, "PARTITIONS=%d\n", particiones);
		fprintf(nuevoArchivo, "COMPACTION_TIME=%ld\n", tiempoCompactacion);
		fclose(nuevoArchivo);
		free(metadataPath);

		//TODO: ASIGNAR UN BLOQUE A CADA PARTICION
		//Creo las particiones
		char *particionPath;
		for(int i = 0; i<particiones; i++){
			particionPath = string_from_format("%s/%d.bin", pathTabla, i);
			nuevoArchivo = fopen(particionPath, "w+");
			fprintf(nuevoArchivo, "SIZE=%d\n", 0);
			fprintf(nuevoArchivo, "BLOCKS=[]\n");
			fclose(nuevoArchivo);
			free(particionPath);
		}
		free(pathTabla);

		list_add(tablasLFS, crearTabla(nombreTabla, consistencia, particiones, tiempoCompactacion));
	}
}
