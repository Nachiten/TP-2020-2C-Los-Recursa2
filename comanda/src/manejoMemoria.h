#ifndef SRC_MANEJOMEMORIA_H_
#define SRC_MANEJOMEMORIA_H_

#include "estructurasCoMAnda.h"
#include "comanda.h"

//cosas que necesita coMAnda para trabajar
void* MEMORIA_PRINCIPAL;
void* AREA_DE_SWAP;

tablas_segmentos_restaurantes* lista_de_pedidos_de_todos_los_restaurantes; //aca empiezan las lista de segmentos que tiene CoMAnda para cada restaurante
//segmentos* tabla_de_segmentos; esto me parece que vuela a la mierda

//funciones de memoria***********************************************************************
//esta porqueria lo que hace es la inicializacion original para tener tablas de pedidos para cda restaurante
void inicializar_lista_de_tablas_de_segmentos_de_restaurantes(tablas_segmentos_restaurantes* listas_de_pedidos);

void inicializar_tabla_de_segmentos(segmentos* laTablaDeSegmentos);

//devuelve la lista de pedidos del restaurante solicitado, si no existe, la crea
tablas_segmentos_restaurantes* selector_de_tabla_de_pedidos(tablas_segmentos_restaurantes* lasListasDePedidosDeRestaurantes, char* nombreDeRestaurante);
//busca si ya existe la lista de pedidos de X restaurante
uint32_t buscar_tabla_de_segmentos_de_restaurante(tablas_segmentos_restaurantes* lasListasDePedidosDeRestaurantes, char* nombreDeRestaurante);
//crea una nueva tabla y devuelve puntero a la tavla creada
tablas_segmentos_restaurantes* crear_tabla_de_pedidos(tablas_segmentos_restaurantes* lasListasDePedidosDeRestaurantes, char* nombreDeRestaurante);


uint32_t buscar_segmento_de_restaurante(segmentos* laTablaDeSegmentos, char* nombreRestaurante);
uint32_t crearSegmento(tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante);

void agregarPedidoARestaurante(tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante, uint32_t numeroDeSegmento, uint32_t idPedido);


#endif /* SRC_MANEJOMEMORIA_H_ */
