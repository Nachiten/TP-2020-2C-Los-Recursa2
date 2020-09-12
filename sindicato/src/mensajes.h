/*
 * mensajes.h
 *
 *  Created on: 4 sep. 2020
 *      Author: utnso
 */

#ifndef SRC_MENSAJES_H_
#define SRC_MENSAJES_H_

#include "sindicato.h"
#include "estructuras.h"

// Mensajes de socket
void consultarPlatos(char* nombreRestaurant);
void confirmarPedido(char* nombreRestaurant, int IDPedido);
void obtenerPedido(char* nombreRestaurant, int IDPedido);
void obtenerRestaurante(char*);
void guardarPedido(char*, int);

// Mensajes de consola
void crearReceta(char*, datosReceta unaReceta);
void crearRestaurant(char*, datosRestaurant unRestaurant);

#endif /* SRC_MENSAJES_H_ */
