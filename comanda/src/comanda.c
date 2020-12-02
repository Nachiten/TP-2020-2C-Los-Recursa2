#include "comanda.h"

int main()
{
	PIDCoMAnda = getpid();
	socketEscucha = 0;
	printf("CoMAnda PID: %u.\n", PIDCoMAnda);
	MEMORIA_PRINCIPAL = NULL;
	AREA_DE_SWAP = NULL;
	numero_de_victima = 0;
	clock_mejorado_primera_iteracion = 1;
	punteroClockM = 0;

	//Cargo las configuraciones del .config
	config = leerConfiguracion("/home/utnso/workspace/tp-2020-2c-Los-Recursa2/configs/comanda.config");
	if(config != NULL)
	{
		puts("Archivo de configuración cargado correctamente.");
	}
	else
	{
		puts("Error al cargar archivo de configuración, abortando...\n");
		abort();
	}

	//cargo el path del archivo log
	LOG_PATH = config_get_string_value(config,"LOG_FILE_PATH");
	//Dejo cargado un logger para loguear los eventos.
	logger = cargarUnLog(LOG_PATH, "Cliente");
	if(logger != NULL)
	{
		puts("Archivo de LOGS cargado correctamente.");
	}
	else
	{
		puts("Error al cargar archivo de LOGS, abortando...");
		abort();
	}
	//cargo IPs y Puertos...
	cliente_IP = config_get_string_value(config,"IP_CLIENTE");
	app_IP = config_get_string_value(config,"IP_APP");
	mi_puerto = config_get_string_value(config,"MI_PUERTO");
	app_puerto = config_get_string_value(config,"PUERTO_APP");
	cliente_puerto = config_get_string_value(config,"PUERTO_CLIENTE");

	puts("Cargando configuraciones de memoria...");
	puts("****************************************");
	TAMANIO_MEMORIA_PRINCIPAL = config_get_int_value(config,"TAMANIO_MEMORIA");
	printf("Tamaño de Memoria Principal: %u bytes.\n", TAMANIO_MEMORIA_PRINCIPAL);
	TAMANIO_AREA_DE_SWAP = config_get_int_value(config,"TAMANIO_SWAP");
	printf("Tamaño del Área de Swapping: %u bytes.\n", TAMANIO_AREA_DE_SWAP);
	ALGOR_REEMPLAZO = config_get_string_value(config,"ALGORITMO_REEMPLAZO");
	printf("Algoritmo de reemplazo: %s.\n", ALGOR_REEMPLAZO);
	puts("****************************************");

	if(TAMANIO_MEMORIA_PRINCIPAL < 32)
	{
		puts("Tamaño de Memoria Principal demasiado chico, ingresar como mínimo 32 bytes.");
		abort();
	}

	if(TAMANIO_AREA_DE_SWAP < 32)
	{
		puts("Tamaño de Memoria de Swap demasiado chico, ingresar como mínimo 32 bytes.");
		abort();
	}

	//meter en hilo de memoria??
	MEMORIA_PRINCIPAL = malloc(TAMANIO_MEMORIA_PRINCIPAL);
	if(MEMORIA_PRINCIPAL != NULL)
	{
		puts("Memoria Principal inicializada.");
	}
	else
	{
		log_error(logger, "Error al inicializar Memoria Principal.");
		puts("Abortando...");
		abort();
	}
	AREA_DE_SWAP = malloc(TAMANIO_AREA_DE_SWAP);
	if(MEMORIA_PRINCIPAL != NULL)
	{
		puts("Área de SWAP inicializada.\n");
	}
	else
	{
		log_error(logger, "Error al inicializar el área de SWAP.");
		puts("Abortando...");
		abort();
	}

	//arrancamos los semaforos para variables globales
	inicializarSemaforos();

	//inicializamos la tabla de segmentos
	lista_de_pedidos_de_todos_los_restaurantes = malloc(sizeof(tablas_segmentos_restaurantes));
	inicializar_lista_de_tablas_de_segmentos_de_restaurantes(lista_de_pedidos_de_todos_los_restaurantes);//si, son nombres de mierda
	lista_de_espacios_en_SWAP = malloc(sizeof(espacio));
	inicializar_lista_de_espacios(lista_de_espacios_en_SWAP, TAMANIO_AREA_DE_SWAP); //este separa logicamente los espacios de SWAP
	lista_de_espacios_en_MP = malloc(sizeof(espacio));
	inicializar_lista_de_espacios(lista_de_espacios_en_MP, TAMANIO_MEMORIA_PRINCIPAL); //este separa logicamente los espacios de MP

	recepcion_mensajes();

	//ToDo matar la lista de las tablas de segmento
	//ToDo matar la tabla de segmentos
	//ToDo matar las tablas de paginas
	//ToDo matar la lista de los espacios existentes en SWAP
	//ToDo matar la lista de los espacios existentes en MP

	//liberamos las memorias reservadas
	free(MEMORIA_PRINCIPAL);
	puts("Memoria Principal Liberada Correctamente.");
	free(AREA_DE_SWAP);
	puts("Área de Swap Liberada Correctamente.");

	return EXIT_SUCCESS;
}

