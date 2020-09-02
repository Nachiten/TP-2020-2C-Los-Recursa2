/*
 * consola.h
 *
 *  Created on: 31 ago. 2020
 *      Author: utnso
 */

#ifndef SRC_CONSOLA_H_
#define SRC_CONSOLA_H_

#include "sindicato.h"

typedef struct datosRestaurant{
	int cantCocineros;
	char* posicion;
	char* afinidad;
	char* platos;
	char* preciosPlatos;
	int cantHornos;
}datosRestaurant;

typedef struct datosPasos{
	char* nombre;
	char* pasos;
	char* tiempoPasos;
}datosPasos;

void obtenerInputConsola();
void printearDatosRestaurant(datosRestaurant);
void printearDatosPasos(datosPasos);
int checkearLongitudArraysStrings(char*, char*);

#endif /* SRC_CONSOLA_H_ */
