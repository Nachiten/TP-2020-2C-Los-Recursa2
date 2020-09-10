#include "manejoMemoria.h"

void inicializar_lista_de_tablas_de_segmentos_de_restaurantes(tablas_segmentos_restaurantes* listas_de_pedidos)
{
	listas_de_pedidos->nombreRestaurante = "";
	listas_de_pedidos->miTablaDePedidos = malloc(sizeof(segmentos));
	inicializar_tabla_de_segmentos(listas_de_pedidos->miTablaDePedidos);
	listas_de_pedidos->anter_lista = NULL;
	listas_de_pedidos->sig_lista = NULL;
}

void inicializar_tabla_de_segmentos(segmentos* laTablaDeSegmentos)
{
	laTablaDeSegmentos->numero_de_segmento = 0;
	laTablaDeSegmentos->mi_tabla = malloc(sizeof(tabla_paginas));
	laTablaDeSegmentos->anter_segmento = NULL;
	laTablaDeSegmentos->sig_segmento = NULL;
}

tablas_segmentos_restaurantes* selector_de_tabla_de_pedidos(tablas_segmentos_restaurantes* lasListasDePedidosDeRestaurantes, char* nombreDeRestaurante)
{
	tablas_segmentos_restaurantes* tablaDePedidosSeleccionada;

	//antes que nada, vemos si ya existe una tabla de pedidos de ese restaurante, o si es nuevo
	if(buscar_tabla_de_segmentos_de_restaurante(lasListasDePedidosDeRestaurantes, nombreDeRestaurante))
	{
		//la lista de pedidos de ese restaurante ya existe

		while(lasListasDePedidosDeRestaurantes->nombreRestaurante != nombreDeRestaurante)
		{
			//me busco la lista de ese restaurante
			lasListasDePedidosDeRestaurantes = lasListasDePedidosDeRestaurantes->sig_lista;
		}

		tablaDePedidosSeleccionada = lasListasDePedidosDeRestaurantes; //lista de pedidos encontrada, devuelvo esto
	}

	else //es un restaurante nuevo, hay que crearle una lista de pedidos
	{
		//devuelvo la lista de pedidos creada
		tablaDePedidosSeleccionada = crear_tabla_de_pedidos(lasListasDePedidosDeRestaurantes, nombreDeRestaurante);
	}

	return tablaDePedidosSeleccionada;
}

uint32_t buscar_tabla_de_segmentos_de_restaurante(tablas_segmentos_restaurantes* lasListasDePedidosDeRestaurantes, char* nombreDeRestaurante)
{
	int encontrado = 0;

	//mientras que no haya llegado al final de la lista de pedidos de ese restaurante...
	while(lasListasDePedidosDeRestaurantes != NULL && encontrado == 0)
	{
		//comparo si encontré el segmento del restaurante que busco
		if(lasListasDePedidosDeRestaurantes->nombreRestaurante == nombreDeRestaurante)
		{
			encontrado = 1;
		}

		//avanzo al siguiente segmento
		lasListasDePedidosDeRestaurantes = lasListasDePedidosDeRestaurantes->sig_lista;
	}

	return encontrado;
}

tablas_segmentos_restaurantes* crear_tabla_de_pedidos(tablas_segmentos_restaurantes* lasListasDePedidosDeRestaurantes, char* nombreDeRestaurante)
{
	tablas_segmentos_restaurantes* nuevaTablaDePedidos;

	//tengo que agregar una nueva lista de pedidos al final de todas
	while(lasListasDePedidosDeRestaurantes->sig_lista != NULL)
	{
		lasListasDePedidosDeRestaurantes = lasListasDePedidosDeRestaurantes->sig_lista;
	}

	lasListasDePedidosDeRestaurantes->sig_lista = nuevaTablaDePedidos;

	//me copio el nombre del restaurante
	nuevaTablaDePedidos->nombreRestaurante = malloc(strlen(nombreDeRestaurante)+1);
	memcpy(nuevaTablaDePedidos->nombreRestaurante, nombreDeRestaurante, strlen(nombreDeRestaurante)+1);
	nuevaTablaDePedidos->cantidadDeSegmentos = 0;
	//inicializo la tabla de pedidos de este restaurante
	nuevaTablaDePedidos->miTablaDePedidos = malloc(sizeof(segmentos));
	inicializar_tabla_de_segmentos(nuevaTablaDePedidos->miTablaDePedidos);
	nuevaTablaDePedidos->anter_lista = lasListasDePedidosDeRestaurantes;
	nuevaTablaDePedidos->sig_lista = NULL;

	//devuelvo la tabla creada
	return nuevaTablaDePedidos;
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

uint32_t crearSegmento(tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante)//ToDo falta completar, problemas con null y numero de segmento
{
	segmentos* laTablaDeSegmentos = tablaDePedidosDelRestaurante->miTablaDePedidos;
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

	if(ultimoSegmento == NULL)
	{
		nuevoSegmento->numero_de_segmento = 0;
	}

	nuevoSegmento->numero_de_segmento = ultimoSegmento->numero_de_segmento + 1;

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





