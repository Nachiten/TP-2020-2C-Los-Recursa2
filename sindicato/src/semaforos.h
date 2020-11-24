/*
 * semaforos.h
 *
 *  Created on: 10 sep. 2020
 *      Author: utnso
 */

#ifndef SRC_SEMAFOROS_H_
#define SRC_SEMAFOROS_H_

#include "sindicato.h"

typedef struct{
	sem_t* semaforo;
	char* nombreRestaurant;
}semRestaurant;

typedef struct{
	sem_t* semaforo;
	char* nombreReceta;
}semReceta;

typedef struct{
	sem_t* semaforo;
	char* nombreRestaurant;
	int numPedido;
}semPedido;

t_list* listaSemRestaurant;
t_list* listaSemPedido;
t_list* listaSemReceta;

//void abrirArchivo1();
//void abrirArchivo2();
//void abrirArchivo3();

// Creacion semaforos
void crearSemaforoRestaurant(char*);
void crearSemaforoPedido(char*, int);
void crearSemaforoReceta(char*);

// Waits y signals
void waitSemaforoRestaurant(char*);
void waitSemaforoPedido(char*, int);
void waitSemaforoReceta(char*);
void signalSemaforoRestaurant(char*);
void signalSemaforoPedido(char*, int);
void signalSemaforoReceta(char*);

// Funciones auxiliares
void crearSemaforosArchivosExistentes();
void crearSemaforosRestaurantesExistentes();
void crearSemaforosRecetasExistentes();
void crearSemaforosPedidosExistentes();
void crearSemaforosPedidosRestaurant(char*);
t_list* escanearCarpetasExistentes(char*);

// Prints
void printearSemaforosRestaurantes();
void printearSemaforosRecetas();
void printearSemaforosPedidos();


#endif /* SRC_SEMAFOROS_H_ */
