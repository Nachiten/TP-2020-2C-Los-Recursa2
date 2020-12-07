/*
 * mensajes.c
 *
 *  Created on: 4 sep. 2020
 *      Author: Nachiten
 */

#include "mensajes.h"



/*
 * Mensajes:
 * Obtener restaurante
 * Obtener receta
 * Obtener pedido
 * Plato listo
 * Guardar plato
 * Consultar platos
 * Guardar pedido
 * Confirmar pedido
 * Terminar pedido

 * Obtener restaurante - Logs hechos -> Tested
 * Consultar platos - Logs hechos -> Tested
 * Obtener receta - Logs hechos -> Tested
 * Guardar pedido - Logs hechos -> Tested
 * Guardar plato - Logs hechos -> Tested
 * Obtener pedido - Logs hechos -> Tested
 * Plato listo - Logs hechos -> Hay que revisar
 * Confirmar pedido - Logs hechos -> Tested
 * Terminar pedido - Logs hechos ->
 *
 */

// ---- MENSAJES POR SOCKET ----

void platoListo(char* nombreRestaurant, int IDPedido, char* nombrePlato, uint32_t socketCliente){
	if ( !existeRestaurant(nombreRestaurant) ){
		printf("ERROR | No existe el restaurant buscado.\n");
		log_info(logger, "[EnvioMSG] RTA_PLATO_LISTO | Valor: FAIL");
		enviarRespuestaBooleana(socketCliente, RESPUESTA_PLATO_LISTO, FAIL);
		return;
	}

	if( !existePedido(nombreRestaurant, IDPedido) ){
		printf("ERROR | No existe el pedido solicitado.\n");
		log_info(logger, "[EnvioMSG] RTA_PLATO_LISTO | Valor: FAIL");
		enviarRespuestaBooleana(socketCliente, RESPUESTA_PLATO_LISTO, FAIL);
		return;
	}

	waitSemaforoPedido(nombreRestaurant, IDPedido);

	char* datosPedido = leerDatosPedido(nombreRestaurant, IDPedido);

	if(!pedidoEstaEnEstado("Confirmado", datosPedido)){
		printf("ERROR | No esta en estado confirmado.\n");
		signalSemaforoPedido(nombreRestaurant, IDPedido);
		free(datosPedido);
		log_info(logger, "[EnvioMSG] RTA_PLATO_LISTO | Valor: FAIL");
		enviarRespuestaBooleana(socketCliente, RESPUESTA_PLATO_LISTO, FAIL);
		return;
	}

	int indexPlatoEnPedido = obtenerPlatoEnPedido(datosPedido, nombrePlato);

	// El plato no esta en el pedido (no debe pasar)
	if (indexPlatoEnPedido == -1){
		printf("ERROR | El plato solicitado no está en el pedido.\n");
		signalSemaforoPedido(nombreRestaurant, IDPedido);
		log_info(logger, "[EnvioMSG] RTA_PLATO_LISTO | Valor: FAIL");
		enviarRespuestaBooleana(socketCliente, RESPUESTA_PLATO_LISTO, FAIL);
		return;
	}

	char* datosNuevos = sumarPlatoListoAPedido(datosPedido, indexPlatoEnPedido);

	t_list* listaBloquesActual = obtenerListaBloquesPedido(nombreRestaurant, IDPedido);

	// Calculo cantidad de bloques que necesito
	int cantidadBloquesNecesarios = cantidadDeBloquesQueOcupa(strlen(datosNuevos));

	// Calculo cantidad de bloques que tengo
	int cantidadBloquesActuales = list_size(listaBloquesActual);

	char* nombreParaLog = generarNombrePedidoParaLog(nombreRestaurant, IDPedido);

	// Pido mas bloques y los agrego en listaBloques si son necesarios
	pedirOLiberarBloquesDeSerNecesario(cantidadBloquesNecesarios, cantidadBloquesActuales, listaBloquesActual, nombreParaLog);

	//free(nombreParaLog);

	// Separar los datos en una lista
	t_list* listaDatosSeparados = separarStringEnBloques(datosNuevos, cantidadBloquesNecesarios);

	// Escribir los datos
	escribirLineasEnBloques(listaBloquesActual, listaDatosSeparados);

	// Generar el path al pedido
	char* pathCarpetaRestaurant = generarPathCarpetaRestaurant(nombreRestaurant);
	char* pathAPedido = generarPathAPedido(pathCarpetaRestaurant, IDPedido);

	// Fijar el size correcto
	fijarValorArchivoA(pathAPedido, strlen(datosNuevos), "SIZE");

	signalSemaforoPedido(nombreRestaurant, IDPedido);

	log_info(logger, "[EnvioMSG] RTA_PLATO_LISTO | Valor: OK");
	enviarRespuestaBooleana(socketCliente, RESPUESTA_PLATO_LISTO, OK);

	destruirListaYElementos(listaBloquesActual);
	destruirListaYElementos(listaDatosSeparados);

	free(nombreParaLog);
	free(pathCarpetaRestaurant);
	free(pathAPedido);

}

