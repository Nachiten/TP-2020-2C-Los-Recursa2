/*
 * mensajes.c
 *
 *  Created on: 4 sep. 2020
 *      Author: utnso
 */

#include "mensajes.h"

// ---- MENSAJES POR SOCKET ----

/*
 * Mensajes Hechos:
 *
 * Consultar platos - Interno listo, falta envio de mensaje
 * Guardar pedido - Interno listo, falta envio mensaje
 * Guardar plato - Interno listo, falta envio de mensaje
 * Confirmar pedido - Interno listo, falta envio mensaje
 * Obtener pedido - Interno listo, falta envio mensaje
 * Obtener restaurante - Interno listo, falta envio mensaje
 * Plato listo - X
 * Obtener receta - X
 * Terminar pedido - X
 *
 */

void guardarPlato(char* nombreRestaurant, int IDPedido, char* nombrePlato, int cantidadPlatos){
	if ( !existeRestaurant(nombreRestaurant) ){
		printf("ERROR | No existe el restaurant buscado.\n");
		// TODO | Retornar fail
		return;
	}

	if( !existePedido(nombreRestaurant, IDPedido) ){
		printf("ERROR | No existe el pedido solicitado.\n");
		// TODO | Retornar fail
		return;
	}

	waitSemaforoPedido(nombreRestaurant, IDPedido);

	char* datosPedido = leerDatosPedido(nombreRestaurant, IDPedido);

	if(!pedidoEstaEnEstado("Pendiente", datosPedido)){
		printf("ERROR | No esta en estado pendiente.\n");
		signalSemaforoPedido(nombreRestaurant, IDPedido);
		free(datosPedido);
		// TODO | Retornar fail
		return;
	}

	/*
	 * Obtener precio de plato y saber si ya existe en restaurant | DONE
	 * Verificar si es plato nuevo o ya existe
	 * if (nuevo) => agregar a la linea platos agregar cantidad y sumar precio
	 * if (ya existe) => solo sumar la cantidad a la ya existente y sumar precio
	 */

	int precioPlato = obtenerPrecioPlatoRestaurant(nombreRestaurant, nombrePlato);

	// No existia el plato
	if (precioPlato == -1){
		printf("ERROR | El plato dado no puede cocinarse en el restaurant.\n");
		signalSemaforoPedido(nombreRestaurant, IDPedido);
		// TODO | Retornar fail
		return;
	}

	printf("Precio plato: %i\n", precioPlato);

	int indexPlatoEnPedido = obtenerPlatoEnPedido(datosPedido, nombrePlato);

	char* nuevosDatos;

	// El plato no esta en el pedido (se debe agregar)
	if (indexPlatoEnPedido == -1){
		printf("El plato no está en el pedido.\n");
		// Se debe agregar el nuevo plato al pedido
		nuevosDatos = agregarPlatoNuevoAPedido(nombrePlato, datosPedido, precioPlato, cantidadPlatos);
	// El plato si esta en el pedido (se debe sumar la cantidad)
	} else {
		printf("El plato si está en el pedido.\n");
		// Se debe agregar la cantidad del plato existente y sumar el precio total
		nuevosDatos = sumarCantidadAPlatoExistente(indexPlatoEnPedido, datosPedido, precioPlato, cantidadPlatos);
	}

	//printf("Datos nuevos:\n%s", nuevosDatos);

	t_list* listaBloquesActual = obtenerListaBloquesPedido(nombreRestaurant, IDPedido);

	// Calculo cantidad de bloques que necesito
	int cantidadBloquesNecesarios = cantidadDeBloquesQueOcupa(strlen(nuevosDatos));

	// Calculo cantidad de bloques que tengo
	int cantidadBloquesActuales = list_size(listaBloquesActual);

	char* nombreParaLog = generarNombrePedidoParaLog(nombreRestaurant, IDPedido);

	// Pido mas bloques y los agrego en listaBloques si son necesarios
	pedirMasBloquesDeSerNecesario(cantidadBloquesNecesarios, cantidadBloquesActuales, listaBloquesActual, nombreParaLog);

	// Separar los datos en una lista
	t_list* listaDatosSeparados = separarStringEnBloques(nuevosDatos, cantidadBloquesNecesarios);

	// Escribir los datos
	escribirLineasEnBloques(listaBloquesActual, listaDatosSeparados);

	// Generar el path al pedido
	char* pathCarpetaRestaurant = generarPathCarpetaRestaurant(nombreRestaurant);
	char* pathAPedido = generarPathAPedido(pathCarpetaRestaurant, IDPedido);

	// Fijar el size correcto
	fijarValorArchivoA(pathAPedido, strlen(nuevosDatos), "SIZE");

	// TODO | Ver si se necesitan bloques nuevos y escribir todos los datos

	signalSemaforoPedido(nombreRestaurant, IDPedido);

	destruirListaYElementos(listaBloquesActual);
	destruirListaYElementos(listaDatosSeparados);
	free(pathCarpetaRestaurant);
	free(pathAPedido);
	free(datosPedido);
	free(nuevosDatos);
}

