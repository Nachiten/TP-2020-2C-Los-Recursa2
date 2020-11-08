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
	laTablaDeSegmentos->id_Pedido = 0;
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
	laTablaDePaginas->nombreDeMorfi = malloc(24); //harcodeado 24 para cumplir con enunciado
	laTablaDePaginas->cargadoEnSWAP = 0;
	laTablaDePaginas->posicionInicialEnSWAP = -1;
	laTablaDePaginas->cargadoEnMEMORIA = 0;
	laTablaDePaginas->numeroDeMarco = -1;
	laTablaDePaginas->anter_pagina = NULL;
	laTablaDePaginas->sig_pagina = NULL;
}

void inicializar_lista_de_espacios(espacio* listaDeEspacios, uint32_t TAMANIO_AREA)
{
	espacio* auxiliarMoverme = listaDeEspacios;
	uint32_t cantidadPosibleDeEspacios = TAMANIO_AREA/32; //necesito saber cuantas paginas/marcos (y por ende espacios) voy a poder tener en SWAP/MEMORIA PPAL
	uint32_t iterador = 0;

	//inicializamos el primer espacio
	listaDeEspacios->numeroDeEspacio = 0;
	listaDeEspacios->espacioOcupado = 0;
	listaDeEspacios->anter_espacio = NULL;
	listaDeEspacios->sig_espacio = NULL;

	while(iterador < cantidadPosibleDeEspacios-1)
	{
		crearNuevoEspacio(auxiliarMoverme);
		auxiliarMoverme = auxiliarMoverme->sig_espacio;
		iterador++;
	}
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

tabla_paginas* crearPagina(tabla_paginas* tablaDePlatosDelPedido, char* nombrePlato, uint32_t cantidadPlatos)
{
	tabla_paginas* auxiliarRecorrer = tablaDePlatosDelPedido;
	tabla_paginas* nuevoPlato = malloc(sizeof(tabla_paginas));

	//me paro al final de la tabla de ̶P̶l̶a̶t̶os Paginas para asignar una nueva
	while(auxiliarRecorrer->sig_pagina != NULL)
	{
		auxiliarRecorrer = auxiliarRecorrer->sig_pagina;
	}

	//si vamos a usar la pagina por default que tiene la tabla de paginas
	if(auxiliarRecorrer->numero_de_pagina == 0)
	{
		//antes que nada mato la pagina nueva que se creó al pedo
		free(nuevoPlato);

		auxiliarRecorrer->cantidadComidaPreparada = 0;
		auxiliarRecorrer->cantidadPedidaComida = cantidadPlatos;
		auxiliarRecorrer->largoPostaDeMorfi = strlen(nombrePlato);
		memcpy(auxiliarRecorrer->nombreDeMorfi, nombrePlato, strlen(nombrePlato)+1);
		auxiliarRecorrer->numero_de_pagina++;

		return auxiliarRecorrer;
	}

	//le armamos una pagina nueva
	else
	{
		nuevoPlato->cantidadComidaPreparada = 0;
		nuevoPlato->cantidadPedidaComida = cantidadPlatos;
		nuevoPlato->nombreDeMorfi = malloc(24); //harcodeado 24 para cumplir con enunciado
		nuevoPlato->largoPostaDeMorfi = strlen(nombrePlato);
		memcpy(nuevoPlato->nombreDeMorfi, nombrePlato, strlen(nombrePlato)+1);
		nuevoPlato->numero_de_pagina = auxiliarRecorrer->numero_de_pagina + 1;
		nuevoPlato->anter_pagina = auxiliarRecorrer;
		nuevoPlato->sig_pagina = NULL;
		nuevoPlato->cargadoEnSWAP = 0;
		nuevoPlato->posicionInicialEnSWAP = -1;
		nuevoPlato->cargadoEnMEMORIA = 0;
		nuevoPlato->numeroDeMarco = -1;

		//"pego" el nuevo plato al final de la lista de platos
		auxiliarRecorrer->sig_pagina = nuevoPlato;

		return nuevoPlato;
	}
}

void crearNuevoEspacio(espacio* unEspacio)
{
	espacio* auxiliarEspacioAnterior = unEspacio;
	espacio* nuevoEspacio = malloc(sizeof(espacio));

	//uno el nuevo espacio
	auxiliarEspacioAnterior->sig_espacio = nuevoEspacio;

	//preparo el espacio nuevo
	nuevoEspacio->numeroDeEspacio = auxiliarEspacioAnterior->numeroDeEspacio + 1;
	nuevoEspacio->espacioOcupado = 0;
	nuevoEspacio->anter_espacio = auxiliarEspacioAnterior;
	nuevoEspacio->sig_espacio = NULL;
}

int32_t buscarPrimerEspacioLibre(espacio* listaDeEspacios)
{
	espacio* auxiliarMoverme = listaDeEspacios;
	int32_t espacioEncontrado = -1;

	//mientras que no encuentre un espacio ocupado...
	while(auxiliarMoverme != NULL && espacioEncontrado == -1)
	{
		if(auxiliarMoverme->espacioOcupado == 0)
		{
			//encontre un espacio, devuelvo cual es
			espacioEncontrado = auxiliarMoverme->numeroDeEspacio;
		}

		//no encontre un espacio libre, avanzo
		else
		{
			auxiliarMoverme = auxiliarMoverme->sig_espacio;
		}
	}

	//devuelvo el numero del espacio encontrado (o no)
	return espacioEncontrado;
}

void marcarEspacioComoOcupado(espacio* listaDeEspacios, uint32_t numeroDeEspacioElegido)
{
	espacio* auxiliarMoverme = listaDeEspacios;

	//mientras que no encuentre el espacio pedido...
	while(auxiliarMoverme != NULL && auxiliarMoverme->numeroDeEspacio != numeroDeEspacioElegido)
	{
		//avanzo hasta encontrarlo
		auxiliarMoverme = auxiliarMoverme->sig_espacio;
	}

	//ya lo encontre, marco como ocupado
	auxiliarMoverme->espacioOcupado = 1;
}

void marcarEspacioComoLibre(espacio* listaDeEspacios, uint32_t numeroDeEspacioElegido)
{
	espacio* auxiliarMoverme = listaDeEspacios;

	//mientras que no encuentre el espacio pedido...
	while(auxiliarMoverme != NULL && auxiliarMoverme->numeroDeEspacio != numeroDeEspacioElegido)
	{
		//avanzo hasta encontrarlo
		auxiliarMoverme = auxiliarMoverme->sig_espacio;
	}

	//ya lo encontre, marco como libre
	auxiliarMoverme->espacioOcupado = 0;
}

void asignarNumeroDeVictima(uint32_t* miNumeroDeVictima)
{
	sem_wait(semaforoNumeroVictima);
	*miNumeroDeVictima = numero_de_victima;
	numero_de_victima++;
	sem_post(semaforoNumeroVictima);
}

tablas_segmentos_restaurantes* selector_de_tabla_de_pedidos(tablas_segmentos_restaurantes* lasListasDePedidosDeRestaurantes, char* nombreDeRestaurante, uint32_t negarCreacion)
{
	tablas_segmentos_restaurantes* auxiliarRecorrerListasDePedidos = lasListasDePedidosDeRestaurantes;
	tablas_segmentos_restaurantes* tablaDePedidosSeleccionada = NULL;
	uint32_t tablaDefault = 0;

	//chequeamos si solo tenemos la tabla de pedidos por default
	if(auxiliarRecorrerListasDePedidos->sig_lista == NULL && strcmp(auxiliarRecorrerListasDePedidos->nombreRestaurante,"") == 0)
	{
		tablaDefault = 1;//se activa el comportamiento para la primera lista de pedidos
	}

	//antes que nada, vemos si ya existe una tabla de pedidos de ese restaurante, o si es nuevo
	if(tablaDefault == 0 && buscar_tabla_de_segmentos_de_restaurante(auxiliarRecorrerListasDePedidos, nombreDeRestaurante))
	{
		//la lista de pedidos de ese restaurante ya existe
		while(strcmp(auxiliarRecorrerListasDePedidos->nombreRestaurante,nombreDeRestaurante) != 0)
		{
			//me busco la lista de ese restaurante
			auxiliarRecorrerListasDePedidos = auxiliarRecorrerListasDePedidos->sig_lista;
		}

		tablaDePedidosSeleccionada = auxiliarRecorrerListasDePedidos; //lista de pedidos encontrada, devuelvo esto
	}

	else //es un restaurante nuevo, hay que crearle una lista de pedidos
	{
		if(negarCreacion == 0) //se crea si Y SOLO SI el flag esta en 0 (no se pueden crear tablas de pedidos a menos que autorice la consigna)
		{
			//si tengo la tabla DEFAULT disponible
			if(tablaDefault == 1)
			{
				//simplemente le asigno el nombre del restaurante nuevo a la lista de pedidos Default
				auxiliarRecorrerListasDePedidos->nombreRestaurante = malloc(strlen(nombreDeRestaurante)+1);
				memcpy(auxiliarRecorrerListasDePedidos->nombreRestaurante, nombreDeRestaurante, strlen(nombreDeRestaurante)+1);

				tablaDePedidosSeleccionada = auxiliarRecorrerListasDePedidos; //la lista de pedidos por default es la seleccionada
			}

			//si o si tengo que crearle una tabla nueva
			else
			{
				//devuelvo la lista de pedidos creada
				tablaDePedidosSeleccionada = crear_tabla_de_pedidos(auxiliarRecorrerListasDePedidos, nombreDeRestaurante);
			}
		}
	}

	return tablaDePedidosSeleccionada;
}

segmentos* selectordePedidoDeRestaurante(tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante, uint32_t numeroDeSegmento)
{
	segmentos* laTablaDeSegmentos = tablaDePedidosDelRestaurante->miTablaDePedidos;
	segmentos* segmentoSeleccionado = NULL;

	//recorro la lista hasta que encuentre el numero del segmento seleccionado
	while(laTablaDeSegmentos->numero_de_segmento != numeroDeSegmento)
	{
		laTablaDeSegmentos = laTablaDeSegmentos->sig_segmento;
	}

	segmentoSeleccionado = laTablaDeSegmentos;

	return segmentoSeleccionado;
}

uint32_t buscar_tabla_de_segmentos_de_restaurante(tablas_segmentos_restaurantes* lasListasDePedidosDeRestaurantes, char* nombreDeRestaurante)
{
	int encontrado = 0;

	//mientras que no haya llegado al final de la lista de pedidos de ese restaurante...
	while(lasListasDePedidosDeRestaurantes != NULL && encontrado == 0)
	{
		//comparo si encontré el segmento del restaurante que busco
		if(strcmp(lasListasDePedidosDeRestaurantes->nombreRestaurante,nombreDeRestaurante) == 0)
		{
			encontrado = 1;
		}

		//avanzo al siguiente segmento
		lasListasDePedidosDeRestaurantes = lasListasDePedidosDeRestaurantes->sig_lista;
	}

	return encontrado;
}

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
		numeroSegmentoBuscado = crearSegmento(laTablaDeSegmentos, idDelPedido);
	}

	return numeroSegmentoBuscado;
}

