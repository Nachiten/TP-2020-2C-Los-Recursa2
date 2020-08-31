/*
 * sindicato.h
 *
 *  Created on: 30 ago. 2020
 *      Author: utnso
 */

#ifndef SRC_SINDICATO_H_
#define SRC_SINDICATO_H_

#include "shared/estructuras.h"
#include "commons/string.h"

typedef struct datosRestaurant{
	char* nombre;
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

int cantidadDeElementosEnArray(char**);

#endif /* SRC_SINDICATO_H_ */
