/*
 * sindicato.h
 *
 *  Created on: 30 ago. 2020
 *      Author: utnso
 */

#ifndef SRC_SINDICATO_H_
#define SRC_SINDICATO_H_

#include <sys/stat.h> // Para tener mkdir
#include "shared/estructuras.h"
#include "shared/cargador.h"
#include "commons/string.h"
#include "commons/bitarray.h"

// Header files mios
#include "consola.h"
#include "iniciarFS.h"
#include "bitarray.h"

char* PUERTO_ESCUCHA;

int cantidadDeElementosEnArray(char**);
void freeDeArray(char**);

char* pathMetadata;
char* pathRestaurantes;
char* pathRecetas;
char* pathBloques;

int BLOCKS;
int BLOCK_SIZE;
char* MAGIC_NUMBER;

#endif /* SRC_SINDICATO_H_ */
