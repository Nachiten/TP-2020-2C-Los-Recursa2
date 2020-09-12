/*
 * estructuras.h
 *
 *  Created on: 12 sep. 2020
 *      Author: utnso
 */

#ifndef SRC_ESTRUCTURAS_H_
#define SRC_ESTRUCTURAS_H_

typedef struct{
	int cantCocineros;
	char* posicion;
	char* afinidad;
	char* platos;
	char* preciosPlatos;
	int cantHornos;
}datosRestaurant;

typedef struct{
	char* pasos;
	char* tiempoPasos;
}datosReceta;

#endif /* SRC_ESTRUCTURAS_H_ */
