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

// *** ESTRUCTURAS DE APP***
typedef struct{
	int32_t socket;
	char* nombre_resto;
	char* ip;
	char* puerto;
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


// ***MANEJO DE MENSAJES***
void inicializar_colas();
void inicializar_semaforos();
int32_t crear_id_pedidos();
void consultar_restaurantes(int32_t socket_cliente);
void seleccionarRestaurante(char* nombreResto, int32_t socket_cliente);
int buscar_restaurante(char* nombreResto);
void crear_pedido(int32_t socket);
int buscar_cliente(int32_t socket);
int buscar_resto(char* nombreResto);
void agregar_restaurante(info_resto* recibidoAgregarRestaurante);
void recibir_respuesta(codigo_operacion cod_op, info_resto* resto, int32_t  socket_cliente, char* nombreResto);

#endif /* SRC_APP_H_ */
