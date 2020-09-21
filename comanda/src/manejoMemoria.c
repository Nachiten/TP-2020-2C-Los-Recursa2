#include "manejoMemoria.h"

void inicializar_lista_de_tablas_de_segmentos_de_restaurantes(tablas_segmentos_restaurantes* listas_de_pedidos)
{
	listas_de_pedidos->nombreRestaurante = "";//chequear si se mantiene asi o si hace agua
	listas_de_pedidos->cantidadDeSegmentos = 1;
	listas_de_pedidos->miTablaDePedidos = malloc(sizeof(segmentos));
	inicializar_tabla_de_segmentos(listas_de_pedidos->miTablaDePedidos);
	listas_de_pedidos->anter_lista = NULL;
	listas_de_pedidos->sig_lista = NULL;
}

void inicializar_tabla_de_segmentos(segmentos* laTablaDeSegmentos)
{
	laTablaDeSegmentos->numero_de_segmento = 0;
	laTablaDeSegmentos->mi_tabla = malloc(sizeof(tabla_paginas));
	inicializar_tabla_de_paginas(laTablaDeSegmentos->mi_tabla);
	laTablaDeSegmentos->anter_segmento = NULL;
	laTablaDeSegmentos->sig_segmento = NULL;
}

void inicializar_tabla_de_paginas(tabla_paginas* laTablaDePaginas)
{
	laTablaDePaginas->numero_de_pagina = 0;
	laTablaDePaginas->numero_de_victima = 0;
	laTablaDePaginas->cantidadPedidaComida = 0;
	laTablaDePaginas->cantidadComidaPreparada = 0;
	laTablaDePaginas->nombreDeMorfi = malloc(24);
	laTablaDePaginas->anter_pagina = NULL;
	laTablaDePaginas->sig_pagina = NULL;
}

