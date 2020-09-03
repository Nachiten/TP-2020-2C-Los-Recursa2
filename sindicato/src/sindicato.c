/*
 * sindicato.c
 *
 *  Created on: 29 ago. 2020
 *      Author: utnso
 */

#include "sindicato.h"

// Devuelve la cantidad de bloques que necesito para un string a escribir
int cantidadDeBloquesQueOcupa(int pesoEnBytes){

	int cantBloques = 0;

	// Se calcula cuantos bloques ocupa en base al peso y al tamaño
	while (pesoEnBytes > 0){
		// El - 4 es ya que cada bloque solo puede utilizar 4 bytes menos del total
		pesoEnBytes-= BLOCK_SIZE - 4;
		cantBloques++;
	}

	printf("La cantidad de bloques es: %i\n", cantBloques);
	return cantBloques;
}

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

void crearRestaurant(char* nombreRestaurant, datosRestaurant unRestaurant){
	// Crear la carpeta del nuevo restaurant
	char* pathCarpetaRestaurant = crearCarpetaRestaurant(nombreRestaurant);
	// Crear el archivo Info.AFIP
	char* pathArchivoRestaurant = crearArchivoVacioEn(pathCarpetaRestaurant, "Info.AFIP");

	llenarBloquesRestaurant(pathArchivoRestaurant, unRestaurant);

	free(pathCarpetaRestaurant);
	free(pathArchivoRestaurant);
}

void llenarBloquesRestaurant(char* pathArchivoRestaurant, datosRestaurant unRestaurant){

	// Armo el string enorme de datos que tengo que escribir
	char* stringRestaurant = generarSringInfoRestaurant(unRestaurant);

	// Calculo la cantidad de bloques necesarios
	int cantBloquesNecesarios = cantidadDeBloquesQueOcupa(strlen(stringRestaurant));

	// Me genero una lista con todos los bloques asignados
	t_list* bloquesAsignados = obtenerPrimerosLibresDeBitmap(cantBloquesNecesarios);

	// Me genero una lista con los datos separados
	t_list* datosSeparadosBloques = separarStringEnBloques(stringRestaurant, cantBloquesNecesarios);

	escribirLineasEnBloques(bloquesAsignados, datosSeparadosBloques);

	// TODO | Llenar SIZE e INICIAL_BLOCK del archivo

	// Fijo el SIZE del archivo
	fijarValorArchivoA(pathArchivoRestaurant, strlen(stringRestaurant), "SIZE");

	int* bloqueInicial = list_get(bloquesAsignados, 0);

	// Fijo el INICIAL_BLOCK del archivo
	fijarValorArchivoA(pathArchivoRestaurant, *bloqueInicial, "INITIAL_BLOCK");

	free(stringRestaurant);

	destuirListaElementos(bloquesAsignados);
	destuirListaElementos(datosSeparadosBloques);
}

void fijarValorArchivoA(char* pathArchivo, int numero, char* valor){
	t_config* datosMetadata = config_create(pathArchivo);

	char* numeroEnString;

	asprintf(&numeroEnString, "%i", numero);

	config_set_value(datosMetadata, valor, numeroEnString);

	config_save(datosMetadata);

	config_destroy(datosMetadata);

	free(numeroEnString);
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

	// TODO Aparte de escribir el dato se debe escribir la referencia al siguiente bloque
	fwrite(dato, strlen(dato) + 1, 1, bloque);

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

		//printf("STRING SEPARADO %i: %s\n", i, stringSeparado);
	}

	return listaStrings;

}

char* crearArchivoVacioEn(char* pathCarpeta, char* nombreArchivo){

	// {puntoMontaje}/Files/pathCarpeta/nombreArchivo
	char* pathCompleto = malloc( strlen(pathCarpeta) + strlen(nombreArchivo) + 2);

	strcpy(pathCompleto, pathCarpeta);
	strcat(pathCompleto, "/");
	strcat(pathCompleto, nombreArchivo);

	FILE* archivo = fopen( pathCompleto , "w" );

	if (archivo == NULL){
		printf("No se pudo crear el archivo %s\n", pathCompleto);
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

char* crearCarpetaRestaurant(char* nombreRestaurant){

	char* pathCarpetaRestaurant = malloc(strlen(pathRestaurantes) + strlen(nombreRestaurant) + 2);

	// {puntoMontaje}/Files/Restaurantes/Restaurante1

	strcpy(pathCarpetaRestaurant, pathRestaurantes);
	strcat(pathCarpetaRestaurant, "/");
	strcat(pathCarpetaRestaurant, nombreRestaurant);

	mkdir(pathCarpetaRestaurant, 0777);

	return pathCarpetaRestaurant;
}

void leerUnBloque(){
	char* datosLeidos = malloc(BLOCK_SIZE - 4 + 1);
	int* bloqueSiguiente = malloc(sizeof(int));

	char* pathBloque = "/home/utnso/Escritorio/afip/Blocks/2.AFIP";

	FILE* bloque = fopen( pathBloque , "r" );

	// TODO Aparte de escribir el dato se debe escribir la referencia al siguiente bloque
	fread(datosLeidos, BLOCK_SIZE - 4 + 1, 1, bloque);

	fread(bloqueSiguiente, sizeof(int), 1, bloque);

	printf("datosLeidos: %s", datosLeidos);
	printf("bloqueSiguiente: %i", *bloqueSiguiente);
}

// Checkea si la carpeta nombreParticular existe dentro de la carpeta nombreCarpeta
// Retorna 1 si la carpeta existe, 0 si no existe
int existeRestaurant(char* nombreRestaurant){

	char* pathCarpetaCompleto = malloc(strlen(pathRestaurantes) + strlen(nombreRestaurant) + 2);

	// {punto_montaje}/Files/Restaurantes/NombreRestaurant

	strcpy(pathCarpetaCompleto, pathRestaurantes);
	strcat(pathCarpetaCompleto, "/");
	strcat(pathCarpetaCompleto, nombreRestaurant);

	//printf("Path carpeta pokemon: %s\n", pathCarpetaPokemon);

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

int main(){
	printf("Comienzo sindicato\n");

	// Inicializar semaforo bitmap
	//semBitmap = malloc(sizeof(sem_t));
	//sem_init(semBitmap, 0, 1);

	char* PUNTO_MONTAJE;
	t_config* config = leerConfig(&PUNTO_MONTAJE);

	// {puntoMontaje}/Metadata
	pathMetadata = crearCarpetaEn(PUNTO_MONTAJE, "/Metadata");
	// {puntoMontaje}/Blocks
	pathBloques = crearCarpetaEn(PUNTO_MONTAJE, "/Blocks");
	// {puntoMontaje}/Restaurantes
	pathFiles = crearCarpetaEn(PUNTO_MONTAJE, "/Files");
	// {puntoMontaje}/Restaurantes
	pathRestaurantes = crearCarpetaEn(PUNTO_MONTAJE, "/Files/Restaurantes");

	// Funcion para leer metadata.bin
	t_config* metadataBin = leerMetadataBin();

	// Si existe el filesystem no se hace nada
	if (existeFilesystem()){
		printf("El filesystem ya existe. No se debe inicializar.\n");
	// Si no existe se genera bitmap y demas verduras
	} else {
		printf("No existe filesystem... inicializando.\n");
		inicializarFileSystem(PUNTO_MONTAJE);
	}

	// ---- A partir de aca el FS ya existe ----

	// Leer input de consola
	obtenerInputConsola();

	//leerUnBloque();

	// Liberaciones finales (a las que nunca se llega)
	config_destroy(config);
	config_destroy(metadataBin);

	return 0;
}