void obtenerReceta(char* nombreReceta, uint32_t socketCliente){

	if (!existeReceta(nombreReceta)){
		printf("ERROR | No existe la receta solicitada\n");

		char* arrayVacio = "[]";

		respuesta_obtener_receta* miRespuesta = malloc(sizeof(respuesta_obtener_receta));

		miRespuesta->sizePasos = strlen(arrayVacio);
		miRespuesta->pasos = malloc(strlen(arrayVacio) + 1);
		strcpy(miRespuesta->pasos, arrayVacio);

		miRespuesta->sizeTiempoPasos = strlen(arrayVacio);
		miRespuesta->tiempoPasos = malloc(strlen(arrayVacio) + 1);
		strcpy(miRespuesta->tiempoPasos, arrayVacio);



		loguearRtaObtenerReceta(miRespuesta);
		mandar_mensaje(miRespuesta, RESPUESTA_OBTENER_RECETA, socketCliente);
		freeRtaObtenerReceta(miRespuesta);

		return;
	}

	waitSemaforoReceta(nombreReceta);

	char* datosReceta = leerDatosReceta(nombreReceta);

	signalSemaforoReceta(nombreReceta);

	char** datosSeparados = string_split(datosReceta, "\n");

	char** lineaPasos = string_split(datosSeparados[0], "=");
	char** lineaTiempoPasos = string_split(datosSeparados[1], "=");

	char* pasos = lineaPasos[1];
	char* tiemposPasos = lineaTiempoPasos[1];

	respuesta_obtener_receta* recetaBuscada = malloc(sizeof(respuesta_obtener_receta));

	recetaBuscada->sizePasos = strlen(pasos);
	recetaBuscada->pasos = malloc(strlen(pasos) + 1);
	strcpy(recetaBuscada->pasos, pasos);

	recetaBuscada->sizeTiempoPasos = strlen(tiemposPasos);
	recetaBuscada->tiempoPasos = malloc(strlen(tiemposPasos) + 1);
	strcpy(recetaBuscada->tiempoPasos, tiemposPasos);

	//printearRespuestaObtenerReceta(miRespuesta);

	loguearRtaObtenerReceta(recetaBuscada);
	mandar_mensaje(recetaBuscada, RESPUESTA_OBTENER_RECETA, socketCliente);

	freeRtaObtenerReceta(recetaBuscada);
	free(datosReceta);
	freeDeArray(datosSeparados);
	freeDeArray(lineaPasos);
	freeDeArray(lineaTiempoPasos);
}

