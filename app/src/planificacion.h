/*
 * planificacion.h
 *
 *  Created on: 28 sep. 2020
 *      Author: utnso
 */

#ifndef SRC_PLANIFICACION_H_
#define SRC_PLANIFICACION_H_

#include "app.h"

t_list* repartidores;
t_queue* colaNew;
t_list* colaReady;
t_list* colaBlock;

int GRADO_MULTIPROCE;
int RETARDO_CICLO_CPU;

// Cantidad de procesos en new
sem_t* contadorProcesosEnNew;
// Cantidad de procesos en ready
//sem_t* contadorProcesosEnReady;
// Mutex para tocar cola new
sem_t* mutexNew;
// Mutex para tocar cola ready
sem_t* mutexReady;
// Mutex para tocar cola block
sem_t* mutexBlock;
// Contador de repartidores disponibles
sem_t* contadorRepartidoresDisp;
// Lista de semaforos binarios para sincronizar la cantidad de hilos exec que se dispongan
t_list* listaSemHabilitarCicloExec;
t_list* listaSemFinalizarCicloExec;
sem_t* habilitarCicloReady;
sem_t* finalizarCicloReady;
sem_t* habilitarCicloBlock;
sem_t* finalizarCicloBlock;

typedef struct{
	int numeroRepartidor;
	int posX;
	int posY;
	int frecuenciaDescanso;
	int tiempoDescanso;
	int asignado;
}repartidor;

typedef struct{
	int instruccionesTotales;
	repartidor* repartidorAsignado;
	int instruccionesRealizadas;
	int posObjetivoX;
	int posObjetivoY;
}pcb_pedido;

void leerPlanificacionConfig(t_config*);
void iniciarPlanificacion();
void freeDeArray(char**);
void agregarANew(pcb_pedido* unPlato);
void iniciarSemaforosPlanificacion();
void asignarRepartidorAPedido(pcb_pedido*);
int modulo(int);
int distanciaDeRepartidorAObjetivo(repartidor*, pcb_pedido*);
void hiloNew_Ready();
void hiloExec(int* numeroHilo);
void printearValorSemaforo(sem_t*, char*);
pcb_pedido* obtenerSiguienteDeReady();
void hiloCiclosMaestro();
void agregarAReady(pcb_pedido*);
int codigoDesalojo(pcb_pedido*);

// SEMAFOROS
void waitSemaforoHabilitarCicloExec(uint32_t);
void signalSemaforoHabilitarCicloExec(uint32_t);
void waitSemaforoFinalizarCicloExec(uint32_t);
void signalSemaforoFinalizarCicloExec(uint32_t);

#endif /* SRC_PLANIFICACION_H_ */
