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
#include "commons/collections/list.h"

typedef enum
{
	CONSULTAR_RESTAURANTES = 1,
	SELECCIONAR_RESTAURANTE,
	OBTENER_RESTAURANTE,
	CONSULTAR_PLATOS,
	GUARDAR_PLATO,
	A_PLATO,//(AÑADIR PLATO)
	PLATO_LISTO,
	CREAR_PEDIDO,
	GUARDAR_PEDIDO,
	CONFIRMAR_PEDIDO,
	CONSULTAR_PEDIDO,
	OBTENER_PEDIDO,
	FINALIZAR_PEDIDO,
	TERMINAR_PEDIDO,
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
	AGREGAR_RESTAURANTE, //cuando un restaurante se reporta en la app para ser tenido en cuenta en CONSULTAR_RESTAURANTES
	ERROR = -1,
	DESCONEXION = 0
}codigo_operacion;

typedef struct
{
	uint32_t size;
	void* stream;
}t_buffer;

typedef struct
{
	codigo_operacion codigo_op;
	t_buffer* buffer;
}t_paquete;



//ESTRUCTURAS DE APP-------------------------------------------------------------

typedef struct{
	int32_t socket;
	char* nombre_resto;
}info_resto;

typedef struct{
	int32_t socket_cliente;
	char* nombre_resto;
	uint32_t id_pedido;
}perfil_cliente;

//ESTRUCTURAS DE RESTAURANTE-----------------------------------------------------


//tentativo
typedef struct{
	uint32_t id_pedido;
    t_list lista_platos;
}t_pedido;

//tentativo
typedef struct{
	uint32_t cantidadPasos;
	char** pasos;
	uint32_t tiempo_pasos[]; //preguntar
}t_receta;


//tentativo
typedef struct{
	uint32_t longitudNombrePlato;
	char* nombrePlato;
	uint32_t cantidadPlatos;
	uint32_t cantLista;
	t_receta receta;
}t_plato;


typedef struct{
	uint32_t id_cocinero;
	char* afinidadPlato;
}t_cocinero;


//ESTRUCTURAS PARA MANDAR MENSAJES-------------------------------------------------------------------------------------------------

//nadie te va a extrañar

typedef struct{
	uint32_t cliente; //seria el PID del cliente
	uint32_t largoNombreRestaurante;
	char* nombreRestaurante;
}seleccionar_restaurante;

typedef struct{
	uint32_t largoNombreRestaurante;
	char* nombreRestaurante;
}obtener_restaurante;

//ToDo crear pedido no tiene estructura? hay que poner una estructura para devolver la ID del pedido

typedef struct{
	uint32_t largoNombreRestaurante;
	char* nombreRestaurante;
	uint32_t idPedido;
}guardar_pedido;

typedef struct{
	char* nombrePlato;
	uint32_t idPedido;
}a_plato;//añadir plato

typedef struct{
	uint32_t largoNombreRestaurante;
	char* nombreRestaurante;
	uint32_t idPedido;
	uint32_t largonombrePlato;
	char* nombrePlato; //comida
	uint32_t cantidadPlatos;
}guardar_plato;

typedef struct{
	uint32_t idPedido;
}confirmar_pedido;

typedef struct{
	uint32_t largoNombreRestaurante;
	char* nombreRestaurante;
	uint32_t idPedido;
	char* nombrePlato; //comida?
}plato_listo;

typedef struct{
	uint32_t idPedido;
}consultar_pedido;

typedef struct{
	uint32_t largoNombreRestaurante;
	char* nombreRestaurante;
	uint32_t idPedido;
}obtener_pedido;

typedef struct{
	uint32_t largoNombreRestaurante;
	char* nombreRestaurante;
	uint32_t idPedido;
}finalizar_pedido; //seria exactamente lo mismo para terminar_pedido. hace falta separarlos?

typedef struct{
	uint32_t cantRestaurantes;
	uint32_t longitudlistaRestaurantes;
	char* listaRestaurantes;
}respuesta_consultar_restaurantes;

typedef struct{
	uint32_t respuesta;
}respuesta_ok_error;

typedef struct{//Todo ver con los ayudantes PORQUE ESTA PICANTE
	uint32_t cantidadCocineros;
	uint32_t posX;
	uint32_t posY;
	uint32_t cantHornos;

	uint32_t longitudAfinidades;
	char* afinidades;

	uint32_t longitudPlatos;
	char* platos;

	uint32_t longitudPrecioPlatos;
	char* precioPlatos;

}respuesta_obtener_restaurante;

typedef struct{
	uint32_t longitudNombresPlatos;
	char* nombresPlatos;
}respuesta_consultar_platos;

typedef struct{
	uint32_t idPedido;
}respuesta_crear_pedido;

typedef struct{
	uint32_t largoNombreRestaurante;
	char* nombreRestaurante;
	uint32_t repartidor;
	uint32_t estado; //ver como implementan la planificacion
	uint32_t cantidadPlatos;
	char** listaplatos[];
}respuesta_consultar_pedido;

typedef struct{
	uint32_t cantPlatos;
	t_plato platos_pedido[];//esto es una lista
}respuesta_obtener_pedido;





#endif /* SHARED_ESTRUCTURAS_H_ */