void cargarPaginasEnMP(tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante, uint32_t numeroDeSegmento)
{
	segmentos* segmentoSeleccionado = selectordePedidoDeRestaurante(tablaDePedidosDelRestaurante, numeroDeSegmento);
	tabla_paginas* tablaDePlatos = segmentoSeleccionado->mi_tabla;
	int32_t numeroDeMarcoEnMP = -10;

	//ahora que tengo el pedido seleccionado, tengo que cargar 1 por 1, todas las paginas en MP si no lo estan
	//avanzo en la lista de platos hasta que llegue al final
	while(tablaDePlatos != NULL)
	{
		//si ya esta cargada en MP, la ignoro, solo me importan las que no esten cargadas
		if(tablaDePlatos->cargadoEnMEMORIA == 0)
		{
			//busco si hay un marco libre en MP para poner la pagina
			sem_wait(semaforoTocarListaEspaciosEnMP);
			numeroDeMarcoEnMP = buscarPrimerEspacioLibre(lista_de_espacios_en_MP);
			//una vez seleccionado lo marco como ocupado para que ningun otro hilo lo quiera usar
			if(numeroDeMarcoEnMP != -1)
			{
				marcarEspacioComoOcupado(lista_de_espacios_en_MP, numeroDeMarcoEnMP);
			}
			sem_post(semaforoTocarListaEspaciosEnMP);

			//si no hay un espacio libre hay que llamar al grim reaper
			if(numeroDeMarcoEnMP == -1)
			{
				sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);
				sem_wait(semaforoTocarListaEspaciosEnMP);
				algoritmo_de_reemplazo(ALGOR_REEMPLAZO, lista_de_pedidos_de_todos_los_restaurantes, lista_de_espacios_en_MP);
				numeroDeMarcoEnMP = buscarPrimerEspacioLibre(lista_de_espacios_en_MP);
				marcarEspacioComoOcupado(lista_de_espacios_en_MP, numeroDeMarcoEnMP);
				sem_post(semaforoTocarListaEspaciosEnMP);
				sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);
			}

			//una vez tengo un marco listo para poner la Página, pongo los datos en MP
			mover_pagina_a_memoriaPrincipal(tablaDePlatos, tablaDePlatos->posicionInicialEnSWAP, numeroDeMarcoEnMP*32);
		}
		//avanzo...
		tablaDePlatos = tablaDePlatos->sig_pagina;
	}
}