void consultarPlatos(char* nombreRestaurant){

	if ( !existeRestaurant(nombreRestaurant) ){
		printf("ERROR | No existe el restaurant buscado.\n");
		// TODO | Retornar valor default (no se cual es)
		return;
	}

	/*
	 * 1 - Leer datos del restaurant
	 * 2 - Obtener la lista de platos
	 * 3 - Genearar mensaje rta
	 * 4 - Devolver el mensaje
	 */

	waitSemaforoRestaurant(nombreRestaurant);

	char* datosRestaurante = leerDatosRestaurant(nombreRestaurant);

	signalSemaforoRestaurant(nombreRestaurant);

	// datosSeparados[3] = PLATOS=[Milanesas,Empanadas,Ensalada]
	char** datosSeparados = string_split(datosRestaurante, "\n");

	// lineaPlatos[1] = lo q esta despues del = --> [Milanesas,Empanadas,Ensalada]
	char** lineaPlatos = string_split(datosSeparados[3], "=");

	int longitudLineaPlatos = strlen(lineaPlatos[1]);

	respuesta_consultar_platos* respuestaPlatos = malloc(sizeof(respuesta_consultar_platos));
	respuestaPlatos->nombresPlatos = malloc(longitudLineaPlatos + 1);

	respuestaPlatos->longitudNombresPlatos = longitudLineaPlatos;
	strcpy(respuestaPlatos->nombresPlatos, lineaPlatos[1]);

	printearRespuestaConsultarPlatos(respuestaPlatos);

	// TODO | Enviar respuesta

	freeDeArray(datosSeparados);
	freeDeArray(lineaPlatos);
	free(datosRestaurante);
}



