/*
 * planificacion.h
 *
 *  Created on: 28 sep. 2020
 *      Author: utnso
 */

#ifndef SRC_PLANIFICACION_H_
#define SRC_PLANIFICACION_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/collections/queue.h>
#include<string.h>
#include<pthread.h>
#include<inttypes.h> // Para tener el uint32_t
#include<semaphore.h> // para los semaforos
#include"shared/estructuras.h"
#include"shared/cargador.h"
#include"shared/socket.h"

//#include "app.h"

// LISTAS/COLAS
t_list* repartidores;
t_queue* colaNew;
t_list* colaReady;
t_list* colaBlock;
t_list* pedidosListos;

// OTROS
t_log* logger;

// VARIABLES DE CONFIG
int GRADO_MULTIPROCE;
int RETARDO_CICLO_CPU;

// SEMAFOROS
sem_t* contadorProcesosEnNew;
sem_t* mutexNew;
sem_t* mutexReady;
sem_t* mutexBlock;
sem_t* mutexPedidosListos;
sem_t* contadorRepartidoresDisp;
// Semaforos para sincronizar hilos con HCM
t_list* listaSemHabilitarCicloExec;
t_list* listaSemFinalizarCicloExec;
sem_t* habilitarCicloBlockReady;
sem_t* finalizarCicloBlockReady;

// *** ESTRUCTURA DE PLANIFICACION ***
typedef struct{
	int numeroRepartidor;
	int posX;
	int posY;
	// Cada cuanto tiene que descansar
	int frecuenciaDescanso;
	// Cuantas instrucciones totales realizo
	// sin descanso
	int instruccionesRealizadas;
	// Cuanto tiempo tiene que descansar
	int tiempoDescanso;
	// Cuanto tiempo ya descanso
	int tiempoDescansado;
	int asignado;
}repartidor;

typedef enum{
	// No esta en block
	NO_BLOCK,
	// No tiene nada que hacer (solo esta descansando)
	DESCANSANDO,
	// Esta esperando un msg (llego al restaurant)
	ESPERANDO_MSG,
	// Esperando para terminar el pedido (llego a cliente)
	ESPERANDO_EXIT,

	// Esta descansando
	//DESCANSANDO,
}accionBlock;

typedef enum{
	RESTAURANTE,
	CLIENTE,
}objetivoViaje;

typedef enum{
	FIFO,
	HRRN,
	SJFSD,
}algoritmo_planif;

algoritmo_planif algoritmo;

typedef struct{
	int pedidoID;
	int instruccionesTotales;
	int instruccionesRealizadas;
	repartidor* repartidorAsignado;
	int posRestauranteX;
	int posRestauranteY;
	int posClienteX;
	int posClienteY;
	objetivoViaje objetivo;
	accionBlock estadoBlocked;
	// Esta cansado o no
	int cansado;
	int tiempoEspera;
}pcb_pedido;

// PEDIDO LISTO
void guardarPedidoListo(int);
void eliminarPedidoListo(int);
int checkearPedidoListo(int);

// HILOS
void hiloBlock_Ready();
void hiloCiclosMaestro();
void hiloExec(int*);
void hiloNew_Ready();

// AGREGAR A COLAS
void agregarABlock(pcb_pedido*);
void agregarAReady(pcb_pedido*);
void agregarANew(pcb_pedido*);
void agregarAExit(pcb_pedido*);

// OTROS
uint32_t valor_para_switch_case(char* algoritmo);
void freeDeArray(char**);
void asignarRepartidorAPedido(pcb_pedido*);
void printearValorSemaforo(sem_t*, char*);
int modulo(int);
int distanciaDeRepartidorAObjetivo(repartidor*, pcb_pedido*);
pcb_pedido* obtenerSiguienteDeReady();
pcb_pedido* obtenerSiguienteHRRN();
pcb_pedido* obtenerSiguienteSJFSD();
int codigoDesalojo(pcb_pedido*);

// SEMAFOROS
void waitSemaforoHabilitarCicloExec(uint32_t);
void signalSemaforoHabilitarCicloExec(uint32_t);
void waitSemaforoFinalizarCicloExec(uint32_t);
void signalSemaforoFinalizarCicloExec(uint32_t);

#endif /* SRC_PLANIFICACION_H_ */
