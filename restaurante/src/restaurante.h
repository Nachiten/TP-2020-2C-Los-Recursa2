/*
 * restaurante.h
 *
 *  Created on: 16 sep. 2020
 *      Author: utnso
 */

#ifndef SRC_RESTAURANTE_H_
#define SRC_RESTAURANTE_H_

#include "utilsRestaurante.h"

//***VARIABLES***
int32_t socket_sindicato;

//***FUNCIONES DEL SERVER***
void process_request(codigo_operacion cod_op, int32_t socket_cliente, uint32_t sizeAAllocar);
void serve_client(int32_t* socket);
void esperar_cliente(int32_t socket_servidor);
void iniciar_server(char* puerto);


#endif /* SRC_RESTAURANTE_H_ */
