#include "cliente.h"

int main(){

	//PIDCliente = getpid();
	socketEscucha = 0;

	ip_destino = malloc(15); //creo que no se puede pasar de esto
	puerto_destino = malloc(10); //10 para ir a lo seguro
	puerto_local = malloc(10); //10 para ir a lo seguro
	//puerto_app = malloc(10);
	//ip_app = malloc(15);

	//Cargo las configuraciones del .config
	config = leerConfiguracion("/home/utnso/workspace/tp-2020-2c-Los-Recursa2/configs/cliente.config");
	idCliente = config_get_string_value(config, "ID_CLIENTE");
	LOG_PATH = config_get_string_value(config,"LOG_FILE_PATH"); //cargo el path del archivo log
	ip_destino = config_get_string_value(config,"IP_DESTINO");
	puerto_destino = config_get_string_value(config,"PUERTO_DESTINO");
	puerto_local = config_get_string_value(config,"PUERTO_LOCAL");
	//puerto_APP = config_get_string_value(config,"PUERTO_APP");
	miPosicionX = config_get_int_value(config, "POSICION_X");
	miPosicionY = config_get_int_value(config, "POSICION_Y");

	//Dejo cargado un logger para loguear los eventos.
	logger = cargarUnLog(LOG_PATH, "Cliente");

	//ToDo levantamos socket para recibir mensajes (hilo)

	//ToDo hay que levantarlo como hilo, y agregar toda la parte del accept, recibir mensaje y el manejo del mensaje recibido


	//Preparar consola que se mantendra activa hasta la terminacion del proceso (hilo)  WIP

	/* 	ToDo VER SI BORRAR LA ESTRUCTURA ESTA
	t_conexion tuplaConexion;
    tuplaConexion.ip_destino = ip_destino;
    tuplaConexion.puerto_destino = puerto_destino;
    tuplaConexion.mi_logger = logger;
    */

//	char* lineaEntera;
//	char* auxLinea;
//    lineaEntera = NULL;
//	size_t longitud = 0;

	comandoParaEjecutar = malloc(sizeof(sem_t));
	sem_init(comandoParaEjecutar, 0, 1);

	while(1)
	{
//		printf("Inserte un comando:\n");
//		//memset(lineaEntera,0,60);
//		getline(&lineaEntera, &longitud, stdin);
//
//		string_trim(&lineaEntera); //ToDo hablar con un ayudante: esto puede que no sea necesario
//
//		if(strcmp(lineaEntera, "") == 0)
//		{
//			printf("No se ingresó ningun comando.\n");
//			free(lineaEntera);
//			continue;
//		}
//
//		auxLinea = malloc(strlen(lineaEntera));
//		//strncpy(auxLinea,lineaEntera,strlen(lineaEntera));
//		strcpy(auxLinea,lineaEntera);
//
//		//string_trim_right(&auxLinea);
		sem_wait(comandoParaEjecutar);
		pthread_create(&hiloConsola, NULL,(void*)obtenerInputConsolaCliente, NULL);
		//pthread_create(&hiloConsola, NULL,(void*)obtenerInputConsolaCliente, auxLinea);
		pthread_detach(hiloConsola);

	}

	/*
	free(lineaEntera);

	//ToDO meter los free que falten
	free(ip_destino);
	free(puerto_destino);
	free(puerto_local);
    */
	//para cerrar el programa
	matarPrograma(logger, config, socketEscucha);

	return EXIT_SUCCESS;
}


