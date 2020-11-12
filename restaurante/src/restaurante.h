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
typedef struct{
	int32_t numPedido;
	int32_t socket_cliente;
}Pedido;


//***VARIABLES***
int32_t socket_sindicato;
sem_t* semId;
sem_t* semLog;
int32_t id_global;
t_list* listaPedidos;

//***MENSAJES DE RESTO***
void consultar_Platos(int32_t);
void crear_Pedido(int32_t);
void aniadir_plato(a_plato*);

//***FUNCIONES***
void inicializar_colas();
int32_t crear_id_pedidos();

//***FUNCIONES DEL SERVER***
void process_request(codigo_operacion cod_op, int32_t socket_cliente, uint32_t sizeAAllocar);
void serve_client(int32_t* socket);
void esperar_cliente(int32_t socket_servidor);
void iniciar_server(char* puerto);


#endif /* SRC_RESTAURANTE_H_ */
