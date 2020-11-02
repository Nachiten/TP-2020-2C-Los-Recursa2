#include "cliente.h"

int main(){

	//PIDCliente = getpid();
	socketEscucha = 0;
	int32_t socketCliente;
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

	socketCliente = establecer_conexion(ip_destino , puerto_destino);
    resultado_de_conexion(socketCliente, logger, "destino");
    if(socketCliente < 1){
    	log_info(logger, "El servidor destino al que me intento conectar no esta vivo, procedo a fallecer.");
    	exit(0);
    }
/*
    handshake* elHandshake = malloc(sizeof(handshake));
    elHandshake->longitudIDCliente = strlen(idCliente);
    elHandshake->posX = miPosicionX;
    elHandshake->posY = miPosicionY;
    elHandshake->id = malloc(elHandshake->longitudIDCliente+1);
    strcpy(elHandshake->id, idCliente);

    mandar_mensaje(elHandshake, HANDSHAKE, socketCliente);
*/

    pthread_create(&hiloNotificaciones, NULL, (void*)recibirNotificaciones, &socketCliente);
    pthread_detach(hiloNotificaciones);
    //close(socketCliente);

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

void recibirNotificaciones(int32_t* socketInicial){
	int32_t sizeAAllocar = 0;
	uint32_t exito = 0;

	while(1){

		los_recv_repetitivos(*socketInicial, &exito, &sizeAAllocar);

		if(exito == 1){

			guardar_pedido* notificacionPedidoFinalizado = malloc(sizeAAllocar);
	        recibir_mensaje(notificacionPedidoFinalizado, FINALIZAR_PEDIDO ,*socketInicial);


			printf("El pedido nro <%d> del restaurante <%s> ha arribado.\n", notificacionPedidoFinalizado->idPedido, notificacionPedidoFinalizado->nombreRestaurante);
			free(notificacionPedidoFinalizado->nombreRestaurante);
			free(notificacionPedidoFinalizado);
		}

		else
		{
			printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.\n");
		}

		}

	}



void obtenerInputConsolaCliente(){
	//char* lineaEntera = NULL;
	//size_t longitud = 0;
	uint32_t switcher;
	respuesta_ok_error* estructuraRespuesta;
    int32_t socketCliente;
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

    //estoy de acuerdo pez

    /*
    CASES LABURADOS:

    Consultar Restaurantes -> Check
    Seleccionar Restaurante -> Check
    Obtener Restaurante -> Check and tested
    Consultar Platos -> Check and tested
    Guardar Plato -> Check and tested
    Aniadir Plato -> Check
    Plato listo ->
    Crear Pedido -> Check
    Guardar Pedido -> Check and tested
    Confirmar Pedido ->
    Consultar Pedido ->
    Obtener Pedido -> Check
    Finalizar Pedido -> Check
    Terminar Pedido -> Check
    Obtener Receta -> Check
    */

    switch(switcher)
    {

	case CONSULTAR_RESTAURANTES:;

	    socketCliente = establecer_conexion(ip_destino, puerto_destino);
	    resultado_de_conexion(socketCliente, logger, "destino");

		mandar_mensaje("nadaxdxd", CONSULTAR_RESTAURANTES, socketCliente);

		los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

		if(exito == 1)
		{
			//respuesta_consultar_restaurantes* estructuraRespuestaConsultaRestaurantes = malloc(sizeof(respuesta_consultar_restaurantes));
			respuesta_consultar_restaurantes* estructuraRespuestaConsultaRestaurantes = malloc(sizeAAllocar);

			recibir_mensaje(estructuraRespuestaConsultaRestaurantes,RESPUESTA_CONSULTAR_R,socketCliente);

			//ToDo hablar con Tomas sobre si esto funcionaria asi o no


			palabrasSeparadas = string_split(estructuraRespuestaConsultaRestaurantes->listaRestaurantes, ",");//falta agregar corchetes

			log_info(logger, "Los restaurantes que se encuentran disponibles son: ");

			while(iterador < estructuraRespuestaConsultaRestaurantes->cantRestaurantes)
			{
				log_info(logger,"%s",palabrasSeparadas[iterador]);
			}

			free(estructuraRespuestaConsultaRestaurantes);
		}

		else
		{
			printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.\n");
		}

    	break;

    case SELECCIONAR_RESTAURANTE:

    	if(palabrasSeparadas[1] == NULL){
    		printf("Se requiere el nombre de un restaurante para operar.\n");
    	}

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
			recibir_mensaje(estructuraRespuesta,RESPUESTA_GUARDAR_PLATO, socketCliente);

			log_info(logger, "El intento de seleccionar el restaurante %s fue: %s.\n", estructuraSeleccRestaur->nombreRestaurante, resultadoDeRespuesta(estructuraRespuesta->respuesta));
		}

		else
		{
			printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.\n");
		}

		free(estructuraSeleccRestaur);//entiendo que no hace falta liberar nombre de restaurante ni id cliente porque solo fueron asignaciones
		free(estructuraRespuesta);
		close(socketCliente);
    	break;

	case OBTENER_RESTAURANTE:

	    if(palabrasSeparadas[1] == NULL){
	    	printf("Es necesario ingresar un nombre para el restaurante, por favor intente nuevamente.\n");
	    	break;
	    }

		strcat(palabrasSeparadas[1],"\0");

		socketCliente = establecer_conexion(ip_destino, puerto_destino);
     	resultado_de_conexion(socketCliente, logger, "destino");

		obtener_restaurante* estructuraObtenerRestaurante = malloc(sizeof(uint32_t) + strlen(palabrasSeparadas[1]));
		estructuraObtenerRestaurante->largoNombreRestaurante = strlen(palabrasSeparadas[1]);
		estructuraObtenerRestaurante->nombreRestaurante = malloc(estructuraObtenerRestaurante->largoNombreRestaurante+1);
		strcpy(estructuraObtenerRestaurante->nombreRestaurante, palabrasSeparadas[1]);

		//emision del mensaje para pedir la info, OBTENER_RESTAURANTE [nombreR]
		mandar_mensaje(estructuraObtenerRestaurante, OBTENER_RESTAURANTE, socketCliente);
		los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

	    if(exito == 1)
		{

		respuesta_obtener_restaurante* estructuraRespuestaObtenerRestaurante = malloc(sizeAAllocar);
		recibir_mensaje(estructuraRespuestaObtenerRestaurante, RESPUESTA_OBTENER_REST, socketCliente);

		log_info(logger, "Los platos del restaurante < %s > son: %s\n", estructuraObtenerRestaurante->nombreRestaurante, estructuraRespuestaObtenerRestaurante->platos);
		log_info(logger, "La cantidad de cocineros del restaurante < %s > son: %d\n", estructuraObtenerRestaurante->nombreRestaurante, estructuraRespuestaObtenerRestaurante->cantidadCocineros);

		free(estructuraRespuestaObtenerRestaurante->platos);
		free(estructuraRespuestaObtenerRestaurante->afinidades);
		free(estructuraRespuestaObtenerRestaurante);

		} else {

		printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.\n");

		}
	    free(estructuraObtenerRestaurante->nombreRestaurante);
	    free(estructuraObtenerRestaurante);
	    close(socketCliente);
		break;


	//ojito ojeron este case reutiliza el serializar de OBTENER_RESTAURANTE y su estructura para mandar, es a propositoooo
	case CONSULTAR_PLATOS:

		if(palabrasSeparadas[1] == NULL){
			printf("Es necesario ingresar un nombre para el restaurante, por favor intente nuevamente.");
		    break;
	    }

		strcat(palabrasSeparadas[1],"\0");

		socketCliente = establecer_conexion(ip_destino, puerto_destino);
		resultado_de_conexion(socketCliente, logger, "destino");

		obtener_restaurante* estructuraAEnviar = malloc(sizeof(uint32_t) + strlen(palabrasSeparadas[1]));
		estructuraAEnviar->largoNombreRestaurante = strlen(palabrasSeparadas[1]);
		estructuraAEnviar->nombreRestaurante = malloc(estructuraAEnviar->largoNombreRestaurante+1);
		strcpy(estructuraAEnviar->nombreRestaurante, palabrasSeparadas[1]);

		//emision del mensaje para pedir la info, CONSULTAR_PLATOS [nombreR], algunos receptores usaran el [nombreR], otros no
		mandar_mensaje(estructuraAEnviar, CONSULTAR_PLATOS, socketCliente);

	    los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

	    if(exito == 1)
	    		{
		respuesta_consultar_platos* estructuraRespuestaConsultarPlatos = malloc(sizeAAllocar);
		recibir_mensaje(estructuraRespuestaConsultarPlatos, RESPUESTA_CONSULTAR_PLATOS, socketCliente);

		log_info(logger, "Los platos del restaurante < %s > consultado son: %s\n", estructuraAEnviar->nombreRestaurante, estructuraRespuestaConsultarPlatos->nombresPlatos);

		free(estructuraRespuestaConsultarPlatos->nombresPlatos);
		free(estructuraRespuestaConsultarPlatos);

		} else {

		printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.\n");

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
		elMensajeGuardarPlato->largoNombreRestaurante = strlen(palabrasSeparadas[1]);
		elMensajeGuardarPlato->nombreRestaurante = malloc(elMensajeGuardarPlato->largoNombreRestaurante+1);
		strcpy(elMensajeGuardarPlato->nombreRestaurante, palabrasSeparadas[1]);
		elMensajeGuardarPlato->idPedido = atoi(palabrasSeparadas[2]);
		elMensajeGuardarPlato->largoNombrePlato = strlen(palabrasSeparadas[3]);
		elMensajeGuardarPlato->nombrePlato = malloc(elMensajeGuardarPlato->largoNombrePlato+1);
		strcpy(elMensajeGuardarPlato->nombrePlato, palabrasSeparadas[3]);
		elMensajeGuardarPlato->cantidadPlatos = atoi(palabrasSeparadas[4]);

		mandar_mensaje(elMensajeGuardarPlato, GUARDAR_PLATO, socketCliente);

		los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

		if(exito == 1)
		{
			recibir_mensaje(estructuraRespuesta,RESPUESTA_GUARDAR_PLATO,socketCliente);

			log_info(logger, "El intento de guardar un pedido fue: %s.\n", resultadoDeRespuesta(estructuraRespuesta->respuesta));
		}

		else
		{
			printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.\n");
		}

		free(elMensajeGuardarPlato->nombreRestaurante);
		free(elMensajeGuardarPlato->nombrePlato);
		free(elMensajeGuardarPlato);
		free(estructuraRespuesta);
		close(socketCliente);
		break;


	case A_PLATO:

		if(palabrasSeparadas[1] == NULL || palabrasSeparadas[2] == NULL){
			printf("Se requiere ingresar el nombre de un plato y el ID de un pedido para operar.\n");
			break;
		}

		strcat(palabrasSeparadas[1],"\0");

		estructuraRespuesta = malloc(sizeof(respuesta_ok_error));
		socketCliente = establecer_conexion(ip_destino , puerto_destino);
		resultado_de_conexion(socketCliente, logger, "destino");

		a_plato* mensajeAniadirPlato = malloc(sizeof(a_plato));
		mensajeAniadirPlato->largoNombrePlato = strlen(palabrasSeparadas[1]);
		mensajeAniadirPlato->nombrePlato = malloc(mensajeAniadirPlato->largoNombrePlato);
		strcpy(mensajeAniadirPlato->nombrePlato, palabrasSeparadas[1]);
		mensajeAniadirPlato->idPedido = atoi(palabrasSeparadas[2]);

		mandar_mensaje(mensajeAniadirPlato, A_PLATO, socketCliente);
		los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

		if(exito == 1)
		{
			recibir_mensaje(estructuraRespuesta,RESPUESTA_A_PLATO,socketCliente);
			printf("El intento de agregar un plato a un pedido fue: %s.\n", resultadoDeRespuesta(estructuraRespuesta->respuesta));
		} else {
			printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.\n");
		}

		free(mensajeAniadirPlato->nombrePlato);
		free(mensajeAniadirPlato);
		free(estructuraRespuesta);
		close(socketCliente);

		break;


	case PLATO_LISTO:


		break;


	case CREAR_PEDIDO:
		strcat(palabrasSeparadas[1],"\0");

		socketCliente = establecer_conexion(ip_destino , puerto_destino);

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

		/* no entiendo para que declarar estas variables de mas, y los malloc extra
		uint32_t idPedido = atoi(palabrasSeparadas[1]);
		char* nombreRestaurante = malloc(strlen(palabrasSeparadas[2])+1);
		strcpy(nombreRestaurante, palabrasSeparadas[2]);
		*/

		guardar_pedido* elMensajeGuardarPedido = malloc(sizeof(uint32_t)*2 + strlen(palabrasSeparadas[2]));
		elMensajeGuardarPedido->idPedido = atoi(palabrasSeparadas[1]);
		elMensajeGuardarPedido->largoNombreRestaurante = strlen(palabrasSeparadas[2]);
		elMensajeGuardarPedido->nombreRestaurante = malloc(elMensajeGuardarPedido->largoNombreRestaurante);
		strcpy(elMensajeGuardarPedido->nombreRestaurante, palabrasSeparadas[2]);

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

//mando msj y recibo respuesta ok/fail

    	break;

    case CONSULTAR_PEDIDO:
    	if(palabrasSeparadas[1] == NULL){
			printf("Se requiere ingresar el numero de un pedido para operar.\n");
			break;
		}
    	socketCliente = establecer_conexion(ip_destino , puerto_destino);
    	resultado_de_conexion(socketCliente, logger, "destino");

    	respuesta_crear_pedido* estructuraConsultarPedido = malloc(sizeof(uint32_t));
    	estructuraConsultarPedido->idPedido = atoi(palabrasSeparadas[1]);

    	mandar_mensaje(estructuraConsultarPedido, CONSULTAR_PEDIDO, socketCliente);
    	los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);
    	if(exito == 1)
    	{
    		/*
    		respuesta_consultar_pedido* estructuraRespuestaConsultarPedido = malloc(sizeAAllocar);
    		recibir_mensaje(estructuraRespuesta, RESPUESTA_CONSULTAR_PEDIDO, socketCliente);
    		*/
    	} else
    	{
    		printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.\n");
    	}

    	break;

    case OBTENER_PEDIDO:
    	if(palabrasSeparadas[1] == NULL || palabrasSeparadas[2] == NULL)
    	{
			puts("El formato correcto para este comando es: OBTENER_PEDIDO [nombre de Restaurante] [ID de Pedido]");
			break;
		}

    	else
    	{
    		socketCliente = establecer_conexion(ip_destino , puerto_destino);
			resultado_de_conexion(socketCliente, logger, "destino");

			obtener_pedido* mensajeObtenerPedido = malloc(sizeof(obtener_pedido));
			mensajeObtenerPedido->largoNombreRestaurante = strlen(palabrasSeparadas[1]);
			mensajeObtenerPedido->nombreRestaurante = malloc(mensajeObtenerPedido->largoNombreRestaurante + 1);
			strcpy(mensajeObtenerPedido->nombreRestaurante, palabrasSeparadas[1]);
			mensajeObtenerPedido->idPedido = atoi(palabrasSeparadas[2]);

			mandar_mensaje(mensajeObtenerPedido, OBTENER_PEDIDO , socketCliente);
			los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

			if(exito == 1)
			{
				respuesta_obtener_pedido* respuestaObtenerPedido = malloc(sizeAAllocar);
				recibir_mensaje(respuestaObtenerPedido, RESPUESTA_OBTENER_PEDIDO ,socketCliente);

				if(strcmp(respuestaObtenerPedido->comidas,"[]") != 0)
				{
					log_info(logger, "Obtuve del pedido %u la/s siguientes comida/s: %s. Su/s cantidad/es: %s. Cantidad ya cocinada: %s"
					,mensajeObtenerPedido->idPedido, respuestaObtenerPedido->comidas, respuestaObtenerPedido->cantTotales, respuestaObtenerPedido->cantListas);
				}

				else
				{
					printf("No existe el pedido %u del restaurante %s.\n",mensajeObtenerPedido->idPedido, mensajeObtenerPedido->nombreRestaurante);
				}

				free(respuestaObtenerPedido->comidas);
				free(respuestaObtenerPedido->cantTotales);
				free(respuestaObtenerPedido->cantListas);
				free(respuestaObtenerPedido);
			} else
			{
				printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.\n");
			}

			free(mensajeObtenerPedido->nombreRestaurante);
			free(mensajeObtenerPedido);
			close(socketCliente);
    	}


    	break;


    case FINALIZAR_PEDIDO:
    	if(palabrasSeparadas[1] == NULL || palabrasSeparadas[2] == NULL){
			printf("Se requiere ingresar el nombre de un restaurante y el ID de un pedido para operar.\n");
			break;
		}

		strcat(palabrasSeparadas[1],"\0");

		socketCliente = establecer_conexion(ip_destino , puerto_destino);
		resultado_de_conexion(socketCliente, logger, "destino");

		guardar_pedido* mensajeFinalizarPedido = malloc(sizeof(guardar_pedido));
		mensajeFinalizarPedido->largoNombreRestaurante = strlen(palabrasSeparadas[1]);
		mensajeFinalizarPedido->nombreRestaurante = malloc(mensajeFinalizarPedido->largoNombreRestaurante);
		strcpy(mensajeFinalizarPedido->nombreRestaurante, palabrasSeparadas[1]);
		mensajeFinalizarPedido->idPedido = atoi(palabrasSeparadas[2]);

		mandar_mensaje(mensajeFinalizarPedido, FINALIZAR_PEDIDO, socketCliente);
		los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

		estructuraRespuesta = malloc(sizeof(respuesta_ok_error));

		if(exito == 1)
		{
			recibir_mensaje(estructuraRespuesta, RESPUESTA_FINALIZAR_PEDIDO ,socketCliente);
			log_info(logger, "El intento de agregar un plato a un pedido fue: %s.\n", resultadoDeRespuesta(estructuraRespuesta->respuesta));
		} else
		{
			printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.\n");
		}

		free(mensajeFinalizarPedido->nombreRestaurante);
		free(mensajeFinalizarPedido);
		free(estructuraRespuesta);
		close(socketCliente);

		break;

    case TERMINAR_PEDIDO:
    	if(palabrasSeparadas[1] == NULL || palabrasSeparadas[2] == NULL){
			printf("Se requiere ingresar el nombre de un restaurante y el ID de un pedido para operar.\n");
			break;
		}

		strcat(palabrasSeparadas[1],"\0");


		socketCliente = establecer_conexion(ip_destino , puerto_destino);
		resultado_de_conexion(socketCliente, logger, "destino");

		guardar_pedido* mensajeTerminarPedido = malloc(sizeof(guardar_pedido));
		mensajeTerminarPedido->largoNombreRestaurante = strlen(palabrasSeparadas[1]);
		mensajeTerminarPedido->nombreRestaurante = malloc(mensajeTerminarPedido->largoNombreRestaurante);
		strcpy(mensajeTerminarPedido->nombreRestaurante, palabrasSeparadas[1]);
		mensajeTerminarPedido->idPedido = atoi(palabrasSeparadas[2]);

		mandar_mensaje(mensajeTerminarPedido, TERMINAR_PEDIDO, socketCliente);
		los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

		estructuraRespuesta = malloc(sizeof(respuesta_ok_error));

		if(exito == 1)
		{
			recibir_mensaje(estructuraRespuesta, RESPUESTA_TERMINAR_PEDIDO ,socketCliente);
			log_info(logger, "El intento de agregar un plato a un pedido fue: %s.\n", resultadoDeRespuesta(estructuraRespuesta->respuesta));
		} else {
			printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.\n");
		}

		free(mensajeTerminarPedido->nombreRestaurante);
		free(mensajeTerminarPedido);
		free(estructuraRespuesta);
		close(socketCliente);

		break;

    case OBTENER_RECETA:
    	if(palabrasSeparadas[1] == NULL){
			printf("Se requiere ingresar el nombre de una receta para poder operar.\n");
			break;
		}

    	socketCliente = establecer_conexion(ip_destino , puerto_destino);
    	resultado_de_conexion(socketCliente, logger, "destino");


    	strcat(palabrasSeparadas[1],"\0");
        obtener_receta* mensajeObtenerReceta = malloc(sizeof(obtener_receta));
        mensajeObtenerReceta->largoNombreReceta = strlen(palabrasSeparadas[1]);
        mensajeObtenerReceta->nombreReceta = malloc(mensajeObtenerReceta->largoNombreReceta);
        strcpy(mensajeObtenerReceta->nombreReceta, palabrasSeparadas[1]);


        mandar_mensaje(mensajeObtenerReceta, OBTENER_RECETA, socketCliente);
        los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

        if(exito == 1)
		{
        	respuesta_obtener_receta* estructuraRespuestaObtenerReceta = malloc(sizeAAllocar);
			recibir_mensaje(estructuraRespuestaObtenerReceta, RESPUESTA_OBTENER_RECETA ,socketCliente);
			log_info(logger, "Los pasos para cocinar el plato < %s > son: %s, con sus tiempos:%s", mensajeObtenerReceta->nombreReceta, estructuraRespuestaObtenerReceta->pasos, estructuraRespuestaObtenerReceta->tiempoPasos);
			free(estructuraRespuestaObtenerReceta->pasos);
			free(estructuraRespuestaObtenerReceta->tiempoPasos);
			free(estructuraRespuestaObtenerReceta);
		} else {
			printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.\n");
		}
        free(mensajeObtenerReceta->nombreReceta);
        free(mensajeObtenerReceta);
        close(socketCliente);

    	break;


    case DESCONEXION:

    	break;

    default:
    	printf("Se ha ingresado un comando no reconocido.");
    	break;
    }

    freeDeArray(palabrasSeparadas);
    free(lineaEntera);
    return;
}
