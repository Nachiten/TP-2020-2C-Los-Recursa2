#include "funcionesCliente.h"

//devuelve el valor en entero en base al string adoptado en consola para el switch (modulo CLIENTE)

uint32_t valor_para_switch_case(char* input) {

	uint32_t switcher;

	//CONSULTAR_RESTAURANTES
	if (strcmp(input,"CONSULTAR_RESTAURANTES") == 0)
	{
		switcher = CONSULTAR_RESTAURANTES;
	}

	//SELECCIONAR_RESTAURANTE [nombreRESTAURANTE]
	if (strcmp(input,"SELECCIONAR_RESTAURANTE") == 0)
	{
		switcher = SELECCIONAR_RESTAURANTE;
	}

	//OBTENER_RESTAURANTE [nombreRESTAURANTE]
	if (strcmp(input,"OBTENER_RESTAURANTE") == 0)
	{
		switcher = OBTENER_RESTAURANTE;
	}

	//CONSULTAR_PLATOS [nombreRESTAURANTE]
	if (strcmp(input,"CONSULTAR_PLATOS") == 0)
	{
		switcher = CONSULTAR_PLATOS;
	}

	//GUARDAR_PLATO [idPEDIDO]
	if (strcmp(input,"GUARDAR_PLATO") == 0)
	{
		switcher = GUARDAR_PLATO;
	}

	//AGREGAR_PLATO [idPEDIDO] [nombreRESTAURANTE] [nombrePLATO] [cantidadPLATO]
	if (strcmp(input,"AGREGAR_PLATO") == 0)
	{
		switcher = A_PLATO;
	}

	//PLATO_LISTO [idPEDIDO] [nombreRESTAURANTE] [nombrePLATO]
	if (strcmp(input,"PLATO_LISTO") == 0)
	{
		switcher = PLATO_LISTO;
	}

	//CREAR_PEDIDO (se hace luego del SELECCIONAR_RESTAURANTE si hablas con app o bien al restaurante de una)
	if (strcmp(input,"CREAR_PEDIDO") == 0)
	{
		switcher = CREAR_PEDIDO;
	}

	//GUARDAR_PEDIDO [idPEDIDO] [nombreRESTAURANTE]
	if (strcmp(input,"GUARDAR_PEDIDO") == 0)
	{
		switcher = GUARDAR_PEDIDO;
	}

	//CONFIRMAR_PEDIDO [idPEDIDO] [nombreRESTAURANTE]
	if (strcmp(input,"CONFIRMAR_PEDIDO") == 0)
	{
		switcher = CONFIRMAR_PEDIDO;
	}

	//CONSULTAR_PEDIDO [idPEDIDO]
	if (strcmp(input,"CONSULTAR_PEDIDO") == 0)
	{
		switcher = CONSULTAR_PEDIDO;
	}

	//OBTENER_PEDIDO [idPEDIDO] [nombreRESTAURANTE]
	if (strcmp(input,"OBTENER_PEDIDO") == 0)
	{
		switcher = OBTENER_PEDIDO;
	}

	//FINALIZAR_PEDIDO [idPEDIDO] [nombreRESTAURANTE]
	if (strcmp(input,"FINALIZAR_PEDIDO") == 0)
	{
		switcher = FINALIZAR_PEDIDO;
	}

	return switcher;
}

// Cuenta la cantidad de elementos en un array
int cantidadDeElementosEnArray(char** array){
	int i = 0;
	while(array[i] != NULL && strcmp(array[i], "\n") != 0){
		i++;
	}
	return i;
}

// Hacer free de un array con cosas
void freeDeArray(char** array){
	int cantidadElementosArray = cantidadDeElementosEnArray(array);

	int i;

	for (i = cantidadElementosArray; i>= 0; i--){
		free(array[i]);
	}

	free(array);
}

void obtenerInputConsolaCliente(t_conexion* tuplaConexion)
{
	char* lineaEntera = NULL;
	size_t longitud = 0;
	uint32_t switcher;
	respuesta_ok_error* estructuraRespuesta;
	uint32_t socketCliente;
	int32_t iterador = 0;

	printf("Inserte un comando:\n");

	getline(&lineaEntera, &longitud, stdin);

	string_trim(&lineaEntera);

	char** palabrasSeparadas = string_split(lineaEntera , " ");

	// El nombre del comando es la primer palabra (POR EL MOMENTO CON GUIONES_BAJOS) -> EJ: CONSULTAR_RESTAURANTES
    char* comandoIngresado = palabrasSeparadas[0];

    printf("El comando solicitado fue: %s. \n", comandoIngresado);

    switcher = valor_para_switch_case(comandoIngresado);


    //todo me parece que por cuestion de "prolijidad" y que despues sea mas facil encontrar cada CASE, lo ideal seria ir poniendolos en orden,
    //como estan declarados en codigo_operacion: consultar restaurante, seleccionar restaurante, obtener restaurante, etc...
    switch(switcher)
    {

	case CONSULTAR_RESTAURANTES:;
		respuesta_consultar_restaurantes* estructuraRespuestaConsultaRestaurantes = malloc(sizeof(respuesta_consultar_restaurantes));

		socketCliente = establecer_conexion(tuplaConexion->ip_destino , tuplaConexion->puerto_destino);
		resultado_de_conexion(socketCliente, tuplaConexion->mi_logger, "destino");

		mandar_mensaje("nadaxdxd", CONSULTAR_RESTAURANTES, socketCliente);

		recibir_mensaje(estructuraRespuestaConsultaRestaurantes,RESPUESTA_CONSULTAR_R,socketCliente);


		//ToDo hablar con Tomas sobre si esto funcionaria asi o no

		/*
		palabrasSeparadas = string_split(estructuraRespuestaConsultaRestaurantes->listaRestaurantes, ",");

		puts("Los restaurantes que se encuentran disponibles son:");

		while(iterador < estructuraRespuestaConsultaRestaurantes->cantRestaurantes)
		{
			printf("%s\n",palabrasSeparadas[iterador]);
		}
		*/

		free(estructuraRespuestaConsultaRestaurantes);
    	break;

    case SELECCIONAR_RESTAURANTE:
    	break;

	case OBTENER_RESTAURANTE:
		break;

    //aca van mas Cases...

    case GUARDAR_PEDIDO:
    	estructuraRespuesta = malloc(sizeof(respuesta_ok_error));

		socketCliente = establecer_conexion(tuplaConexion->ip_destino , tuplaConexion->puerto_destino);
		resultado_de_conexion(socketCliente, tuplaConexion->mi_logger, "destino");

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
