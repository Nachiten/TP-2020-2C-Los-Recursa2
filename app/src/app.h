#ifndef SRC_APP_H_
#define SRC_APP_H_

#include "planificacion.h"


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
	uint32_t id_global;
	uint32_t id_pedido_resto;
	int perfilActivo;
	int32_t posX;
	int32_t posY;
}perfil_cliente;


//***VARIABLES***
t_list* listaRestos;
t_list* listaPedidos;


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
int32_t id_global;
int32_t socket_commanda;

// ***SETUP***
void inicializar_colas();
void inicializar_semaforos();

// ***MENSAJES DE APP***
void consultar_restaurantes(int32_t);
void seleccionarRestaurante(char*, int32_t);
void crear_pedido(int32_t);
void aniadir_plato(a_plato*);
void plato_Listo(plato_listo*, int32_t);
void confirmar_Pedido(confirmar_pedido*);
void agregar_restaurante(info_resto*);

// ***MANEJO DE MENSAJES***
int32_t crear_id_pedidos();
void crear_pedido(int32_t);
int buscar_pedido_por_id(uint32_t);
int buscar_pedido_por_id_y_resto(uint32_t, info_resto*);
int buscar_cliente(int32_t);
int buscar_resto(char*);
int buscar_resto_por_socket(int32_t);
void recibir_respuesta(codigo_operacion, info_resto*, perfil_cliente*);
void pedido_entregado(int32_t id_pedido);

// ***SERVIDOR***
void process_request(codigo_operacion, int32_t, uint32_t) ;
void serve_client(int32_t*);
void esperar_cliente(int32_t);
void iniciar_server(char*);

// INICIALIZACION PLANIF
void iniciarSemaforosCiclos();
void iniciarSemaforosPlanificacion();
void leerPlanificacionConfig(t_config*);
void iniciarPlanificacion();

#endif /* SRC_APP_H_ */