void actualizarTodosLosPlatosConDatosDeMP(tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante, uint32_t numeroDeSegmento)
{
	segmentos* segmentoSeleccionado = selectordePedidoDeRestaurante(tablaDePedidosDelRestaurante, numeroDeSegmento);
	tabla_paginas* tablaDePlatos = segmentoSeleccionado->mi_tabla;

	//avanzo en la lista de platos hasta que llegue al final
	while(tablaDePlatos != NULL)
	{
		tomar_datos_de_MP(tablaDePlatos);

		//avanzo...
		tablaDePlatos = tablaDePlatos->sig_pagina;
	}
}

//este guacho ya tiene los semaforos dentro de la propia funcion!!!
uint32_t verificarExistenciaDePedido (tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante, uint32_t idDelPedido)
{
	sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);
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
	sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);

	return existe;
}

uint32_t verificarExistenciaDePlato(segmentos* segmentoSeleccionado, char* nombrePlato)//ToDo probare
{
	tabla_paginas* tablaDePlatos = segmentoSeleccionado->mi_tabla;
	uint32_t existe = 0;

	//avanzo en la tabla de paginas a ver si existe una de el plato solicitado
	while(tablaDePlatos != NULL)
	{
		//si la pagina esta en MP, me traigo sus datos de ahi
		if(tablaDePlatos->cargadoEnMEMORIA == 1)
		{
			tomar_datos_de_MP(tablaDePlatos);
		}

		//si no esta en MP, los tengo que buscar de SWAP
		else
		{
			tomar_datos_de_SWAP(tablaDePlatos);
		}

		//una vez tengo los datos, comparo nombres de platos a ver si ya existe
		if(strcmp(tablaDePlatos->nombreDeMorfi,nombrePlato) == 0)
		{
			existe = 1;
		}

		//avanzo...
		tablaDePlatos = tablaDePlatos->sig_pagina;
	}

	//a lo ultimo, vuelvo a censurar todos los datos de los platos del pedido
	sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);
	borrar_datos_de_todos_los_platos_del_pedido(segmentoSeleccionado);
	sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);

	//devuelvo el resultado
	return existe;
}