void confirmarPedido(char* nombreRestaurant, int IDPedido){

	if ( !existeRestaurant(nombreRestaurant) ){
		printf("ERROR | No existe el restaurant buscado.\n");
		// TODO | Retornar fail
		return;
	}

	if( !existePedido(nombreRestaurant, IDPedido) ){
		printf("ERROR | No existe el pedido solicitado.\n");
		// TODO | Retornar fail
		return;
	}

	waitSemaforoPedido(nombreRestaurant, IDPedido);

	char* datosPedido = leerDatosPedido(nombreRestaurant, IDPedido);

	if(!pedidoEstaEnEstado("Pendiente", datosPedido)){
		printf("ERROR | No esta en estado pendiente.\n");
		signalSemaforoPedido(nombreRestaurant, IDPedido);
		// TODO | Retornar fail
		return;
	}

	char* nuevosDatosPedido = cambiarEstadoPedidoA("Confirmado", datosPedido);

	printf("Nuevos datos pedido:\n%s", nuevosDatosPedido);

	/*
	 * 1 - Calcular cantidad de bloques a utilizar
	 * 2 - Obtener bloques actuales
	 * 3 - Pedir bloques si es necesario
	 * 4 - Escribir nuevos datos en bloques
	 * 5 - Fijar size nuevo
	 */

	t_list* listaBloquesActual = obtenerListaBloquesPedido(nombreRestaurant, IDPedido);

	// Calculo cantidad de bloques que necesito
	int cantidadBloquesNecesarios = cantidadDeBloquesQueOcupa(strlen(nuevosDatosPedido));

	// Calculo cantidad de bloques que tengo
	int cantidadBloquesActuales = list_size(listaBloquesActual);

	char* nombreParaLog = generarNombrePedidoParaLog(nombreRestaurant, IDPedido);

	// Pido mas bloques y los agrego en listaBloques si son necesarios
	pedirMasBloquesDeSerNecesario(cantidadBloquesNecesarios, cantidadBloquesActuales, listaBloquesActual, nombreParaLog);

	// Separar los datos en una lista
	t_list* listaDatosSeparados = separarStringEnBloques(nuevosDatosPedido, cantidadBloquesNecesarios);

	// Escribir los datos
	escribirLineasEnBloques(listaBloquesActual, listaDatosSeparados);

	// Generar el path al pedido
	char* pathCarpetaRestaurant = generarPathCarpetaRestaurant(nombreRestaurant);
	char* pathAPedido = generarPathAPedido(pathCarpetaRestaurant, IDPedido);

	// Fijar el size correcto
	fijarValorArchivoA(pathAPedido, strlen(nuevosDatosPedido), "SIZE");

	signalSemaforoPedido(nombreRestaurant, IDPedido);

	// TODO | Retornar OK

	free(datosPedido);
	free(nuevosDatosPedido);
	destruirListaYElementos(listaBloquesActual);
	destruirListaYElementos(listaDatosSeparados);
	free(pathCarpetaRestaurant);
	free(pathAPedido);

}