tablas_segmentos_restaurantes* selector_de_tabla_de_pedidos(tablas_segmentos_restaurantes* lasListasDePedidosDeRestaurantes, char* nombreDeRestaurante, uint32_t negarCreacion)
{
	tablas_segmentos_restaurantes* tablaDePedidosSeleccionada = NULL;
	uint32_t tablaDefault = 0;

	//chequeamos si solo tenemos la tabla de pedidos por default
	if(lasListasDePedidosDeRestaurantes->sig_lista == NULL && strcmp(lasListasDePedidosDeRestaurantes->nombreRestaurante,"") == 0)
	{
		tablaDefault = 1;//se activa el comportamiento para la primera lista de pedidos
	}

	//antes que nada, vemos si ya existe una tabla de pedidos de ese restaurante, o si es nuevo
	if(tablaDefault == 0 && buscar_tabla_de_segmentos_de_restaurante(lasListasDePedidosDeRestaurantes, nombreDeRestaurante))
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
		if(negarCreacion == 0) //se crea si Y SOLO SI el flag esta en 0 (no se pueden crear tablas de pedidos a menos que autorice la consigna)
		{
			//si tengo la tabla DEFAULT disponible
			if(tablaDefault == 1)
			{
				//simplemente le asigno el nombre del restaurante nuevo a la lista de pedidos Default
				lasListasDePedidosDeRestaurantes->nombreRestaurante = malloc(strlen(nombreDeRestaurante)+1);
				memcpy(lasListasDePedidosDeRestaurantes->nombreRestaurante, nombreDeRestaurante, strlen(nombreDeRestaurante)+1);

				tablaDePedidosSeleccionada = lasListasDePedidosDeRestaurantes; //la lista de pedidos por default es la seleccionada
			}

			//si o si tengo que crearle una tabla nueva
			else
			{
				//devuelvo la lista de pedidos creada
				tablaDePedidosSeleccionada = crear_tabla_de_pedidos(lasListasDePedidosDeRestaurantes, nombreDeRestaurante);
			}
		}
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
	tablas_segmentos_restaurantes* nuevaTablaDePedidos = malloc(sizeof(tablas_segmentos_restaurantes));

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

//ToDo volver a revisar esto cuando tenga que agregar platos a un pedido
uint32_t buscar_segmento_de_pedido(tablas_segmentos_restaurantes* laTablaDeSegmentos, uint32_t idDelPedido)
{
	segmentos* tablaDePedidos = laTablaDeSegmentos->miTablaDePedidos;
	int encontrado = 0;
	uint32_t numeroSegmentoBuscado = 0;

	//mientras que no haya llegado al final de la tabla de segmentos y no haya encontrado el segmento que busco...
	while(tablaDePedidos != NULL && encontrado == 0)
	{
		//comparo si encontré el segmento del restaurante que busco
		if(tablaDePedidos->id_Pedido == idDelPedido)
		{
			numeroSegmentoBuscado = tablaDePedidos->numero_de_segmento;
			encontrado = 1;
		}

		//avanzo al siguiente segmento
		tablaDePedidos = tablaDePedidos->sig_segmento;
	}

	//si no encontre el segmento, tengo que crearlo, si lo encontre, solo se devuelve
	if(encontrado == 0)
	{
		numeroSegmentoBuscado = crearSegmento(tablaDePedidos, idDelPedido);
	}

	return numeroSegmentoBuscado;
}

uint32_t verificarExistenciaDePedido (tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante, uint32_t idDelPedido)
{
	segmentos* tablaDePedidos = tablaDePedidosDelRestaurante->miTablaDePedidos;
	uint32_t existe = 0;

	while(tablaDePedidos != NULL)
	{
		if(tablaDePedidos->id_Pedido == idDelPedido)
		{
			existe = 1;
		}
		tablaDePedidos = tablaDePedidos->sig_segmento;
	}

	return existe;
}

//ToDo verificar si hace falta saber el numero de segmento creado
uint32_t crearSegmento(tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante, uint32_t idDelPedido)
{
	segmentos* tablaDePedidos = tablaDePedidosDelRestaurante->miTablaDePedidos;
	segmentos* ultimoSegmento = NULL;

	//si el primer segmento es el que cree por default, hay que usar ese
	if(tablaDePedidos->id_Pedido == 0)
	{
		tablaDePedidos->id_Pedido = idDelPedido;
		return tablaDePedidos->numero_de_segmento;
	}

	//si no, se crea un nuevo segmento
	else
	{
		segmentos* nuevoSegmento = malloc(sizeof(segmentos));

		tablaDePedidosDelRestaurante->cantidadDeSegmentos++; //como creo un nuevo segmento, le sumo 1

		//mientras no haya llegado al final de la tabla de Segmentos
		while(tablaDePedidos->sig_segmento != NULL)
		{
			//avanzo...
			tablaDePedidos = tablaDePedidos->sig_segmento;
		}
		//llegue al ultimo
		ultimoSegmento = tablaDePedidos;

		//"inicializo" el nuevo segmento
		ultimoSegmento->sig_segmento = nuevoSegmento; //lo unimos al final de la tabla

		nuevoSegmento->id_Pedido = idDelPedido;
		nuevoSegmento->anter_segmento = ultimoSegmento;
		nuevoSegmento->sig_segmento = NULL;
		nuevoSegmento->numero_de_segmento = ultimoSegmento->numero_de_segmento + 1;
		nuevoSegmento->mi_tabla = malloc(sizeof(tabla_paginas));
		inicializar_tabla_de_paginas(nuevoSegmento->mi_tabla);

		//devuelvo el numero del segmento creado
		return nuevoSegmento->numero_de_segmento;
	}
}

uint32_t verificarExistenciaDePlato(segmentos* segmentoSeleccionado, char* nombrePlato)
{
	tabla_paginas* tablaDePlatos = segmentoSeleccionado->mi_tabla;
	uint32_t existe = 0;

	//avanzo en la tabla de paginas a ver si existe una de el plato solicitado
	while(tablaDePlatos != NULL)
	{
		//comparo nombres de platos
		if(strcmp(tablaDePlatos->nombreDeMorfi,nombrePlato) == 0)
		{
			existe = 1;
		}
		tablaDePlatos = tablaDePlatos->sig_pagina;
	}

	return existe;
}

void agregarPlatoARestaurante(tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante, uint32_t numeroDeSegmento, char* nombrePlato)
{
	segmentos* laTablaDeSegmentos = tablaDePedidosDelRestaurante->miTablaDePedidos;
	segmentos* segmentoSeleccionado = NULL;

	//recorro la lista hasta que encuentre el numero del segmento seleccionado
	while(laTablaDeSegmentos->numero_de_segmento != numeroDeSegmento)
	{
		laTablaDeSegmentos = laTablaDeSegmentos->sig_segmento;
	}

	segmentoSeleccionado = laTablaDeSegmentos;

	//vemos si existe el plato en el pedido
	if(verificarExistenciaDePlato(segmentoSeleccionado, nombrePlato))
	{

	}

	//es un plato nuevo que agregar al pedido ToDo terminar
	else
	{

	}
	//toDo  terminar cuando inicialice tabla de paginas
	//segmentoSeleccionado->mi_tabla->


}





