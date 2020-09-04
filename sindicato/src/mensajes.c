/*
 * mensajes.c
 *
 *  Created on: 4 sep. 2020
 *      Author: utnso
 */

#include "mensajes.h"

// ---- MENSAJES POR SOCKET ----

// Obtiene todos los datos de un restaurant
void obtenerRestaurante(char* nombreRestaurante){
	if ( !existeRestaurant(nombreRestaurante)){
		printf("ERROR | No existe el restaurant buscado.\n");
		// TODO | Retornar valor default (no se cual es)
		return;
	}

	char* pathInfoRestaurant = generarPathInfoRestaurant(nombreRestaurante);

	int sizeBytes = leerValorArchivo(pathInfoRestaurant, "SIZE");
	int bloqueInicial = leerValorArchivo(pathInfoRestaurant, "INITIAL_BLOCK");

	printf("Size: %i\n", sizeBytes);
	printf("BloqueInicial: %i\n", bloqueInicial);

	char* datosLeidos = leerDatosBloques(sizeBytes, bloqueInicial);

	// TODO | Se debe procesar los datos leidos y devolver la respuesta

	printf("Datos leidos:\n%s", datosLeidos);
}

// Generar un nuevo pedido vacio en el restaurante
void guardarPedido(char* nombreRestaurante){
	if (!existeRestaurant(nombreRestaurante)){
		printf("ERROR | No existe el restaurant solicitado.\n");
		// TODO | Se debe enviar un mensaje con respuesta negativa (Fail)
		return;
	}

	int pedidoAGenerar = obtenerUltimoPedido(nombreRestaurante) + 1;

	char* pathCarpetaRestaurant = generarPathCarpetaRestaurant(nombreRestaurante);

	char* numeroPedidoString;

	asprintf(&numeroPedidoString, "%i", pedidoAGenerar);

	char* pedido = "Pedido";
	char* extension = ".AFIP";

	char* nombrePedido = malloc(strlen(pedido) + strlen(numeroPedidoString) + strlen(extension) + 1);

	// Pedido1.AFIP
	strcpy(nombrePedido, pedido);
	strcat(nombrePedido, numeroPedidoString);
	strcat(nombrePedido, extension);

	char* pathPedido = crearArchivoVacioEn(pathCarpetaRestaurant, nombrePedido);

	char* stringPedidoDefault = generarStringPedidoDefault();

	char* de = " de ";

	char* nombrePedidoParaLog = malloc(strlen(nombrePedido) + strlen(de) + strlen(nombreRestaurante) + 1);

	strcpy(nombrePedidoParaLog, nombrePedido);
	strcat(nombrePedidoParaLog, de);
	strcat(nombrePedidoParaLog, nombreRestaurante);

	llenarBloquesConString(pathPedido, stringPedidoDefault, nombrePedidoParaLog);

	log_info(logger, "Se creo un nuevo pedido llamado %s", nombrePedidoParaLog);

	// TODO | Se debe dar la respuesta afirmativa

	free(pathCarpetaRestaurant);
	free(numeroPedidoString);
	free(nombrePedido);
	free(pathPedido);
	free(stringPedidoDefault);
	free(nombrePedidoParaLog);
}

// ---- MENSAJES DE CONSOLA ----

void crearRestaurant(char* nombreRestaurant, datosRestaurant unRestaurant){
	// Crear la carpeta del nuevo restaurant
	char* pathCarpetaRestaurant = crearCarpetaRestaurant(nombreRestaurant);
	// Crear el archivo Info.AFIP
	char* pathArchivoRestaurant = crearArchivoVacioEn(pathCarpetaRestaurant, "Info.AFIP");

	// Armo el string enorme de datos que tengo que escribir
	char* stringRestaurant = generarSringInfoRestaurant(unRestaurant);

	llenarBloquesConString(pathArchivoRestaurant, stringRestaurant, nombreRestaurant);

	log_info(logger, "Se creo un nuevo restaurant llamado %s", nombreRestaurant);

	free(pathCarpetaRestaurant);
	free(pathArchivoRestaurant);
	free(stringRestaurant);
}

void crearReceta(char* nombreReceta, datosReceta unaReceta){

	char* extension = ".AFIP";

	char* nombreArchivoReceta = malloc(strlen(nombreReceta) + strlen(extension) + 1);

	strcpy(nombreArchivoReceta, nombreReceta);
	strcat(nombreArchivoReceta, extension);

	char* pathArchivoReceta = crearArchivoVacioEn(pathRecetas, nombreArchivoReceta);

	// Armo el string completo de datos a escribir en los bloques
	char* stringReceta = generarStringInfoReceta(unaReceta);

	llenarBloquesConString(pathArchivoReceta, stringReceta, nombreReceta);

	log_info(logger, "Se creo una nueva receta llamada %s", nombreReceta);

	free(nombreArchivoReceta);
	free(pathArchivoReceta);
	free(stringReceta);
}

