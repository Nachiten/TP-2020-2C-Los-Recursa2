/*
 * sindicato.c
 *
 *  Created on: 29 ago. 2020
 *      Author: Nachiten
 */

#include "sindicato.h"

char* generarNombrePedidoParaLog(char* nombreRestaurante, int numeroPedido){

	char* numeroPedidoString;

	asprintf(&numeroPedidoString, "%i" ,numeroPedido);

	char* pedido = "Pedido";
	char* de = " de ";

	char* nombrePedidoParaLog = malloc(strlen(pedido) + strlen(numeroPedidoString) + strlen(de) + strlen(nombreRestaurante) + 1);

	strcpy(nombrePedidoParaLog, pedido);
	strcat(nombrePedidoParaLog, numeroPedidoString);
	strcat(nombrePedidoParaLog, de);
	strcat(nombrePedidoParaLog, nombreRestaurante);

	// PedidoN de NombreRestaurant
	return nombrePedidoParaLog;
}

char* sumarPlatoListoAPedido(char* datosPedido, int indexPlatoEnPedido){
	/*
	 *  ESTADO_PEDIDO=Confirmado
	 *	LISTA_PLATOS=[Milanesa,Empanadas,Ensalada]
	 * 	CANTIDAD_PLATOS=[2,12,1]
	 *	CANTIDAD_LISTA=[1,6,0]
	 *	PRECIO_TOTAL=1150
	 *
	 *	Se suma a cantidadLista en el indexPlatoEnPedido
	 *	Se corrobora que cantidadLista no sea > a cantidadPlatos
	 */

	char** datosSeparados = string_split(datosPedido, "\n");

	char** lineaCantidadLista = string_split(datosSeparados[3], "=");

	// Se le suma 1 a la cantidad lista
	char* nuevoArrayCantidadLista = sumarCantidadAStringPlatos(lineaCantidadLista[1], 1, indexPlatoEnPedido);

	char** lineaCantidadTotal = string_split(datosSeparados[2], "=");

	if (!cantidadListaNoEsMayorATotal(nuevoArrayCantidadLista, lineaCantidadTotal[1], indexPlatoEnPedido)){
		printf("ERROR | La cantidad lista no puede ser mayor a la total");
		nuevoArrayCantidadLista = string_duplicate(lineaCantidadLista[1]);
	}

	char* nuevosDatos = string_new();

	string_append(&nuevosDatos, datosSeparados[0]);
	string_append(&nuevosDatos, "\n");
	string_append(&nuevosDatos, datosSeparados[1]);
	string_append(&nuevosDatos, "\n");
	string_append(&nuevosDatos, datosSeparados[2]);
	string_append(&nuevosDatos, "\n");

	char* CANTIDAD_LISTA = "CANTIDAD_LISTA=";

	string_append(&nuevosDatos, CANTIDAD_LISTA);
	string_append(&nuevosDatos, nuevoArrayCantidadLista);
	string_append(&nuevosDatos, "\n");

	string_append(&nuevosDatos, datosSeparados[4]);
	string_append(&nuevosDatos, "\n");

	free(nuevoArrayCantidadLista);
	freeDeArray(datosSeparados);
	freeDeArray(lineaCantidadLista);
	freeDeArray(lineaCantidadTotal);

	printf("Nuevos datos:\n%s", nuevosDatos);

	return nuevosDatos;
}

// Compara los arrays cantidadLista y cantidadTotal
// para saber que cantidad lista sea <= a cantidadTotal en el index indicado
// Retorna 1 si se cumple, 0 sino
int cantidadListaNoEsMayorATotal(char* cantidadLista, char* cantidadTotal, int index){

	int retorno = 0;

	char** cantidadListaArray = string_get_string_as_array(cantidadLista);
	char** cantidadTotalArray = string_get_string_as_array(cantidadTotal);

	int cantidadTotalValor = atoi(cantidadTotalArray[index]);
	int cantidadListaValor = atoi(cantidadListaArray[index]);

	if (cantidadListaValor <= cantidadTotalValor){
		retorno = 1;
	}

	freeDeArray(cantidadListaArray);
	freeDeArray(cantidadTotalArray);

	return retorno;
}


// Sumar la cantidad de un plato que ya existe en un pedido
char* sumarCantidadAPlatoExistente(int indexPlatoEnPedido, char* datosPedido, int precioPlato, int cantidadPlatos){
	/*
	 *  ESTADO_PEDIDO=Confirmado
	 *	LISTA_PLATOS=[Milanesa,Empanadas,Ensalada]
	 * 	CANTIDAD_PLATOS=[2,12,1]
	 *	CANTIDAD_LISTA=[1,6,0]
	 *	PRECIO_TOTAL=1150
	 *
	 *	Se suma la cantidad a cantidad_plato[indexPlatoEnPedido]
	 *	Se suma el precio total
	 */

	char** datosSeparados = string_split(datosPedido, "\n");

	char** lineaCantidadTotal = string_split(datosSeparados[2], "=");

	char* nuevoArrayCantidadTotal = sumarCantidadAStringPlatos(lineaCantidadTotal[1], cantidadPlatos, indexPlatoEnPedido);

	char** lineaPrecioTotalSeparada = string_split(datosSeparados[4], "=");

	char* precioTotalString = sumarPrecioTotal(lineaPrecioTotalSeparada[1], precioPlato, cantidadPlatos);

	char* CANTIDAD_PLATOS = "CANTIDAD_PLATOS=";
	char* PRECIO_TOTAL = "PERCIO_TOTAL=";

	char* nuevosDatos = string_new();

	// Pego el estado pedido anterior
	string_append(&nuevosDatos, datosSeparados[0]);
	string_append(&nuevosDatos, "\n");
	// Pego la lista de nombres platos anterior
	string_append(&nuevosDatos, datosSeparados[1]);
	string_append(&nuevosDatos, "\n");

	// Pego las nuevas cantidades totales
	string_append(&nuevosDatos, CANTIDAD_PLATOS);
	string_append(&nuevosDatos, nuevoArrayCantidadTotal);
	string_append(&nuevosDatos, "\n");

	// Pego la lista de cantidad lista anterior
	string_append(&nuevosDatos, datosSeparados[3]);
	string_append(&nuevosDatos, "\n");

	// Pego el nuevo precio total
	string_append(&nuevosDatos, PRECIO_TOTAL);
	string_append(&nuevosDatos, precioTotalString);
	string_append(&nuevosDatos, "\n");

	freeDeArray(datosSeparados);
	freeDeArray(lineaCantidadTotal);
	freeDeArray(lineaPrecioTotalSeparada);

	free(nuevoArrayCantidadTotal);
	free(precioTotalString);

	return nuevosDatos;
}

