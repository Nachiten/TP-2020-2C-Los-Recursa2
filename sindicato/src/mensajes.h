/*
 * mensajes.h
 *
 *  Created on: 4 sep. 2020
 *      Author: utnso
 */

#ifndef SRC_MENSAJES_H_
#define SRC_MENSAJES_H_

#include "sindicato.h"

void obtenerRestaurante(char*);
void guardarPedido(char*, int);
void consultarPlatos(char* nombreRestaurant);
void confirmarPedido(char* nombreRestaurant, int IDPedido);
//void crearReceta(char*, datosReceta);
//void crearRestaurant(char*, datosRestaurant);

#endif /* SRC_MENSAJES_H_ */