tabla_paginas* agregarPlatoAPedido(tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante, uint32_t numeroDeSegmento, char* nombrePlato, uint32_t cantidadPlatos)
{
	segmentos* laTablaDeSegmentos = tablaDePedidosDelRestaurante->miTablaDePedidos;
	segmentos* segmentoSeleccionado = NULL;
	tabla_paginas* tablaDePlatos = NULL;

	//recorro la lista hasta que encuentre el numero del segmento seleccionado
	while(laTablaDeSegmentos->numero_de_segmento != numeroDeSegmento)
	{
		laTablaDeSegmentos = laTablaDeSegmentos->sig_segmento;
	}

	segmentoSeleccionado = laTablaDeSegmentos;
	tablaDePlatos = segmentoSeleccionado->mi_tabla;

	//vemos si existe el plato en el pedido
	if(verificarExistenciaDePlato(segmentoSeleccionado, nombrePlato))
	{
		//mietras que no encuentre el plato con el nombre que me llego
		while(strcmp(tablaDePlatos->nombreDeMorfi,nombrePlato) == 1)
		{
			//avanzo
			tablaDePlatos = tablaDePlatos->sig_pagina;
		}

		//sumo la cantidad de platos nueva que llego
		tablaDePlatos->cantidadPedidaComida += cantidadPlatos;
	}

	//es un plato nuevo que agregar al pedido
	else
	{
		tablaDePlatos = crearPagina(tablaDePlatos, nombrePlato, cantidadPlatos);
	}

	//devuelvo un puntero al plato, ya sea uno nuevo, o el plato al que le sumamos cantidades
	return tablaDePlatos;
}

