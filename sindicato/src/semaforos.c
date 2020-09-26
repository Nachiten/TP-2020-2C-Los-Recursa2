/*
 * semaforos.c
 *
 *  Created on: 10 sep. 2020
 *      Author: Nachiten
 */

#include "semaforos.h"

/*
 * Semaforos de restaurantes Info.AFIP
 * Semaforos de restaurantes PedidoN.AFIP
 * Semaforos de recetas
 */

// Testing semaforos
void abrirArchivo1(){

	waitSemaforoRestaurant("ElDestino");

	printf("Hilo 1 abrio el archivo restaurant\n");

	printf("SLEEP 5 SEGUNDOS HILO 1\n");
	sleep(5);

	signalSemaforoRestaurant("ElDestino");

	printf("Hilo 1 cerro el archivo restaurant\n");
}

void abrirArchivo2(){

	waitSemaforoRestaurant("ElDestino");

	printf("Hilo 2 abrio el archivo restaurant\n");

	printf("SLEEP 5 SEGUNDOS HILO 1\n");
	sleep(5);

	signalSemaforoRestaurant("ElDestino");

	printf("Hilo 2 cerro el archivo restaurant\n");
}

void abrirArchivo3(){

	waitSemaforoRestaurant("ElDestino");

	printf("Hilo 3 abrio el archivo restaurant\n");

	printf("SLEEP 5 SEGUNDOS HILO 1\n");
	sleep(5);

	signalSemaforoRestaurant("ElDestino");

	printf("Hilo 3 cerro el archivo restaurant\n");
}

void crearSemaforoRestaurant(char* nombreRestaurant){
	semRestaurant* semaforoRestaurant = malloc(sizeof(semRestaurant));

	sem_t* semaforoNuevo = malloc(sizeof(sem_t));

	sem_init(semaforoNuevo, 0, 1);

	semaforoRestaurant->nombreRestaurant = nombreRestaurant;
	semaforoRestaurant->semaforo = semaforoNuevo;

	list_add(listaSemRestaurant, semaforoRestaurant);
}

void crearSemaforoPedido(char* nombreRestaurant, int numPedido){
	semPedido* semaforoPedido = malloc(sizeof(semPedido));

	sem_t* semaforoNuevo = malloc(sizeof(sem_t));

	sem_init(semaforoNuevo, 0, 1);

	semaforoPedido->nombreRestaurant = nombreRestaurant;
	semaforoPedido->numPedido = numPedido;
	semaforoPedido->semaforo = semaforoNuevo;

	list_add(listaSemPedido, semaforoPedido);
}

void crearSemaforoReceta(char* nombreReceta){
	semReceta* semaforoReceta = malloc(sizeof(semReceta));

	sem_t* semaforoNuevo = malloc(sizeof(sem_t));

	sem_init(semaforoNuevo, 0, 1);

	char* nombreRecetaCopia = malloc(strlen(nombreReceta) + 1);
	strcpy(nombreRecetaCopia, nombreReceta);

	semaforoReceta->nombreReceta = nombreRecetaCopia;
	semaforoReceta->semaforo = semaforoNuevo;

	list_add(listaSemReceta, semaforoReceta);
}

void waitSemaforoRestaurant(char* nombreRestaurant){
	int i;

	for ( i = 0; i< list_size(listaSemRestaurant); i++){
		semRestaurant* semaforoActual = list_get(listaSemRestaurant, i);

		if (strcmp(nombreRestaurant, semaforoActual->nombreRestaurant) == 0){

			sem_wait(semaforoActual->semaforo);
			//printf("WAIT | %s\n", pokemon);
			return;
		}
	}
	printf("No se encontro el semaforo restaurant deseado\n");
}

void signalSemaforoRestaurant(char* nombreRestaurant){
	int i;

	for ( i = 0; i< list_size(listaSemRestaurant); i++){
		semRestaurant* semaforoActual = list_get(listaSemRestaurant, i);

		if (strcmp(nombreRestaurant, semaforoActual->nombreRestaurant) == 0){

			sem_post(semaforoActual->semaforo);
			//printf("SIGNAL | %s\n", pokemon);
			return;
		}
	}
	printf("No se encontro el semaforo restaurant deseado\n");
}

void waitSemaforoPedido(char* nombreRestaurant, int numPedido){
	int i;

	for ( i = 0; i< list_size(listaSemPedido); i++){
		semPedido* semaforoActual = list_get(listaSemPedido, i);

		if (strcmp(nombreRestaurant, semaforoActual->nombreRestaurant) == 0
				&& numPedido == semaforoActual->numPedido){

			sem_wait(semaforoActual->semaforo);
			//printf("WAIT | %s\n", pokemon);
			return;
		}
	}
	printf("No se encontro el semaforo pedido deseado\n");
}

void signalSemaforoPedido(char* nombreRestaurant, int numPedido){
	int i;

	for ( i = 0; i< list_size(listaSemPedido); i++){
		semPedido* semaforoActual = list_get(listaSemPedido, i);

		if (strcmp(nombreRestaurant, semaforoActual->nombreRestaurant) == 0
				&& numPedido == semaforoActual->numPedido){

			sem_post(semaforoActual->semaforo);
			//printf("WAIT | %s\n", pokemon);
			return;
		}
	}
	printf("No se encontro el semaforo pedido deseado\n");
}

