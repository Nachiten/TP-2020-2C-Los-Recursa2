#include "utilsRestaurante.h"



void obtenerMetadataRestaurante(){

	uint32_t socket_cliente;

	//me trato de conectar con sindicato que deberia estar levantado esperando que le vaya a pedir la info
    socket_cliente = establecer_conexion(ip_sindicato, puerto_sindicato);
    resultado_de_conexion(socket_cliente, logger, "destino");

    //emision del mensaje para pedir la info, OBTENER_RESTAURANTE [nombreR]
    mandar_mensaje(nombreRestaurante, OBTENER_RESTAURANTE, socket_cliente);

    respuesta_obtener_restaurante* estructuraRespuestaObtenerRestaurante = malloc(sizeof(respuesta_obtener_restaurante));

    //recepcion del choclo divino
    recibir_mensaje(estructuraRespuestaObtenerRestaurante, RESPUESTA_OBTENER_R, socket_cliente);


    miPosicionX = estructuraRespuestaObtenerRestaurante->posX;
    miPosicionY = estructuraRespuestaObtenerRestaurante->posY;
    cantHornos = estructuraRespuestaObtenerRestaurante->cantHornos;
    cantRecetas = estructuraRespuestaObtenerRestaurante->cantidadRecetas;
    cantCocineros = estructuraRespuestaObtenerRestaurante->cantidadCocineros;
    //ver como parsear la lista de cocineros y recetas


}



void inicializarRestaurante(){

	configuracion = leerConfiguracion("/home/utnso/workspace/tp-2020-2c-Los-Recursa2/configs/restaurante.config");
    LOG_PATH = config_get_string_value(configuracion,"LOG_FILE_PATH"); //cargo el path del archivo log
	ip_sindicato = config_get_string_value(configuracion,"IP_SINDICATO");
	puerto_sindicato = config_get_string_value(configuracion,"PUERTO_SINDICATO");
	puerto_local = config_get_string_value(configuracion,"PUERTO_ESCUCHA");
	ip_app = config_get_string_value(configuracion,"IP_APP");
	puerto_app = config_get_string_value(configuracion,"PUERTO_APP");
    nombreRestaurante = config_get_string_value(configuracion, "NOMBRE_RESTAURANTE");
    quantumElegido = config_get_int_value(configuracion, "QUANTUM");
    algoritmoElegido = config_get_string_value(configuracion, "ALGORITMO_PLANIFICACION");

    logger = cargarUnLog(LOG_PATH, "Cliente");

    //comunicarme con sindicato -> socket -> mensaje OBTENER_RESTAURANTE

    obtenerMetadataRestaurante();


}
