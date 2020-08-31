/*
 * bitarray.h
 *
 *  Created on: 31 ago. 2020
 *      Author: utnso
 */

#ifndef SRC_BITARRAY_H_
#define SRC_BITARRAY_H_

#include "sindicato.h"

t_bitarray* crearBitArray(char*);
void vaciarBitArray(t_bitarray*);
void guardarBitArrayEnArchivo(char*);

#endif /* SRC_BITARRAY_H_ */