// Le suma cantidad a un plato en el array de platosTotales
char* sumarCantidadAStringPlatos(char* stringPlatos, int cantidad, int indexArray){
	// stringPlatos = [1,20,50]

	// Sumar index 15 en index 2 => [1,20,65]

	char** preciosSeparados = string_get_string_as_array(stringPlatos);

	int cantidadActual = atoi(preciosSeparados[indexArray]);

	int cantidadTotal = cantidadActual + cantidad;

	asprintf(&preciosSeparados[indexArray], "%i", cantidadTotal);

	char* stringArrayADevolver = generarStringArray(preciosSeparados);

	freeDeArray(preciosSeparados);

	return stringArrayADevolver;
}

// Genera un stringArray de la forma [1,2,3] a partir de los elementosArray
char* generarStringArray(char** elementosArray){
	char* stringArray = string_new();

	string_append(&stringArray, "[");
	string_append(&stringArray, "]");

	int cantElementos = cantidadDeElementosEnArray(elementosArray);

	int i;

	// Itero y agrego cada elemento del elementosArray en el string generado
	for (i = 0; i < cantElementos; i++){
		char* stringMasElemento = agregarElementoEnStringArray(stringArray, elementosArray[i]);
		free(stringArray);
		stringArray = stringMasElemento;
	}

	return stringArray;
}

// Agregar un nuevo plato a un pedido que no lo tiene
char* agregarPlatoNuevoAPedido(char* nombrePlato, char* datosPedido, int precioPlato, int cantidadPlato){

	/*
	 *  ESTADO_PEDIDO=Confirmado
	 *	LISTA_PLATOS=[Milanesa,Empanadas,Ensalada]
	 * 	CANTIDAD_PLATOS=[2,12,1]
	 *	CANTIDAD_LISTA=[1,6,0]
	 *	PRECIO_TOTAL=1150
	 *
	 *	EstadoPedido queda igual
	 *	Agregar pedido a lista_platos = nombrePlato
	 *	Agregar cantidad_platos = cantidadPlato
	 *	Agregar cantidad_lista = 0
	 *  Sumar a precio_total la cantidad precioPlato * cantidadPlato
	 */

	char* datosNuevos = string_new();

	char** datosSeparados = string_split(datosPedido, "\n");

	char** lineaPlatosSeparada = string_split(datosSeparados[1], "=");
	char** lineaCantTotalSeparada = string_split(datosSeparados[2], "=");
	char** lineaCantListaSeparada = string_split(datosSeparados[3], "=");
	char** lineaPrecioTotalSeparada = string_split(datosSeparados[4], "=");

	char* cantTotalString;

	asprintf(&cantTotalString, "%i", cantidadPlato);

	char* lineaPlatosNueva = agregarElementoEnStringArray(lineaPlatosSeparada[1], nombrePlato);
	char* lineaCantTotalNueva = agregarElementoEnStringArray(lineaCantTotalSeparada[1], cantTotalString);
	char* lineaCantListaNueva = agregarElementoEnStringArray(lineaCantListaSeparada[1], "0");

	char* precioTotalString = sumarPrecioTotal(lineaPrecioTotalSeparada[1], precioPlato, cantidadPlato);

//	printf("lineaPlatosNueva: %s\n", lineaPlatosNueva);
//	printf("lineaCantTotalNueva: %s\n", lineaCantTotalNueva);
//	printf("lineaCantListaNueva: %s\n", lineaCantListaNueva);
//	printf("precioTotalString: %s\n", precioTotalString);

	char* LISTA_PLATOS = "LISTA_PLATOS=";
	char* CANTIDAD_PLATOS = "CANTIDAD_PLATOS=";
	char* CANTIDAD_LISTA = "CANTIDAD_LISTA=";
	char* PRECIO_TOTAL = "PRECIO_TOTAL=";

	// Pego el estado que estaba
	string_append(&datosNuevos, datosSeparados[0]);
	string_append(&datosNuevos, "\n");

	// Pego la lista de nombres de platos
	string_append(&datosNuevos, LISTA_PLATOS);
	string_append(&datosNuevos, lineaPlatosNueva);
	string_append(&datosNuevos, "\n");

	// Pego las cantidades totales
	string_append(&datosNuevos, CANTIDAD_PLATOS);
	string_append(&datosNuevos, lineaCantTotalNueva);
	string_append(&datosNuevos, "\n");

	// Pego las cantidades listas
	string_append(&datosNuevos, CANTIDAD_LISTA);
	string_append(&datosNuevos, lineaCantListaNueva);
	string_append(&datosNuevos, "\n");

	// Pego el precio total
	string_append(&datosNuevos, PRECIO_TOTAL);
	string_append(&datosNuevos, precioTotalString);
	string_append(&datosNuevos, "\n");

	freeDeArray(datosSeparados);
	freeDeArray(lineaPlatosSeparada);
	freeDeArray(lineaCantTotalSeparada);
	freeDeArray(lineaCantListaSeparada);
	freeDeArray(lineaPrecioTotalSeparada);

	free(cantTotalString);
	free(lineaPlatosNueva);
	free(lineaCantTotalNueva);
	free(lineaCantListaNueva);
	free(precioTotalString);

	return datosNuevos;
}

// Suma el valor precioPlato * cantidadPlato al valor inicial del precio de un plato
char* sumarPrecioTotal(char* precioInicialString, int precioPlato, int cantidadPlato){

	int precioInicial = atoi(precioInicialString);

	int precioFinal = precioInicial + precioPlato * cantidadPlato;

	char* precioFinalString;

	asprintf(&precioFinalString, "%i", precioFinal);

	return precioFinalString;
}

// Agrega un elemento al final de un stringArray
char* agregarElementoEnStringArray(char* stringArray, char* nombreElemento){

	// stringArray = [cosa,cosa2,cosa3]
	// elemento = cosa4
	// Resultado: [cosa,cosa2,cosa3,cosa4]

	char** arrayElementos = string_get_string_as_array(stringArray);

	int cantidadElementosAnterior = cantidadDeElementosEnArray(arrayElementos);

	char* stringADevolver = string_substring(stringArray, 0, strlen(stringArray) - 1);

	if (cantidadElementosAnterior > 0){
		string_append(&stringADevolver, ",");
	}

	string_append(&stringADevolver, nombreElemento);
	string_append(&stringADevolver, "]");

	freeDeArray(arrayElementos);

	return stringADevolver;
}

