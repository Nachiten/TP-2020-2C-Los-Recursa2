#include "utilsRestaurante.h"



void obtenerMetadataRestaurante(){

	//me trato de conectar con sindicato que deberia estar levantado esperando que le vaya a pedir la info
	socket_sindicato = establecer_conexion(ip_sindicato, puerto_sindicato);
    resultado_de_conexion(socket_sindicato, logger, "destino");

    obtener_restaurante* estructura = malloc(sizeof(obtener_restaurante));
    estructura->largoNombreRestaurante = strlen(nombreRestaurante);
    estructura->nombreRestaurante = malloc(estructura->largoNombreRestaurante+1);
    strcpy(estructura->nombreRestaurante, nombreRestaurante);

    //emision del mensaje para pedir la info, OBTENER_RESTAURANTE [nombreR]
    mandar_mensaje(estructura, OBTENER_RESTAURANTE, socket_sindicato);

    free(estructura->nombreRestaurante);
    free(estructura);

    printf("pude mandar la solicitud de metadata a sindic.\n");

    //recibo el codigo de operacion, ya se que va a ser RESPUESTA_OBTENER_R
    codigo_operacion codigoRecibido;
    bytesRecibidos(recv(socket_sindicato, &codigoRecibido, sizeof(codigo_operacion), MSG_WAITALL));

    printf("El codigo recibido del emisor es: %d", codigoRecibido);

    uint32_t sizePayload;
    bytesRecibidos(recv(socket_sindicato, &sizePayload, sizeof(uint32_t), MSG_WAITALL));

    printf("El size del buffer/payload para la metadata es: %u", sizePayload);

    respuesta_obtener_restaurante* estructuraRespuestaObtenerRestaurante = malloc(sizePayload);

   //recepcion del choclo divino
    recibir_mensaje(estructuraRespuestaObtenerRestaurante, RESPUESTA_OBTENER_REST, socket_sindicato);

    printf("pude recibir toda la de metadata de sindic.\n");


    // SI > 0 => True, si = 0 => false
    if (cantidadDeElementosEnArray(listaPlatos)){
    	printf("Hay platos\n");
    } else {
    	printf("No hay platos (no hay restaurant)\n");
    	exit(-2);
    }

    //trabajo interno con la metadata recibida
    miPosicionX = estructuraRespuestaObtenerRestaurante->posX;
    miPosicionY = estructuraRespuestaObtenerRestaurante->posY;
    cantHornos = estructuraRespuestaObtenerRestaurante->cantHornos;
    cantCocineros = estructuraRespuestaObtenerRestaurante->cantidadCocineros;

    //rescato las variables char* en punteros globales aparte para reutilizarlas en los mensajes futuros que tenga q contestar
    platos = malloc(estructuraRespuestaObtenerRestaurante->longitudPlatos+1);
    strcpy(platos, estructuraRespuestaObtenerRestaurante->platos);

    afinidades = malloc(estructuraRespuestaObtenerRestaurante->longitudAfinidades+1);
    strcpy(afinidades, estructuraRespuestaObtenerRestaurante->afinidades);

    listaPlatos = string_get_string_as_array(platos);
    listaAfinidades = string_get_string_as_array(afinidades);

    free(estructuraRespuestaObtenerRestaurante->platos);
    free(estructuraRespuestaObtenerRestaurante->afinidades);
    free(estructuraRespuestaObtenerRestaurante->precioPlatos);
    free(estructuraRespuestaObtenerRestaurante);
}



void crearColasPlanificacion(){
   int i=0;
   listaDeColasReady = list_create();
   while(listaAfinidades[i] != NULL){
     cola_ready* nuevaCola = malloc(sizeof(cola_ready));
     nuevaCola->afinidad = malloc(strlen(listaAfinidades[i])+1);
     strcpy(nuevaCola->afinidad, listaAfinidades[i]);
     nuevaCola->cola = list_create();
     list_add(listaDeColasReady, nuevaCola);
   }

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
    log_info(logger, "Obtuve de config el nombre: %s" , nombreRestaurante);

    //comunicarme con sindicato -> socket -> mensaje OBTENER_RESTAURANTE

    obtenerMetadataRestaurante();

    crearColasPlanificacion();



}