void agregar_pagina_a_swap(tabla_paginas* tablaDePlatosDelPedido, uint32_t posicionInicial)
{
	uint32_t desplazamiento = posicionInicial;

	sem_wait(semaforoTocarSWAP);

	//pongo en memoria SWAP la cantidad de platos pedidos
	memcpy(AREA_DE_SWAP + desplazamiento, &(tablaDePlatosDelPedido->cantidadPedidaComida), sizeof(tablaDePlatosDelPedido->cantidadPedidaComida));
	desplazamiento += sizeof(tablaDePlatosDelPedido->cantidadPedidaComida);

	//pongo en memoria SWAP la cantidad de platos preparados
	memcpy(AREA_DE_SWAP + desplazamiento, &(tablaDePlatosDelPedido->cantidadComidaPreparada), sizeof(tablaDePlatosDelPedido->cantidadComidaPreparada));
	desplazamiento += sizeof(tablaDePlatosDelPedido->cantidadComidaPreparada);

	//pongo en memoria SWAP el nombre del plato
	memcpy(AREA_DE_SWAP + desplazamiento, tablaDePlatosDelPedido->nombreDeMorfi, strlen(tablaDePlatosDelPedido->nombreDeMorfi)+1);
	desplazamiento += strlen(tablaDePlatosDelPedido->nombreDeMorfi)+1;

	sem_post(semaforoTocarSWAP);

	//actualizamos la pagina
	sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);
	tablaDePlatosDelPedido->cargadoEnSWAP = 1;
	tablaDePlatosDelPedido->posicionInicialEnSWAP = posicionInicial;
	borrar_datos_del_plato(tablaDePlatosDelPedido); //le borro los datos para que solo existan en MP
	sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);
}

