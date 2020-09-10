/*
 * semaforos.c
 *
 *  Created on: 10 sep. 2020
 *      Author: utnso
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
	semPedido* semaforoRestaurant = malloc(sizeof(semRestaurant));

	sem_t* semaforoNuevo = malloc(sizeof(sem_t));

	sem_init(semaforoNuevo, 0, 1);

	semaforoRestaurant->nombreRestaurant = nombreRestaurant;
	semaforoRestaurant->numPedido = numPedido;
	semaforoRestaurant->semaforo = semaforoNuevo;

	list_add(listaSemPedido, semaforoRestaurant);
}

void crearSemaforoReceta(char* nombreReceta){
	semReceta* semaforoReceta = malloc(sizeof(semRestaurant));

	sem_t* semaforoNuevo = malloc(sizeof(sem_t));

	sem_init(semaforoNuevo, 0, 1);

	semaforoReceta->nombreReceta = nombreReceta;
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