void recepcion_mensajes()
{
	socketEscucha = reservarSocket(mi_puerto);

	while(1)
	{
		esperar_conexiones(socketEscucha);
	}
}

void esperar_conexiones(int32_t miSocket)
{
	int32_t* socket_conexion_establecida = malloc(sizeof(int32_t));
	struct sockaddr_in dir_cliente;
	socklen_t tam_direccion = sizeof(struct sockaddr_in);

	//espera una conexion
	puts("Estoy esperando una conexión...");
	*socket_conexion_establecida = accept(miSocket, (void*) &dir_cliente, &tam_direccion);

	//una vez se establece una conexion, se intenta recibir un mensaje
	pthread_create(&hilo_recibir_mensajes,NULL,(void*)escuchar_mensajes,socket_conexion_establecida);
	pthread_detach(hilo_recibir_mensajes);
}

void escuchar_mensajes(int32_t* socket_conexion_establecida)
{
	int32_t bytesRecibidosCodOP = 0;
	int32_t recibidosSize = 0;
	int32_t sizeAAllocar = 0;
	codigo_operacion cod_op;

	//recibo codigo de op
	bytesRecibidosCodOP = recv(*socket_conexion_establecida, &cod_op, sizeof(cod_op), MSG_WAITALL);
	bytesRecibidos(bytesRecibidosCodOP);

	//seguro en contra de que me llegue un CONSULTAR_RESTAURANTES mala leche
	if(cod_op != CONSULTAR_RESTAURANTES)
	{
		//si se cayo la conexion, basicamente no hacemos hada
		if(bytesRecibidosCodOP < 1)
		{
			cod_op = 0;
			sizeAAllocar = 0;
		}

		//si la conexion NO se cayo, intento recibir lo que sigue
		else
		{
			//recibo tamaño de lo que sigue
			recibidosSize = recv(*socket_conexion_establecida, &sizeAAllocar, sizeof(int32_t), MSG_WAITALL);
			bytesRecibidos(recibidosSize);

			//si se cayo la conexion, no se hace nada con esto
			if(recibidosSize < 1)
			{
				cod_op = 0;
				sizeAAllocar = 0;
			}

		}

		if(cod_op != 0)
		{
			printf("Tamaño del Payload: %i.\n", sizeAAllocar);
		}

		//mando lo que me llego para que lo procesen
		procesar_mensaje(cod_op, sizeAAllocar, *socket_conexion_establecida);

	}

	else
	{
		puts("El mensaje mala leche fue recibido y felizmente ignorado.");
	}
	free(socket_conexion_establecida);
}

