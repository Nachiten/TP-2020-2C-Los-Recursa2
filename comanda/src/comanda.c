#include "comanda.h"

int main()
{
	PIDCoMAnda = getpid();
	socketEscucha = 0;
	printf("CoMAnda PID: %u.\n", PIDCoMAnda);
	MEMORIA_PRINCIPAL = NULL;
	AREA_DE_SWAP = NULL;
	numero_de_victima = 0;

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
	FRECUENCIA_COMPACTACION = config_get_int_value(config,"FRECUENCIA_COMPACTACION");
	printf("Frecuencia de compactación: %u.\n", FRECUENCIA_COMPACTACION);
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

	//ToDo matar la lista de las tablas de segmento
	//ToDo matar la tabla de segmentos
	//ToDo matar las tablas de paginas
	//ToDo matar la lista de los espacios existentes en SWAP
	//ToDo matar la lista de los espacios existentes en MP

	//puts("antes recepcion");
	recepcion_mensajes();
	//puts("despues recepcion");



	//liberamos las memorias reservadas
	free(MEMORIA_PRINCIPAL);
	puts("Memoria Principal Liberada Correctamente.");
	free(AREA_DE_SWAP);
	puts("Área de Swap Liberada Correctamente.");


	return EXIT_SUCCESS;
}

//ToDo ver si estos 2 sirven para algo (en comanda.h)
	//socketApp;
	//socketCliente;


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
}

