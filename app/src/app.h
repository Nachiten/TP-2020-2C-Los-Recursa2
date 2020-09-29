#ifndef SRC_APP_H_
#define SRC_APP_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/collections/queue.h>
#include<string.h>
#include<pthread.h>
#include <inttypes.h> // Para tener el uint32_t
#include <semaphore.h> // para los semaforos
#include "shared/estructuras.h"
#include"shared/cargador.h"
#include"shared/socket.h"

// *** ESTRUCTURAS DE APP***
typedef struct{
	int32_t socket;
	char* nombre_resto;
	char* ip;
	char* puerto;
	char* posicion;
}info_resto;

typedef struct{
	int32_t socket_cliente;
	char* nombre_resto;
	uint32_t id_pedido;
	int perfilActivo;
}perfil_cliente;



//***VARIABLES***
t_list* listaRestos;
t_list* listaPedidos;

t_log* logger;
t_config* config;

//***cosas de config**
char* LOG_PATH;
char* mi_puerto;
char* puerto_commanda;
char* ip_commanda;
char* platos_default;
int32_t posX_resto;
int32_t posY_resto;

sem_t* semId;
sem_t* semLog;
int32_t id_inicial_pedidos;
int32_t socket_commanda;


// ***SERVIDOR***
void process_request(codigo_operacion cod_op, int32_t socket_cliente, uint32_t sizeAAllocar) ;
void serve_client(int32_t* socket);
void esperar_cliente(int32_t socket_servidor);
void iniciar_server(char* puerto);

// ***SETUP***
void inicializar_colas();
void inicializar_semaforos();

// ***MENSAJES DE APP***
void consultar_restaurantes(int32_t socket_cliente);
void seleccionarRestaurante(char* nombreResto, int32_t socket_cliente);
void crear_pedido(int32_t socket);
void aniadir_plato(a_plato* recibidoAPlato);
void agregar_restaurante(info_resto* recibidoAgregarRestaurante);

// ***MANEJO DE MENSAJES***
int32_t crear_id_pedidos();
void crear_pedido(int32_t socket);
int buscar_pedido(uint32_t id_pedido);
int buscar_cliente(int32_t socket);
int buscar_resto(char* nombreResto);
void recibir_respuesta(codigo_operacion cod_op, info_resto* resto, perfil_cliente* cliente);

#endif /* SRC_APP_H_ */
