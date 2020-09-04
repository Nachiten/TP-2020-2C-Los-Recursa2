#ifndef SHARED_ESTRUCTURAS_H_
#define SHARED_ESTRUCTURAS_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h> // Para tener el uint32_t
#include <unistd.h>
#include <signal.h>
#include <readline/readline.h>
#include "commons/config.h"
#include "commons/log.h"

typedef enum
{
	CONSULTAR_RESTAURANTES = 1,
	SELECCIONAR_RESTAURANTE = 2,
	OBTENER_RESTAURANTE = 3,
	CONSULTAR_PLATOS = 4,
	GUARDAR_PLATO = 5,
	A_PLATO = 6,//(AÑADIR PLATO)
	PLATO_LISTO = 7,
	CREAR_PEDIDO = 8,
	GUARDAR_PEDIDO = 9,
	CONFIRMAR_PEDIDO = 10,
	CONSULTAR_PEDIDO = 11,
	OBTENER_PEDIDO = 12,
	FINALIZAR_PEDIDO = 13,//estos 2 hay que consultarlos (finalizar y terminar)
	TERMINAR_PEDIDO = 14,//estos 2 hay que consultarlos (finalizar y terminar)
	RESPUESTA_CONSULTAR_R,
	RESPUESTA_SELECCIONAR_R,
	RESPUESTA_OBTENER_R,
	RESPUESTA_CONSULTAR_PLATOS,
	RESPUESTA_GUARDAR_PLATO,
	RESPUESTA_A_PLATO, //(RESPUESTA AÑADIR PLATO)
	RESPUESTA_PLATO_LISTO,
	RESPUESTA_CREAR_PEDIDO,
	RESPUESTA_GUARDAR_PEDIDO,
	RESPUESTA_CONFIRMAR_PEDIDO,
	RESPUESTA_CONSULTAR_PEDIDO,
	RESPUESTA_OBTENER_PEDIDO,
	RESPUESTA_FINALIZAR_PEDIDO,
	RESPUESTA_TERMINAR_PEDIDO,
	ERROR = -1,
	DESCONEXION = 0
}codigo_operacion;

typedef struct
{
	uint32_t size;
	void* stream;
} t_buffer;

typedef struct
{
	codigo_operacion codigo_op;
	t_buffer* buffer;
} t_paquete;

typedef struct{
	uint32_t IDCocinero;
	char* afinidadPlato;
}cocinero;

typedef struct{
	uint32_t cantidadPasos;
	char** pasos;
	uint32_t tiempo_pasos[];
}receta;

typedef struct{
	char* nombrePlato; //comida?
	uint32_t cantidadPlatos;
	uint32_t cantLista; //Todo preguntar a los ayudantes
}item_pedido;

//ESTRUCTURAS PARA MANDAR MENSAJES-------------------------------------------------------------------------------------------------

typedef struct{
//ToDo ver si poner algo
}consultar_restaurante;

//Todo respuesta a consultar restaurante?

typedef struct{
	uint32_t cliente; //seria el PID del cliente
	uint32_t restaurante; //seria el PID del restaurante
}seleccionar_restaurante;

typedef struct{
	uint32_t restaurante; //seria el PID del restaurante
	//en cualquier otro caso que no sea de Restaurante al Sindicato, poner 0
}obtener_restaurante;

//ToDo crear pedido no tiene estructura? hay que poner una estructura para devolver la ID del pedido

typedef struct{
	char* nombreRestaurante;
	uint32_t idPedido;

	uint32_t restaurante; //seria el PID del restaurante. ToDO ver si dejar esto aca
}guardar_pedido;

typedef struct{
	char* nombrePlato;
	uint32_t idPedido;
}a_plato;//añadir plato

typedef struct{//ToDo hay que ver si dejamos el PID de restaurante, o un string
	char* nombreRestaurante;
	uint32_t restaurante; //seria el PID del restaurante. ToDO ver si dejar esto aca

	uint32_t idPedido;
	char* nombrePlato; //comida?
	uint32_t cantidadPlatos;
}guardar_plato;

typedef struct{
	uint32_t idPedido;
}confirmar_pedido;

typedef struct{
	char* nombreRestaurante;
	uint32_t restaurante; //seria el PID del restaurante. ToDO ver si dejar esto aca
	uint32_t idPedido;
	char* nombrePlato; //comida?
}plato_listo;

typedef struct{
	uint32_t idPedido;
}consultar_pedido;

typedef struct{
	char* nombreRestaurante;
	uint32_t restaurante; //seria el PID del restaurante. ToDO ver si dejar esto aca
	uint32_t idPedido;
}obtener_pedido;

typedef struct{
	char* nombreRestaurante;
	uint32_t restaurante; //seria el PID del restaurante. ToDO ver si dejar esto aca
	uint32_t idPedido;
}finalizar_pedido; //seria exactamente lo mismo para terminar_pedido. hace falta separarlos?

typedef struct{
	uint32_t cantRestaurantes;
	char** listaRestaurantes[];
}respuesta_consultar_restaurantes;

typedef struct{
	uint32_t respuesta;
}respuesta_ok_error;

typedef struct{//Todo ver con los ayudantes PORQUE ESTA PICANTE
	uint32_t cantidadCocineros;
	cocinero losCocineros;//esto es una lista
	uint32_t posX;
	uint32_t posY;
	uint32_t cantidadRecetas;
	receta las_recetas;//esto es una lista
	uint32_t cantHornos;
}respuesta_obtener_restaurante;

typedef struct{
	uint32_t cantidadPlatos;
	char** listaplatos[];
}respuesta_consultar_platos;

typedef struct{
	uint32_t idPedido;
}respuesta_crear_pedido;

typedef struct{
	char* nombreRestaurante;
	uint32_t restaurante; //seria el PID del restaurante. ToDO ver si dejar esto aca
	uint32_t repartidor;
	uint32_t estado; //ver como implementan la planificacion
	uint32_t cantidadPlatos;
	char** listaplatos[];
}respuesta_consultar_pedido;

typedef struct{
	item_pedido platos_pedido;//esto es una lista
}respuesta_obtener_pedido;





#endif /* SHARED_ESTRUCTURAS_H_ */
