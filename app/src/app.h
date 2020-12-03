#ifndef SRC_APP_H_
#define SRC_APP_H_

#include "planificacion.h"


// *** ESTRUCTURAS DE APP***

t_config* config;

//***cosas de config**
char* LOG_PATH;
char* mi_puerto;
char* platos_default;

int32_t posX_resto;
int32_t posY_resto;

sem_t* semId;
int32_t id_global;
int32_t id_restoDefault;
int32_t socket_commanda;

pthread_t planificacion;

// ***SETUP***
void inicializar_listas();
void inicializar_semaforos();

// ***MENSAJES DE APP***
void consultarRestaurantes(int32_t);
void seleccionarRestaurante(seleccionar_restaurante*, int32_t);
void consultarPlatos(consultar_platos*,int32_t);
void crearPedido(crear_pedido*, int32_t);
void aniadirPlato(a_plato*, int32_t);
void platoListo(plato_listo*, int32_t);
void confirmarPedido(confirmar_pedido*, int32_t);
void registrarRestaurante(agregar_restaurante*, int32_t);
void registrarHandshake(handshake*, int32_t);

// ***MANEJO DE MENSAJES***
int32_t crear_id_pedidos();
int buscarPedidoPorIDGlobal(uint32_t);
int buscarAsociacion(char*);
int buscarRestaurante(char*);
void pedidoEentregado(int32_t id_pedido);

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
