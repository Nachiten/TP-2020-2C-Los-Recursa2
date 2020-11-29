#include "cliente.h"

int main(int cantArg, char* argumentos[]){

	char* pathConfig;

	// Si no hay ningun parametro del path de config genero valor default
	if (cantArg < 2){
		printf("El path de la config no está especificado, tomando valor default.\nValor default: /home/utnso/workspace/tp-2020-2c-Los-Recursa2/configs/cliente.config\n");
		pathConfig = "/home/utnso/workspace/tp-2020-2c-Los-Recursa2/configs/cliente.config";
	// Si hay un parametro asumo que es el path de la config y lo uso
	} else {
		pathConfig = argumentos[1];
	}

	//PIDCliente = getpid();
	socketEscucha = 0;

	ip_destino = malloc(15); //creo que no se puede pasar de esto
	puerto_destino = malloc(10); //10 para ir a lo seguro
	puerto_local = malloc(10); //10 para ir a lo seguro
	//puerto_app = malloc(10);
	//ip_app = malloc(15);

	//Cargo las configuraciones del .config
	config = leerConfiguracion(pathConfig);

	if (config == NULL){
		printf("ERROR | El path de la config especificado no es valido.\n");
		exit(2);
	}

	idCliente = config_get_string_value(config, "ID_CLIENTE");
	LOG_PATH = config_get_string_value(config,"LOG_FILE_PATH"); //cargo el path del archivo log
	ip_destino = config_get_string_value(config,"IP_DESTINO");
	puerto_destino = config_get_string_value(config,"PUERTO_DESTINO");
	puerto_local = config_get_string_value(config,"PUERTO_LOCAL");
	//puerto_APP = config_get_string_value(config,"PUERTO_APP");
	miPosicionX = config_get_int_value(config, "POSICION_X");
	miPosicionY = config_get_int_value(config, "POSICION_Y");

	//Dejo cargado un logger para loguear los eventos.
	logger = cargarUnLogDeCliente(LOG_PATH, "Cliente");

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

	semLog = malloc(sizeof(sem_t));
	sem_init(semLog,0,1);

	socketDelHandshake = establecer_conexion(ip_destino , puerto_destino);
    resultado_de_conexion(socketDelHandshake, logger, "destino");
    if(socketDelHandshake < 1){
    	sem_wait(semLog);
    	log_info(logger, "El servidor destino al que me intento conectar no esta vivo, procedo a fallecer.");
    	sem_post(semLog);
    	exit(0);
    }

    handshake* elHandshake = malloc(sizeof(handshake));
    elHandshake->longitudIDCliente = strlen(idCliente);
    elHandshake->posX = miPosicionX;
    elHandshake->posY = miPosicionY;
    elHandshake->id = malloc(elHandshake->longitudIDCliente+1);
    strcpy(elHandshake->id, idCliente);

    mandar_mensaje(elHandshake, HANDSHAKE, socketDelHandshake);

/*
    pthread_create(&hiloNotificaciones, NULL, (void*)recibirNotificaciones, &socketCliente);
    pthread_detach(hiloNotificaciones);
*/
	while(1)
	{
		sem_wait(comandoParaEjecutar);
		pthread_create(&hiloConsola, NULL,(void*)obtenerInputConsolaCliente, NULL);
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

/*
void recibirNotificaciones(int32_t* socketInicial){
	int32_t sizeAAllocar = 0;
	uint32_t exito = 0;

	while(1){

		los_recv_repetitivos(*socketInicial, &exito, &sizeAAllocar);

		if(exito == 1){

			guardar_pedido* notificacionPedidoFinalizado = malloc(sizeAAllocar);
	        recibir_mensaje(notificacionPedidoFinalizado, FINALIZAR_PEDIDO ,*socketInicial);
	        respuesta_ok_error* respuestaNotificacion = malloc(sizeof(respuesta_ok_error));

	        if(notificacionPedidoFinalizado->idPedido > 0 && notificacionPedidoFinalizado->largoNombreRestaurante>0){
	        	respuestaNotificacion->respuesta = 1;
	        	mandar_mensaje(respuestaNotificacion, RESPUESTA_FINALIZAR_PEDIDO, *socketInicial);
	        	sem_wait(semLog);
	        	log_info(logger, "El pedido nro <%d> del restaurante <%s> ha arribado.\n", notificacionPedidoFinalizado->idPedido, notificacionPedidoFinalizado->nombreRestaurante);
	            sem_post(semLog);
	        } else {
	        	respuestaNotificacion->respuesta = 0;
	            mandar_mensaje(respuestaNotificacion, RESPUESTA_FINALIZAR_PEDIDO, *socketInicial);
	        }


			free(notificacionPedidoFinalizado->nombreRestaurante);
			free(notificacionPedidoFinalizado);
			free(respuestaNotificacion);

		}

		else
		{
			printf("Ocurrió un error al intentar recibir la notificacion de finalizacion de un pedido.\n");
		}

		}

	}
*/


void obtenerInputConsolaCliente(){
	char* lineaEntera = NULL;
	uint32_t switcher;
	respuesta_ok_error* estructuraRespuesta;
    int32_t socketCliente;
	int32_t sizeAAllocar = 0;
	uint32_t exito = 0;
	size_t longitud = 0;

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

	char** palabrasSeparadas = string_split(lineaEntera , " ");

	// El nombre del comando es la primer palabra (POR EL MOMENTO CON GUIONES_BAJOS) -> EJ: CONSULTAR_RESTAURANTES
    char* comandoIngresado = malloc(strlen(palabrasSeparadas[0])+1);
    strcpy(comandoIngresado,palabrasSeparadas[0]);
    //printf("El comando solicitado fue: %s. \n", comandoIngresado);
    sem_post(comandoParaEjecutar);
    switcher = valor_para_switch_case(comandoIngresado);



    //todo me parece que por cuestion de "prolijidad" y que despues sea mas facil encontrar cada CASE, lo ideal seria ir poniendolos en orden,
    //como estan declarados en codigo_operacion: consultar restaurante, seleccionar restaurante, obtener restaurante, etc...

    //estoy de acuerdo pez

    /*
    CASES LABURADOS:

    Consultar Restaurantes -> Check and tested
    Seleccionar Restaurante -> Check and tested
    Obtener Restaurante -> Check and tested
    Consultar Platos -> Check and tested (Ver temas parametros)
    Guardar Plato -> Check and tested
    Aniadir Plato -> Check and tested
    Plato listo -> Check and tested
    Crear Pedido -> Check
    Guardar Pedido -> Check and tested
    Confirmar Pedido -> Check
    Consultar Pedido -> Check and tested
    Obtener Pedido -> Check and tested
    Finalizar Pedido -> Check and tested
    Terminar Pedido -> Check
    Obtener Receta -> Check and tested
    */

    switch(switcher)
    {

	case CONSULTAR_RESTAURANTES:

	    socketCliente = establecer_conexion(ip_destino, puerto_destino);
	    resultado_de_conexion(socketCliente, logger, "destino");

		mandar_mensaje(" ", CONSULTAR_RESTAURANTES, socketCliente);

		los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

		if(exito == 1)
		{
			respuesta_consultar_restaurantes* estructuraRespuestaConsultaRestaurantes = malloc(sizeAAllocar);

			recibir_mensaje(estructuraRespuestaConsultaRestaurantes,RESPUESTA_CONSULTAR_R,socketCliente);

			//sem_wait(semLog);
			log_info(logger, "Los restaurantes que se encuentran disponibles son: %s" ,
					estructuraRespuestaConsultaRestaurantes->listaRestaurantes);
			//sem_post(semLog);

            free(estructuraRespuestaConsultaRestaurantes->listaRestaurantes);
			free(estructuraRespuestaConsultaRestaurantes);
		}
		else
		{
			printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.\n");
		}
		close(socketCliente);
    	break;

    case SELECCIONAR_RESTAURANTE:

    	if(palabrasSeparadas[1] == NULL){
    		printf("El formato correcto es: SELECCIONAR_RESTAURANTE [nombreRest].\n");
    		break;
    	}
    	strcat(palabrasSeparadas[1],"\0");

    	estructuraRespuesta = malloc(sizeof(respuesta_ok_error));

		socketCliente = establecer_conexion(ip_destino, puerto_destino);
		resultado_de_conexion(socketCliente, logger, "destino");

		seleccionar_restaurante* estructuraSeleccRestaur = malloc(sizeof(seleccionar_restaurante));
		estructuraSeleccRestaur->largoIDCliente = strlen(idCliente);
		estructuraSeleccRestaur->idCliente = malloc(strlen(idCliente)+1);
		strcpy(estructuraSeleccRestaur->idCliente, idCliente);
		estructuraSeleccRestaur->largoNombreRestaurante = strlen(palabrasSeparadas[1]);
		estructuraSeleccRestaur->nombreRestaurante = malloc(strlen(palabrasSeparadas[1])+1);
		strcpy(estructuraSeleccRestaur->nombreRestaurante, palabrasSeparadas[1]);
		mandar_mensaje(estructuraSeleccRestaur, SELECCIONAR_RESTAURANTE, socketCliente);
		los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

		if(exito == 1) {
			recibir_mensaje(estructuraRespuesta,RESPUESTA_SELECCIONAR_R, socketCliente);
			sem_wait(semLog);
			log_info(logger, "El intento de seleccionar el restaurante %s fue: %s.\n",
					estructuraSeleccRestaur->nombreRestaurante,
					resultadoDeRespuesta(estructuraRespuesta->respuesta));
			sem_post(semLog);
		} else {
			printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.\n");
		}
		free(estructuraSeleccRestaur->idCliente);
		free(estructuraSeleccRestaur->nombreRestaurante);
		free(estructuraSeleccRestaur);
		free(estructuraRespuesta);
		close(socketCliente);
    	break;

	case OBTENER_RESTAURANTE:

		if(palabrasSeparadas[1] == NULL){
			printf("El formato correcto es: OBTENER_RESTAURANTE [nombreRest].\n");
		    break;
		}

		strcat(palabrasSeparadas[1],"\0");

		socketCliente = establecer_conexion(ip_destino, puerto_destino);
     	resultado_de_conexion(socketCliente, logger, "destino");

		obtener_restaurante* estructuraObtenerRestaurante = malloc(sizeof(obtener_restaurante));
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
		sem_wait(semLog);
		log_info(logger, "Los platos del restaurante < %s > son: %s\n", estructuraObtenerRestaurante->nombreRestaurante, estructuraRespuestaObtenerRestaurante->platos);
		log_info(logger, "La cantidad de cocineros del restaurante < %s > son: %d\n", estructuraObtenerRestaurante->nombreRestaurante, estructuraRespuestaObtenerRestaurante->cantidadCocineros);
		sem_post(semLog);
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
			printf("El formato correcto es: CONSULTAR_PLATOS [nombreRest].\n");
			break;
		}

		strcat(palabrasSeparadas[1],"\0");

		socketCliente = establecer_conexion(ip_destino, puerto_destino);
		resultado_de_conexion(socketCliente, logger, "destino");

		consultar_platos* estructuraAEnviar = malloc(sizeof(uint32_t) + strlen(palabrasSeparadas[1]));
		estructuraAEnviar->sizeNombre = strlen(palabrasSeparadas[1]);
		estructuraAEnviar->nombreResto = malloc(estructuraAEnviar->sizeNombre+1);
		strcpy(estructuraAEnviar->nombreResto, palabrasSeparadas[1]);
		estructuraAEnviar->sizeId = strlen(idCliente);
		estructuraAEnviar->id = malloc(estructuraAEnviar->sizeId+1);
		strcpy(estructuraAEnviar->id, idCliente);

		//emision del mensaje para pedir la info, CONSULTAR_PLATOS [nombreR], algunos receptores usaran el [nombreR], otros no
		mandar_mensaje(estructuraAEnviar, CONSULTAR_PLATOS, socketCliente);

	    los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

	    if(exito == 1)
	    		{
		respuesta_consultar_platos* estructuraRespuestaConsultarPlatos = malloc(sizeAAllocar);
		recibir_mensaje(estructuraRespuestaConsultarPlatos, RESPUESTA_CONSULTAR_PLATOS, socketCliente);
		sem_wait(semLog);
		log_info(logger, "Los platos del restaurante < %s > consultado son: %s\n", estructuraAEnviar->nombreResto, estructuraRespuestaConsultarPlatos->nombresPlatos);
		sem_post(semLog);
		//printf("Los platos del restaurante < %s > consultado son: %s\n", estructuraAEnviar->nombreRestaurante, estructuraRespuestaConsultarPlatos->nombresPlatos);

		free(estructuraRespuestaConsultarPlatos->nombresPlatos);
		free(estructuraRespuestaConsultarPlatos);

		} else {

		printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.\n");

		}

        free(estructuraAEnviar->nombreResto);
        free(estructuraAEnviar->id);
		free(estructuraAEnviar);
		close(socketCliente);
		break;

	case GUARDAR_PLATO:

		if(palabrasSeparadas[1] == NULL || palabrasSeparadas[2] == NULL || palabrasSeparadas[3] == NULL || palabrasSeparadas[4] == NULL){
		    printf("El formato correcto es: GUARDAR_PLATO [nombreRest] [idPedido] [nombrePlato] [cantidadPlato].\n");
		    break;
		}

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
			sem_wait(semLog);
			log_info(logger, "El intento de guardar un plato en un pedido fue: %s.\n", resultadoDeRespuesta(estructuraRespuesta->respuesta));
			sem_post(semLog);
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
			printf("El formato correcto es: ANIADIR_PLATO [nombrePlato] [idPedido].\n");
			break;
		}

		strcat(palabrasSeparadas[1],"\0");

		estructuraRespuesta = malloc(sizeof(respuesta_ok_error));
		socketCliente = establecer_conexion(ip_destino , puerto_destino);
		resultado_de_conexion(socketCliente, logger, "destino");

		a_plato* mensajeAniadirPlato = malloc(sizeof(a_plato));
		mensajeAniadirPlato->largoNombrePlato = strlen(palabrasSeparadas[1]);
		mensajeAniadirPlato->nombrePlato = malloc(mensajeAniadirPlato->largoNombrePlato+1);
		strcpy(mensajeAniadirPlato->nombrePlato, palabrasSeparadas[1]);
		mensajeAniadirPlato->idPedido = atoi(palabrasSeparadas[2]);

		mandar_mensaje(mensajeAniadirPlato, A_PLATO, socketCliente);
		los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

		if(exito == 1)
		{
			recibir_mensaje(estructuraRespuesta,RESPUESTA_A_PLATO,socketCliente);
			sem_wait(semLog);
			log_info(logger, "El intento de agregar un plato a un pedido fue: %s.\n", resultadoDeRespuesta(estructuraRespuesta->respuesta));
			sem_post(semLog);
		} else {
			printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.\n");
		}

		free(mensajeAniadirPlato->nombrePlato);
		free(mensajeAniadirPlato);
		free(estructuraRespuesta);
		close(socketCliente);
		break;


	case PLATO_LISTO:

		if(palabrasSeparadas[1] == NULL || palabrasSeparadas[2] == NULL || palabrasSeparadas[3] == NULL ){
			printf("El formato correcto es: PLATO_LISTO [nombreRest] [idPedido] [nombrePlato].\n");
			break;
		}
		estructuraRespuesta = malloc(sizeof(respuesta_ok_error));
		socketCliente = establecer_conexion(ip_destino , puerto_destino);
		resultado_de_conexion(socketCliente, logger, "destino");

		plato_listo* mensajePlatoListo = malloc(sizeof(plato_listo));
		mensajePlatoListo->largoNombreRestaurante = strlen(palabrasSeparadas[1]);
		mensajePlatoListo->nombreRestaurante = malloc(mensajePlatoListo->largoNombreRestaurante+1);
		strcpy(mensajePlatoListo->nombreRestaurante, palabrasSeparadas[1]);
		mensajePlatoListo->idPedido = atoi(palabrasSeparadas[2]);
		mensajePlatoListo->largoNombrePlato = strlen(palabrasSeparadas[3]);
		mensajePlatoListo->nombrePlato = malloc(mensajePlatoListo->largoNombrePlato+1);
		strcpy(mensajePlatoListo->nombrePlato, palabrasSeparadas[3]);

		mandar_mensaje(mensajePlatoListo, PLATO_LISTO, socketCliente);
		los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

		if(exito == 1)
		{
			recibir_mensaje(estructuraRespuesta,RESPUESTA_PLATO_LISTO,socketCliente);
			sem_wait(semLog);
			log_info(logger, "La respuesta a notificar el plato listo fue: %s.\n", resultadoDeRespuesta(estructuraRespuesta->respuesta));
			sem_post(semLog);
		} else {
			printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.\n");
		}
		free(mensajePlatoListo->nombreRestaurante);
		free(mensajePlatoListo->nombrePlato);
		free(mensajePlatoListo);
		free(estructuraRespuesta);
		close(socketCliente);

		break;


	case CREAR_PEDIDO:

		socketCliente = establecer_conexion(ip_destino , puerto_destino);

		crear_pedido* mensajeCrearPedido = malloc(sizeof(crear_pedido));
		mensajeCrearPedido->sizeId = strlen(idCliente);
		mensajeCrearPedido->id = malloc(mensajeCrearPedido->sizeId+1);
		strcpy(mensajeCrearPedido->id,idCliente);

		mandar_mensaje(mensajeCrearPedido,CREAR_PEDIDO, socketCliente);

		los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);
		if(exito == 1)
		{
			respuesta_crear_pedido* estructuraRespuestaCrearPedido = malloc(sizeAAllocar);

			recibir_mensaje(estructuraRespuestaCrearPedido,RESPUESTA_CREAR_PEDIDO,socketCliente);

			if(estructuraRespuestaCrearPedido->idPedido == 0){
				printf("Hubo un error al intentar crear tu pedido.\n");
			}
			else
			{
				sem_wait(semLog);
				log_info(logger, "La creacion del pedido fue atendida, y su codigo corresponde a: %d",
						estructuraRespuestaCrearPedido->idPedido);
				sem_post(semLog);
			}

			free(estructuraRespuestaCrearPedido);
		}
		else
		{
			printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.\n");
		}

		free(mensajeCrearPedido->id);
		free(mensajeCrearPedido);
		close(socketCliente);

		break;


    case GUARDAR_PEDIDO:

    	if(palabrasSeparadas[1] == NULL || palabrasSeparadas[2] == NULL){
			printf("El formato correcto es: GUARDAR_PEDIDO [nombreRest] [idPedido].\n");
			break;
		}

    	strcat(palabrasSeparadas[1],"\0"); //IMPORTANTISIMO

    	estructuraRespuesta = malloc(sizeof(respuesta_ok_error));

    	socketCliente = establecer_conexion(ip_destino , puerto_destino);

		guardar_pedido* elMensajeGuardarPedido = malloc(sizeof(guardar_pedido));
		elMensajeGuardarPedido->idPedido = atoi(palabrasSeparadas[2]);
		elMensajeGuardarPedido->largoNombreRestaurante = strlen(palabrasSeparadas[1]);
		elMensajeGuardarPedido->nombreRestaurante = malloc(elMensajeGuardarPedido->largoNombreRestaurante+1);
		strcpy(elMensajeGuardarPedido->nombreRestaurante, palabrasSeparadas[1]);

		//printf("ID: %u \n", elMensajeGuardarPedido->idPedido);
		//printf("nombre: %s \n", elMensajeGuardarPedido->nombreRestaurante);

    	mandar_mensaje(elMensajeGuardarPedido, GUARDAR_PEDIDO, socketCliente);

		los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

		if(exito == 1)
		{
			recibir_mensaje(estructuraRespuesta,RESPUESTA_GUARDAR_PEDIDO,socketCliente);
			sem_wait(semLog);
			log_info(logger, "El intento de guardar un pedido fue: %s.\n", resultadoDeRespuesta(estructuraRespuesta->respuesta));
			sem_post(semLog);
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

     	//ToDo testear esto!!!
    case CONFIRMAR_PEDIDO:

    	if(palabrasSeparadas[1] == NULL || palabrasSeparadas[2] == NULL){
			printf("El formato correcto es: CONFIRMAR_PEDIDO [nombreRest] [idPedido].\n");
			break;
    	}
    	strcat(palabrasSeparadas[1],"\0"); //IMPORTANTISIMO

		estructuraRespuesta = malloc(sizeof(respuesta_ok_error));
		plato_listo* recibirPlatoListoExclusivo;
		finalizar_pedido* recibirFinalizarPedidoExclusivo;
		codigo_operacion cod_op_exclusivo = PLATO_LISTO;
		confirmar_pedido* elMensajeConfirmarPedido = malloc(sizeof(guardar_pedido));

		//establezco la conexion para empezar a mandar
		socketCliente = establecer_conexion(ip_destino , puerto_destino);

		//cargo los datos de la confirmacion
		elMensajeConfirmarPedido->idPedido = atoi(palabrasSeparadas[2]);
		elMensajeConfirmarPedido->largoNombreRestaurante = strlen(palabrasSeparadas[1]);
		elMensajeConfirmarPedido->nombreRestaurante = malloc(elMensajeConfirmarPedido->largoNombreRestaurante+1);
		strcpy(elMensajeConfirmarPedido->nombreRestaurante, palabrasSeparadas[1]);

		//mando el mensaje de confirmar pedido
		mandar_mensaje(elMensajeConfirmarPedido, CONFIRMAR_PEDIDO, socketCliente);

		//y espero su respuesta para ponerme a esperar los PLATO LISTO y FINALIZAR PEDIDO
		los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);

		//me llego una respuesta de confirmar pedido
		if(exito == 1)
		{
			recibir_mensaje(estructuraRespuesta,RESPUESTA_CONFIRMAR_PEDIDO,socketCliente);

			sem_wait(semLog);
			log_info(logger, "El intento de confirmar un pedido fue: %s.\n", resultadoDeRespuesta(estructuraRespuesta->respuesta));
			sem_post(semLog);

			//mientras que no sea el flujo directo a CoMAnda, me pongo a esperar todos los mensajes
			if(strcmp(puerto_destino, "5002") != 0)
			{
				//se confirmó el pedido, asi que a esperar todos los avisos de cosas listas
				if(estructuraRespuesta->respuesta == 1)
				{
					//a continuacion, el bucle de los PLATO LISTO
					los_recv_repetitivos_VERSIONESPECIAL(socketCliente, &exito, &sizeAAllocar, &cod_op_exclusivo);
					while(cod_op_exclusivo != FINALIZAR_PEDIDO)
					{
						if(exito == 1)
						{
							if(cod_op_exclusivo == PLATO_LISTO)
							{
								recibirPlatoListoExclusivo = malloc(sizeof(sizeAAllocar));
								recibir_mensaje(recibirPlatoListoExclusivo, PLATO_LISTO , socketCliente);

								sem_wait(semLog);
								log_info(logger, "Esta listo el plato <%s> del restaurante <%s> (Pedido %u).\n", recibirPlatoListoExclusivo->nombrePlato, recibirPlatoListoExclusivo->nombreRestaurante, recibirPlatoListoExclusivo->idPedido);
								sem_post(semLog);

								//aviso que me llego el plato listo
								estructuraRespuesta->respuesta = 1;
								mandar_mensaje(estructuraRespuesta, RESPUESTA_PLATO_LISTO, socketCliente);

								free(recibirPlatoListoExclusivo->nombrePlato);
								free(recibirPlatoListoExclusivo->nombreRestaurante);
								free(recibirPlatoListoExclusivo);
							}
						}
						//y sigo recibiendo
						los_recv_repetitivos_VERSIONESPECIAL(socketCliente, &exito, &sizeAAllocar, &cod_op_exclusivo);
					}

					//me llego un FINALIZAR_PEDIDO por fin
					recibirFinalizarPedidoExclusivo= malloc(sizeof(sizeAAllocar));

					recibir_mensaje(recibirFinalizarPedidoExclusivo, FINALIZAR_PEDIDO, socketCliente);

					sem_wait(semLog);
					log_info(logger, "Esta Finalizado el Pedido %u del restaurante <%s>.\n", recibirFinalizarPedidoExclusivo->nombreRestaurante, recibirFinalizarPedidoExclusivo->idPedido);
					sem_post(semLog);

					estructuraRespuesta->respuesta = 1;
					mandar_mensaje(estructuraRespuesta, RESPUESTA_FINALIZAR_PEDIDO, socketCliente);

					free(recibirFinalizarPedidoExclusivo->nombreRestaurante);
					free(recibirFinalizarPedidoExclusivo);
				}
			}
		}

		//a la mierda tod0, ni me aceptaron el confirmar pedido
		else
		{
			puts("Falló la recepción de la respuesta de confirmar pedido.");
		}

		free(elMensajeConfirmarPedido->nombreRestaurante); //porfa no olvidarse de este free, tambien es importante, no solo liberar la estructura, sino nos va a caber
		free(elMensajeConfirmarPedido);
		free(estructuraRespuesta);
		close(socketCliente); //siempre cerrar socket cuando se termina de usar
    	break;


    case CONSULTAR_PEDIDO:

    	if(palabrasSeparadas[1] == NULL){
			printf("El formato correcto es: CONSULTAR_PEDIDO [idPedido].\n");
			break;
		}

    	socketCliente = establecer_conexion(ip_destino , puerto_destino);
    	resultado_de_conexion(socketCliente, logger, "destino");

    	consultar_pedido* estructuraConsultarPedido = malloc(sizeof(uint32_t));
    	estructuraConsultarPedido->idPedido = atoi(palabrasSeparadas[1]);

    	mandar_mensaje(estructuraConsultarPedido, CONSULTAR_PEDIDO, socketCliente);
    	los_recv_repetitivos(socketCliente, &exito, &sizeAAllocar);
    	if(exito == 1)
    	{
    		respuesta_consultar_pedido* estructuraRespuestaConsultarPedido = malloc(sizeAAllocar);
    		recibir_mensaje(estructuraRespuestaConsultarPedido, RESPUESTA_CONSULTAR_PEDIDO, socketCliente);

    		//re piola
    		mostrar_el_estado_del_pedido_consultar_pedido(estructuraConsultarPedido, estructuraRespuestaConsultarPedido, logger, semLog);

			/*
    		sem_wait(semLog);
    		log_info(logger, "El pedido < %d > del restaurante < %s >, trajo los campos:\nRepartidor: %s\nEstado: %d\nComidas: %s"
    			,estructuraConsultarPedido->idPedido, estructuraRespuestaConsultarPedido->nombreRestaurante
				,estructuraRespuestaConsultarPedido->estado
				,estructuraRespuestaConsultarPedido->comidas);
    		sem_post(semLog);
    		*/

    		free(estructuraRespuestaConsultarPedido->nombreRestaurante);
    		free(estructuraRespuestaConsultarPedido->comidas);
    		free(estructuraRespuestaConsultarPedido->cantTotales);
    		free(estructuraRespuestaConsultarPedido->cantListas);
    	} else {
    		printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.\n");
    	}

    	free(estructuraConsultarPedido);
        close(socketCliente);

    	break;

    case OBTENER_PEDIDO:
    	if(palabrasSeparadas[1] == NULL || palabrasSeparadas[2] == NULL)
    	{
			printf("El formato correcto es: OBTENER_PEDIDO [nombreRest] [idPedido]");
			break;
		}
    	strcat(palabrasSeparadas[1],"\0");

    	socketCliente = establecer_conexion(ip_destino , puerto_destino);
		resultado_de_conexion(socketCliente, logger, "destino");

		guardar_pedido* mensajeObtenerPedido = malloc(sizeof(guardar_pedido));
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

				//re piola
				mostrar_el_estado_del_pedido_obtener_pedido(mensajeObtenerPedido, respuestaObtenerPedido, logger, semLog);

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

    	break;


    case FINALIZAR_PEDIDO:
    	if(palabrasSeparadas[1] == NULL || palabrasSeparadas[2] == NULL){
			printf("El formato correcto es: FINALIZAR_PEDIDO [nombreRest] [idPedido].\n");
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
			sem_wait(semLog);
			log_info(logger, "El intento de Finalizar un Pedido fue: %s.\n", resultadoDeRespuesta(estructuraRespuesta->respuesta));
			sem_post(semLog);
		} else {
			printf("Ocurrió un error al intentar recibir la respuesta de este mensaje.\n");
		}

		free(mensajeFinalizarPedido->nombreRestaurante);
		free(mensajeFinalizarPedido);
		free(estructuraRespuesta);
		close(socketCliente);

		break;

    case TERMINAR_PEDIDO:
    	if(palabrasSeparadas[1] == NULL || palabrasSeparadas[2] == NULL){
    		printf("El formato correcto es: TERMINAR_PEDIDO [nombreRest] [idPedido].\n");
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
			sem_wait(semLog);
			log_info(logger, "El intento de agregar un plato a un pedido fue: %s.\n", resultadoDeRespuesta(estructuraRespuesta->respuesta));
			sem_post(semLog);
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
    		printf("El formato correcto es: OBTENER_RECETA [nombreReceta].\n");
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
			sem_wait(semLog);
			log_info(logger, "Los pasos para cocinar el plato < %s > son: %s, con sus tiempos: %s", mensajeObtenerReceta->nombreReceta, estructuraRespuestaObtenerReceta->pasos, estructuraRespuestaObtenerReceta->tiempoPasos);
			sem_post(semLog);
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
    	printf("Se ha ingresado un comando no reconocido. \n");
    	break;
    }

    freeDeArray(palabrasSeparadas);
    free(lineaEntera);
    return;
}
