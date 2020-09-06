#include "manejoMemoria.h"

void inicializar_tabla_de_segmentos(segmentos* laTablaDeSegmentos)
{
	laTablaDeSegmentos->numero_de_segmento = 0;
	laTablaDeSegmentos->nombreRestaurante = "";
	laTablaDeSegmentos->mi_tabla = malloc(sizeof(tabla_paginas));
	laTablaDeSegmentos->anter_segmento = NULL;
	laTablaDeSegmentos->sig_segmento = NULL;
}

uint32_t buscar_segmento_de_restaurante(segmentos* laTablaDeSegmentos, char* nombreDeRestaurante)
{
	int encontrado = 0;
	uint32_t numeroSegmentoBuscado = 0;

	//mientras que no haya llegado al final de la tabla de segmentos y no haya encontrado el segmento que busco...
	while(laTablaDeSegmentos != NULL && encontrado == 0)
	{
		//comparo si encontré el segmento del restaurante que busco
		if(laTablaDeSegmentos->nombreRestaurante == nombreDeRestaurante)
		{
			numeroSegmentoBuscado = laTablaDeSegmentos->numero_de_segmento;
			encontrado = 1;
		}

		//avanzo al siguiente segmento
		laTablaDeSegmentos = laTablaDeSegmentos->sig_segmento;
	}

	//si no encontre el segmento, tengo que crearlo, si lo encontre, solo se devuelve
	if(encontrado == 0)
	{
		numeroSegmentoBuscado = crearSegmento(laTablaDeSegmentos, nombreDeRestaurante);
	}

	return numeroSegmentoBuscado;
}

uint32_t crearSegmento(segmentos* laTablaDeSegmentos, char* nombreDeRestaurante)
{
	segmentos* ultimoSegmento = NULL;
	segmentos* nuevoSegmento = malloc(sizeof(segmentos));

	//mientras no haya llegado al final de la tabla de Segmentos
	while(laTablaDeSegmentos->sig_segmento != NULL)
	{
		//avanzo...
		laTablaDeSegmentos = laTablaDeSegmentos->sig_segmento;
	}
	//llegue al ultimo
	ultimoSegmento = laTablaDeSegmentos;

	//"inicializo" el nuevo segmento
	ultimoSegmento->sig_segmento = nuevoSegmento; //lo unimos al final de la tabla

	nuevoSegmento->anter_segmento = ultimoSegmento;
	nuevoSegmento->sig_segmento = NULL;
	nuevoSegmento->numero_de_segmento = ultimoSegmento->numero_de_segmento + 1;
	nuevoSegmento->nombreRestaurante = malloc(strlen(nombreDeRestaurante)+1); //ToDo consultar este malloc
	nuevoSegmento->nombreRestaurante = nombreDeRestaurante;
	nuevoSegmento->mi_tabla = malloc(sizeof(tabla_paginas));
	//toDo inicializar la tabla de paginas

	//devuelvo el numero del segmento creado
	return nuevoSegmento->numero_de_segmento;
}

void agregarPedidoARestaurante(segmentos* laTablaDeSegmentos, uint32_t numeroDeSegmento, uint32_t idPedido)
{
	segmentos* segmentoSeleccionado = NULL;

	//recorro la lista hasta que encuentre el numero del segmento seleccionado
	while(laTablaDeSegmentos->numero_de_segmento != numeroDeSegmento)
	{
		laTablaDeSegmentos = laTablaDeSegmentos->sig_segmento;
	}

	segmentoSeleccionado = laTablaDeSegmentos;

	//toDo  terminar cuando inicialice tabla de paginas
	//segmentoSeleccionado->mi_tabla->


}





