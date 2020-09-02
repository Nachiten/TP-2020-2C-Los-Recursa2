/*
 * iniciarFS.c
 *
 *  Created on: 31 ago. 2020
 *      Author: utnso
 */

#include "iniciarFS.h"

void inicializarFileSystem(char* PUNTO_MONTAJE){

	printf("PathRestaurantes: %s", pathRestaurantes);

	// puntoMontaje/Recetas
	pathRecetas = crearCarpetaEn(PUNTO_MONTAJE, "/Files/Recetas");

	char* BITARRAY = malloc(BLOCKS / 8);

	// Crear los bloques dentro de carpeta /Blocks
	crearBloques();

	// Genero el bitarray
	t_bitarray* bitArrayBloques = crearBitArray(BITARRAY);

	// Piso todos los bits a 0
	vaciarBitArray(bitArrayBloques);

	// Lo guardo en archivo
	guardarBitArrayEnArchivo(BITARRAY);

	bitarray_destroy(bitArrayBloques);
	free(BITARRAY);
}

char* crearCarpetaEn(char* pathPuntoMontaje, char* nombreCarpeta){

	// Path de la carpeta {punto_montaje}/nombreCarpeta
	char* pathCarpeta = malloc(strlen(pathPuntoMontaje) + strlen(nombreCarpeta) + 1);
	// Hacer este malloc explota por sf | = malloc(strlen(pathPuntoMontaje + 1));

	// Copiando el punto de montaje al path de la carpeta
	strcpy(pathCarpeta, pathPuntoMontaje);

	// Agrego el path de la carpeta /nombreCarpeta
	strcat(pathCarpeta, nombreCarpeta);

	mkdir(pathCarpeta, 0777);

	return pathCarpeta;
}

void crearBloques(){

	int bloqueActual;

	for(bloqueActual = 1 ; bloqueActual <= BLOCKS ; bloqueActual++ ){

		// Array de chars para meter el int convertido a array
		char* enteroEnLetras = malloc(5 * sizeof(char));

		// Genero un array de chars que es el numero de bloque pasado a string
		sprintf(enteroEnLetras, "%i", bloqueActual);

		char* extension = ".AFIP";

		// 5 = longitud del numero + 1 por la /
		char* nombreArchivo = malloc( strlen(enteroEnLetras) + strlen(extension) + 2 );

		// El nombre de archivo es de la forma /{num}.bin | Ejemplo: /35.AFIP
		strcpy(nombreArchivo, "/");
		strcat(nombreArchivo, enteroEnLetras);
		strcat(nombreArchivo, extension);

		// Creo una copia del path de /Blocks para no modificarlo
		char* pathBloqueActual = malloc(strlen(pathBloques) + strlen(nombreArchivo) + 1);

		// Le pego el path de /Blocks
		strcpy(pathBloqueActual, pathBloques);

		// Le pego el valor hardcodeado 1.bin
		strcat(pathBloqueActual, nombreArchivo);

		FILE* bloque = fopen( pathBloqueActual , "w" );

		if (bloque == NULL){
			printf("No se pudo crear el bloque %i", bloqueActual);
			exit(3);
		}

		fclose(bloque);

		free(nombreArchivo);
		free(pathBloqueActual);
		free(enteroEnLetras);
	}

}

int existeFilesystem(){
	int retorno = 0;

	char* nombreBitmap = "/Bitmap.bin";

	char* pathBitmap = malloc(strlen(pathMetadata) + strlen(nombreBitmap) + 1);

	strcpy(pathBitmap, pathMetadata);
	strcat(pathBitmap, nombreBitmap);

	FILE* bitmap = fopen(pathBitmap, "r");

	// Si el archivo se encuentra
	if (bitmap != NULL){
		retorno = 1;
		fclose(bitmap);
	}

	free(pathBitmap);

	return retorno;

}


