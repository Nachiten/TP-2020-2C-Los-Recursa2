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
void consultarPlatos_sindicato(char*, uint32_t);
void confirmarPedido_sindicato(char*, int, uint32_t);
void obtenerPedido(char*, int, uint32_t);
void obtenerRestaurante_sindicato(char*, uint32_t);
void guardarPedido_sindicato(char*, int, uint32_t);
void guardarPlato(char*, int, char*, int, uint32_t);
void terminarPedido(char*, int, uint32_t);
void platoListo(char*, int, char*, uint32_t);
void obtenerReceta(char*, uint32_t);

// Mensajes de consola
void crearReceta(char*, datosReceta);
void crearRestaurant(char*, datosRestaurant);

// Otros
void enviarRespuestaBooleana(uint32_t, codigo_operacion, respuestaBool);
respuesta_obtener_pedido* generarRtaObtenerPedidoDefault();

// Frees
void freeRtaObtenerPedido(respuesta_obtener_pedido* rta);
void freeRtaObtenerReceta(respuesta_obtener_receta* rta);
void freeRtaObtenerRestaurante(respuesta_obtener_restaurante* rta);

// Logs
void loguearRtaObtenerPedido(respuesta_obtener_pedido* rta);
void loguearRtaObtenerReceta(respuesta_obtener_receta* rta);
void loguearRtaObtenerRestaurante(respuesta_obtener_restaurante* rta);
void loguearRtaConsultarPlatos(respuesta_consultar_platos* rta);


#endif /* SRC_MENSAJES_H_ */
