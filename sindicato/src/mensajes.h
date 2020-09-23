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
void consultarPlatos(char*);
void confirmarPedido(char*, int);
void obtenerPedido(char*, int);
void obtenerRestaurante(char*, uint32_t);
void guardarPedido(char*, int);
void guardarPlato(char*, int, char*, int);
void terminarPedido(char*, int);

// Mensajes de consola
void crearReceta(char*, datosReceta unaReceta);
void crearRestaurant(char*, datosRestaurant unRestaurant);

#endif /* SRC_MENSAJES_H_ */