void mover_pagina_a_memoriaPrincipal(tabla_paginas* tablaDePlatosDelPedido, uint32_t posicionInicialDeSWAP, uint32_t posicionInicialDeMEMORIA)
{
	//copia de memoria SWAP a memoria principal
	sem_wait(semaforoTocarSWAP);
	sem_wait(semaforoTocarMP);
	memcpy(MEMORIA_PRINCIPAL + posicionInicialDeMEMORIA, AREA_DE_SWAP + posicionInicialDeSWAP, 32);
	sem_post(semaforoTocarMP);
	sem_post(semaforoTocarSWAP);

	//log obligatorio
	sem_wait(semaforoLogger);
	log_info(logger,"Se agregó una nueva pagina a Memoria Principal, posición inicial del Marco: %p", MEMORIA_PRINCIPAL + posicionInicialDeMEMORIA);
	sem_post(semaforoLogger);

	//actualizamos datos de la pagina + le asignamos su numero de victima
	sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);
	asignarNumeroDeVictima(&tablaDePlatosDelPedido->numero_de_victima);
	tablaDePlatosDelPedido->cargadoEnMEMORIA = 1;
	tablaDePlatosDelPedido->numeroDeMarco = posicionInicialDeMEMORIA/32;
	sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);
}

void actualizar_pagina_en_SWAP (tabla_paginas* laPagina)
{
	uint32_t posicionInicialDeMEMORIA = laPagina->numeroDeMarco * 32;
	uint32_t posicionInicialDeSWAP = laPagina->posicionInicialEnSWAP;

	sem_wait(semaforoTocarSWAP);
	sem_wait(semaforoTocarMP);
	memcpy(AREA_DE_SWAP + posicionInicialDeSWAP, MEMORIA_PRINCIPAL + posicionInicialDeMEMORIA, 32);
	sem_post(semaforoTocarMP);
	sem_post(semaforoTocarSWAP);
}

void tomar_datos_de_MP(tabla_paginas* platoDelPedido)
{
	uint32_t desplazamiento = platoDelPedido->numeroDeMarco * 32;

	sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);

	//nos traemos los datos desde MP...
	sem_wait(semaforoTocarMP);

	//obtengo cantidad de platos pedidos
	memcpy(&platoDelPedido->cantidadPedidaComida, MEMORIA_PRINCIPAL + desplazamiento, sizeof(platoDelPedido->cantidadPedidaComida));
	desplazamiento += sizeof(platoDelPedido->cantidadPedidaComida);

	//obtengo cantidad de platos preparados
	memcpy(&platoDelPedido->cantidadComidaPreparada, MEMORIA_PRINCIPAL + desplazamiento, sizeof(platoDelPedido->cantidadComidaPreparada));
	desplazamiento += sizeof(platoDelPedido->cantidadComidaPreparada);

	//por ultimo, obtengo el nombre del plato
	memcpy(platoDelPedido->nombreDeMorfi, MEMORIA_PRINCIPAL + desplazamiento, 24); //harcodeado el 24 para cumplir con la norma del enunciado

	sem_post(semaforoTocarMP);

	//por ultimo hago la correccion de agregarle su \0 al final del nombre posta del morfi
	platoDelPedido->nombreDeMorfi[platoDelPedido->largoPostaDeMorfi] = '\0';

	sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);
}

void tomar_datos_de_SWAP(tabla_paginas* platoDelPedido)
{
	uint32_t desplazamiento = platoDelPedido->posicionInicialEnSWAP * 32;

	sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);

	//nos traemos los datos desde SWAP...
	sem_wait(semaforoTocarSWAP);

	//obtengo cantidad de platos pedidos
	memcpy(&platoDelPedido->cantidadPedidaComida, AREA_DE_SWAP + desplazamiento, sizeof(platoDelPedido->cantidadPedidaComida));
	desplazamiento += sizeof(platoDelPedido->cantidadPedidaComida);

	//obtengo cantidad de platos preparados
	memcpy(&platoDelPedido->cantidadComidaPreparada, AREA_DE_SWAP + desplazamiento, sizeof(platoDelPedido->cantidadComidaPreparada));
	desplazamiento += sizeof(platoDelPedido->cantidadComidaPreparada);

	//por ultimo, obtengo el nombre del plato
	memcpy(platoDelPedido->nombreDeMorfi, AREA_DE_SWAP + desplazamiento, 24); //harcodeado el 24 para cumplir con la norma del enunciado

	sem_post(semaforoTocarSWAP);

	//por ultimo hago la correccion de agregarle su \0 al final del nombre posta del morfi
	platoDelPedido->nombreDeMorfi[platoDelPedido->largoPostaDeMorfi] = '\0';

	sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);
}

