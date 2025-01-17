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
	OBTENER_RECETA,
	RESPUESTA_CONSULTAR_R,
	RESPUESTA_SELECCIONAR_R,
	RESPUESTA_OBTENER_REST,
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
	RESPUESTA_OBTENER_RECETA,
	AGREGAR_RESTAURANTE, //cuando un restaurante se reporta en la app para ser tenido en cuenta en CONSULTAR_RESTAURANTES
	RESPUESTA_AGREGAR_RESTAURANTE,
	HANDSHAKE,
	ERROR = -1,
	DESCONEXION = 0
}codigo_operacion;

typedef enum
{
	NADA_CARGADO = 0,
	PENDIENTE,
	CONFIRMADO,
	TERMINADO
}estado_de_pedido;

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

//ESTRUCTURAS PARA MANDAR MENSAJES-------------------------------------------------------------------------------------------------

//nadie te va a extrañar

typedef struct{
	uint32_t largoIDCliente;
	char* idCliente;
	uint32_t largoNombreRestaurante;
	char* nombreRestaurante;
}seleccionar_restaurante;

typedef struct{
	uint32_t largoNombreRestaurante;
	char* nombreRestaurante;
}obtener_restaurante;

typedef struct{
	uint32_t largoNombreRestaurante;
	char* nombreRestaurante;
	uint32_t idPedido;
}guardar_pedido;

typedef struct{
	uint32_t largoNombrePlato;
	char* nombrePlato;
	uint32_t idPedido;
	uint32_t sizeId;     //lo agrego "de manera rustica" porque sin esto app esta en bolas
	char* id;             //lo agrego "de manera rustica" porque sin esto app esta en bolas
}a_plato;//añadir plato

typedef struct{
	uint32_t largoNombreRestaurante;
	char* nombreRestaurante;
	uint32_t idPedido;
	uint32_t largoNombrePlato;
	char* nombrePlato; //comida
	uint32_t cantidadPlatos;
}guardar_plato;


typedef struct{
	uint32_t largoNombreRestaurante;
	char* nombreRestaurante;
	uint32_t idPedido;
}confirmar_pedido;

typedef struct{
	uint32_t largoNombreRestaurante;
	char* nombreRestaurante;
	uint32_t idPedido;
	uint32_t largoNombrePlato;
	char* nombrePlato;
}plato_listo;

typedef struct{
	uint32_t idPedido;
	uint32_t sizeId;  //lo agrego "de manera rustica" porque sin esto app esta en bolas
	char* id;         //lo agrego "de manera rustica" porque sin esto app esta en bolas
}consultar_pedido;

//ATENCION!!!! Reutilizamos serializacion de GUARDAR_PEDIDO
typedef struct{
	uint32_t largoNombreRestaurante;
	char* nombreRestaurante;
	uint32_t idPedido;
}obtener_pedido;

//ATENCION!!!! Reutilizamos serializacion de GUARDAR_PEDIDO
typedef struct{
	uint32_t largoNombreRestaurante;
	char* nombreRestaurante;
	uint32_t idPedido;
}finalizar_pedido; //seria exactamente lo mismo para terminar_pedido. hace falta separarlos?

typedef struct{
	uint32_t largoNombreReceta;
    char* nombreReceta;
}obtener_receta;

typedef struct{
	uint32_t longitudIDCliente;
	char* id;
	uint32_t posX;
	uint32_t posY;
}handshake;

typedef struct{
	uint32_t cantRestaurantes;
	uint32_t longitudListaRestaurantes;
	char* listaRestaurantes;
}respuesta_consultar_restaurantes;

typedef struct{
	uint32_t respuesta;
}respuesta_ok_error;

typedef struct{
	uint32_t cantidadCocineros;
	uint32_t posX;
	uint32_t posY;
	uint32_t cantHornos;
	uint32_t cantPedidos;

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
	estado_de_pedido estado;
	uint32_t sizeComidas;
	char* comidas;
	uint32_t sizeCantTotales;
	char* cantTotales;
	uint32_t sizeCantListas;
	char* cantListas;
}respuesta_consultar_pedido;

typedef struct{
	estado_de_pedido estado;
	uint32_t sizeComidas;
	char* comidas;
	uint32_t sizeCantTotales;
	char* cantTotales;
	uint32_t sizeCantListas;
	char* cantListas;
}respuesta_obtener_pedido;

typedef struct{
	uint32_t sizePasos;
	char* pasos;
	uint32_t sizeTiempoPasos;
	char* tiempoPasos;
}respuesta_obtener_receta;

typedef struct{
	uint32_t sizeMensaje;
	char* mensaje;
}pedido_finalizado;

typedef struct{
	uint32_t sizeNombre;
	char* nombreResto;
	uint32_t sizeId;        //lo agrego "de manera rustica" porque sin esto app esta en bolas
	char* id;               //lo agrego "de manera rustica" porque sin esto app esta en bolas
}consultar_platos;

typedef struct{
	uint32_t sizeId;        //lo agrego "de manera rustica" porque sin esto app esta en bolas
	char* id;               //lo agrego "de manera rustica" porque sin esto app esta en bolas
}crear_pedido;

typedef struct{
	uint32_t largoNombreRestaurante;
	char* nombreRestaurante;
	uint32_t largoIp;
	char* ip;
	uint32_t largoPuerto;
	char* puerto;
	uint32_t posX;
	uint32_t posY;
}agregar_restaurante;

#endif /* SHARED_ESTRUCTURAS_H_ */