void guardarPlato(char* nombreRestaurant, int IDPedido, char* nombrePlato, int cantidadPlatos, uint32_t socketCliente){
	if ( !existeRestaurant(nombreRestaurant) ){
		printf("ERROR | No existe el restaurant buscado.\n");
		log_info(logger, "[EnvioMSG] RTA_GUARDAR_PLATO | Valor: FAIL");
		enviarRespuestaBooleana(socketCliente, RESPUESTA_GUARDAR_PLATO, FAIL);
		return;
	}

	if( !existePedido(nombreRestaurant, IDPedido) ){
		printf("ERROR | No existe el pedido solicitado.\n");
		log_info(logger, "[EnvioMSG] RTA_GUARDAR_PLATO | Valor: FAIL");
		enviarRespuestaBooleana(socketCliente, RESPUESTA_GUARDAR_PLATO, FAIL);
		return;
	}

	waitSemaforoPedido(nombreRestaurant, IDPedido);

	char* datosPedido = leerDatosPedido(nombreRestaurant, IDPedido);

	if(!pedidoEstaEnEstado("Pendiente", datosPedido)){
		printf("ERROR | No esta en estado pendiente.\n");
		signalSemaforoPedido(nombreRestaurant, IDPedido);
		free(datosPedido);
		log_info(logger, "[EnvioMSG] RTA_GUARDAR_PLATO | Valor: FAIL");
		enviarRespuestaBooleana(socketCliente, RESPUESTA_GUARDAR_PLATO, FAIL);
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
		free(datosPedido);
		log_info(logger, "[EnvioMSG] RTA_GUARDAR_PLATO | Valor: FAIL");
		enviarRespuestaBooleana(socketCliente, RESPUESTA_GUARDAR_PLATO, FAIL);
		return;
	}

	int indexPlatoEnPedido = obtenerPlatoEnPedido(datosPedido, nombrePlato);

	char* nuevosDatos;

	// El plato no esta en el pedido (se debe agregar)
	if (indexPlatoEnPedido == -1){
		//printf("El plato no está en el pedido.\n");
		// Se debe agregar el nuevo plato al pedido
		nuevosDatos = agregarPlatoNuevoAPedido(nombrePlato, datosPedido, precioPlato, cantidadPlatos);
	// El plato si esta en el pedido (se debe sumar la cantidad)
	} else {
		//printf("El plato si está en el pedido.\n");
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
	pedirOLiberarBloquesDeSerNecesario(cantidadBloquesNecesarios, cantidadBloquesActuales, listaBloquesActual, nombreParaLog);

	free(nombreParaLog);

	// Separar los datos en una lista
	t_list* listaDatosSeparados = separarStringEnBloques(nuevosDatos, cantidadBloquesNecesarios);

	// Escribir los datos
	escribirLineasEnBloques(listaBloquesActual, listaDatosSeparados);

	// Generar el path al pedido
	char* pathCarpetaRestaurant = generarPathCarpetaRestaurant(nombreRestaurant);
	char* pathAPedido = generarPathAPedido(pathCarpetaRestaurant, IDPedido);

	// Fijar el size correcto
	fijarValorArchivoA(pathAPedido, strlen(nuevosDatos), "SIZE");

	signalSemaforoPedido(nombreRestaurant, IDPedido);

	// Envio respuesta OK
	log_info(logger, "[EnvioMSG] RTA_GUARDAR_PLATO | Valor: OK");
	enviarRespuestaBooleana(socketCliente, RESPUESTA_GUARDAR_PLATO, OK);

	destruirListaYElementos(listaBloquesActual);
	destruirListaYElementos(listaDatosSeparados);
	free(pathCarpetaRestaurant);
	free(pathAPedido);
	free(datosPedido);
	free(nuevosDatos);
}

void consultarPlatos_sindicato(char* nombreRestaurant, uint32_t socket_cliente){

	if ( !existeRestaurant(nombreRestaurant) ){
		printf("ERROR | No existe el restaurant buscado.\n");

		char* arrayVacio = "[]";

		respuesta_consultar_platos* respuestaMensaje = malloc(
				sizeof(respuesta_consultar_platos)
			);

		respuestaMensaje->longitudNombresPlatos = strlen(arrayVacio);
		respuestaMensaje->nombresPlatos = malloc(strlen(arrayVacio)+1);
		strcpy(respuestaMensaje->nombresPlatos, arrayVacio);

		loguearRtaConsultarPlatos(respuestaMensaje);
		mandar_mensaje(respuestaMensaje, RESPUESTA_CONSULTAR_PLATOS, socket_cliente);

		free(respuestaMensaje->nombresPlatos);
		free(respuestaMensaje);
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

	//printearRespuestaConsultarPlatos(respuestaPlatos);

	// Se envia la respuesta
	loguearRtaConsultarPlatos(respuestaPlatos);
	mandar_mensaje(respuestaPlatos, RESPUESTA_CONSULTAR_PLATOS, socket_cliente);

	freeDeArray(datosSeparados);
	freeDeArray(lineaPlatos);
	free(datosRestaurante);
	free(respuestaPlatos->nombresPlatos);
	free(respuestaPlatos);
}

void terminarPedido(char* nombreRestaurant, int IDPedido, uint32_t socketCliente){

	if ( !existeRestaurant(nombreRestaurant) ){
		printf("ERROR | No existe el restaurant buscado.\n");
		log_info(logger, "[EnvioMSG] RTA_TERMINAR_PEDIDO | Valor: FAIL");
		enviarRespuestaBooleana(socketCliente, RESPUESTA_TERMINAR_PEDIDO, FAIL);
		return;
	}

	if( !existePedido(nombreRestaurant, IDPedido) ){
		printf("ERROR | No existe el pedido solicitado.\n");
		log_info(logger, "[EnvioMSG] RTA_TERMINAR_PEDIDO | Valor: FAIL");
		enviarRespuestaBooleana(socketCliente, RESPUESTA_TERMINAR_PEDIDO, FAIL);
		return;
	}

	waitSemaforoPedido(nombreRestaurant, IDPedido);

	char* datosPedido = leerDatosPedido(nombreRestaurant, IDPedido);

	if(!pedidoEstaEnEstado("Confirmado", datosPedido)){
		printf("ERROR | No está en estado confirmado.\n");
		signalSemaforoPedido(nombreRestaurant, IDPedido);
		free(datosPedido);
		log_info(logger, "[EnvioMSG] RTA_TERMINAR_PEDIDO | Valor: FAIL");
		enviarRespuestaBooleana(socketCliente, RESPUESTA_TERMINAR_PEDIDO, FAIL);
		return;
	}

	char* nuevosDatosPedido = cambiarEstadoPedidoA("Terminado", datosPedido);

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
	pedirOLiberarBloquesDeSerNecesario(cantidadBloquesNecesarios, cantidadBloquesActuales, listaBloquesActual, nombreParaLog);

	free(nombreParaLog);

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

	// Retorna OK
	log_info(logger, "[EnvioMSG] RTA_TERMINAR_PEDIDO | Valor: OK");
	enviarRespuestaBooleana(socketCliente, RESPUESTA_TERMINAR_PEDIDO, OK);

	free(datosPedido);
	free(nuevosDatosPedido);
	destruirListaYElementos(listaBloquesActual);
	destruirListaYElementos(listaDatosSeparados);
	free(pathCarpetaRestaurant);
	free(pathAPedido);

}


void confirmarPedido_sindicato(char* nombreRestaurant, int IDPedido, uint32_t socketCliente){

	if ( !existeRestaurant(nombreRestaurant) ){
		printf("ERROR | No existe el restaurant buscado.\n");
		log_info(logger, "[EnvioMSG] RTA_CONFIRMAR_PEDIDO | Valor: FAIL");
		enviarRespuestaBooleana(socketCliente, RESPUESTA_CONFIRMAR_PEDIDO, FAIL);
		return;
	}

	if( !existePedido(nombreRestaurant, IDPedido) ){
		printf("ERROR | No existe el pedido solicitado.\n");
		log_info(logger, "[EnvioMSG] RTA_CONFIRMAR_PEDIDO | Valor: FAIL");
		enviarRespuestaBooleana(socketCliente, RESPUESTA_CONFIRMAR_PEDIDO, FAIL);
		return;
	}

	waitSemaforoPedido(nombreRestaurant, IDPedido);

	char* datosPedido = leerDatosPedido(nombreRestaurant, IDPedido);

	if(!pedidoEstaEnEstado("Pendiente", datosPedido)){
		printf("ERROR | No esta en estado pendiente.\n");
		signalSemaforoPedido(nombreRestaurant, IDPedido);
		free(datosPedido);
		log_info(logger, "[EnvioMSG] RTA_CONFIRMAR_PEDIDO | Valor: FAIL");
		enviarRespuestaBooleana(socketCliente, RESPUESTA_CONFIRMAR_PEDIDO, FAIL);
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
	pedirOLiberarBloquesDeSerNecesario(cantidadBloquesNecesarios, cantidadBloquesActuales, listaBloquesActual, nombreParaLog);

	free(nombreParaLog);

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

	// Respuesta OK
	log_info(logger, "[EnvioMSG] RTA_CONFIRMAR_PEDIDO | Valor: OK");
	enviarRespuestaBooleana(socketCliente, RESPUESTA_CONFIRMAR_PEDIDO, OK);

	free(datosPedido);
	free(nuevosDatosPedido);
	destruirListaYElementos(listaBloquesActual);
	destruirListaYElementos(listaDatosSeparados);
	free(pathCarpetaRestaurant);
	free(pathAPedido);

}

void obtenerPedido(char* nombreRestaurant, int IDPedido, uint32_t socketCliente){
	if ( !existeRestaurant(nombreRestaurant)){
		printf("ERROR | No existe el restaurant buscado.\n");
		respuesta_obtener_pedido* rtaDefault = generarRtaObtenerPedidoDefault();

		loguearRtaObtenerPedido(rtaDefault);
		mandar_mensaje(rtaDefault, RESPUESTA_OBTENER_PEDIDO, socketCliente);

		freeRtaObtenerPedido(rtaDefault);

		return;
	}

	if(!existePedido(nombreRestaurant, IDPedido)){
		printf("ERROR | No existe el pedido solicitado");
		respuesta_obtener_pedido* rtaDefault = generarRtaObtenerPedidoDefault();

		loguearRtaObtenerPedido(rtaDefault);
		mandar_mensaje(rtaDefault, RESPUESTA_OBTENER_PEDIDO, socketCliente);

		freeRtaObtenerPedido(rtaDefault);
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
	char** lineaEstadoPedido = string_split(datosSeparados[0], "=");
	char** lineaNombresPlatos = string_split(datosSeparados[1], "=");
	char** lineaCantidadesTotales = string_split(datosSeparados[2], "=");
	char** lineaCantidadesListas = string_split(datosSeparados[3], "=");

	char* estadoPedidoString = lineaEstadoPedido[1];
	char* nombresPlatos = lineaNombresPlatos[1];
	char* cantidadesTotales = lineaCantidadesTotales[1];
	char* cantidadesListas = lineaCantidadesListas[1];

	int estadoPedido = estadoPedidoDeStringAInt(estadoPedidoString);

	respuesta_obtener_pedido* respuestaPedido = malloc(sizeof(respuesta_obtener_pedido));

	respuestaPedido->estado = estadoPedido;
	respuestaPedido->sizeComidas = strlen(nombresPlatos);
	respuestaPedido->comidas = malloc(strlen(nombresPlatos) + 1);
	strcpy(respuestaPedido->comidas, nombresPlatos);

	respuestaPedido->sizeCantTotales = strlen(cantidadesTotales);
	respuestaPedido->cantTotales = malloc(strlen(cantidadesTotales) + 1);
	strcpy(respuestaPedido->cantTotales, cantidadesTotales);

	respuestaPedido->sizeCantListas = strlen(cantidadesListas);
	respuestaPedido->cantListas = malloc(strlen(cantidadesListas) + 1);
	strcpy(respuestaPedido->cantListas, cantidadesListas);

	//printearRespuestaObtenerPedido(respuestaPedido);

	// Se envia respuesta correcta
	loguearRtaObtenerPedido(respuestaPedido);
	mandar_mensaje(respuestaPedido, RESPUESTA_OBTENER_PEDIDO, socketCliente);

	freeRtaObtenerPedido(respuestaPedido);

	free(datosPedido);

	freeDeArray(datosSeparados);
	freeDeArray(lineaEstadoPedido);
	freeDeArray(lineaNombresPlatos);
	freeDeArray(lineaCantidadesTotales);
	freeDeArray(lineaCantidadesListas);
}

// Obtiene todos los datos de un restaurant
void obtenerRestaurante_sindicato(char* nombreRestaurante, uint32_t socket_cliente){
	if ( !existeRestaurant(nombreRestaurante)){
		printf("ERROR | No existe el restaurant buscado.\n");

		char* arrayVacio = "[]";

		respuesta_obtener_restaurante* respuestaMensaje = malloc(
				sizeof(respuesta_obtener_restaurante)
			);

		respuestaMensaje->cantidadCocineros = 0;
		respuestaMensaje->cantHornos = 0;
		respuestaMensaje->posX = 0;
		respuestaMensaje->posY = 0;
		respuestaMensaje->cantPedidos = 0;

		respuestaMensaje->longitudAfinidades = strlen(arrayVacio);
		respuestaMensaje->afinidades = malloc(strlen(arrayVacio)+1);
		strcpy(respuestaMensaje->afinidades, arrayVacio);

		respuestaMensaje->longitudPlatos = strlen(arrayVacio);
		respuestaMensaje->platos = malloc(strlen(arrayVacio)+1);
		strcpy(respuestaMensaje->platos, arrayVacio);

		respuestaMensaje->longitudPrecioPlatos = strlen(arrayVacio);
		respuestaMensaje->precioPlatos = malloc(strlen(arrayVacio)+1);
		strcpy(respuestaMensaje->precioPlatos, arrayVacio);

		loguearRtaObtenerRestaurante(respuestaMensaje);
		mandar_mensaje(respuestaMensaje, RESPUESTA_OBTENER_REST, socket_cliente);

		freeRtaObtenerRestaurante(respuestaMensaje);
		return;
	}

	waitSemaforoRestaurant(nombreRestaurante);

	char* datosRestaurant = leerDatosRestaurant(nombreRestaurante);

	signalSemaforoRestaurant(nombreRestaurante);

	char** datosSeparados = string_split(datosRestaurant, "\n");

	/*
	AFINIDAD_COCINEROS=[Milanesas] [2]
	PLATOS=[Milanesas,Empanadas,Ensalada] [3]
	PRECIO_PLATOS=[200,50,150] [4]
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
			);

	respuestaMensaje->cantidadCocineros = cantCocineros;
	respuestaMensaje->cantHornos = cantHornos;
	respuestaMensaje->posX = posX;
	respuestaMensaje->posY = posY;

	respuestaMensaje->longitudAfinidades = strlen(lineaAfinidadesSeparada[1]);
	respuestaMensaje->afinidades = malloc(strlen(lineaAfinidadesSeparada[1])+1);
	strcpy(respuestaMensaje->afinidades, lineaAfinidadesSeparada[1]);

	respuestaMensaje->longitudPlatos = strlen(lineaPlatosSeparada[1]);
    respuestaMensaje->platos = malloc(strlen(lineaPlatosSeparada[1])+1);
	strcpy(respuestaMensaje->platos, lineaPlatosSeparada[1]);

	respuestaMensaje->longitudPrecioPlatos = strlen(lineaPrecioPlatosSeparada[1]);
	respuestaMensaje->precioPlatos = malloc(strlen(lineaPrecioPlatosSeparada[1])+1);
	strcpy(respuestaMensaje->precioPlatos, lineaPrecioPlatosSeparada[1]);

	respuestaMensaje->cantPedidos = calcularCantidadPedidos(nombreRestaurante);
	//printf("La cantidad de pedidos es %i\n", respuestaMensaje->cantPedidos);

	//printearRespuestaObtenerRestaurante(respuestaMensaje);

	// Se envia respuesta
	loguearRtaObtenerRestaurante(respuestaMensaje);
	mandar_mensaje(respuestaMensaje, RESPUESTA_OBTENER_REST, socket_cliente);

	freeRtaObtenerRestaurante(respuestaMensaje);

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
void guardarPedido_sindicato(char* nombreRestaurante, int IDPedido, uint32_t socketCliente){
	if (!existeRestaurant(nombreRestaurante)){
		printf("ERROR | No existe el restaurant solicitado.\n");
		// Se envia respuesta fail
		log_info(logger, "[EnvioMSG] RTA_GUARDAR_PEDIDO | Valor: FAIL");
		enviarRespuestaBooleana(socketCliente, RESPUESTA_GUARDAR_PEDIDO, FAIL);
		return;
	}

	if (existePedido(nombreRestaurante, IDPedido)){
		printf("ERROR | Ya existe el pedido con ID solicitado.\n");
		// Se envia respuesta fail
		log_info(logger, "[EnvioMSG] RTA_GUARDAR_PEDIDO | Valor: FAIL");
		enviarRespuestaBooleana(socketCliente, RESPUESTA_GUARDAR_PEDIDO, FAIL);
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

	// Se envia respuesta afirmativa
	log_info(logger, "[EnvioMSG] RTA_GUARDAR_PEDIDO | Valor: OK");
	enviarRespuestaBooleana(socketCliente, RESPUESTA_GUARDAR_PEDIDO, OK);

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

void enviarRespuestaBooleana(uint32_t socketCliente, codigo_operacion codOp, respuestaBool valorRespuesta){

	respuesta_ok_error* rta = malloc(sizeof(respuesta_ok_error));

	rta->respuesta = valorRespuesta;

	mandar_mensaje(rta, codOp, socketCliente);

	free(rta);
}

// --- LOGS ---

void loguearRtaObtenerPedido(respuesta_obtener_pedido* rta){
	log_info(logger, "[EnvioMSG] RTA_OBTENER_PEDIDO con valores: estado=%s | comidas=%s | cantTotales=%s | cantListas=%s",
			estadoAString(rta->estado) , rta->comidas, rta->cantTotales, rta->cantListas);
}

void loguearRtaObtenerReceta(respuesta_obtener_receta* rta){
	log_info(logger, "[EnvioMSG] RTA_OBTENER_RECETA con valores: pasos=%s | tiempoPasos=%s",
			rta->pasos, rta->tiempoPasos);
}

void loguearRtaObtenerRestaurante(respuesta_obtener_restaurante* rta){
	log_info(logger, "[EnvioMSG] RTA_OBTENER_REST con valores: cantCocineros=%i | posX=%i | posY=%i "
			"| cantHornos=%i | afinidades=%s | platos=%s | precioPlatos=%s | cantPedidos=%i",
			rta->cantidadCocineros, rta->posX, rta->posY,
			rta->cantHornos, rta->afinidades, rta->platos, rta->precioPlatos, rta->cantPedidos);
}

void loguearRtaConsultarPlatos(respuesta_consultar_platos* rta){
	log_info(logger, "[EnvioMSG] RTA_CONSULTAR_PLATOS con valores: nombresPlatos=%s", rta->nombresPlatos);
}

// --- FREES ---

void freeRtaObtenerReceta(respuesta_obtener_receta* rta){
	free(rta->pasos);
	free(rta->tiempoPasos);
	free(rta);
}

void freeRtaObtenerPedido(respuesta_obtener_pedido* rta){
	free(rta->comidas);
	free(rta->cantTotales);
	free(rta->cantListas);
	free(rta);
}

void freeRtaObtenerRestaurante(respuesta_obtener_restaurante* rta){
	free(rta->precioPlatos);
	free(rta->platos);
	free(rta->afinidades);
	free(rta);
}

// --- FUNCIONES AUXILIARES

respuesta_obtener_pedido* generarRtaObtenerPedidoDefault(){
	respuesta_obtener_pedido* rta = malloc(sizeof(respuesta_obtener_pedido));

	char* arrayVacio = "[]";
    rta->estado = NADA_CARGADO;
	rta->sizeCantListas = strlen(arrayVacio);
	rta->sizeCantTotales = strlen(arrayVacio);
	rta->sizeComidas = strlen(arrayVacio);

	rta->cantListas = malloc(strlen(arrayVacio) + 1);
	strcpy(rta->cantListas, arrayVacio);

	rta->cantTotales = malloc(strlen(arrayVacio) + 1);
	strcpy(rta->cantTotales, arrayVacio);

	rta->comidas = malloc(strlen(arrayVacio) + 1);
	strcpy(rta->comidas, arrayVacio);

	return rta;
}

int calcularCantidadPedidos(char* nombreRestaurant){

    int cantidadPedidos = 0;

	char* pathCarpetaRestaurant = generarPathCarpetaRestaurant(nombreRestaurant);

	struct dirent *archivoLeido;

	// Retorna un puntero al directorio | {puntoMontaje}/Files
	DIR *dr = opendir(pathCarpetaRestaurant);

	if (dr == NULL)
	{
		printf("No se pudo abrir el directorio actual" );
	}

	while ((archivoLeido = readdir(dr)) != NULL)
	{
		// Nombre del archivo leido dentro del directorio
		char* punteroANombre = archivoLeido->d_name;

		// Si el archivo es . .. o Info.AFIP es ignorado
		if (strcmp(punteroANombre, ".") == 0 || strcmp(punteroANombre, "..") == 0 || strcmp(punteroANombre, "Info.AFIP") == 0){
			continue;
		}

		// Si el nombre de archivo es un pedido entonces lo sumo
		cantidadPedidos++;
	}

	closedir(dr);
	free(pathCarpetaRestaurant);

	return cantidadPedidos;
}

char* estadoAString(estado_de_pedido estado){

	char* retorno;

	switch(estado){
		case PENDIENTE:
			retorno = "Pendiente";
		break;
		case CONFIRMADO:
			retorno = "Confirmado";
		break;
		case TERMINADO:
			retorno = "Terminado";
		break;
		default:
			retorno = "Ninguno";
		break;
	}

	return retorno;
}

int estadoPedidoDeStringAInt(char* estadoPedidoString){

	int retorno = NADA_CARGADO;

	if (strcmp(estadoPedidoString, "Pendiente") == 0){
		retorno = PENDIENTE;
	} else if (strcmp(estadoPedidoString, "Confirmado") == 0){
		retorno = CONFIRMADO;
	} else if (strcmp(estadoPedidoString, "Terminado") == 0){
		retorno = TERMINADO;
	}

	return retorno;
}