void waitSemaforoReceta(char* nombreReceta){
	int i;

	for ( i = 0; i< list_size(listaSemReceta); i++){
		semReceta* semaforoActual = list_get(listaSemReceta, i);

		if (strcmp(nombreReceta, semaforoActual->nombreReceta) == 0){

			sem_wait(semaforoActual->semaforo);
			//printf("WAIT | %s\n", pokemon);
			return;
		}
	}
	printf("No se encontro el semaforo receta deseado\n");
}

void signalSemaforoReceta(char* nombreReceta){
	int i;

	for ( i = 0; i< list_size(listaSemReceta); i++){
		semReceta* semaforoActual = list_get(listaSemReceta, i);

		if (strcmp(nombreReceta, semaforoActual->nombreReceta) == 0){

			sem_post(semaforoActual->semaforo);
			//printf("SIGNAL | %s\n", pokemon);
			return;
		}
	}
	printf("No se encontro el semaforo receta deseado\n");
}

void crearSemaforosArchivosExistentes(){
	crearSemaforosRestaurantesExistentes();
	crearSemaforosRecetasExistentes();
	crearSemaforosPedidosExistentes();
}

void printearSemaforosRestaurantes(){
	int i;
	for (i = 0; i<list_size(listaSemRestaurant); i++){

		semRestaurant* unSemaforoRestaurant = list_get(listaSemRestaurant, i);

		printf("Nombre restaurant: %s\n", unSemaforoRestaurant->nombreRestaurant);
	}

}

void printearSemaforosRecetas(){
	int i;
	for (i = 0; i<list_size(listaSemReceta); i++){

		semReceta* unSemaforoRestaurant = list_get(listaSemReceta, i);

		printf("Nombre receta: %s\n", unSemaforoRestaurant->nombreReceta);
	}
}

void printearSemaforosPedidos(){
	int i;
	for (i = 0; i<list_size(listaSemPedido); i++){

		semPedido* unPedido = list_get(listaSemPedido, i);

		printf("Numero pedido: %i\n", unPedido->numPedido);
		printf("Nombre restaurant de pedido: %s\n", unPedido->nombreRestaurant);
	}
}

void crearSemaforosRestaurantesExistentes()
{
	t_list* listaNombresRestaurant = escanearCarpetasExistentes(pathRestaurantes);

	int i;

	int cantElementos = list_size(listaNombresRestaurant);

	for(i = cantElementos - 1; i >= 0; i--)
	{
		char* unRestaurant = list_get(listaNombresRestaurant, i);

		crearSemaforoRestaurant(unRestaurant);
	}

	//destruirListaYElementos(listaNombresRestaurant);
}

void crearSemaforosRecetasExistentes()
{
	t_list* listaNombresRecetas = escanearCarpetasExistentes(pathRecetas);

	int i;

	int cantElementos = list_size(listaNombresRecetas);

	for(i = cantElementos - 1; i >= 0; i--)
	{
		char* unaReceta = list_get(listaNombresRecetas, i);

		crearSemaforoReceta(unaReceta);
	}

	destruirListaYElementos(listaNombresRecetas);
}

void crearSemaforosPedidosExistentes(){
	t_list* listaNombresRestaurant = escanearCarpetasExistentes(pathRestaurantes);

	int i;
	for (i = 0; i < list_size(listaNombresRestaurant); i++){

		char* nombreRestaurant = list_get(listaNombresRestaurant, i);
		crearSemaforosPedidosRestaurant(nombreRestaurant);
	}
	destruirListaYElementos(listaNombresRestaurant);
}

void crearSemaforosPedidosRestaurant(char* nombreRestaurant){
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

		// Nombre: PedidoNNN.AFIP
		char** archivoSeparado = string_split(punteroANombre, ".");

		char* numeroPedidoString = string_substring_from(archivoSeparado[0], 6);

		int numeroPedido = atoi(numeroPedidoString);

		char* nombreRestaurantCopia = malloc(strlen(nombreRestaurant) + 1);

		strcpy(nombreRestaurantCopia, nombreRestaurant);

		crearSemaforoPedido(nombreRestaurantCopia, numeroPedido);

		freeDeArray(archivoSeparado);
		free(numeroPedidoString);
	}

	closedir(dr);
	free(pathCarpetaRestaurant);
}

t_list* escanearCarpetasExistentes(char* path){
	struct dirent *archivoLeido;

	t_list* listaCarpetasExistenes = list_create();

	// Retorna un puntero al directorio | {puntoMontaje}/Files
	DIR *dr = opendir(path);

	if (dr == NULL)
	{
		printf("No se pudo abrir el directorio actual" );
	}

	while ((archivoLeido = readdir(dr)) != NULL)
	{
		// Nombre del archivo leido dentro del directorio
		char* punteroANombre = archivoLeido->d_name;

		// Si el archivo es . .. o Metadata.bin es ignorado
		if (strcmp(punteroANombre, ".") == 0 || strcmp(punteroANombre, "..") == 0){
			continue;
		}

		// Se separa el archivo para sacar el nombre antes de la extension
		char** nombreSeparado = string_split(punteroANombre, ".");

		// Se crea una copia del puntero para meterlo en la lista
		char* punteroANombreLista = malloc(strlen(nombreSeparado[0]) + 1);
		strcpy(punteroANombreLista, nombreSeparado[0]);

		// Si es una carpeta pokemon lo agrego a la lista
		list_add(listaCarpetasExistenes, punteroANombreLista);

		freeDeArray(nombreSeparado);
	}

	closedir(dr);

	return listaCarpetasExistenes;

}

