#include "cliente.h"

int main(){

	PIDCliente = getpid();
	socketEscucha = 0;

	ip_destino = malloc(15); //creo que no se puede pasar de esto
	puerto_destino = malloc(10); //10 para ir a lo seguro
	puerto_local = malloc(10); //10 para ir a lo seguro

	//Cargo las configuraciones del .config
	config = leerConfiguracion("/home/utnso/workspace/tp-2020-2c-Los-Recursa2/configs/cliente.config");
	LOG_PATH = config_get_string_value(config,"LOG_FILE_PATH"); //cargo el path del archivo log
	ip_destino = config_get_string_value(config,"IP_DESTINO");
	puerto_destino = config_get_string_value(config,"PUERTO_DESTINO");
	puerto_local = config_get_string_value(config,"PUERTO_LOCAL");
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

    char* lineaEntera = malloc(30); //con 30 caracteres estamos piola, no?
    lineaEntera = NULL;
	size_t longitud = 0;

	while(1)
	{
		printf("Inserte un comando:\n");
		getline(&lineaEntera, &longitud, stdin);

		string_trim(&lineaEntera);

		pthread_create(&hiloConsola, NULL,(void*)obtenerInputConsolaCliente, lineaEntera);
		//pthread_join(hiloConsola, NULL);
		pthread_detach(hiloConsola);

	}

	//ToDO meter los free que falten
	free(ip_destino);
	free(puerto_destino);
	free(puerto_local);

	//para cerrar el programa
	matarPrograma(logger, config, socketEscucha);

	return EXIT_SUCCESS;
}


//void obtenerInputConsolaCliente(t_conexion* tuplaConexion)
void obtenerInputConsolaCliente(char* lineaEntera)
{
	//char* lineaEntera = NULL;
	//size_t longitud = 0;
	uint32_t switcher;
	respuesta_ok_error* estructuraRespuesta;
	uint32_t socketCliente;
	int32_t iterador = 0;
	char* nombreRestaurante;

//	printf("Inserte un comando:\n");
//
//	getline(&lineaEntera, &longitud, stdin);
//
//	string_trim(&lineaEntera);

	char** palabrasSeparadas = string_split(lineaEntera , " ");

	// El nombre del comando es la primer palabra (POR EL MOMENTO CON GUIONES_BAJOS) -> EJ: CONSULTAR_RESTAURANTES
    char* comandoIngresado = palabrasSeparadas[0];

    //printf("El comando solicitado fue: %s. \n", comandoIngresado);

    switcher = valor_para_switch_case(comandoIngresado);


    //todo me parece que por cuestion de "prolijidad" y que despues sea mas facil encontrar cada CASE, lo ideal seria ir poniendolos en orden,
    //como estan declarados en codigo_operacion: consultar restaurante, seleccionar restaurante, obtener restaurante, etc...
    switch(switcher)
    {

	case CONSULTAR_RESTAURANTES:;
		respuesta_consultar_restaurantes* estructuraRespuestaConsultaRestaurantes = malloc(sizeof(respuesta_consultar_restaurantes));

		//socketCliente = establecer_conexion(tuplaConexion->ip_destino , tuplaConexion->puerto_destino);
		socketCliente = establecer_conexion(ip_destino , puerto_destino);
		resultado_de_conexion(socketCliente, logger, "destino");

		if(socketCliente < 1)
		{
			puts("estoy en el if");
			break;
		}



		mandar_mensaje("nadaxdxd", CONSULTAR_RESTAURANTES, socketCliente);

		recibir_mensaje(estructuraRespuestaConsultaRestaurantes,RESPUESTA_CONSULTAR_R,socketCliente);


		//ToDo hablar con Tomas sobre si esto funcionaria asi o no


		palabrasSeparadas = string_split(estructuraRespuestaConsultaRestaurantes->listaRestaurantes, ",");//falta agregar corchetes

		puts("Los restaurantes que se encuentran disponibles son:");

		while(iterador < estructuraRespuestaConsultaRestaurantes->cantRestaurantes)
		{
			printf("%s\n",palabrasSeparadas[iterador]);
		}


		free(estructuraRespuestaConsultaRestaurantes);
    	break;

    case SELECCIONAR_RESTAURANTE:
    	break;

	case OBTENER_RESTAURANTE:;

		strcat(palabrasSeparadas[1],"\0");

		printf("El nombre que quiero procesar es: %s\n", palabrasSeparadas[1]);
		printf("La longitud del nombre a procesar es: %d\n", strlen(palabrasSeparadas[1]));

		//me trato de conectar con sindicato que deberia estar levantado esperando que le vaya a pedir la info
		socketCliente = establecer_conexion(ip_destino, puerto_destino);
		resultado_de_conexion(socketCliente, logger, "destino");

		obtener_restaurante* estructura = malloc(sizeof(obtener_restaurante));
		estructura->largoNombreRestaurante = strlen(palabrasSeparadas[1]);
//		estructura->nombreRestaurante = malloc(estructura->largoNombreRestaurante+1);
		estructura->nombreRestaurante = palabrasSeparadas[1];

		//emision del mensaje para pedir la info, OBTENER_RESTAURANTE [nombreR]
		mandar_mensaje(estructura, OBTENER_RESTAURANTE, socketCliente);

		free(estructura->nombreRestaurante);
		free(estructura);



		printf("pude mandar la solicitud de metadata a sindic.\n");


		//aca hago recibir_mensaje pero no haria nada interno mas que capaz loggear lo devuelto

		break;

    //aca van mas Cases...

    case GUARDAR_PEDIDO:
    	estructuraRespuesta = malloc(sizeof(respuesta_ok_error));

    	socketCliente = establecer_conexion(ip_destino , puerto_destino);
		resultado_de_conexion(socketCliente, logger, "destino");

		/* no entiendo para que declarar estas variables de mas, y los malloc extra
		uint32_t idPedido = atoi(palabrasSeparadas[1]);
		char* nombreRestaurante = malloc(strlen(palabrasSeparadas[2])+1);
		strcpy(nombreRestaurante, palabrasSeparadas[2]);
		*/

		guardar_pedido* elMensaje = malloc(sizeof(guardar_pedido));
		elMensaje->idPedido = atoi(palabrasSeparadas[1]);
		elMensaje->largoNombreRestaurante = strlen(palabrasSeparadas[2]);
		elMensaje->nombreRestaurante = malloc(elMensaje->largoNombreRestaurante + 1);
		elMensaje->nombreRestaurante = palabrasSeparadas[2];

		//   ¿¿¿¿¿Crearia un hilo mas para mandar el socket junto con el id pedido, el nombre del resto y el tamanio????? entiendo que no
		//      pthread_t hiloMensaje;
		//      pthread_create(&hiloMensaje, NULL, mandar_mensaje ?), &socketCliente);
		//      pthread_detach(hiloMensaje);

		mandar_mensaje(elMensaje, GUARDAR_PEDIDO, socketCliente);

		recibir_mensaje(estructuraRespuesta,RESPUESTA_GUARDAR_PEDIDO,socketCliente);

		printf("El intento de guardar un pedido fue: %s.\n", resultadoDeRespuesta(estructuraRespuesta->respuesta));

		free(elMensaje->nombreRestaurante); //porfa no olvidarse de este free, tambien es importante, no solo liberar la estructura, sino nos va a caber
		free(elMensaje);
		free(estructuraRespuesta);
		close(socketCliente); //siempre cerrar socket cuando se termina de usar
      break;








    default:
    	puts("Input no reconocida.");
    	break;
    }
    freeDeArray(palabrasSeparadas);
}
