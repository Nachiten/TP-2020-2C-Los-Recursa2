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

void abrirArchivo1();
void abrirArchivo2();
void abrirArchivo3();
void crearSemaforoRestaurant(char*);
void crearSemaforoPedido(char*, int);
void crearSemaforoReceta(char* nombreReceta);
void waitSemaforoRestaurant(char*);
void signalSemaforoRestaurant(char*);
void waitSemaforoPedido(char*, int);
void signalSemaforoPedido(char*, int);
void waitSemaforoReceta(char* nombreReceta);
void signalSemaforoReceta(char* nombreReceta);

void crearSemaforosArchivosExistentes();
void crearSemaforosRestaurantesExistentes();
void crearSemaforosRecetasExistentes();
void crearSemaforosPedidosExistentes();
void crearSemaforosPedidosRestaurant(char* nombreRestaurant);
t_list* escanearCarpetasExistentes(char* path);


#endif /* SRC_SEMAFOROS_H_ */
