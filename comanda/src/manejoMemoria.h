#ifndef SRC_MANEJOMEMORIA_H_
#define SRC_MANEJOMEMORIA_H_

#include "estructurasCoMAnda.h"
#include "comanda.h"

//cosas que necesita coMAnda para trabajar
void* MEMORIA_PRINCIPAL;
void* AREA_DE_SWAP;

tablas_segmentos_restaurantes* lista_de_pedidos_de_todos_los_restaurantes; //aca empiezan las lista de segmentos que tiene CoMAnda para cada restaurante
//segmentos* tabla_de_segmentos; esto me parece que vuela a la mierda

//funciones de memoria****************************************************************************************************************************
//esta porqueria lo que hace es la inicializacion original para tener tablas de pedidos para cda restaurante
void inicializar_lista_de_tablas_de_segmentos_de_restaurantes(tablas_segmentos_restaurantes* listas_de_pedidos);
void inicializar_tabla_de_segmentos(segmentos* laTablaDeSegmentos);
void inicializar_tabla_de_paginas(tabla_paginas* laTablaDePaginas);

//devuelve la lista de pedidos del restaurante solicitado, si no existe, la crea
tablas_segmentos_restaurantes* selector_de_tabla_de_pedidos(tablas_segmentos_restaurantes* lasListasDePedidosDeRestaurantes, char* nombreDeRestaurante, uint32_t negarCreacion);
//busca si ya existe la lista de pedidos de X restaurante
uint32_t buscar_tabla_de_segmentos_de_restaurante(tablas_segmentos_restaurantes* lasListasDePedidosDeRestaurantes, char* nombreDeRestaurante);
//crea una nueva tabla y devuelve puntero a la tabla creada
tablas_segmentos_restaurantes* crear_tabla_de_pedidos(tablas_segmentos_restaurantes* lasListasDePedidosDeRestaurantes, char* nombreDeRestaurante);
//para ver si ya existe el pedido, y devolver FAIL, o si no (Existe = 1, no existe = 0)
uint32_t verificarExistenciaDePedido (tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante, uint32_t idDelPedido);
//esto crea un nuevo pedido (segumento = pedido)
uint32_t crearSegmento(tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante, uint32_t idDelPedido);



uint32_t buscar_segmento_de_pedido(tablas_segmentos_restaurantes* laTablaDeSegmentos, uint32_t idDelPedido);

//busco si existe el plato con el nombre provisto en la lista de platos del pedido seleccionado (Existe = 1, no existe = 0)
uint32_t verificarExistenciaDePlato(segmentos* segmentoSeleccionado, char* nombrePlato);
//esto agrega un plato a un pedido (segmento) ya existente
void agregarPlatoARestaurante(tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante, uint32_t numeroDeSegmento);


#endif /* SRC_MANEJOMEMORIA_H_ */
