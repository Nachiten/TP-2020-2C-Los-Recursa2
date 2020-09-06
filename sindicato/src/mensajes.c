/*
 * mensajes.c
 *
 *  Created on: 4 sep. 2020
 *      Author: utnso
 */

#include "mensajes.h"

// ---- MENSAJES POR SOCKET ----

void consultarPlatos(char* nombreRestaurante){
	// NO PUEDO PORQUE HAY ERROR EN LA CONSIGNA D:
}

void obtenerPedido(char* nombreRestaurant, int IDPedido){
	if ( !existeRestaurant(nombreRestaurant)){
		printf("ERROR | No existe el restaurant buscado.\n");
		// TODO | Retornar valor default (no se cual es)
		return;
	}

	//char* datosRestaurant = leerDatosRestaurant(nombreRestaurante);

	if(!existePedido(nombreRestaurant, IDPedido)){
		printf("ERROR | No existe el pedido solicitado");
		// TODO | Retornar valor default
		return;
	}

	char* datosPedido = leerDatosPedido(nombreRestaurant, IDPedido);

	char** datosSeparados = string_split(datosPedido, "\n");

	/* datosSeparados[1] => nombresPlatos
	 * datosSeparados[2] => cantTotal
	 * datosSeparados[3] => cantLista
	 */

	// Separo la linea con = | Linea : PLATOS=[platoA,platoB,platoC]
	char** listaNombresPlatos = string_split(datosSeparados[1], "=");
	char** listaCantidadesTotales = string_split(datosSeparados[2], "=");
	char** listaCantidadesListas = string_split(datosSeparados[3], "=");

	char** nombresLista = string_get_string_as_array(listaNombresPlatos[1]);
	char** cantTotalLista = string_get_string_as_array(listaCantidadesTotales[1]);
	char** cantListaLista = string_get_string_as_array(listaCantidadesListas[1]);

	int cantidadDePlatos = cantidadDeElementosEnArray(nombresLista);

	respuesta_obtener_pedido* respuestaPedido = malloc(sizeof(respuesta_obtener_pedido) + cantidadDePlatos * sizeof(plato));

	respuestaPedido->cantPlatos = cantidadDePlatos;

	int i;
	for (i = 0; i< cantidadDePlatos; i++){
		respuestaPedido->platos_pedido[i].nombrePlato = nombresLista[i];
		respuestaPedido->platos_pedido[i].cantidadPlatos = cantTotalLista[i];
		respuestaPedido->platos_pedido[i].cantLista = cantListaLista[i];

//		printf("Nombre %i: %s\n", i, nombresLista[i]);
//		printf("CantidadTotal %i: %s\n", i, cantTotalLista[i]);
//		printf("CantidadLista %i: %s\n", i, cantListaLista[i]);
	}

	// TODO | Enviar el mensaje con la respuesta

	free(datosPedido);

	freeDeArray(datosSeparados);
	freeDeArray(listaNombresPlatos);
	freeDeArray(listaCantidadesTotales);
	freeDeArray(listaCantidadesListas);
	freeDeArray(nombresLista);
	freeDeArray(cantTotalLista);
	freeDeArray(cantListaLista);
}

// Obtiene todos los datos de un restaurant
void obtenerRestaurante(char* nombreRestaurante){
	if ( !existeRestaurant(nombreRestaurante)){
		printf("ERROR | No existe el restaurant buscado.\n");
		// TODO | Retornar valor default (no se cual es)
		return;
	}

	char* datosRestaurant = leerDatosRestaurant(nombreRestaurante);

	// TODO | Cerrar archivo con semaforos

	// TODO | Se debe procesar los datos leidos y devolver la respuesta

	printf("Datos leidos:\n%s", datosRestaurant);

	free(datosRestaurant);
}

// Generar un nuevo pedido vacio en el restaurante
void guardarPedido(char* nombreRestaurante, int IDPedido){
	if (!existeRestaurant(nombreRestaurante)){
		printf("ERROR | No existe el restaurant solicitado.\n");
		// TODO | Se debe enviar un mensaje con respuesta negativa (Fail)
		return;
	}

	if (existePedido(nombreRestaurante, IDPedido)){
		printf("ERROR | Ya existe el pedido con ID solicitado.\n");
		// TODO | Se debe enviar un mensaje con respuesta negativa (Fail)
		return;
	}

	char* numeroPedidoString;

	asprintf(&numeroPedidoString, "%i", IDPedido);

	char* pedido = "Pedido";
	char* extension = ".AFIP";

	char* nombrePedido = malloc(strlen(pedido) + strlen(numeroPedidoString) + strlen(extension) + 1);

	// Pedido1.AFIP
	strcpy(nombrePedido, pedido);
	strcat(nombrePedido, numeroPedidoString);
	strcat(nombrePedido, extension);

	char* pathCarpetaRestaurant = generarPathCarpetaRestaurant(nombreRestaurante);

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

