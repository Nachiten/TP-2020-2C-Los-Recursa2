#include "utilsRestaurante.h"



void obtenerMetadataRestaurante(){

	uint32_t socket_cliente;

	//me trato de conectar con sindicato que deberia estar levantado esperando que le vaya a pedir la info
    socket_cliente = establecer_conexion(ip_sindicato, puerto_sindicato);
    resultado_de_conexion(socket_cliente, logger, "destino");

    obtener_restaurante* estructura = malloc(sizeof(obtener_restaurante));
    estructura->largoNombreRestaurante = strlen(nombreRestaurante);
    estructura->nombreRestaurante = malloc(estructura->largoNombreRestaurante+1);
    estructura->nombreRestaurante = nombreRestaurante;

    printf("El nombre rancio que estoy por mandar es: %s \n", estructura->nombreRestaurante);

    //emision del mensaje para pedir la info, OBTENER_RESTAURANTE [nombreR]
    mandar_mensaje(estructura, OBTENER_RESTAURANTE, socket_cliente);

    free(estructura->nombreRestaurante);
    free(estructura);

    printf("pude mandar la solicitud de metadata a sindic.\n");

    //recibo el codigo de operacion, ya se que va a ser RESPUESTA_OBTENER_R
    codigo_operacion codigoRecibido;
    bytesRecibidos(recv(socket_cliente, &codigoRecibido, sizeof(codigo_operacion), MSG_WAITALL));

    printf("El codigo recibido del emisor es: %d", codigoRecibido);

    uint32_t sizePayload;
    bytesRecibidos(recv(socket_cliente, &sizePayload, sizeof(uint32_t), MSG_WAITALL));

    printf("El size del buffer/payload para la metadata es: %u", sizePayload);

    respuesta_obtener_restaurante* estructuraRespuestaObtenerRestaurante = malloc(sizePayload);

   //recepcion del choclo divino
    recibir_mensaje(estructuraRespuestaObtenerRestaurante, RESPUESTA_OBTENER_R, socket_cliente);

    printf("pude recibir toda la de metadata de sindic.\n");


    //trabajo interno con la metadata recibida
    miPosicionX = estructuraRespuestaObtenerRestaurante->posX;
    miPosicionY = estructuraRespuestaObtenerRestaurante->posY;
    cantHornos = estructuraRespuestaObtenerRestaurante->cantHornos;
    cantCocineros = estructuraRespuestaObtenerRestaurante->cantidadCocineros;

    printf("Voy a tener %d cocineros/cpus. \n", cantCocineros);
    printf("Los platos que ofrece el restaurante son: %s \n", estructuraRespuestaObtenerRestaurante->platos);
    printf("Las afinidades de los cocineros son: %s \n", estructuraRespuestaObtenerRestaurante->afinidades);


    //aca pasan cosas en el medio

    free(estructuraRespuestaObtenerRestaurante->platos);
    free(estructuraRespuestaObtenerRestaurante->afinidades);
    free(estructuraRespuestaObtenerRestaurante->precioPlatos);
    free(estructuraRespuestaObtenerRestaurante);
}

//void crearColasPlanificacion(){
//
//	for(int i=0; i<cantCocineros; i++){
//
//
//		}
//
//}


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
    log_info(logger, "Obtuve de config el nombre: %s" , nombreRestaurante);

    //comunicarme con sindicato -> socket -> mensaje OBTENER_RESTAURANTE

    obtenerMetadataRestaurante();

//    crearColasPlanificacion();


}



