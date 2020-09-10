#ifndef SRC_APP_H_
#define SRC_APP_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/string.h>
#include<string.h>
#include<pthread.h>
#include <inttypes.h> // Para tener el uint32_t
#include <semaphore.h> // para los semaforos
#include "shared/estructuras.h"
#include"shared/cargador.h"
#include"shared/socket.h"

t_list* listaRestos;
t_list* listaPedidos;

// ***SERVIDOR***
void process_request(codigo_operacion cod_op, int32_t socket_cliente, uint32_t sizeAAllocar) ;
void serve_client(int32_t* socket);
void esperar_cliente(int32_t socket_servidor);
void iniciar_server(char* ip, char* puerto);

// ***MANEJO DE MENSAJES***
void consultar_restaurantes(int32_t socket_cliente);
void seleccionarRestaurante(char* nombreResto, int32_t socket_cliente);
int buscar_restaurante(char* nombreResto);
int32_t buscar_socket_resto(char* nombreResto);
void agregar_restaurante(info_resto* recibidoAgregarRestaurante);

#endif /* SRC_APP_H_ */
