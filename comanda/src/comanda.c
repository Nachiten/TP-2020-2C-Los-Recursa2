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
	void inicializarSemaforos();

	//inicializamos la tabla de segmentos
	lista_de_pedidos_de_todos_los_restaurantes = malloc(sizeof(tablas_segmentos_restaurantes));
	inicializar_lista_de_tablas_de_segmentos_de_restaurantes(lista_de_pedidos_de_todos_los_restaurantes);//si, son nombres de mierda


	//ToDo matar la lista de las tablas de segmento
	//ToDo matar la tabla de segmentos
	//ToDo matar las tablas de paginas


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

//ToDo ver si estos 2 sirven para algo (seguro que si)
	//int32_t socketApp;
	//int32_t socketCliente;


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

	//si se cayo la conexion, basicamente no hacemos hada
	if(bytesRecibidosCodOP < 1)
	{
		cod_op = 0;
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
			sizeAAllocar = 0;
		}

	}

	printf("Tamaño del Payload: %i.\n", sizeAAllocar);

	//mando lo que me llego para que lo procesen
	procesar_mensaje(cod_op, sizeAAllocar, *socket_conexion_establecida);
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

	tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante = NULL;
	uint32_t numeroDeSegmento;

	/*ToDo CODIGOS DE OPERACION QUE FALTAN CODEAR:
	 GUARDAR_PEDIDO
	 GUARDAR_PLATO
	 OBTENER_PEDIDO
	 CONFIRMAR_PEDIDO
	 PLATO_LISTO
	 FINALIZAR_PEDIDO -> finalizar pedido y terminar pedido son practicamente lo mismo
	 */
    switch(cod_op)
    {
        case GUARDAR_PEDIDO://ToDo meterle semaforos para manejar la lista de pedidos de los restaurantes
        	recibidoGuardarPedido = malloc(sizeAAllocar);
        	recibir_mensaje(recibidoGuardarPedido, cod_op, socket);

        	resultado = malloc(sizeof(respuesta_ok_error));

        	//buscamos la tabla de pedidos del restaurante seleccionado, si no existe, se crea
        	tablaDePedidosDelRestaurante = selector_de_tabla_de_pedidos(lista_de_pedidos_de_todos_los_restaurantes, recibidoGuardarPedido->nombreRestaurante, 0);

        	//si la ID del pedido ya existe
        	if(verificarExistenciaDePedido (tablaDePedidosDelRestaurante, recibidoGuardarPedido->idPedido))
        	{
        		//devolvemos FAIL por que no se pudo crear un nuevo pedido
        		resultado->respuesta = 0;
        	}

        	else
        	{
        		//creamos un nuevo segmento para el nuevo pedido
				crearSegmento(tablaDePedidosDelRestaurante, recibidoGuardarPedido->idPedido);

				//por ultimo avisamos que el pedido fue agregado correctamente
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
        	tablaDePedidosDelRestaurante = selector_de_tabla_de_pedidos(lista_de_pedidos_de_todos_los_restaurantes, recibidoGuardarPlato->nombreRestaurante, 1);

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
            		numeroDeSegmento = buscar_segmento_de_pedido(tablaDePedidosDelRestaurante, recibidoGuardarPlato->idPedido);

        			//al segmento que corresponde al pedido, se le agrega el nuevo plato si no existia
            		agregarPlatoAPedido(tablaDePedidosDelRestaurante, numeroDeSegmento, recibidoGuardarPlato->nombrePlato, recibidoGuardarPlato->cantidadPlatos);
        		}

        		//el pedido no existe
        		else
        		{
        			resultado->respuesta = 0;
        		}


        	}




			free(recibidoGuardarPlato);
        	break;

        case OBTENER_PEDIDO:
        	recibidoObtenerPedido = malloc(sizeAAllocar);
        	recibir_mensaje(recibidoObtenerPedido, cod_op, socket);

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

        default://no deberia pasar nunca por aca, solo esta para que desaparezca el warning
        	puts("PASE POR EL CASO DEFAULT DEL SWITCH DE PROCESAR MENSAJE!!!! BUSCAR ERROR!!!!");
        	break;
    }

    //ToDo agregar un close para el socket?
}

void inicializarSemaforos() //Todo matar semaforos?
{
	semaforoNumeroVictima = malloc(sizeof(sem_t));
	semaforoLogger = malloc(sizeof(sem_t));

	sem_init(semaforoNumeroVictima, 0, 1);
	sem_init(semaforoLogger, 0, 1);
}