void procesar_mensaje(codigo_operacion cod_op, int32_t sizeAAllocar, int32_t socket)
{
	guardar_pedido* recibidoGuardarPedido;
	guardar_plato* recibidoGuardarPlato;
	obtener_pedido* recibidoObtenerPedido;
	confirmar_pedido* recibidoConfirmarPedido;
	plato_listo* recibidoPlatoListo;
	finalizar_pedido* recibidoFinalizarPedido;

	respuesta_ok_error* resultado;
	respuesta_obtener_pedido* resultadoObtenerPedido;

	tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante = NULL;
	uint32_t numeroDeSegmento;
	tabla_paginas* plato_creado_o_editado = NULL;
	int32_t numeroDeEspacioEnSwap = -10;
	int32_t numeroDeMarcoEnMP = -10;

	/*ToDo CODIGOS DE OPERACION QUE FALTAN CODEAR:
	 GUARDAR_PLATO
	 OBTENER_PEDIDO
	 CONFIRMAR_PEDIDO
	 PLATO_LISTO
	 FINALIZAR_PEDIDO -> finalizar pedido y terminar pedido son practicamente lo mismo
	 */
    switch(cod_op)
    {
        case GUARDAR_PEDIDO:
        	recibidoGuardarPedido = malloc(sizeAAllocar);
        	recibir_mensaje(recibidoGuardarPedido, cod_op, socket);

        	resultado = malloc(sizeof(respuesta_ok_error));

        	//buscamos la tabla de pedidos del restaurante seleccionado, si no existe, se crea
        	sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);
        	tablaDePedidosDelRestaurante = selector_de_tabla_de_pedidos(lista_de_pedidos_de_todos_los_restaurantes, recibidoGuardarPedido->nombreRestaurante, 0);
        	sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);

        	//si la ID del pedido ya existe
        	if(verificarExistenciaDePedido (tablaDePedidosDelRestaurante, recibidoGuardarPedido->idPedido))
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

        	mandar_mensaje(resultado,RESPUESTA_GUARDAR_PEDIDO,socket);

        	free(recibidoGuardarPedido->nombreRestaurante); //shit, esto me va a armar quilombo ToDo revisar
			free(recibidoGuardarPedido);
			free(resultado);
        	break;

        case GUARDAR_PLATO:
        	recibidoGuardarPlato = malloc(sizeAAllocar);
        	recibir_mensaje(recibidoGuardarPlato, cod_op, socket);

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
        		if(verificarExistenciaDePedido (tablaDePedidosDelRestaurante, recibidoGuardarPlato->idPedido))
        		{
        			//tomamos el numero de segmento del pedido
        			sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);
            		numeroDeSegmento = buscar_segmento_de_pedido(tablaDePedidosDelRestaurante, recibidoGuardarPlato->idPedido);
            		sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);

        			//al segmento que corresponde al pedido, se le agrega el nuevo plato si no existia
            		sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);
            		//obtengo el plato editado, o creado en caso de que no exista antes
            		plato_creado_o_editado = agregarPlatoAPedido(tablaDePedidosDelRestaurante, numeroDeSegmento, recibidoGuardarPlato->nombrePlato, recibidoGuardarPlato->cantidadPlatos);
            		sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);

            		//busco si hay un espacio libre en SWAP para poner la pagina nueva/editada
            		sem_wait(semaforoTocarListaEspaciosEnSWAP);
            		numeroDeEspacioEnSwap = buscarPrimerEspacioLibre(lista_de_espacios_en_SWAP);
            		sem_post(semaforoTocarListaEspaciosEnSWAP);

            		if(numeroDeEspacioEnSwap == -1)
            		{
            			//ToDo si no hay un espacio libre hay que llamar al grim reaper
            		}

            		//ToDO temporalmente asumido que siempre existe espacio en SWAP
            		agregar_pagina_a_swap(plato_creado_o_editado, numeroDeEspacioEnSwap*32);

            		//busco si hay un marco libre en MP para poner la pagina nueva/editada
					sem_wait(semaforoTocarListaEspaciosEnMP);
					numeroDeMarcoEnMP = buscarPrimerEspacioLibre(lista_de_espacios_en_MP);
					//una vez seleccionado lo marco como ocupado para que ningun otro hilo lo quiera usar
					marcarEspacioComoOcupado(lista_de_espacios_en_MP, numeroDeMarcoEnMP);
					sem_post(semaforoTocarListaEspaciosEnMP);

					if(numeroDeMarcoEnMP == -1)
					{
						//ToDo si no hay un espacio libre hay que llamar al grim reaper
					}

					//ToDO temporalmente asumido que siempre existen marcos libres en MP
					mover_pagina_a_memoriaPrincipal(plato_creado_o_editado, numeroDeEspacioEnSwap*32, numeroDeMarcoEnMP*32);

					//por ultimo, respondemos que no crasheo nada
					resultado->respuesta = 1;
        		}

        		//el pedido no existe
        		else
        		{
        			resultado->respuesta = 0;
        		}
        	}

        	mandar_mensaje(resultado,RESPUESTA_GUARDAR_PLATO,socket);

        	free(recibidoGuardarPlato->nombreRestaurante);
        	free(recibidoGuardarPlato->nombrePlato);
			free(recibidoGuardarPlato);
        	break;

        case OBTENER_PEDIDO:
        	recibidoObtenerPedido = malloc(sizeAAllocar);
        	recibir_mensaje(recibidoObtenerPedido, cod_op, socket);

        	resultadoObtenerPedido = malloc(sizeof(respuesta_obtener_pedido)); //ToDo hacer los free dentro de CADA CASO POSIBLE, si es por fuera revienta

        	printf("Buscando datos del pedido %u del restaurante %s...\n", recibidoObtenerPedido->idPedido, recibidoObtenerPedido->nombreRestaurante);

        	//buscamos la tabla de pedidos de dicho restaurante, si no existe, se envia FAIL
        	sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);
        	tablaDePedidosDelRestaurante = selector_de_tabla_de_pedidos(lista_de_pedidos_de_todos_los_restaurantes, recibidoObtenerPedido->nombreRestaurante, 1);
        	sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);

        	//no existe este restaurante, se responde el default
        	if(tablaDePedidosDelRestaurante == NULL)
        	{
        		puts("El restaurante solicitado no existe.");
        		resultadoObtenerPedido->comidas = malloc(2);
        		strcpy(resultadoObtenerPedido->comidas, "[]");
        		//resultadoObtenerPedido->comidas = "[]";
        		resultadoObtenerPedido->sizeComidas = strlen(resultadoObtenerPedido->comidas);
        		//resultadoObtenerPedido->cantTotales = "[]";
        		resultadoObtenerPedido->cantTotales = malloc(2);
        		strcpy(resultadoObtenerPedido->cantTotales, "[]");
        		resultadoObtenerPedido->sizeCantTotales = strlen(resultadoObtenerPedido->cantTotales);
        		//resultadoObtenerPedido->cantListas = "[]";
        		resultadoObtenerPedido->cantListas = malloc(2);
        		strcpy(resultadoObtenerPedido->cantListas, "[]");
        		resultadoObtenerPedido->sizeCantListas = strlen(resultadoObtenerPedido->cantListas);

            	mandar_mensaje(resultadoObtenerPedido,RESPUESTA_OBTENER_PEDIDO,socket);

            	free(resultadoObtenerPedido);
        	}

        	//la tabla si existe, buscamos el segmento...
        	else
        	{
        		//obtenemos el numero del segmento del restaurante que nos piden, si no existe, se envia FAIL
        		if(verificarExistenciaDePedido (tablaDePedidosDelRestaurante, recibidoObtenerPedido->idPedido))
        		{
        			//tomamos el numero de segmento del pedido
        			sem_wait(semaforoTocarListaPedidosTodosLosRestaurantes);
            		numeroDeSegmento = buscar_segmento_de_pedido(tablaDePedidosDelRestaurante, recibidoObtenerPedido->idPedido);
            		sem_post(semaforoTocarListaPedidosTodosLosRestaurantes);


        		}

        		//el pedido no existe
        		else
        		{
        			puts("El pedido solicitado no existe.");
            		resultadoObtenerPedido->comidas = "[]";
            		resultadoObtenerPedido->sizeComidas = strlen(resultadoObtenerPedido->comidas);
            		resultadoObtenerPedido->cantTotales = "[]";
            		resultadoObtenerPedido->sizeCantTotales = strlen(resultadoObtenerPedido->cantTotales);
            		resultadoObtenerPedido->cantListas = "[]";
            		resultadoObtenerPedido->sizeCantListas = strlen(resultadoObtenerPedido->cantListas);

                	mandar_mensaje(resultadoObtenerPedido,RESPUESTA_OBTENER_PEDIDO,socket);

                	free(resultadoObtenerPedido);
        		}
        	}

			free(recibidoObtenerPedido);
        	break;

        case CONFIRMAR_PEDIDO:
        	recibidoConfirmarPedido = malloc(sizeAAllocar);
        	recibir_mensaje(recibidoConfirmarPedido, cod_op, socket);


        	free(recibidoConfirmarPedido);
        	break;

        case PLATO_LISTO:
        	recibidoPlatoListo = malloc(sizeAAllocar);
			recibir_mensaje(recibidoPlatoListo, cod_op, socket);


			free(recibidoPlatoListo);

        	break;

        case FINALIZAR_PEDIDO:
        	recibidoFinalizarPedido = malloc(sizeAAllocar);
        	recibir_mensaje(recibidoFinalizarPedido, cod_op, socket);

			free(recibidoFinalizarPedido);

        	break;

        case DESCONEXION:
        	//no hago un carajo porque se cerro la conexion
        	break;

        case HANDSHAKE:
        	puts("Se recibió un HANDSHAKE, que fue exitosamente ignorado.");
        	break;

        default://no deberia pasar nunca por aca, solo esta para que desaparezca el warning
        	puts("PASE POR EL CASO DEFAULT DEL SWITCH DE PROCESAR MENSAJE!!!! BUSCAR ERROR!!!!");
        	//puts("El mensaje mala leche fue recibido y felizmente ignorado.");
        	break;
    }

    //ToDo agregar un close para el socket?
}

void inicializarSemaforos() //Todo matar semaforos?
{
	semaforoNumeroVictima = malloc(sizeof(sem_t));
	semaforoLogger = malloc(sizeof(sem_t));
	semaforoTocarListaPedidosTodosLosRestaurantes = malloc(sizeof(sem_t));
	semaforoTocarListaEspaciosEnSWAP = malloc(sizeof(sem_t));
	semaforoTocarListaEspaciosEnMP = malloc(sizeof(sem_t));

	sem_init(semaforoNumeroVictima, 0, 1);
	sem_init(semaforoLogger, 0, 1);
	sem_init(semaforoTocarListaPedidosTodosLosRestaurantes, 0, 1);
	sem_init(semaforoTocarListaEspaciosEnSWAP, 0, 1);
	sem_init(semaforoTocarListaEspaciosEnMP, 0, 1);
}