void algoritmo_de_reemplazo(char* ALGOR_REEMPLAZO, tablas_segmentos_restaurantes* lasListasDePedidosDeRestaurantes, espacio* lista_de_espacios_en_MP) //AKA Grim Reaper
{
	tablas_segmentos_restaurantes* recorrerRestaurantes = lasListasDePedidosDeRestaurantes;
	segmentos* recorrerSegmentos = NULL;
	tabla_paginas* recorrerPlatos = NULL;
	tabla_paginas* victima = NULL;
	uint32_t primeraIteracion = 1;

	sem_wait(semaforoLogger);
	log_info(logger,"--Inicializado Algoritmo de reemplazo de Páginas--");
	sem_post(semaforoLogger);

	if(strcmp(ALGOR_REEMPLAZO, "LRU") == 0)
	{
		//mientras haya restaurantes que revisar...
		while(recorrerRestaurantes != NULL)
		{
			//selecciono el primer segmento
			recorrerSegmentos = recorrerRestaurantes->miTablaDePedidos;

			//mientras haya pedidos que revisar...
			while(recorrerSegmentos != NULL)
			{
				//selecciono el primer plato
				recorrerPlatos = recorrerSegmentos->mi_tabla;

				//mientras haya platos que revisar...
				while(recorrerPlatos != NULL)
				{
					//solo me interesa si esta cargada en MP
					if(recorrerPlatos->cargadoEnMEMORIA == 1)
					{
						if(primeraIteracion == 1)
						{
							primeraIteracion = 0;
							victima = recorrerPlatos; //por ser la primera iteracion, asumo que la primer pagina que encuentre va al matadero
						}

						//el chequeo posta
						if(victima->numero_de_victima > recorrerPlatos->numero_de_victima)
						{
							victima = recorrerPlatos;
						}
					}

					//avanzo a revisar el siguiente plato
					recorrerPlatos = recorrerPlatos->sig_pagina;
				}

				//avanzo a revisar el siguiente pedido
				recorrerSegmentos = recorrerSegmentos->sig_segmento;
			}

			//avanzo a revisar el siguiente restaurante
			recorrerRestaurantes = recorrerRestaurantes->sig_lista;
		}

		//en este punto ya tengo la pagina q vamos a volar para SWAP
		sem_wait(semaforoLogger);
		log_info(logger,"La víctima seleccionada para el reemplazo se encuentra en el marco: %i", victima->numeroDeMarco);
		sem_post(semaforoLogger);

		//hay que mover la pagina a SWAP
		actualizar_pagina_en_SWAP(victima);

		//marcamos el marco como libre para que lo puedan usar
		sem_wait(semaforoTocarListaEspaciosEnSWAP);
		marcarEspacioComoLibre(lista_de_espacios_en_MP, victima->numeroDeMarco);
		sem_post(semaforoTocarListaEspaciosEnMP);

		//actualizamos el dato de la pagina (que ya no esta en MP)
		victima->cargadoEnMEMORIA = 0;
		victima->numeroDeMarco = -1;
	}

	else
	{
		if(strcmp(ALGOR_REEMPLAZO, "CLOCK_MEJ") == 0)
		{
			puts("ALGOR PENDIENTE");//ToDo
			abort();
		}

		else
		{
			puts("Pusieron un algoritmo que nada que ver, me prendo fuego");
			abort();
		}
	}
}

