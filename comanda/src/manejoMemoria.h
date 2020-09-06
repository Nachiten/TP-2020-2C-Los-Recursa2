#ifndef SRC_MANEJOMEMORIA_H_
#define SRC_MANEJOMEMORIA_H_

#include "estructurasCoMAnda.h"
#include "comanda.h"

//cosas que necesita coMAnda para trabajar
void* MEMORIA_PRINCIPAL;
void* AREA_DE_SWAP;

segmentos* tabla_de_segmentos; //aca empieza la lista de segmentos que tiene CoMAnda

//funciones de memoria***********************************************************************
void inicializar_tabla_de_segmentos(segmentos* laTablaDeSegmentos);
uint32_t buscar_segmento_de_restaurante(segmentos* laTablaDeSegmentos, char* nombreRestaurante);
uint32_t crearSegmento(segmentos* laTablaDeSegmentos, char* nombreDeRestaurante);

void agregarPedidoARestaurante(segmentos* laTablaDeSegmentos, uint32_t numeroDeSegmento, uint32_t idPedido);


#endif /* SRC_MANEJOMEMORIA_H_ */
