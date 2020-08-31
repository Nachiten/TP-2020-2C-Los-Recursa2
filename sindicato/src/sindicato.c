/*
 * sindicato.c
 *
 *  Created on: 29 ago. 2020
 *      Author: utnso
 */

#include "sindicato.h"

int cantidadDeElementosEnArray(char** array){
	int i = 0;
	while(array[i] != NULL && strcmp(array[i], "\n") != 0){
		i++;
	}
	return i;
}

// Hacer free de un array con cosas
void freeDeArray(char** array){
	int cantidadElementosArray = cantidadDeElementosEnArray(array);

	int i;

	for (i = cantidadElementosArray; i>= 0; i--){
		free(array[i]);
	}

	free(array);
}

t_config* leerConfig(char** PUNTO_MONTAJE){

	t_config* config = leerConfiguracion("/home/utnso/workspace/tp-2020-2c-Los-Recursa2/configs/sindicato.config");

	if (config == NULL){
		printf("No se pudo leer la config!!");
		exit(1);
	}

	*PUNTO_MONTAJE = config_get_string_value(config, "PUNTO_MONTAJE");
	PUERTO_ESCUCHA = config_get_string_value(config, "PUERTO_ESCUCHA");

	return config;
}

t_config* leerMetadataBin(){

	// Nombre del archivo metadata
	char* archivoMetadata = "/Metadata.AFIP";

	// Path de la carpeta {punto_montaje}/Metadata/Metadata.bin
	char* pathMetadataBin = malloc(strlen(pathMetadata) + strlen(archivoMetadata) + 1);

	// Le pego el path de metadata
	strcpy(pathMetadataBin, pathMetadata);
	// Le pego al final el path del archivo
	strcat(pathMetadataBin, archivoMetadata);

	// Leo el archivo
	t_config* metadataBin = leerConfiguracion(pathMetadataBin);

	if (metadataBin == NULL){
		printf("No se pudo leer el archivo Metadata/Metadata.AFIP");
		exit(2);
	}

	// Obtengo los valores (pasados por referencia)
	BLOCKS = config_get_int_value(metadataBin, "BLOCKS");
	BLOCK_SIZE = config_get_int_value(metadataBin, "BLOCK_SIZE");
	MAGIC_NUMBER = config_get_string_value(metadataBin,"MAGIC_NUMBER" );

	if (BLOCKS % 8 != 0){
		printf("ERROR | La cantidad de bloques debe ser multiplo de 8\n");
		exit(5);
	}

	free(pathMetadataBin);

//	printf("Blocks: %i\n", BLOCKS);
//	printf("BlockSize: %i\n", BLOCK_SIZE);
//	printf("MagicNumber: %s\n", MAGIC_NUMBER);

	return metadataBin;
}

int main(){
	printf("Comienzo sindicato\n");

	// Leer input de consola
	//while(1)
	//obtenerInputConsola();

	char* PUNTO_MONTAJE;
	t_config* config = leerConfig(&PUNTO_MONTAJE);

	// puntoMontaje/Metadata
	pathMetadata = crearCarpetaEn(PUNTO_MONTAJE, "/Metadata");
	// puntoMontaje/Blocks
	pathBloques = crearCarpetaEn(PUNTO_MONTAJE, "/Blocks");
	// puntoMontaje/Restaurantes
	pathRestaurantes = crearCarpetaEn(PUNTO_MONTAJE, "/Restaurantes");
	// puntoMontaje/Recetas
	pathRecetas = crearCarpetaEn(PUNTO_MONTAJE, "/Recetas");

	// Funcion para leer metadata.bin
	t_config* metadataBin = leerMetadataBin();

	// Si no existe el filesystem
	if (existeFilesystem()){
		printf("El filesystem ya existe. No se debe inicializar.\n");
	} else {
		printf("No existe filesystem... inicializando.\n");
		inicializarFileSystem();
	}

	// Liberaciones finales (a las que nunca se llega)
	config_destroy(config);
	config_destroy(metadataBin);

	return 0;
}
