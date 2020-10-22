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

// *** ESTRUCTURA DE PLANIFICACION ***
typedef struct{
	int numeroRepartidor;
	int posX;
	int posY;
	// Cada cuanto tiene que descansar
	int frecuenciaDescanso;
	// Cuanto tiempo tiene que descansar
	int tiempoDescanso;
	// Cuanto tiempo ya descanso
	int tiempoDescansado;
	int asignado;
}repartidor;

typedef enum{
	// No esta en blocked
	NO,
	// Esta esperando un msg
	ESPERANDO_MSG,
	// Esta descansando
	DESCANSANDO,
}estadoBlock;

typedef struct{
	int pedidoID;
	int instruccionesTotales;
	repartidor* repartidorAsignado;
	int instruccionesRealizadas;
	int posObjetivoX;
	int posObjetivoY;
	estadoBlock estadoBlocked;
	char* nombre_resto;
	int tiempoEspera;
}pcb_pedido;

// *** ESTRUCTURAS DE APP***
typedef struct{
	int32_t socket;
	char* nombre_resto;
	char* ip;
	char* puerto;
	int32_t posX;
	int32_t posY;
}info_resto;

typedef struct{
	int32_t socket_cliente;
	char* nombre_resto;
	uint32_t id_pedido;
	int perfilActivo;
	int32_t posX;
	int32_t posY;
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
char* algoritmo_planif;
int32_t posX_resto;
int32_t posY_resto;

sem_t* semId;
sem_t* semLog;
int32_t id_inicial_pedidos;
int32_t socket_commanda;

// ***SETUP***
void inicializar_colas();
void inicializar_semaforos();

// ***MENSAJES DE APP***
void consultar_restaurantes(int32_t socket_cliente);
void seleccionarRestaurante(char* nombreResto, int32_t socket_cliente);
void crear_pedido(int32_t socket);
void aniadir_plato(a_plato* recibidoAPlato);
void plato_Listo(plato_listo* platoListo);
void confirmar_Pedido(confirmar_pedido* pedido);
void agregar_restaurante(info_resto* recibidoAgregarRestaurante);

// ***MANEJO DE MENSAJES***
int32_t crear_id_pedidos();
void crear_pedido(int32_t socket);
int buscar_pedido(uint32_t id_pedido);
int buscar_cliente(int32_t socket);
int buscar_cliente_id(uint32_t id_pedido_buscado);
int buscar_resto(char* nombreResto);
void recibir_respuesta(codigo_operacion cod_op, info_resto* resto, perfil_cliente* cliente);

// ***SERVIDOR***
void process_request(codigo_operacion cod_op, int32_t socket_cliente, uint32_t sizeAAllocar) ;
void serve_client(int32_t* socket);
void esperar_cliente(int32_t socket_servidor);
void iniciar_server(char* puerto);

#endif /* SRC_APP_H_ */