void pedirOLiberarBloquesDeSerNecesario(int cantidadBloquesNecesarios, int cantidadBloquesActuales, t_list* listaBloquesActual, char* nombre){
	// Si necesito agregar bloques nuevos
	if (cantidadBloquesNecesarios > cantidadBloquesActuales){
		int cantidadBloquesAPedir = cantidadBloquesNecesarios - cantidadBloquesActuales;

		// Pido la cantidad de bloques que necesito
		t_list* bloquesPedidos = obtenerPrimerosLibresDeBitmap(cantidadBloquesAPedir);

		loguearAsignacionBloques(nombre, bloquesPedidos);

		// Agrego los bloques nuevos a la lista de bloques
		list_add_all(listaBloquesActual, bloquesPedidos);

		list_destroy(bloquesPedidos);
	// Necesito quitar bloques
	} else if (cantidadBloquesNecesarios < cantidadBloquesActuales){

		printf("Se liberan bloques..\n");
		int cantidadBloquesALiberar = cantidadBloquesActuales - cantidadBloquesNecesarios;

		int cantidadBloquesEnLista = list_size(listaBloquesActual);

		while (cantidadBloquesALiberar > 0){
			int* punteroABloque = list_remove(listaBloquesActual, cantidadBloquesEnLista - 1);

			liberarUnBloque(*punteroABloque - 1);

			loguearLiberacionBloque(nombre, *punteroABloque);

			free(punteroABloque);

			cantidadBloquesALiberar--;
			cantidadBloquesEnLista--;
		}

	}
}

int obtenerPlatoEnPedido(char* datosPedido, char* nombrePlato){
	int retorno = -1;

	char** datosSeparados = string_split(datosPedido, "\n");

	char** lineaPlatosSeparada = string_split(datosSeparados[1], "=");

	retorno = encontrarElementoEnArray(nombrePlato, lineaPlatosSeparada[1]);

	freeDeArray(datosSeparados);
	freeDeArray(lineaPlatosSeparada);

	return retorno;
}

// Devuelve el precio de un plato en caso de encontrarlo, si no lo encuentra devuelve -1
int obtenerPrecioPlatoRestaurant(char* nombreRestaurant, char* nombrePlato){

	int retorno = -1;

	char* datosRestaurant = leerDatosRestaurant(nombreRestaurant);

	char** datosSeparados = string_split(datosRestaurant, "\n");

	// Obtengo la lista de platos en lineaPlatos[1]
	char** lineaPlatos = string_split(datosSeparados[3], "=");

	// Encuentro el index
	int indexPlato = encontrarElementoEnArray(nombrePlato, lineaPlatos[1]);

	// Existe el plato en alguna posicion
	if (indexPlato != -1){

		// Obtengo la linea PRECIO_PLATOS=[3,4,5]
		char** preciosListaString = string_split(datosSeparados[4], "=");

		// Obtengo la lista 3,4,5
		char** listaPrecios = string_get_string_as_array(preciosListaString[1]);

		// Retorno el precio del plato
		retorno = atoi(listaPrecios[indexPlato]);

		freeDeArray(preciosListaString);
		freeDeArray(listaPrecios);
	}

	free(datosRestaurant);
	freeDeArray(datosSeparados);
	freeDeArray(lineaPlatos);


	return retorno;
}

// Devuelve el index donde aparece stringAEncontrar y -1 si no se encuentra
int encontrarElementoEnArray(char* stringAEncontrar, char* arrayString){

	int retorno = -1;

	char** stringSeparado = string_get_string_as_array(arrayString);

	int cantElementos = cantidadDeElementosEnArray(stringSeparado);

	int i;
	for(i = 0; i < cantElementos; i++){
		if (strcmp(stringSeparado[i], stringAEncontrar) == 0){
			retorno = i;
		}
	}

	freeDeArray(stringSeparado);

	return retorno;
}