void obtenerPedido(char* nombreRestaurant, int IDPedido){
	if ( !existeRestaurant(nombreRestaurant)){
		printf("ERROR | No existe el restaurant buscado.\n");
		// TODO | Retornar valor default (no se cual es)
		return;
	}

	if(!existePedido(nombreRestaurant, IDPedido)){
		printf("ERROR | No existe el pedido solicitado");
		// TODO | Retornar valor default
		return;
	}

	waitSemaforoPedido(nombreRestaurant, IDPedido);

	char* datosPedido = leerDatosPedido(nombreRestaurant, IDPedido);

	signalSemaforoPedido(nombreRestaurant, IDPedido);

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

	// TODO | Ver como se responde
	// IDEA: Struct:
	/*
	 * uint32_t sizeComidas;
	 * char* comidas;
	 * uint32_t sizeCantTotales;
	 * char* cantTotales;
	 * uint32_t sizeCantListas;
	 * char* cantListas;
	 */

//	respuesta_obtener_pedido* respuestaPedido = malloc(sizeof(respuesta_obtener_pedido) + cantidadDePlatos * sizeof(plato));
//
//	respuestaPedido->cantPlatos = cantidadDePlatos;
//
//	int i;
//	for (i = 0; i< cantidadDePlatos; i++){
//		respuestaPedido->platos_pedido[i].longitudNombrePlato = strlen(nombresLista[i]);
//		respuestaPedido->platos_pedido[i].nombrePlato = nombresLista[i];
//		respuestaPedido->platos_pedido[i].cantidadPlatos = atoi(cantTotalLista[i]);
//		respuestaPedido->platos_pedido[i].cantLista = atoi(cantListaLista[i]);
//
////		printf("Nombre %i: %s\n", i, nombresLista[i]);
////		printf("CantidadTotal %i: %s\n", i, cantTotalLista[i]);
////		printf("CantidadLista %i: %s\n", i, cantListaLista[i]);
//	}

	//printearRespuestaObtenerPedido(respuestaPedido);

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
void obtenerRestaurante(char* nombreRestaurante, uint32_t socket_cliente){
	if ( !existeRestaurant(nombreRestaurante)){
		printf("ERROR | No existe el restaurant buscado.\n");
		// TODO | Retornar valor default (no se cual es)
		return;
	}

	waitSemaforoRestaurant(nombreRestaurante);

	char* datosRestaurant = leerDatosRestaurant(nombreRestaurante);

	signalSemaforoRestaurant(nombreRestaurante);

	char** datosSeparados = string_split(datosRestaurant, "\n");

	/*
	 *
	 * AFINIDAD_COCINEROS=[Milanesas] [2]
	PLATOS=[Milanesas,Empanadas,Ensalada] [3]
	PRECIO_PLATOS=[200,50,150] [4]
	 *
	 *
	 */

	char** lineaAfinidadesSeparada = string_split(datosSeparados[2], "=");
	char** lineaPlatosSeparada = string_split(datosSeparados[3], "=");
	char** lineaPrecioPlatosSeparada = string_split(datosSeparados[4], "=");

	// Numero de cantidadHornos
	char** lineaCantCocinerosSeparada = string_split(datosSeparados[0], "=");
	// Posicion = [3,4]
	char** lineaPosicionSeparada = string_split(datosSeparados[1], "=");
	// Numero cantidadHornos
	char** lineaCantHornosSeparada = string_split(datosSeparados[5], "=");

	char** posicionesSeparadas = string_get_string_as_array(lineaPosicionSeparada[1]);

	uint32_t posX = atoi(posicionesSeparadas[0]);
	uint32_t posY = atoi(posicionesSeparadas[1]);

	uint32_t cantCocineros = atoi(lineaCantCocinerosSeparada[1]);
	uint32_t cantHornos = atoi(lineaCantHornosSeparada[1]);

	respuesta_obtener_restaurante* respuestaMensaje = malloc(
			sizeof(respuesta_obtener_restaurante)
//			+ strlen(lineaAfinidadesSeparada[1])
//			+ strlen(lineaPlatosSeparada[1])
//			+ strlen(lineaPrecioPlatosSeparada[1]) + 3
			);

	respuestaMensaje->cantidadCocineros = cantCocineros;
	respuestaMensaje->cantHornos = cantHornos;
	respuestaMensaje->posX = posX;
	respuestaMensaje->posY = posY;

	respuestaMensaje->longitudAfinidades = strlen(lineaAfinidadesSeparada[1]);
	respuestaMensaje->afinidades = malloc(strlen(lineaAfinidadesSeparada[1])+1);
	respuestaMensaje->afinidades = lineaAfinidadesSeparada[1];

	respuestaMensaje->longitudPlatos = strlen(lineaPlatosSeparada[1]);
	respuestaMensaje->platos = malloc(strlen(lineaPlatosSeparada[1])+1);
	respuestaMensaje->platos = lineaPlatosSeparada[1];

	respuestaMensaje->longitudPrecioPlatos = strlen(lineaPrecioPlatosSeparada[1]);
	respuestaMensaje->precioPlatos = malloc(strlen(lineaPrecioPlatosSeparada[1])+1);
	respuestaMensaje->precioPlatos = lineaPrecioPlatosSeparada[1];

	printearRespuestaObtenerRestaurante(respuestaMensaje);


	mandar_mensaje(respuestaMensaje, RESPUESTA_OBTENER_R, socket_cliente);

	// TODO | Se debe procesar los datos leidos y devolver la respuesta



	//printf("Datos leidos:\n%s", datosRestaurant);

	free(datosRestaurant);
	freeDeArray(datosSeparados);
	freeDeArray(lineaAfinidadesSeparada);
	freeDeArray(lineaPlatosSeparada);
	freeDeArray(lineaPrecioPlatosSeparada);
	freeDeArray(lineaCantCocinerosSeparada);
	freeDeArray(lineaPosicionSeparada);
	freeDeArray(lineaCantHornosSeparada);
	freeDeArray(posicionesSeparadas);
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

	char* nombrePedidoParaLog = generarNombrePedidoParaLog(nombreRestaurante, IDPedido);

	llenarBloquesConString(pathPedido, stringPedidoDefault, nombrePedidoParaLog);

	log_info(logger, "Se creo un nuevo pedido llamado %s", nombrePedidoParaLog);

	crearSemaforoPedido(nombreRestaurante, IDPedido);

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

	crearSemaforoRestaurant(nombreRestaurant);

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

	crearSemaforoReceta(nombreReceta);

	free(nombreArchivoReceta);
	free(pathArchivoReceta);
	free(stringReceta);
}