void borrar_datos_del_plato(tabla_paginas* platoDelPedido)
{
	platoDelPedido->cantidadPedidaComida = 0;
	platoDelPedido->cantidadComidaPreparada = 0;
	strcpy(platoDelPedido->nombreDeMorfi, "<Censored>");
}

void borrar_datos_de_todos_los_platos_del_pedido(segmentos* tablaDePedidos)
{
	tabla_paginas* selector_de_plato = tablaDePedidos->mi_tabla;

	//avanzo en las paginas 1 por 1 hasta llegar al final
	while(selector_de_plato != NULL)
	{
		//borro los datos del plato seleccionado
		borrar_datos_del_plato(selector_de_plato);
		//avanzo...
		selector_de_plato = selector_de_plato->sig_pagina;
	}
}

void preparar_datos_de_platos_con_formato_de_obtener_pedido(segmentos* tablaDePedidos, respuesta_obtener_pedido* resultadoObtenerPedido)
{
	tabla_paginas* selector_de_plato = tablaDePedidos->mi_tabla;

	char* nombresComidas = malloc(100);
	char* cantidadesPedidasComidas = malloc(100);
	char* cantidadesPreparadasComidas = malloc(100);
	char* charParaPisar = malloc (5);

	strcpy(nombresComidas, "[");
	strcpy(cantidadesPedidasComidas, "[");
	strcpy(cantidadesPreparadasComidas, "[");

	//avanzo en las paginas 1 por 1 hasta llegar al final
	while(selector_de_plato != NULL)
	{
		//agrego el nombre del plato a la lista
		strcat(nombresComidas, selector_de_plato->nombreDeMorfi);

		//agrego la cantidad pedida del plato a la lista
		sprintf(charParaPisar, "%u", selector_de_plato->cantidadPedidaComida);
		strcat(cantidadesPedidasComidas, charParaPisar);

		//agrego la cantidad preparada de esa comida a la lista
		sprintf(charParaPisar, "%u", selector_de_plato->cantidadComidaPreparada);
		strcat(cantidadesPreparadasComidas, charParaPisar);

		//por ultimo, si hay un "siguiente plato", les agrego una , a las 3 listas para continuar con el siguiente
		if(selector_de_plato->sig_pagina != NULL)
		{
			strcat(nombresComidas, ",");
			strcat(cantidadesPedidasComidas, ",");
			strcat(cantidadesPreparadasComidas, ",");
		}

		//avanzo...
		selector_de_plato = selector_de_plato->sig_pagina;
	}

	//les pego el corchete final a cada una de las listas
	strcat(nombresComidas, "]");
	strcat(cantidadesPedidasComidas, "]");
	strcat(cantidadesPreparadasComidas, "]");

	//cargo los datos en la estructura que voy a mandar
	resultadoObtenerPedido->sizeComidas = strlen(nombresComidas);
	resultadoObtenerPedido->comidas = malloc(resultadoObtenerPedido->sizeComidas + 1);
	strcpy(resultadoObtenerPedido->comidas, nombresComidas);

	resultadoObtenerPedido->sizeCantTotales = strlen(cantidadesPedidasComidas);
	resultadoObtenerPedido->cantTotales = malloc(resultadoObtenerPedido->sizeCantTotales + 1);
	strcpy(resultadoObtenerPedido->cantTotales, cantidadesPedidasComidas);

	resultadoObtenerPedido->sizeCantListas = strlen(cantidadesPreparadasComidas);
	resultadoObtenerPedido->cantListas = malloc(resultadoObtenerPedido->sizeCantListas + 1);
	strcpy(resultadoObtenerPedido->cantListas, cantidadesPreparadasComidas);

	//por ultimo, librero los strings que arme para armar las listas de datos
	free(nombresComidas);
	free(cantidadesPedidasComidas);
	free(cantidadesPreparadasComidas);
	free(charParaPisar);
}