// Devuelve la cantidad de bloques que necesito para un string a escribir
int cantidadDeBloquesQueOcupa(int pesoEnBytes){

	int cantBloques = 0;

	// Se calcula cuantos bloques ocupa en base al peso y al tamaño
	while (pesoEnBytes > 0){
		// El - 4 es ya que cada bloque solo puede utilizar 4 bytes menos del total
		pesoEnBytes-= BLOCK_SIZE - 4;
		cantBloques++;
	}

	//printf("La cantidad de bloques es: %i\n", cantBloques);
	return cantBloques;
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

t_config* leerConfig(char** PUNTO_MONTAJE){

	t_config* config = leerConfiguracion("/home/utnso/workspace/tp-2020-2c-Los-Recursa2/configs/sindicato.config");

	if (config == NULL){
		printf("No se pudo leer la config!!");
		exit(1);
	}

	*PUNTO_MONTAJE = config_get_string_value(config, "PUNTO_MONTAJE");
	PUERTO_ESCUCHA = config_get_string_value(config, "PUERTO_ESCUCHA");
	IP_SINDICATO = config_get_string_value(config, "IP_SINDICATO");

	return config;
}

t_config* leerMetadataBin(){

	// Nombre del archivo metadata
	char* archivoMetadata = "/Metadata.AFIP";

	// Path de la carpeta {punto_montaje}/Metadata/Metadata.bin
	char* pathMetadataBin = malloc(strlen(pathMetadata) + strlen(archivoMetadata) + 1);

	// Le pego el path de metadata
	strcpy(pathMetadataBin, pathMetadata);
	// Le pego al final el path del archivo
	strcat(pathMetadataBin, archivoMetadata);

	// Leo el archivo
	t_config* metadataBin = leerConfiguracion(pathMetadataBin);

	if (metadataBin == NULL){
		printf("No se pudo leer el archivo Metadata/Metadata.AFIP");
		exit(2);
	}

	// Obtengo los valores (pasados por referencia)
	BLOCKS = config_get_int_value(metadataBin, "BLOCKS");
	BLOCK_SIZE = config_get_int_value(metadataBin, "BLOCK_SIZE");
	MAGIC_NUMBER = config_get_string_value(metadataBin,"MAGIC_NUMBER" );

	if (BLOCKS % 8 != 0){
		printf("ERROR | La cantidad de bloques debe ser multiplo de 8\n");
		exit(5);
	}

	free(pathMetadataBin);

//	printf("Blocks: %i\n", BLOCKS);
//	printf("BlockSize: %i\n", BLOCK_SIZE);
//	printf("MagicNumber: %s\n", MAGIC_NUMBER);

	return metadataBin;
}

void testingEscribirBloqueRestaurant(){

	int bloqueSig = 3;

	FILE* bloque = fopen( "25.AFIP" , "w" );

	fwrite(&bloqueSig, sizeof(bloqueSig), 1, bloque);

	fclose(bloque);

}

void testingLeerBloqueRestaurant(){

	int* dato = malloc(sizeof(int));

	FILE* bloque = fopen( "25.AFIP" , "r" );

	fread(dato, sizeof(dato), 1, bloque);

	printf("Dato leido: %i", *dato);

	fclose(bloque);
}

// Generar el string que se debe pegar en los bloques
char* generarSringInfoRestaurant(datosRestaurant unRestaurant){

	// Strings que se usan para guardar los datos
	char* CANTIDAD_COCINEROS = "CANTIDAD_COCINEROS=";
	char* POSICION = "POSICION=";
	char* AFINIDAD_COCINEROS = "AFINIDAD_COCINEROS=";
	char* PLATOS = "PLATOS=";
	char* PRECIO_PLATOS = "PRECIO_PLATOS=";
	char* CANTIDAD_HORNOS = "CANTIDAD_HORNOS=";

	// String donde se guardaran los datos despues del =
	char* cantCocineros;
	char* posicion;
	char* afinidadCocineros;
	char* platos;
	char* preciosPlatos;
	char* cantHornos;

	asprintf(&cantCocineros, "%i\n", unRestaurant.cantCocineros);
	asprintf(&posicion, "%s\n", unRestaurant.posicion);
	asprintf(&afinidadCocineros, "%s\n", unRestaurant.afinidad);
	asprintf(&platos, "%s\n", unRestaurant.platos);
	asprintf(&preciosPlatos, "%s\n", unRestaurant.preciosPlatos);
	asprintf(&cantHornos, "%i\n", unRestaurant.cantHornos);

	char* stringCompleto = malloc(strlen(CANTIDAD_COCINEROS) + strlen(POSICION) + strlen(AFINIDAD_COCINEROS) +
			strlen(PLATOS) + strlen(PRECIO_PLATOS) + strlen(CANTIDAD_HORNOS) +
			strlen(cantCocineros) + strlen(posicion) + strlen(afinidadCocineros) +
			strlen(platos) + strlen(preciosPlatos) + strlen(cantHornos) + 1
			);

	strcpy(stringCompleto, CANTIDAD_COCINEROS);
	strcat(stringCompleto, cantCocineros);
	strcat(stringCompleto, POSICION);
	strcat(stringCompleto, posicion);
	strcat(stringCompleto, AFINIDAD_COCINEROS);
	strcat(stringCompleto, afinidadCocineros);
	strcat(stringCompleto, PLATOS);
	strcat(stringCompleto, platos);
	strcat(stringCompleto, PRECIO_PLATOS);
	strcat(stringCompleto, preciosPlatos);
	strcat(stringCompleto, CANTIDAD_HORNOS);
	strcat(stringCompleto, cantHornos);

	free(cantCocineros);
	free(posicion);
	free(afinidadCocineros);
	free(platos);
	free(preciosPlatos);
	free(cantHornos);

	//printf("String completo:\n%s", stringCompleto);
	printf("Tamaño total: %i\n", strlen(stringCompleto));

	return stringCompleto;
}

char* generarStringInfoReceta(datosReceta unaReceta){
	// Strings que se usan para guardar los datos
	char* PASOS = "PASOS=";
	char* TIEMPO_PASOS = "TIEMPO_PASOS=";

	// String donde se guardaran los datos despues del =
	char* pasos;
	char* tiempoPasos;

	asprintf(&pasos, "%s\n", unaReceta.pasos);
	asprintf(&tiempoPasos, "%s\n", unaReceta.tiempoPasos);

	char* stringCompleto = malloc(strlen(PASOS) + strlen(TIEMPO_PASOS) +
			strlen(pasos) + strlen(tiempoPasos) + 1);

	strcpy(stringCompleto, PASOS);
	strcat(stringCompleto, pasos);
	strcat(stringCompleto, TIEMPO_PASOS);
	strcat(stringCompleto, tiempoPasos);

	free(pasos);
	free(tiempoPasos);

	return stringCompleto;
}

/*
 * Llenar bloques de un nuevo archivo con datos
 * @params:
 * pathArchivo: Archivo donde se guardan los datos
 * stringAEscribir: String completo que se debe escribir en el archivo
 * nombre: Nombre para loguear asignacion de bloques
 */
void llenarBloquesConString(char* pathArchivo, char* stringAEscribir, char* nombre){

	// Calculo la cantidad de bloques necesarios
	int cantBloquesNecesarios = cantidadDeBloquesQueOcupa(strlen(stringAEscribir));

	// Me genero una lista con todos los bloques asignados
	t_list* bloquesAsignados = obtenerPrimerosLibresDeBitmap(cantBloquesNecesarios);

	loguearAsignacionBloques(nombre, bloquesAsignados);

	// Me genero una lista con los datos separados
	t_list* datosSeparadosBloques = separarStringEnBloques(stringAEscribir, cantBloquesNecesarios);

	// Escribo los datos en los bloques
	escribirLineasEnBloques(bloquesAsignados, datosSeparadosBloques);

	// Fijo el SIZE del archivo
	fijarValorArchivoA(pathArchivo, strlen(stringAEscribir), "SIZE");

	int* bloqueInicial = list_get(bloquesAsignados, 0);

	// Fijo el INICIAL_BLOCK del archivo
	fijarValorArchivoA(pathArchivo, *bloqueInicial, "INITIAL_BLOCK");

	destruirListaYElementos(bloquesAsignados);
	destruirListaYElementos(datosSeparadosBloques);
}

void loguearAsignacionBloques(char* nombreEntidad, t_list* listaBloquesAsignados){

	char* stringBloques = string_new();

	int i;
	// Genero un string de la forma 1, 3, 4, 5
	for(i = 0; i< list_size(listaBloquesAsignados); i++){
		char* stringBloque;

		// Obtengo el bloque
		int* punteroABloque = list_get(listaBloquesAsignados, i);

		if (i == list_size(listaBloquesAsignados) - 1)
			// Si es el ultimo elemento no agrego una , al final
			asprintf(&stringBloque, "%i", *punteroABloque);
		else
			// Caso contrario va una coma
			asprintf(&stringBloque, "%i,", *punteroABloque);

		// Pego el string creado al string total de bloques
		string_append(&stringBloques, stringBloque);

		free(stringBloque);
	}

	log_info(logger, "Se le asignan los bloques %s a %s", stringBloques, nombreEntidad);

	free(stringBloques);
}

void loguearLiberacionBloque(char* nombre, int numeroBloque){
	log_info(logger, "Se libera el bloque %i de %s", numeroBloque, nombre);
}

void fijarValorArchivoA(char* pathArchivo, int valor, char* clave){
	t_config* datosMetadata = config_create(pathArchivo);

	char* numeroEnString;

	asprintf(&numeroEnString, "%i", valor);

	config_set_value(datosMetadata, clave, numeroEnString);

	config_save(datosMetadata);

	config_destroy(datosMetadata);

	free(numeroEnString);
}

int leerValorArchivo(char* pathArchivo, char* clave){

	t_config* datosArchivo = config_create(pathArchivo);

	int valorLeido = config_get_int_value(datosArchivo, clave);

	config_destroy(datosArchivo);

	return valorLeido;
}

// Escribir las lineas en listaDatosBloques en los bloques listaBloquesAOcupar
void escribirLineasEnBloques(t_list* listaBloquesAOcupar, t_list* listaDatosBloques){
	if (list_size(listaBloquesAOcupar) != list_size(listaDatosBloques)){
		printf("ERROR | La cantidad de bloques a escribir debe coincidir con la cantidad de datos a escribir");
	}

	int i;

	// Se recorre la lista de bloques que se deben ocupar
	for (i = 0 ; i < list_size(listaBloquesAOcupar) ; i++){
		char* datoAEscribir = list_get(listaDatosBloques, i);
		int* bloqueAOcupar = list_get(listaBloquesAOcupar, i);

		int siguienteBloque;

		// Si es el ultimo bloque => siguiente bloque es 0
		if (i == list_size(listaBloquesAOcupar) - 1){
			siguienteBloque = 0;
		// Si no es el ultimo bloque => siguienteBloque es el numero del siguiente bloque
		} else {
			int* punteroSiguienteBloque = list_get(listaBloquesAOcupar, i + 1);
		    siguienteBloque = *punteroSiguienteBloque;
		}

		// Testing
		//printf("En el bloque %i se escribira el dato:\n%s\n", *bloqueAOcupar , datoAEscribir );

		escribirDatoEnBloque(datoAEscribir, *bloqueAOcupar, siguienteBloque);
	}
}

// Escribir un dato en un bloque determinado
void escribirDatoEnBloque(char* dato, int numBloque, int siguienteBloque){
	// Array de chars para meter el int convertido a array
	char* enteroEnLetras;

	// Genero un array de chars que es el numero de bloque pasado a string
	asprintf(&enteroEnLetras, "%i", numBloque);

	char* extension = ".AFIP";

	// Longitud del entero pasado a char + la extension + / + \n
	char* nombreArchivo = malloc( strlen(enteroEnLetras) + strlen(extension) + 2 );

	// El nombre de archivo es de la forma /{num}.bin | Ejemplo: /35.bin
	strcpy(nombreArchivo, "/");
	strcat(nombreArchivo, enteroEnLetras);
	strcat(nombreArchivo, extension);

	// Creo una copia del path de /Blocks para no modificarlo
	char* pathBloque = malloc(strlen(pathBloques) + strlen(nombreArchivo) + 1);

	// Le pego el path de /Blocks
	strcpy(pathBloque, pathBloques);

	// Le pego el valor hardcodeado 1.bin
	strcat(pathBloque, nombreArchivo);

	FILE* bloque = fopen( pathBloque , "w" );

	// Se escribe el dato en el bloque
	fwrite(dato, strlen(dato) + 1, 1, bloque);

	// Si no es el ultimo bloque se escrie la referencia al siguiente
	if (siguienteBloque != 0)
	fwrite(&siguienteBloque, sizeof(int), 1, bloque);

	fclose(bloque);
	free(nombreArchivo);
	free(pathBloque);
	free(enteroEnLetras);
}

t_list* separarStringEnBloques(char* lineaAEscribir, int cantBloques){

	t_list* listaStrings = list_create();
	int i;

	for (i = 0; i < cantBloques; i++){
		// Recortar la longitud del bloque del string
		char* stringSeparado = string_substring(lineaAEscribir, i * (BLOCK_SIZE - 4), BLOCK_SIZE - 4);
		// Agrego el bloque recortado a la lista de strings
		list_add(listaStrings, stringSeparado);
	}
	return listaStrings;
}

// Crear un nuevo archivo de SIZE e INICIAL_BLOCK vacio
char* crearArchivoVacioEn(char* pathCarpeta, char* nombreArchivo){

	// {puntoMontaje}/Files/pathCarpeta/nombreArchivo
	char* pathCompleto = malloc( strlen(pathCarpeta) + strlen(nombreArchivo) + 2);

	strcpy(pathCompleto, pathCarpeta);
	strcat(pathCompleto, "/");
	strcat(pathCompleto, nombreArchivo);

	FILE* archivo = fopen( pathCompleto , "w" );

	if (archivo == NULL){
		printf("ERROR | No se pudo crear el archivo %s\n", pathCompleto);
		exit(3);
	}

	fclose(archivo);

	t_config* datosArchivo = config_create(pathCompleto);

	// Valores default del archivo
	config_set_value(datosArchivo, "SIZE", "0");
	config_set_value(datosArchivo, "INITIAL_BLOCK", "0");

	config_save(datosArchivo);

	config_destroy(datosArchivo);

	return pathCompleto;
}

// Crear la carpeta de un nuevo restaurant
char* crearCarpetaRestaurant(char* nombreRestaurant){

	char* pathCarpetaRestaurant = malloc(strlen(pathRestaurantes) + strlen(nombreRestaurant) + 2);

	// {puntoMontaje}/Files/Restaurantes/Restaurante1

	strcpy(pathCarpetaRestaurant, pathRestaurantes);
	strcat(pathCarpetaRestaurant, "/");
	strcat(pathCarpetaRestaurant, nombreRestaurant);

	mkdir(pathCarpetaRestaurant, 0777);

	return pathCarpetaRestaurant;
}

// TESTING | Probando leer un bloque y accediendo al numero del siguiente
void leerUnBloque(){
	char* datosLeidos = malloc(BLOCK_SIZE - 4 + 1);
	int* bloqueSiguiente = malloc(sizeof(int));

	char* pathBloque = "/home/utnso/Escritorio/afip/Blocks/2.AFIP";

	FILE* bloque = fopen( pathBloque , "r" );

	// Leyendo bloque
	fread(datosLeidos, BLOCK_SIZE - 4 + 1, 1, bloque);

	// Leyendo referencia a siguiente bloque
	fread(bloqueSiguiente, sizeof(int), 1, bloque);

	printf("datosLeidos: %s", datosLeidos);
	printf("bloqueSiguiente: %i", *bloqueSiguiente);
}

char* generarPathCarpetaRestaurant(char* nombreRestaurant){
	char* pathCarpetaCompleto = malloc(strlen(pathRestaurantes) + strlen(nombreRestaurant) + 2);

	// {punto_montaje}/Files/Restaurantes/NombreRestaurant

	strcpy(pathCarpetaCompleto, pathRestaurantes);
	strcat(pathCarpetaCompleto, "/");
	strcat(pathCarpetaCompleto, nombreRestaurant);

	return pathCarpetaCompleto;
}

// Checkea si la existe el restaurant con nombre nombreRestaurant
// Retorna 1 si existe, 0 si no existe
int existeRestaurant(char* nombreRestaurant){

	char* pathCarpetaCompleto = generarPathCarpetaRestaurant(nombreRestaurant);

	int retorno;

	DIR* dir = opendir(pathCarpetaCompleto);
	if (dir) {
		// La carpeta existe
		retorno = 1;
	} else if (ENOENT == errno) {
		// La carpeta no existe
		retorno = 0;
	} else {
		printf("Hubo un error inesperado al abrir una carpeta pokemon D:");
		retorno = -1;
	}

	closedir(dir);

	free(pathCarpetaCompleto);
	return retorno;
}

// Checkea la existencia de una receta en el FS con nombre nombreReceta
// Retorna 1 si existe, 0 si no existe
int existeReceta(char* nombreReceta){

	char* extension = ".AFIP";

	char* pathRecetaCompleto = malloc(strlen(pathRecetas) + strlen(nombreReceta) + strlen(extension) + 2);

	// {puntoMontaje}/Files/Recetas/nombreReceta.AFIP
	strcpy(pathRecetaCompleto, pathRecetas);
	strcat(pathRecetaCompleto, "/");
	strcat(pathRecetaCompleto, nombreReceta);
	strcat(pathRecetaCompleto, extension);

	FILE * archivoReceta = NULL;

	archivoReceta = fopen(pathRecetaCompleto, "r");

	// El archivo receta no existe
	if (archivoReceta == NULL){
		return 0;
	// El archivo si existe
	} else {
		return 1;
	}
}

char* generarPathInfoRestaurant(char* nombreRestaurante){

	char* archivoInfo = "/Info.AFIP";

	char* pathInfoRestaurant = malloc(strlen(pathRestaurantes) + strlen(nombreRestaurante) + strlen(archivoInfo) + 2);

	// {puntoMontaje}/Files/Restaurantes/nombreRestaurante/Info.AFIP

	strcpy(pathInfoRestaurant, pathRestaurantes);
	strcat(pathInfoRestaurant, "/");
	strcat(pathInfoRestaurant, nombreRestaurante);
	strcat(pathInfoRestaurant, archivoInfo);

	return pathInfoRestaurant;
}

char* generarPathAReceta(char* nombreReceta){

	char* extension = ".AFIP";

	// Expected: pathReceta/nombreReceta.AFIP

	char* pathAReceta = malloc(strlen(pathRecetas) + strlen(nombreReceta) + strlen(extension) + 2);

	strcpy(pathAReceta, pathRecetas);
	strcat(pathAReceta, "/");
	strcat(pathAReceta, nombreReceta);
	strcat(pathAReceta, extension);

	return pathAReceta;
}

t_list* leerListaBloquesAsignados(int sizeBytes, int bloqueInicial){
	t_list* listaBloques = list_create();

	char* pathSiguienteBloque = generarPathABloque(bloqueInicial);

	int* punteroABloqueInicial = malloc(sizeof(int));

	*punteroABloqueInicial = bloqueInicial;

	list_add(listaBloques, punteroABloqueInicial);

	int cantidadBloquesALeer = cantidadDeBloquesQueOcupa(sizeBytes);

	int i;

	int cantidadALeer = sizeBytes;

	for (i = 0; i < cantidadBloquesALeer; i++){

		FILE* bloqueALeer = fopen(pathSiguienteBloque, "r");

		// Estoy leyendo el ultimo bloque
		if (i == cantidadBloquesALeer - 1){
		// No es el ultimo bloque
		} else {

			// Me muevo a la posicion donde empieza el numero que apunta al siguiente bloque
			fseek(bloqueALeer, BLOCK_SIZE - 4 + 1, SEEK_SET);

			free(pathSiguienteBloque);

			int* siguienteBloque = malloc(sizeof(int));

			// Leo el puntero al siguiente bloque para la proxima iteracion
			fread(siguienteBloque, sizeof(int), 1, bloqueALeer);

			pathSiguienteBloque = generarPathABloque(*siguienteBloque);

			list_add(listaBloques, siguienteBloque);

			cantidadALeer -= BLOCK_SIZE - 4;
		}

		fclose(bloqueALeer);
	}

	return listaBloques;
}

char* leerDatosBloques(int sizeBytes, int bloqueInicial){

	char* datosCompletosLeidos = string_new();

	char* pathSiguienteBloque = generarPathABloque(bloqueInicial);

	int cantidadBloquesALeer = cantidadDeBloquesQueOcupa(sizeBytes);

	int i;

	int cantidadALeer = sizeBytes;

	for (i = 0; i < cantidadBloquesALeer; i++){

		FILE* bloqueALeer = fopen(pathSiguienteBloque, "r");

		char* lineaActualLeida;

		// Estoy leyendo el ultimo bloque
		if (i == cantidadBloquesALeer - 1){
			lineaActualLeida = malloc(cantidadALeer + 1);

			// Leo solo lo que me queda por leer (sin leer puntero a ningun bloque)
			fread(lineaActualLeida, cantidadALeer + 1, 1, bloqueALeer);

		// No es el ultimo bloque
		} else {
			lineaActualLeida = malloc(BLOCK_SIZE - 4 + 1);

			// Leo el tamaño completo del bloque
			fread(lineaActualLeida, BLOCK_SIZE - 4 + 1, 1, bloqueALeer);

			free(pathSiguienteBloque);

			int* siguienteBloque = malloc(sizeof(int));

			// Leo el puntero al siguiente bloque para la proxima iteracion
			fread(siguienteBloque, sizeof(int), 1, bloqueALeer);

			pathSiguienteBloque = generarPathABloque(*siguienteBloque);

			free(siguienteBloque);

			cantidadALeer -= BLOCK_SIZE - 4;
		}

		// Pego el bloque leido al string total
		string_append(&datosCompletosLeidos, lineaActualLeida);

		fclose(bloqueALeer);
		free(lineaActualLeida);
	}

	return datosCompletosLeidos;
}

// Obtener la lista de bloques asignados a un pedido
t_list* obtenerListaBloquesPedido(char* nombreRestaurant, int IDPedido){

	char* pathRestaurant = generarPathCarpetaRestaurant(nombreRestaurant);

	char* pathAPedido = generarPathAPedido(pathRestaurant, IDPedido);

	int sizeBytes = leerValorArchivo(pathAPedido, "SIZE");
	int bloqueInicial = leerValorArchivo(pathAPedido, "INITIAL_BLOCK");

	t_list* listaBloques = leerListaBloquesAsignados(sizeBytes, bloqueInicial);

	free(pathRestaurant);
	free(pathAPedido);

	return listaBloques;
}

char* generarPathABloque(int numeroBloque){
	char* numeroEnString;

	char* extension = ".AFIP";

	asprintf(&numeroEnString, "%i", numeroBloque);

	// pathBloques/numeroEnString.AFIP

	char* pathCompleto = malloc(strlen(pathBloques) + strlen(numeroEnString) + strlen(extension) + 2);

	strcpy(pathCompleto, pathBloques);
	strcat(pathCompleto, "/");
	strcat(pathCompleto, numeroEnString);
	strcat(pathCompleto, extension);

	free(numeroEnString);

	return pathCompleto;
}

int existePedido(char* nombreRestaurante, int IDPedido){
	int retorno = 0;

	struct dirent *archivoLeido;

	//{punto_montaje}/Files/nombreRestaurant
	char* pathRestaurant = generarPathCarpetaRestaurant(nombreRestaurante);

	// Retorna un puntero al directorio
	DIR *dr = opendir(pathRestaurant);

	if (dr == NULL)
	{
		printf("ERROR | No se pudo abrir el directorio del restaurant %s\n", nombreRestaurante);
		exit(4);
	}

	char* IDenString;

	// Pedido1.AFIP
	char* pedido = "Pedido";
	char* extension = ".AFIP";

	asprintf(&IDenString, "%i", IDPedido);

	char* archivoBuscado = malloc(strlen(pedido) + strlen(IDenString) + strlen(extension) + 1);

	strcpy(archivoBuscado, pedido);
	strcat(archivoBuscado, IDenString);
	strcat(archivoBuscado, extension);

	while ((archivoLeido = readdir(dr)) != NULL)
	{
		// Nombre del archivo leido dentro del directorio
		char* nombreArchivo = archivoLeido->d_name;

		if (strcmp(nombreArchivo, archivoBuscado) == 0){
			retorno = 1;
		}
	}

	closedir(dr);
	free(pathRestaurant);

	return retorno;
}

char* generarStringPedidoDefault(){
	char* stringPedidoDefault = "ESTADO_PEDIDO=Pendiente\n"
					"LISTA_PLATOS=[aaaaaaaaaaaaaaaaaaaa,bbbbbbbbbbbbbbbbb,ccccccccccccccc]\n"
					"CANTIDAD_PLATOS=[10000,20000,30000]\n"
					"CANTIDAD_LISTA=[10000,20000,30000]\n"
					"PRECIO_TOTAL=0\n";

	printf("TamanioEnBytes: %i", strlen(stringPedidoDefault));

	// Mide 92
	// Neccesito 88

	// 30 + 30 = 60

	// VERSION CORRECTA
//	char* stringPedidoDefault = "ESTADO_PEDIDO=Pendiente\n"
//				"LISTA_PLATOS=[]\n"
//				"CANTIDAD_PLATOS=[]\n"
//				"CANTIDAD_LISTA=[]\n"
//				"PRECIO_TOTAL=0\n";

	char* pedidoDefault = malloc(strlen(stringPedidoDefault) + 1);

	strcpy(pedidoDefault, stringPedidoDefault);

	return pedidoDefault;
}

char* leerDatosRestaurant(char* nombreRestaurante){
	char* pathInfoRestaurant = generarPathInfoRestaurant(nombreRestaurante);

	int sizeBytes = leerValorArchivo(pathInfoRestaurant, "SIZE");
	int bloqueInicial = leerValorArchivo(pathInfoRestaurant, "INITIAL_BLOCK");

	free(pathInfoRestaurant);

	return leerDatosBloques(sizeBytes, bloqueInicial);
}

char* leerDatosReceta(char* nombreReceta){
	char* pathReceta = generarPathAReceta(nombreReceta);

	int sizeBytes = leerValorArchivo(pathReceta, "SIZE");
	int bloqueInicial = leerValorArchivo(pathReceta, "INITIAL_BLOCK");

	free(pathReceta);

	return leerDatosBloques(sizeBytes, bloqueInicial);
}

char* generarPathAPedido(char* pathRestaurant, int IDPedido){
	char* IDenString;

	// Pedido1.AFIP
	char* pedido = "Pedido";
	char* extension = ".AFIP";

	asprintf(&IDenString, "%i", IDPedido);

	char* nombreArchivoPedido = malloc(strlen(pedido) + strlen(IDenString) + strlen(extension) + 1);

	strcpy(nombreArchivoPedido, pedido);
	strcat(nombreArchivoPedido, IDenString);
	strcat(nombreArchivoPedido, extension);

	char* pathAPedido = malloc(strlen(pathRestaurant) + strlen(nombreArchivoPedido) + 2);

	//pathRestaurantes/nombreRestaurant/Pedido1.AFIP
	strcpy(pathAPedido, pathRestaurant);
	strcat(pathAPedido, "/");
	strcat(pathAPedido, nombreArchivoPedido);

	free(nombreArchivoPedido);
	free(IDenString);

	return pathAPedido;
}

char* leerDatosPedido(char* nombreRestaurant, int IDPedido){

	char* pathCarpetaRestaurant = generarPathCarpetaRestaurant(nombreRestaurant);

	char* pathAPedido = generarPathAPedido(pathCarpetaRestaurant, IDPedido);

	int sizeBytes = leerValorArchivo(pathAPedido, "SIZE");
	int bloqueInicial = leerValorArchivo(pathAPedido, "INITIAL_BLOCK");

	free(pathCarpetaRestaurant);
	free(pathAPedido);

	return leerDatosBloques(sizeBytes, bloqueInicial);
}

int pedidoEstaEnEstado(char* nombreEstado, char* datosPedido){
	int retorno = 0;

	// Separo cada linea en un array
	char** datosSeparados = string_split(datosPedido, "\n");

	// lineaEstado[0] = "ESTADO", lineaEstado[1] = "Pendiente"
	char** lineaEstado = string_split(datosSeparados[0], "=");

	// Si el estado actual es Pendiente
	if (strcmp(lineaEstado[1], nombreEstado) == 0){
		retorno = 1;
	}

	freeDeArray(datosSeparados);
	freeDeArray(lineaEstado);

	return retorno;
}

// Cambia el estado del pedido dado el nuevo nombre y el string de datos
char* cambiarEstadoPedidoA(char* nombreEstado, char* datosPedido){

	int unChar;

	// Busco desde donde tengo que recortar los datosPedido (donde termina la primer linea)
	for(unChar = 0; unChar < strlen(datosPedido); unChar++){
		if (datosPedido[unChar] == '\n'){
			break;
		}
	}

	// Recorto la parte siguiente de los datos
	char* datosSinLineaPedido = string_substring_from(datosPedido, unChar);

	char* lineaPedido = "ESTADO_PEDIDO=";

	// String con la lineaPedido el nuevo estado y el resto de los datos
	char* lineaPedidoCompleta = malloc(strlen(lineaPedido) + strlen(nombreEstado) + strlen(datosSinLineaPedido) + 1);

	strcpy(lineaPedidoCompleta, lineaPedido);
	strcat(lineaPedidoCompleta, nombreEstado);
	strcat(lineaPedidoCompleta, datosSinLineaPedido);

	free(datosSinLineaPedido);

	return lineaPedidoCompleta;
}

void printearRespuestaConsultarPlatos(respuesta_consultar_platos* unaRta){
	printf("Longitud total del string: %i\n", unaRta->longitudNombresPlatos);

	printf("Linea platos: %s", unaRta->nombresPlatos);
}

void printearRespuestaObtenerPedido(respuesta_obtener_pedido* unaRta){
	printf("SizeComidas: %i\n",unaRta->sizeComidas );
	printf("Comidas: %s\n",unaRta->comidas );

	printf("SizeCantTotales: %i\n",unaRta->sizeCantTotales );
	printf("CantTotales: %s\n",unaRta->cantTotales );

	printf("SizeCantListas: %i\n",unaRta->sizeCantListas );
	printf("CantListas: %s\n",unaRta->cantListas );

}

void printearRespuestaObtenerRestaurante(respuesta_obtener_restaurante* unaRta){

	printf("CantidadCocineros: %i\n", unaRta->cantidadCocineros);
	printf("CantidadHornos: %i\n", unaRta->cantHornos);
	printf("PosX: %i\n", unaRta->posX);
	printf("PosY: %i\n", unaRta->posY);

	printf("SizeAfinidades: %i\n", unaRta->longitudAfinidades);
	printf("Afinidades: %s\n", unaRta->afinidades);

	printf("SizePlatos: %i\n", unaRta->longitudPlatos);
	printf("Platos: %s\n", unaRta->platos);

	printf("SizePreciosPlatos: %i\n", unaRta->longitudPrecioPlatos);
	printf("PreciosPlatos: %s\n", unaRta->precioPlatos);


}

void printearRespuestaObtenerReceta(respuesta_obtener_receta* unaRta){
	printf("SizePasos: %u\n",unaRta->sizePasos );
	printf("Pasos: %s\n",unaRta->pasos );

	printf("SizeTiempoPasos: %i\n",unaRta->sizeTiempoPasos );
	printf("TiempoPasos: %s\n",unaRta->tiempoPasos );
}

int main(){
	printf("Comienzo sindicato\n");

	// Inicializar semaforo bitmap
	semBitmap = malloc(sizeof(sem_t));
	sem_init(semBitmap, 0, 1);

	// Inicializacion listas de semaforos archivos
	listaSemPedido = list_create();
	listaSemRestaurant = list_create();
	listaSemReceta = list_create();

	char* PUNTO_MONTAJE;
	// Leer la config
	t_config* config = leerConfig(&PUNTO_MONTAJE);

	// Crear logger
	logger = cargarUnLog("/home/utnso/workspace/tp-2020-2c-Los-Recursa2/sindicato/Logs/sindicato.log", "SINDICATO");

	// Crear carpetas del FS
	// {puntoMontaje}/Metadata
	pathMetadata = crearCarpetaEn(PUNTO_MONTAJE, "/Metadata");
	// {puntoMontaje}/Blocks
	pathBloques = crearCarpetaEn(PUNTO_MONTAJE, "/Blocks");
	// {puntoMontaje}/Files
	pathFiles = crearCarpetaEn(PUNTO_MONTAJE, "/Files");
	// {puntoMontaje}/Files/Restaurantes
	pathRestaurantes = crearCarpetaEn(PUNTO_MONTAJE, "/Files/Restaurantes");
	// {puntoMontaje}/Files/Recetas
	pathRecetas = crearCarpetaEn(PUNTO_MONTAJE, "/Files/Recetas");

	// Funcion para leer Metadata.AFIP
	t_config* metadataBin = leerMetadataBin();

	// Si existe el filesystem no se hace nada
	if (existeFilesystem()){
		printf("El filesystem ya existe. No se debe inicializar.\n");
		// Se generan todos los semaforos de los archivos que ya existan
		crearSemaforosArchivosExistentes();
	// Si no existe se genera bitmap y demas verduras
	} else {
		printf("No existe filesystem... inicializando.\n");
		inicializarFileSystem(PUNTO_MONTAJE);
	}

    // Testing | Printear semaforos
//  printearSemaforosRestaurantes();
//  printearSemaforosPedidos();
//  printearSemaforosRecetas();

	// ---- A partir de aca el FS ya existe ----

//	obtenerRestaurante("ElDestino");

	// Testing semaforos
//	char* restaurant1 = "ElDestino";
//	crearSemaforoRestaurant(restaurant1);
//	pthread_t hilo1;
//	pthread_t hilo2;
//	pthread_t hilo3;
//	pthread_create(&hilo1, NULL, (void*)abrirArchivo1, NULL);
//	pthread_create(&hilo2, NULL, (void*)abrirArchivo2, NULL);
//	pthread_create(&hilo3, NULL, (void*)abrirArchivo3, NULL);
//	pthread_join(hilo1, NULL);
//	pthread_join(hilo2, NULL);
//	pthread_join(hilo3, NULL);


	pthread_t hiloConsola;
	// Hilo para leer el input de la consola
    pthread_create(&hiloConsola, NULL, (void*)obtenerInputConsola, NULL);
    pthread_detach(hiloConsola);

    // Iniciar servidor para recibir mensajes
    iniciar_server(PUERTO_ESCUCHA);

//  pthread_join(hiloConsola, NULL);

	// Liberaciones finales (a las que nunca se llega)
	config_destroy(config);
	config_destroy(metadataBin);
	log_destroy(logger);

	return 0;
}
