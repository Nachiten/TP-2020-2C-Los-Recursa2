/*
 * bitarray.c
 *
 *  Created on: 31 ago. 2020
 *      Author: utnso
 */

#include "bitarray.h"

t_bitarray* crearBitArray(char* bitarray){
	// Se crea en cantidad de bytes no bits. (por eso hago cantBloques / 8)
	return bitarray_create_with_mode(bitarray, BLOCKS / 8, MSB_FIRST);
	// 'bitarray' es un malloc de cantBloques / 8
}

void vaciarBitArray(t_bitarray* bitArray){

	int i;

	for (i = 0; i < BLOCKS; i++){
		bitarray_clean_bit(bitArray, i);
	}
}

void guardarBitArrayEnArchivo(char* bitArray){
	char* pathBitmap = "/Bitmap.bin";

	char* pathCompleto = malloc(strlen(pathMetadata) + strlen(pathBitmap) + 1);

	// Copio path de metadata
	strcpy(pathCompleto, pathMetadata);

	// Pego el path de bitmap
	strcat(pathCompleto, pathBitmap);

	FILE* bitmapArchivo = fopen( pathCompleto , "w" );

	// Me muevo al principio del archivo
	fseek( bitmapArchivo, 0, SEEK_SET );

	fwrite(bitArray, BLOCKS / 8, 1, bitmapArchivo);

	fclose(bitmapArchivo);

	free(pathCompleto);
}