//void obtenerInputConsolaCliente(t_conexion* tuplaConexion)
void obtenerInputConsolaCliente(){
	//char* lineaEntera = NULL;
	//size_t longitud = 0;
	uint32_t switcher;
	respuesta_ok_error* estructuraRespuesta;
	uint32_t socketCliente;
	int32_t iterador = 0;
	int32_t sizeAAllocar = 0;
	uint32_t exito = 0;
	char* lineaEntera;
	lineaEntera = NULL;
	size_t longitud = 0;

	//char* nombreRestaurante; puede que este al pedo

	printf("Inserte un comando:\n");

	getline(&lineaEntera, &longitud, stdin);

	string_trim(&lineaEntera);

	if(strcmp(lineaEntera, "") == 0)
	  {
		printf("No se ingresó ningun comando.\n");
		free(lineaEntera);
		sem_post(comandoParaEjecutar);
		return;
	  }



	//char** palabrasSeparadas = string_split(lineaEntera , " ");
	char** palabrasSeparadas = string_split(lineaEntera , " ");

	// El nombre del comando es la primer palabra (POR EL MOMENTO CON GUIONES_BAJOS) -> EJ: CONSULTAR_RESTAURANTES
    char* comandoIngresado = malloc(strlen(palabrasSeparadas[0])+1);
    strcpy(comandoIngresado,palabrasSeparadas[0]);

    sem_post(comandoParaEjecutar);

    //printf("El comando solicitado fue: %s. \n", comandoIngresado);

    switcher = valor_para_switch_case(comandoIngresado);


    //todo me parece que por cuestion de "prolijidad" y que despues sea mas facil encontrar cada CASE, lo ideal seria ir poniendolos en orden,
    //como estan declarados en codigo_operacion: consultar restaurante, seleccionar restaurante, obtener restaurante, etc...


    /*
    CASES LABURADOS:

    Consultar Restaurantes -> Check
    Seleccionar Restaurante -> Check
    Obtener Restaurante -> Check
    Consultar Platos -> Check
    Guardar Plato -> Check
    Aniadir Plato ->
    Plato listo ->
    Crear Pedido -> Check
    Guardar Pedido -> Check
    Confirmar Pedido ->
    Consultar Pedido ->
    Obtener Pedido ->
    Finalizar Pedido ->
    Terminar Pedido ->
    Obtener Receta ->
    */




    switch(switcher)
    {

	case CONSULTAR_RESTAURANTES:;
		//socketCliente = establecer_conexion(tuplaConexion->ip_destino , tuplaConexion->puerto_destino);
		socketCliente = establecer_conexion(ip_destino , puerto_destino);
		resultado_de_conexion(socketCliente, logger, "destino");

		if(socketCliente < 1)
		{
			puts("estoy en el if \n");
			break;
		}

		mandar_mensaje("nadaxdxd", CONSULTAR_RESTAURANTES, socketCliente);

		los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

		if(exito == 1)
		{
			//respuesta_consultar_restaurantes* estructuraRespuestaConsultaRestaurantes = malloc(sizeof(respuesta_consultar_restaurantes));
			respuesta_consultar_restaurantes* estructuraRespuestaConsultaRestaurantes = malloc(sizeAAllocar);

			recibir_mensaje(estructuraRespuestaConsultaRestaurantes,RESPUESTA_CONSULTAR_R,socketCliente);

			//ToDo hablar con Tomas sobre si esto funcionaria asi o no


			palabrasSeparadas = string_split(estructuraRespuestaConsultaRestaurantes->listaRestaurantes, ",");//falta agregar corchetes

			puts("Los restaurantes que se encuentran disponibles son: \n");

			while(iterador < estructuraRespuestaConsultaRestaurantes->cantRestaurantes)
			{
				printf("%s\n",palabrasSeparadas[iterador]);
			}

			free(estructuraRespuestaConsultaRestaurantes);
		}

		else
		{
			printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.\n");
		}

    	break;

    case SELECCIONAR_RESTAURANTE:

    	strcat(palabrasSeparadas[1],"\0");


    	estructuraRespuesta = malloc(sizeof(respuesta_ok_error));

		socketCliente = establecer_conexion(ip_destino, puerto_destino);
		resultado_de_conexion(socketCliente, logger, "destino");

		seleccionar_restaurante* estructuraSeleccRestaur = malloc(sizeof(uint32_t)*2 + strlen(idCliente) + strlen(palabrasSeparadas[1]));
		estructuraSeleccRestaur->idCliente = idCliente;
		estructuraSeleccRestaur->largoIDCliente = strlen(idCliente);
		estructuraSeleccRestaur->nombreRestaurante = palabrasSeparadas[1];
		estructuraSeleccRestaur->largoNombreRestaurante = strlen(palabrasSeparadas[1]);

		mandar_mensaje(estructuraSeleccRestaur, SELECCIONAR_RESTAURANTE, socketCliente);

		los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

		if(exito == 1)
		{
			recibir_mensaje(estructuraRespuesta,RESPUESTA_GUARDAR_PLATO,socketCliente);

			printf("El intento de seleccionar un restaurante fue: %s.\n", resultadoDeRespuesta(estructuraRespuesta->respuesta));
		}

		else
		{
			printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.");
		}

		free(estructuraSeleccRestaur);//entiendo que no hace falta liberar nombre de restaurante ni id cliente porque solo fueron asignaciones
		free(estructuraRespuesta);
		close(socketCliente);
    	break;

	case OBTENER_RESTAURANTE:;

	    if(palabrasSeparadas[1] == NULL){
	    	printf("Es necesario ingresar un nombre para el restaurante, por favor intente nuevamente.\n");
	    	break;
	    }

		strcat(palabrasSeparadas[1],"\0");


		printf("El nombre que quiero procesar es: %s\n", palabrasSeparadas[1]);
		printf("La longitud del nombre a procesar es: %d\n", strlen(palabrasSeparadas[1]));

		//me trato de conectar con sindicato que deberia estar levantado esperando que le vaya a pedir la info
		socketCliente = establecer_conexion(ip_destino, puerto_destino);
		resultado_de_conexion(socketCliente, logger, "destino");

		obtener_restaurante* estructura = malloc(sizeof(uint32_t) + strlen(palabrasSeparadas[1]));
		estructura->largoNombreRestaurante = strlen(palabrasSeparadas[1]);
//		estructura->nombreRestaurante = malloc(estructura->largoNombreRestaurante+1);
		estructura->nombreRestaurante = palabrasSeparadas[1];

		//emision del mensaje para pedir la info, OBTENER_RESTAURANTE [nombreR]
		mandar_mensaje(estructura, OBTENER_RESTAURANTE, socketCliente);

		free(estructura);

		printf("pude mandar la solicitud de metadata a sindic.\n");


		/*
		//aca hago recibir_mensaje pero no haria nada interno mas que capaz loggear lo devuelto
		los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);
		*/

		break;


	//ojito ojeron este case reutiliza el serializar de OBTENER_RESTAURANTE y su estructura para mandar, es a propositoooo
	case CONSULTAR_PLATOS:

		if(palabrasSeparadas[1] == NULL){
			printf("Es necesario ingresar un nombre para el restaurante, por favor intente nuevamente.");
		break;
			    }

		strcat(palabrasSeparadas[1],"\0");

		printf("El restaurante al que le quiero consultar los platitos es: %s\n", palabrasSeparadas[1]);
		obtener_restaurante* estructuraAEnviar = malloc(sizeof(uint32_t) + strlen(palabrasSeparadas[1]));
		estructuraAEnviar->largoNombreRestaurante = strlen(palabrasSeparadas[1]);
		estructuraAEnviar->nombreRestaurante = malloc(estructuraAEnviar->largoNombreRestaurante+1);
		strcpy(estructuraAEnviar->nombreRestaurante, palabrasSeparadas[1]);

		socketCliente = establecer_conexion(ip_destino, puerto_destino);
		resultado_de_conexion(socketCliente, logger, "destino");

		//emision del mensaje para pedir la info, CONSULTAR_PLATOS [nombreR], algunos receptores usaran el [nombreR], otros no
		mandar_mensaje(estructuraAEnviar, CONSULTAR_PLATOS, socketCliente);

	    los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

	    if(exito == 1)
	    		{
	    	respuesta_consultar_platos* estructuraRespuestaConsultarPlatos = malloc(sizeAAllocar);
	    	recibir_mensaje(estructuraRespuestaConsultarPlatos, RESPUESTA_CONSULTAR_PLATOS, socketCliente);

            printf("Los platitos del restaurante consultado son: %s\n", estructuraRespuestaConsultarPlatos->nombresPlatos);

            free(estructuraRespuestaConsultarPlatos->nombresPlatos);
            free(estructuraRespuestaConsultarPlatos);

	    	} else {

	    	printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.");

	        }

        free(estructuraAEnviar->nombreRestaurante);
		free(estructuraAEnviar);
		close(socketCliente);
		break;

	case GUARDAR_PLATO:

			strcat(palabrasSeparadas[1],"\0"); //IMPORTANTISIMO
			strcat(palabrasSeparadas[3],"\0");

			estructuraRespuesta = malloc(sizeof(respuesta_ok_error));

			socketCliente = establecer_conexion(ip_destino , puerto_destino);
			resultado_de_conexion(socketCliente, logger, "destino");

			guardar_plato* elMensajeGuardarPlato = malloc(sizeof(uint32_t)*4 + strlen(palabrasSeparadas[1]) + strlen(palabrasSeparadas[3]));
			elMensajeGuardarPlato->nombreRestaurante = palabrasSeparadas[1];
			elMensajeGuardarPlato->largoNombreRestaurante = strlen(palabrasSeparadas[1]);
			elMensajeGuardarPlato->idPedido = atoi(palabrasSeparadas[2]);
			elMensajeGuardarPlato->nombrePlato = palabrasSeparadas[3];
			elMensajeGuardarPlato->largonombrePlato = strlen(palabrasSeparadas[3]);
			elMensajeGuardarPlato->cantidadPlatos = atoi(palabrasSeparadas[4]);

			mandar_mensaje(elMensajeGuardarPlato, GUARDAR_PLATO, socketCliente);

			los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

			if(exito == 1)
			{
				recibir_mensaje(estructuraRespuesta,RESPUESTA_GUARDAR_PLATO,socketCliente);

				printf("El intento de guardar un pedido fue: %s.\n", resultadoDeRespuesta(estructuraRespuesta->respuesta));
			}

			else
			{
				printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.\n");
			}

			//free(elMensajeGuardarPlato->nombreRestaurante); //ToDo revisar
			//free(elMensajeGuardarPlato->nombrePlato);
			free(elMensajeGuardarPlato);
			free(estructuraRespuesta);
			close(socketCliente);
			break;


	case A_PLATO:


		break;


	case PLATO_LISTO:


		break;


	case CREAR_PEDIDO:
		strcat(palabrasSeparadas[1],"\0");

		socketCliente = establecer_conexion(ip_destino , puerto_destino);
		resultado_de_conexion(socketCliente, logger, "destino");

		mandar_mensaje("vacio",CREAR_PEDIDO,socketCliente);

		los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

		if(exito == 1)
		{
			//respuesta_consultar_restaurantes* estructuraRespuestaConsultaRestaurantes = malloc(sizeof(respuesta_consultar_restaurantes));
			respuesta_crear_pedido* estructuraRespuestaCrearPedido = malloc(sizeAAllocar);

			recibir_mensaje(estructuraRespuestaCrearPedido,RESPUESTA_CREAR_PEDIDO,socketCliente);

			if(estructuraRespuestaCrearPedido->idPedido < 0){
				printf("Hubo un error al intentar crear tu pedido, compruebe que haya seleccionado un restaurante.\n");
			}
			else
			{
				printf("La creacion del pedido fue atendida, y su codigo corresponde a: %d", estructuraRespuestaCrearPedido->idPedido);
			}

			free(estructuraRespuestaCrearPedido);

		}
		else
		{
			printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.\n");
		}

		break;


    case GUARDAR_PEDIDO:
    	strcat(palabrasSeparadas[2],"\0"); //IMPORTANTISIMO

    	estructuraRespuesta = malloc(sizeof(respuesta_ok_error));

    	socketCliente = establecer_conexion(ip_destino , puerto_destino);
		resultado_de_conexion(socketCliente, logger, "destino");

		/* no entiendo para que declarar estas variables de mas, y los malloc extra
		uint32_t idPedido = atoi(palabrasSeparadas[1]);
		char* nombreRestaurante = malloc(strlen(palabrasSeparadas[2])+1);
		strcpy(nombreRestaurante, palabrasSeparadas[2]);
		*/

		guardar_pedido* elMensajeGuardarPedido = malloc(sizeof(uint32_t)*2 + strlen(palabrasSeparadas[2]));
		elMensajeGuardarPedido->idPedido = atoi(palabrasSeparadas[1]);
		elMensajeGuardarPedido->largoNombreRestaurante = strlen(palabrasSeparadas[2]);
		//elMensaje->nombreRestaurante = malloc(elMensaje->largoNombreRestaurante + 1);
		//strcpy(elMensaje->nombreRestaurante, palabrasSeparadas[2]);
		elMensajeGuardarPedido->nombreRestaurante = palabrasSeparadas[2];

		printf("ID: %u \n", elMensajeGuardarPedido->idPedido);
		printf("nombre: %s \n", elMensajeGuardarPedido->nombreRestaurante);

		//   ¿¿¿¿¿Crearia un hilo mas para mandar el socket junto con el id pedido, el nombre del resto y el tamanio????? entiendo que no
		//      pthread_t hiloMensaje;
		//      pthread_create(&hiloMensaje, NULL, mandar_mensaje ?), &socketCliente);
		//      pthread_detach(hiloMensaje);

		mandar_mensaje(elMensajeGuardarPedido, GUARDAR_PEDIDO, socketCliente);

		los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

		if(exito == 1)
		{
			recibir_mensaje(estructuraRespuesta,RESPUESTA_GUARDAR_PEDIDO,socketCliente);

			printf("El intento de guardar un pedido fue: %s.\n", resultadoDeRespuesta(estructuraRespuesta->respuesta));
		}

		else
		{
			printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.\n");
		}

		//free(elMensajeGuardarPedido->nombreRestaurante); //porfa no olvidarse de este free, tambien es importante, no solo liberar la estructura, sino nos va a caber
		free(elMensajeGuardarPedido);
		free(estructuraRespuesta);
		close(socketCliente); //siempre cerrar socket cuando se termina de usar
     	break;


    case CONFIRMAR_PEDIDO:


    	break;

    case CONSULTAR_PEDIDO:


    	break;

    case FINALIZAR_PEDIDO:


    	break;

    case TERMINAR_PEDIDO:


    	break;

    case OBTENER_RECETA:


    	break;


    default:
    	puts("Input no reconocida.\n");
    	break;
    }

    freeDeArray(palabrasSeparadas);
    free(lineaEntera);
    return;
}
