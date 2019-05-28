/*
 * FileSystem.c
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */

#include "FileSystem.h"

void levantarFileSystem(){
	FILE *Archivo;
	char *puntoMontaje = string_from_format("%s", configuracion->PUNTO_MONTAJE);

	//Carpeta Metadata
	char *pathMetadata = string_from_format("%s/Metadata", puntoMontaje);
	mkdir(pathMetadata, 0777);

		//Archivo Metadata.bin
		char *metadata = string_from_format("%s/Metadata.bin", pathMetadata);
		Archivo = fopen(metadata, "w+");
		fprintf(Archivo, "BLOCK_SIZE=%d\n", 64);
		fprintf(Archivo, "BLOCKS=%d\n", 5192);
		fprintf(Archivo, "MAGIC_NUMBER=LISSANDRA\n");
		fclose(Archivo);
		free(metadata);

		//Archivo Bitmap.bin
		char *bitmap = string_from_format("%s/Bitmap.bin", pathMetadata);
		Archivo = fopen(bitmap, "w+");
		//CREAR BITMAP

		fclose(Archivo);
		free(bitmap);

	free(pathMetadata);

	//Carpeta Tables
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
				list_add(tablasLFS, crearTabla(dir->d_name, archivoConfigSacarIntDe(archivoConfig, "CONSISTENCY"), particiones, tiempoComp));
				free(tablaPath);
				free(tablaMetadataPath);
				archivoConfigDestruir(archivoConfig);
			}
		}
		closedir(tables);
	}
	free(tablesPath);

	//Carpeta Bloques
	char *bloquesPath = string_from_format("%s/Bloques", puntoMontaje);
	mkdir(bloquesPath, 0777);
	free(bloquesPath);

	free(puntoMontaje);

	return;
}
void fileSystem(){

}
void destruirFileSystem(){

}

/*
int main(){
	configuracion = malloc(sizeof(ConfiguracionLFS));
	configurar(configuracion);

	tablasLFS = list_create();
	levantarFileSystem();
	createFS("A", 1, 5, 123);
	createFS("A", 1, 5, 123);
	createFS("B", 1, 2, 123);

	Tabla *a = list_get(tablasLFS, 0);
	printf("tabla: %s particiones=%d\n", a->nombreTabla, a->metadata->particiones);
	a = list_get(tablasLFS, 1);
	printf("tabla: %s particiones=%d\n", a->nombreTabla, a->metadata->particiones);
	list_destroy_and_destroy_elements(tablasLFS, (void*) tablaDestruir);

	puts("destruyo");
}
*/

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

		//ASIGNAR UN BLOQUE A CADA PARTICION
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