void procesar_mensaje(codigo_operacion cod_op, int32_t sizeAAllocar, int32_t el_socket)
{
	guardar_pedido* recibidoGuardarPedido;
	guardar_plato* recibidoGuardarPlato;
	obtener_pedido* recibidoObtenerPedido;
	confirmar_pedido* recibidoConfirmarPedido;
	plato_listo* recibidoPlatoListo;
	finalizar_pedido* recibidoFinalizarPedido;
	handshake* recibidoHandshake;

	respuesta_ok_error* resultado;
	respuesta_obtener_pedido* resultadoObtenerPedido;

	tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante = NULL;
	uint32_t numeroDeSegmento;
	tabla_paginas* plato_creado_o_editado = NULL;
	segmentos* segmentoSeleccionado = NULL;
	int32_t numeroDeEspacioEnSwap = -10;
	int32_t numeroDeMarcoEnMP = -10;

    switch(cod_op)
    {
        case GUARDAR_PEDIDO:
        	recibidoGuardarPedido = malloc(sizeof(guardar_pedido));
        	recibir_mensaje(recibidoGuardarPedido, cod_op, el_socket);

        	resultado = malloc(sizeof(respuesta_ok_error));

        	//buscamos la tabla de pedidos del restaurante seleccionado, si no existe, se crea
        	sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);
        	tablaDePedidosDelRestaurante = selector_de_tabla_de_pedidos(lista_de_pedidos_de_todos_los_restaurantes, recibidoGuardarPedido->nombreRestaurante, 0);
        	sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);

        	//si la ID del pedido ya existe
        	if(verificarExistenciaDePedido(tablaDePedidosDelRestaurante, recibidoGuardarPedido->idPedido, 0))
        	{
        		//devolvemos FAIL por que no se pudo crear un nuevo pedido
        		resultado->respuesta = 0;
        	}

        	else
        	{
        		//creamos un nuevo segmento para el nuevo pedido
            	sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);
				crearSegmento(tablaDePedidosDelRestaurante, recibidoGuardarPedido->idPedido);
	        	sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);

				//por ultimo avisamos que el pedido fue agregado correctamente
	        	printf("--Se creó un nuevo pedido para %s con ID %u--\n", recibidoGuardarPedido->nombreRestaurante, recibidoGuardarPedido->idPedido);
				resultado->respuesta = 1;
        	}

        	mandar_mensaje(resultado,RESPUESTA_GUARDAR_PEDIDO,el_socket);

        	free(recibidoGuardarPedido->nombreRestaurante);
			free(recibidoGuardarPedido);
			free(resultado);
        	break;

        case GUARDAR_PLATO:
        	recibidoGuardarPlato = malloc(sizeof(guardar_plato));
        	recibir_mensaje(recibidoGuardarPlato, cod_op, el_socket);

        	resultado = malloc(sizeof(respuesta_ok_error));

        	//buscamos la tabla de pedidos de dicho restaurante, si no existe, se envia FAIL
        	sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);
        	tablaDePedidosDelRestaurante = selector_de_tabla_de_pedidos(lista_de_pedidos_de_todos_los_restaurantes, recibidoGuardarPlato->nombreRestaurante, 1);
        	sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);

        	if(tablaDePedidosDelRestaurante == NULL)
        	{
        		resultado->respuesta = 0;
        	}

        	//la tabla si existe, buscamos el segmento...
        	else
        	{
        		//obtenemos el numero del segmento del restaurante que nos piden, si no existe, se envia FAIL
        		if(verificarExistenciaDePedido(tablaDePedidosDelRestaurante, recibidoGuardarPlato->idPedido, 0))
        		{
        			//tomamos el numero de segmento del pedido
        			sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);
            		numeroDeSegmento = buscar_segmento_de_pedido(tablaDePedidosDelRestaurante, recibidoGuardarPlato->idPedido);
            		sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);

        			//al segmento que corresponde al pedido, se le agrega el nuevo plato si no existia

            		//obtengo el plato editado, o creado en caso de que no exista antes
            		plato_creado_o_editado = agregarPlatoAPedido(tablaDePedidosDelRestaurante, numeroDeSegmento, recibidoGuardarPlato->nombrePlato, recibidoGuardarPlato->cantidadPlatos);

            		//si no esta en SWAP, le busco un lugar
            		if(plato_creado_o_editado->cargadoEnSWAP == 0)
            		{
                		//busco si hay un espacio libre en SWAP para poner la pagina nueva/editada
                		sem_wait(semaforoTocarListaEspaciosEnSWAP);
                		numeroDeEspacioEnSwap = buscarPrimerEspacioLibre(lista_de_espacios_en_SWAP);
                		//una vez seleccionado lo marco como ocupado para que ningun otro hilo lo quiera usar
                		marcarEspacioComoOcupado(lista_de_espacios_en_SWAP, numeroDeEspacioEnSwap);
                		sem_post(semaforoTocarListaEspaciosEnSWAP);
            		}

            		else//si ya estaba en SWAP, le doy el espacio que tenia antes
            		{
            			numeroDeEspacioEnSwap = plato_creado_o_editado->posicionInicialEnSWAP;
            		}

            		if(numeroDeEspacioEnSwap != -1)
            		{
						agregar_pagina_a_swap(plato_creado_o_editado, numeroDeEspacioEnSwap*32);

						sem_wait(semaforoTocarListaEspaciosEnMP);
						//si la pagina no esta en MP, le busco un lugar
						if(plato_creado_o_editado->cargadoEnMEMORIA == 0)
						{
							//busco si hay un marco libre en MP para poner la pagina nueva/editada
							numeroDeMarcoEnMP = buscarPrimerEspacioLibre(lista_de_espacios_en_MP);
						}

						else//si ya estaba en MP, le doy el espacio que tenia antes
						{
							numeroDeMarcoEnMP = plato_creado_o_editado->numeroDeMarco;
						}

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
							sem_wait(semaforoAlgoritmoReemplazo);
							algoritmo_de_reemplazo(ALGOR_REEMPLAZO, lista_de_pedidos_de_todos_los_restaurantes, lista_de_espacios_en_MP);
							sem_post(semaforoAlgoritmoReemplazo);
							numeroDeMarcoEnMP = buscarPrimerEspacioLibre(lista_de_espacios_en_MP);
							marcarEspacioComoOcupado(lista_de_espacios_en_MP, numeroDeMarcoEnMP);
							sem_post(semaforoTocarListaEspaciosEnMP);
							sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);
						}

						//una vez tengo un marco listo para poner la Página, pongo los datos en MP
						mover_pagina_a_memoriaPrincipal(plato_creado_o_editado, numeroDeEspacioEnSwap*32, numeroDeMarcoEnMP*32);
						printf("--Se guardó %u plato/s %s para %s, en el pedido de ID %u--\n",recibidoGuardarPlato->cantidadPlatos, recibidoGuardarPlato->nombrePlato, recibidoGuardarPlato->nombreRestaurante, recibidoGuardarPlato->idPedido);

						//por ultimo, respondemos que no crasheo nada
						resultado->respuesta = 1;
            		}

            		//si no hay espacio en SWAP se deniega la solicitud
            		else
            		{
            			puts("SWAP se encuentra hasta las manos, no se puede poner ninguna Página más por el momento.");
            			resultado->respuesta = 0;
            		}
        		}

        		//el pedido no existe
        		else
        		{
        			resultado->respuesta = 0;
        		}
        	}

        	mandar_mensaje(resultado,RESPUESTA_GUARDAR_PLATO,el_socket);

        	free(resultado);
        	free(recibidoGuardarPlato->nombreRestaurante);
        	free(recibidoGuardarPlato->nombrePlato);
			free(recibidoGuardarPlato);
        	break;

        case OBTENER_PEDIDO:
        	recibidoObtenerPedido = malloc(sizeof(obtener_pedido));
        	recibir_mensaje(recibidoObtenerPedido, cod_op, el_socket);

        	resultadoObtenerPedido = malloc(sizeof(respuesta_obtener_pedido));

        	printf("Buscando datos del pedido %u del restaurante %s...\n", recibidoObtenerPedido->idPedido, recibidoObtenerPedido->nombreRestaurante);

        	//buscamos la tabla de pedidos de dicho restaurante, si no existe, se envia FAIL
        	sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);
        	tablaDePedidosDelRestaurante = selector_de_tabla_de_pedidos(lista_de_pedidos_de_todos_los_restaurantes, recibidoObtenerPedido->nombreRestaurante, 1);
        	sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);

        	//no existe este restaurante, se responde el default
        	if(tablaDePedidosDelRestaurante == NULL)
        	{
        		puts("El restaurante solicitado no existe.");
        		resultadoObtenerPedido->estado = NADA_CARGADO;
        		resultadoObtenerPedido->comidas = malloc(3);
        		strcpy(resultadoObtenerPedido->comidas, "[]");
        		resultadoObtenerPedido->sizeComidas = strlen(resultadoObtenerPedido->comidas);
        		resultadoObtenerPedido->cantTotales = malloc(3);
        		strcpy(resultadoObtenerPedido->cantTotales, "[]");
        		resultadoObtenerPedido->sizeCantTotales = strlen(resultadoObtenerPedido->cantTotales);
        		resultadoObtenerPedido->cantListas = malloc(3);
        		strcpy(resultadoObtenerPedido->cantListas, "[]");
        		resultadoObtenerPedido->sizeCantListas = strlen(resultadoObtenerPedido->cantListas);
        	}

        	//la tabla si existe, buscamos el segmento...
        	else
        	{
        		//obtenemos el numero del segmento del restaurante que nos piden, si no existe, se envia FAIL
        		if(verificarExistenciaDePedido(tablaDePedidosDelRestaurante, recibidoObtenerPedido->idPedido, 1))
        		{
        			//tomamos el numero de segmento del pedido
        			sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);
            		numeroDeSegmento = buscar_segmento_de_pedido(tablaDePedidosDelRestaurante, recibidoObtenerPedido->idPedido);
            		sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);

            		//hay que cargar cada una de las paginas del pedido en MP
            		cargarPaginasEnMP(tablaDePedidosDelRestaurante, numeroDeSegmento);

            		//una vez cargadas en MP, copio los datos de MP sobre los platos del pedido
            		actualizarTodosLosPlatosConDatosDeMP(tablaDePedidosDelRestaurante, numeroDeSegmento);

            		//una vez tengo las listas, me armo los datos de resultadoObtenerPedido
            		sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);
            		preparar_datos_de_platos_con_formato_de_obtener_pedido(selectordePedidoDeRestaurante(tablaDePedidosDelRestaurante, numeroDeSegmento), resultadoObtenerPedido);
            		sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);

            		//reseteo los datos de las listas
            		sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);
            		borrar_datos_de_todos_los_platos_del_pedido(selectordePedidoDeRestaurante(tablaDePedidosDelRestaurante, numeroDeSegmento));
            		sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);
        		}

        		//el pedido no existe
        		else
        		{
        			puts("El pedido solicitado no existe o no tiene platos cargados.");
        			resultadoObtenerPedido->estado = NADA_CARGADO;
            		resultadoObtenerPedido->comidas = malloc(3);
            		strcpy(resultadoObtenerPedido->comidas, "[]");
            		resultadoObtenerPedido->sizeComidas = strlen(resultadoObtenerPedido->comidas);
            		resultadoObtenerPedido->cantTotales = malloc(3);
            		strcpy(resultadoObtenerPedido->cantTotales, "[]");
            		resultadoObtenerPedido->sizeCantTotales = strlen(resultadoObtenerPedido->cantTotales);
            		resultadoObtenerPedido->cantListas = malloc(3);
            		strcpy(resultadoObtenerPedido->cantListas, "[]");
            		resultadoObtenerPedido->sizeCantListas = strlen(resultadoObtenerPedido->cantListas);
        		}
        	}

        	//por ultimo, mando el mensaje con los datos que correspondan
        	mandar_mensaje(resultadoObtenerPedido,RESPUESTA_OBTENER_PEDIDO,el_socket);

        	free(resultadoObtenerPedido->comidas);
        	free(resultadoObtenerPedido->cantTotales);
        	free(resultadoObtenerPedido->cantListas);
        	free(resultadoObtenerPedido);
			free(recibidoObtenerPedido);
        	break;

        case CONFIRMAR_PEDIDO:
        	recibidoConfirmarPedido = malloc(sizeof(confirmar_pedido));
        	recibir_mensaje(recibidoConfirmarPedido, cod_op, el_socket);

        	resultado = malloc(sizeof(respuesta_ok_error));

        	printf("Buscando datos del pedido %u del restaurante %s...\n", recibidoConfirmarPedido->idPedido, recibidoConfirmarPedido->nombreRestaurante);

        	//buscamos la tabla de pedidos de dicho restaurante, si no existe, se envia FAIL
			sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);
			tablaDePedidosDelRestaurante = selector_de_tabla_de_pedidos(lista_de_pedidos_de_todos_los_restaurantes, recibidoConfirmarPedido->nombreRestaurante, 1);
			sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);

			//no existe este restaurante, se responde FAIL
			if(tablaDePedidosDelRestaurante == NULL)
			{
				puts("El restaurante solicitado no existe.");
				resultado->respuesta = 0;
			}

			//la tabla si existe, buscamos el segmento...
			else
			{
				//obtenemos el numero del segmento del restaurante que nos piden, si no existe, se envia FAIL
				if(verificarExistenciaDePedido(tablaDePedidosDelRestaurante, recibidoConfirmarPedido->idPedido, 0))
				{
					//tomamos el numero de segmento del pedido
					sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);
					numeroDeSegmento = buscar_segmento_de_pedido(tablaDePedidosDelRestaurante, recibidoConfirmarPedido->idPedido);
					segmentoSeleccionado = selectordePedidoDeRestaurante(tablaDePedidosDelRestaurante, numeroDeSegmento);
					sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);

					//verificamos si se encuentra en estado PENDIENTE
					if(verificarEstado(segmentoSeleccionado, PENDIENTE))
					{
						//pasamos el pedido a CONFIRMADO
						cambiarEstado(segmentoSeleccionado, CONFIRMADO);
						resultado->respuesta = 1;
						puts("El pedido solicitado ahora se encuentra en estado CONFIRMADO.");
					}

					//nop
					else
					{
						puts("El pedido solicitado NO se encuentra en estado PENDIENTE.");
						resultado->respuesta = 0;
					}

				}

				//el pedido no existe
				else
				{
					puts("El pedido solicitado no existe.");
					resultado->respuesta = 0;
				}
			}

			//mando el resultado de lo que consultaron
			mandar_mensaje(resultado,RESPUESTA_CONFIRMAR_PEDIDO,el_socket);

        	free(resultado);
        	free(recibidoConfirmarPedido->nombreRestaurante);
        	free(recibidoConfirmarPedido);
        	break;

        case PLATO_LISTO:
        	recibidoPlatoListo = malloc(sizeof(plato_listo));
			recibir_mensaje(recibidoPlatoListo, cod_op, el_socket);

			resultado = malloc(sizeof(respuesta_ok_error));

			printf("Buscando datos del pedido %u del restaurante %s...\n", recibidoPlatoListo->idPedido, recibidoPlatoListo->nombreRestaurante);

			//buscamos la tabla de pedidos de dicho restaurante, si no existe, se envia FAIL
			sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);
			tablaDePedidosDelRestaurante = selector_de_tabla_de_pedidos(lista_de_pedidos_de_todos_los_restaurantes, recibidoPlatoListo->nombreRestaurante, 1);
			sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);

			//no existe este restaurante, se responde FAIL
			if(tablaDePedidosDelRestaurante == NULL)
			{
				puts("El restaurante solicitado no existe.");
				resultado->respuesta = 0;
			}

			//la tabla si existe, buscamos el segmento...
			else
			{
				//obtenemos el numero del segmento del restaurante que nos piden, si no existe, se envia FAIL
				if(verificarExistenciaDePedido (tablaDePedidosDelRestaurante, recibidoPlatoListo->idPedido, 0))
				{
					//tomamos el numero de segmento del pedido
					sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);
					numeroDeSegmento = buscar_segmento_de_pedido(tablaDePedidosDelRestaurante, recibidoPlatoListo->idPedido);
					segmentoSeleccionado = selectordePedidoDeRestaurante(tablaDePedidosDelRestaurante, numeroDeSegmento);
					sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);

					//verificamos si existe el plato
					if(verificarExistenciaDePlato(segmentoSeleccionado, recibidoPlatoListo->nombrePlato))
					{
						//cargamos todas las paginas del pedido en MP antes de continuar...
						cargarPaginasEnMP(tablaDePedidosDelRestaurante, numeroDeSegmento);

						//verificamos si se encuentra en estado CONFIRMADO
						if(verificarEstado(segmentoSeleccionado, CONFIRMADO))
						{
							puts("Aumentando en 1 la cantidad de platos preparados...");
							aumentarCantidadLista(segmentoSeleccionado, recibidoPlatoListo->nombrePlato);
							resultado->respuesta = 1;
						}

						//nop
						else
						{
							puts("El pedido solicitado NO se encuentra en estado CONFIRMADO.");
							resultado->respuesta = 0;
						}
					}

					//nop
					else
					{
						puts("El plato solicitado no existe en este pedido.");
						resultado->respuesta = 0;
					}
				}

				//el pedido no existe
				else
				{
					puts("El pedido solicitado no existe.");
					resultado->respuesta = 0;
				}
			}

			//mando el resultado de lo que consultaron
			mandar_mensaje(resultado,RESPUESTA_CONFIRMAR_PEDIDO,el_socket);

			free(resultado);
			free(recibidoPlatoListo->nombreRestaurante);
			free(recibidoPlatoListo->nombrePlato);
			free(recibidoPlatoListo);
        	break;

        case FINALIZAR_PEDIDO:
        	recibidoFinalizarPedido = malloc(sizeof(finalizar_pedido));
        	recibir_mensaje(recibidoFinalizarPedido, cod_op, el_socket);

        	resultado = malloc(sizeof(respuesta_ok_error));

			printf("Buscando datos del pedido %u del restaurante %s...\n", recibidoFinalizarPedido->idPedido, recibidoFinalizarPedido->nombreRestaurante);

			//buscamos la tabla de pedidos de dicho restaurante, si no existe, se envia FAIL
			sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);
			tablaDePedidosDelRestaurante = selector_de_tabla_de_pedidos(lista_de_pedidos_de_todos_los_restaurantes, recibidoFinalizarPedido->nombreRestaurante, 1);
			sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);

			//no existe este restaurante, se responde FAIL
			if(tablaDePedidosDelRestaurante == NULL)
			{
				puts("El restaurante solicitado no existe.");
				resultado->respuesta = 0;
			}

			//la tabla si existe, buscamos el segmento...
			else
			{
				//obtenemos el numero del segmento del restaurante que nos piden, si no existe, se envia FAIL
				if(verificarExistenciaDePedido (tablaDePedidosDelRestaurante, recibidoFinalizarPedido->idPedido, 0))
				{
					//tomamos el numero de segmento del pedido
					sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);
					numeroDeSegmento = buscar_segmento_de_pedido(tablaDePedidosDelRestaurante, recibidoFinalizarPedido->idPedido);
					sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);

					//hora de dejar este mundo...
					printf("Procedo a matar el pedido %u.\n", recibidoFinalizarPedido->idPedido);
					matarPedido(tablaDePedidosDelRestaurante, numeroDeSegmento);
					resultado->respuesta = 1;
				}

				//el pedido no existe
				else
				{
					puts("El pedido solicitado no existe.");
					resultado->respuesta = 0;
				}
			}

			//mando el resultado de lo que consultaron
			mandar_mensaje(resultado,RESPUESTA_CONFIRMAR_PEDIDO,el_socket);

			free(resultado);
			free(recibidoFinalizarPedido->nombreRestaurante);
			free(recibidoFinalizarPedido);
        	break;

        case DESCONEXION:
        	//no hago un carajo porque se cerro la conexion
        	break;

        case HANDSHAKE:
        	recibidoHandshake = malloc(sizeof(handshake));
			recibir_mensaje(recibidoHandshake,HANDSHAKE,el_socket);
			free(recibidoHandshake->id);
			free(recibidoHandshake);

        	puts("Se recibió un HANDSHAKE, que fue exitosamente ignorado.");
        	break;

        default:
        	puts("El mensaje mala leche fue recibido y felizmente ignorado.");
        	break;
    }
}

void inicializarSemaforos()
{
	semaforoNumeroVictima = malloc(sizeof(sem_t));
	semaforoLogger = malloc(sizeof(sem_t));
	semaforoTocarListaPedidosTodosLosRestaurantes = malloc(sizeof(sem_t));
	semaforoTocarListaEspaciosEnSWAP = malloc(sizeof(sem_t));
	semaforoTocarListaEspaciosEnMP = malloc(sizeof(sem_t));
	semaforoTocarMP = malloc(sizeof(sem_t));
	semaforoTocarSWAP = malloc(sizeof(sem_t));
	semaforoAlgoritmoReemplazo = malloc(sizeof(sem_t));

	sem_init(semaforoNumeroVictima, 0, 1);
	sem_init(semaforoLogger, 0, 1);
	sem_init(semaforoTocarListaPedidosTodosLosRestaurantes, 0, 1);
	sem_init(semaforoTocarListaEspaciosEnSWAP, 0, 1);
	sem_init(semaforoTocarListaEspaciosEnMP, 0, 1);
	sem_init(semaforoAlgoritmoReemplazo, 0, 1);
	sem_init(semaforoTocarMP, 0, 1);
	sem_init(semaforoTocarSWAP, 0, 1);
}

