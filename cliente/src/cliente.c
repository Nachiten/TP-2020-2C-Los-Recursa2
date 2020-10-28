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
    	log_trace(logger, "El servidor destino al que me intento conectar no esta vivo, procedo a fallecer.");
    	exit(0);
    }
/*
    handshake* elHandshake = malloc(sizeof(handshake));
    elHandshake->longitudIDCliente = strlen(idCliente)+1;
    elHandshake->id = malloc(elHandshake->longitudIDCliente);
    strcpy(elHandshake->id, idCliente);

    mandar_mensaje(elHandshake, HANDSHAKE, socketCliente);
*/
    close(socketCliente);

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
		pthread_create(&hiloConsola, NULL,(void*)obtenerInputConsolaCliente, &socketCliente);
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


void obtenerInputConsolaCliente(int32_t* socketClienteInicial){
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
    Obtener Restaurante -> Check and tested (!)
    Consultar Platos -> Check and tested
    Guardar Plato -> Check and tested (!)
    Aniadir Plato -> Check
    Plato listo ->
    Crear Pedido -> Check
    Guardar Pedido -> Check and tested (!)
    Confirmar Pedido ->
    Consultar Pedido ->
    Obtener Pedido ->
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

			log_trace(logger, "Los restaurantes que se encuentran disponibles son: ");

			while(iterador < estructuraRespuestaConsultaRestaurantes->cantRestaurantes)
			{
				log_trace(logger,"%s",palabrasSeparadas[iterador]);
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
			recibir_mensaje(estructuraRespuesta,RESPUESTA_GUARDAR_PLATO, socketCliente);

			log_trace(logger, "El intento de seleccionar el restaurante %s fue: %s.\n", estructuraSeleccRestaur->nombreRestaurante, resultadoDeRespuesta(estructuraRespuesta->respuesta));
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
		estructuraObtenerRestaurante->largoNombreRestaurante = strlen(palabrasSeparadas[1])+1;
		estructuraObtenerRestaurante->nombreRestaurante = malloc(estructuraObtenerRestaurante->largoNombreRestaurante);
		strcpy(estructuraObtenerRestaurante->nombreRestaurante, palabrasSeparadas[1]);

		//emision del mensaje para pedir la info, OBTENER_RESTAURANTE [nombreR]
		mandar_mensaje(estructuraObtenerRestaurante, OBTENER_RESTAURANTE, socketCliente);
		los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

	    if(exito == 1)
		{

		respuesta_obtener_restaurante* estructuraRespuestaObtenerRestaurante = malloc(sizeAAllocar);
		recibir_mensaje(estructuraRespuestaObtenerRestaurante, RESPUESTA_OBTENER_REST, socketCliente);

		log_trace(logger, "Los platos del restaurante < %s > son: %s\n", estructuraObtenerRestaurante->nombreRestaurante, estructuraRespuestaObtenerRestaurante->platos);
		log_trace(logger, "La cantidad de cocineros del restaurante < %s > son: %d\n", estructuraObtenerRestaurante->nombreRestaurante, estructuraRespuestaObtenerRestaurante->cantidadCocineros);

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
		estructuraAEnviar->largoNombreRestaurante = strlen(palabrasSeparadas[1])+1;
		estructuraAEnviar->nombreRestaurante = malloc(estructuraAEnviar->largoNombreRestaurante+1);
		strcpy(estructuraAEnviar->nombreRestaurante, palabrasSeparadas[1]);

		//emision del mensaje para pedir la info, CONSULTAR_PLATOS [nombreR], algunos receptores usaran el [nombreR], otros no
		mandar_mensaje(estructuraAEnviar, CONSULTAR_PLATOS, socketCliente);

	    los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

	    if(exito == 1)
	    		{
		respuesta_consultar_platos* estructuraRespuestaConsultarPlatos = malloc(sizeAAllocar);
		recibir_mensaje(estructuraRespuestaConsultarPlatos, RESPUESTA_CONSULTAR_PLATOS, socketCliente);

		log_trace(logger, "Los platos del restaurante < %s > consultado son: %s\n", estructuraAEnviar->nombreRestaurante, estructuraRespuestaConsultarPlatos->nombresPlatos);

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

			log_trace(logger, "El intento de guardar un pedido fue: %s.\n", resultadoDeRespuesta(estructuraRespuesta->respuesta));
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

		if(palabrasSeparadas[1] == NULL || palabrasSeparadas[2] == NULL){
			printf("Se requiere ingresar el nombre de un plato y el ID de un pedido para operar.\n");
			break;
		}

		strcat(palabrasSeparadas[1],"\0");

		estructuraRespuesta = malloc(sizeof(respuesta_ok_error));
		socketCliente = establecer_conexion(ip_destino , puerto_destino);
		resultado_de_conexion(socketCliente, logger, "destino");

		a_plato* mensajeAniadirPlato = malloc(sizeof(a_plato));
		mensajeAniadirPlato->largoNombrePlato = strlen(palabrasSeparadas[1])+1;
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
    	if(palabrasSeparadas[1] == NULL || palabrasSeparadas[2] == NULL){
			printf("Se requiere ingresar el nombre de un restaurante y el ID de un pedido para operar.\n");
			break;
		}

		strcat(palabrasSeparadas[1],"\0");

		socketCliente = establecer_conexion(ip_destino , puerto_destino);
		resultado_de_conexion(socketCliente, logger, "destino");

		estructuraRespuesta = malloc(sizeof(respuesta_ok_error));


		finalizar_pedido* mensajeFinalizarPedido = malloc(sizeof(finalizar_pedido));
		mensajeFinalizarPedido->largoNombreRestaurante = strlen(palabrasSeparadas[1])+1;
		mensajeFinalizarPedido->nombreRestaurante = malloc(mensajeFinalizarPedido->largoNombreRestaurante);
		strcpy(mensajeFinalizarPedido->nombreRestaurante, palabrasSeparadas[1]);
		mensajeFinalizarPedido->idPedido = atoi(palabrasSeparadas[2]);

		mandar_mensaje(mensajeFinalizarPedido, FINALIZAR_PEDIDO, socketCliente);
		los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

		if(exito == 1)
		{
			recibir_mensaje(estructuraRespuesta, RESPUESTA_FINALIZAR_PEDIDO ,socketCliente);
			log_trace(logger, "El intento de agregar un plato a un pedido fue: %s.\n", resultadoDeRespuesta(estructuraRespuesta->respuesta));
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

		estructuraRespuesta = malloc(sizeof(respuesta_ok_error));
		socketCliente = establecer_conexion(ip_destino , puerto_destino);
		resultado_de_conexion(socketCliente, logger, "destino");

		finalizar_pedido* mensajeTerminarPedido = malloc(sizeof(finalizar_pedido));
		mensajeTerminarPedido->largoNombreRestaurante = strlen(palabrasSeparadas[1])+1;
		mensajeTerminarPedido->nombreRestaurante = malloc(mensajeTerminarPedido->largoNombreRestaurante);
		strcpy(mensajeTerminarPedido->nombreRestaurante, palabrasSeparadas[1]);
		mensajeTerminarPedido->idPedido = atoi(palabrasSeparadas[2]);

		mandar_mensaje(mensajeTerminarPedido, TERMINAR_PEDIDO, socketCliente);
		los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

		if(exito == 1)
		{
			recibir_mensaje(estructuraRespuesta, RESPUESTA_TERMINAR_PEDIDO ,socketCliente);
			printf("El intento de agregar un plato a un pedido fue: %s.\n", resultadoDeRespuesta(estructuraRespuesta->respuesta));
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
        mensajeObtenerReceta->largoNombreReceta = strlen(palabrasSeparadas[1])+1;
        mensajeObtenerReceta->nombreReceta = malloc(mensajeObtenerReceta->largoNombreReceta);
        strcpy(mensajeObtenerReceta->nombreReceta, palabrasSeparadas[1]);


        mandar_mensaje(mensajeObtenerReceta, OBTENER_RECETA, socketCliente);
        los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

        if(exito == 1)
		{
        	respuesta_obtener_receta* estructuraRespuestaObtenerReceta = malloc(sizeAAllocar);
			recibir_mensaje(estructuraRespuestaObtenerReceta, RESPUESTA_OBTENER_RECETA ,socketCliente);
			printf("Los pasos para cocinar el plato solicitado son: %s\n", estructuraRespuestaObtenerReceta->pasos);
			printf("Y sus tiempos:%s \n", estructuraRespuestaObtenerReceta->tiempoPasos);
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

    case HANDSHAKE:


    	break;


    case DESCONEXION:

    	break;

    default:
    	puts("Input no reconocida.\n");
    	break;
    }

    freeDeArray(palabrasSeparadas);
    free(lineaEntera);
    return;
}
