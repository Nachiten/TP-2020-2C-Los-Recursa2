/*
 * consola.h
 *
 *  Created on: 31 ago. 2020
 *      Author: utnso
 */

#ifndef SRC_CONSOLA_H_
#define SRC_CONSOLA_H_

#include "sindicato.h"
#include "estructuras.h"

void obtenerInputConsola();
void printearDatosRestaurant(datosRestaurant unRestaurant);
void printearDatosPasos(datosReceta unaReceta);
int checkearLongitudArraysStrings(char*, char*);
int existeRestaurant(char*);

#endif /* SRC_CONSOLA_H_ */
