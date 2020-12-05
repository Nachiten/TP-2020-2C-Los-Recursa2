/*
 * restaurante.h
 *
 *  Created on: 16 sep. 2020
 *      Author: utnso
 */

#ifndef SRC_RESTAURANTE_H_
#define SRC_RESTAURANTE_H_

#include "utilsRestaurante.h"
//***STRUCTS***

//***VARIABLES***
pthread_t planificacion;

//***MENSAJES DE RESTO***
void consultar_Platos(int32_t);
void crear_Pedido(crear_pedido*, int32_t);
void aniadir_plato(a_plato*,int32_t);
void confirmar_Pedido(int32_t, int32_t);

//***FUNCIONES***
int32_t crear_id_pedidos();
int preparar_pcb_plato(uint32_t, char*, char*);


//***FUNCIONES DEL SERVER***
void process_request(codigo_operacion, int32_t, uint32_t);
void serve_client(int32_t*);
void esperar_cliente(int32_t);
void iniciar_server(char*);


#endif /* SRC_RESTAURANTE_H_ */
