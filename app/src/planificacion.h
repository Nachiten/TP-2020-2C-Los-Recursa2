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
sem_t* habilitarCicloBlockReady;
sem_t* finalizarCicloBlockReady;

typedef struct{
	int numeroRepartidor;
	int posX;
	int posY;
	// Cada cuanto tiene que descansar
	int frecuenciaDescanso;
	// Cuanto tiempo tiene que descansar
	int tiempoDescanso;
	// Cuanto tiempo ya descanso
	int tiempoDescansado;
	int asignado;
}repartidor;

typedef enum{
	// No esta en blocked
	NO,
	// Esta esperando un msg
	ESPERANDO_MSG,
	// Esta descansando
	DESCANSANDO,
}estadoBlock;

typedef struct{
	int pedidoID;
	int instruccionesTotales;
	repartidor* repartidorAsignado;
	int instruccionesRealizadas;
	int posObjetivoX;
	int posObjetivoY;
	estadoBlock estadoBlocked;
}pcb_pedido;

// HILOS
void hiloBlock_Ready();
void hiloCiclosMaestro();
void hiloExec(int*);
void hiloNew_Ready();

// AGREGAR A COLAS
void agregarABlock(pcb_pedido*);
void agregarAReady(pcb_pedido*);
void agregarANew(pcb_pedido*);

// INICIALIZACION
void iniciarSemaforosCiclos();
void iniciarSemaforosPlanificacion();
void leerPlanificacionConfig(t_config*);

// OTROS
void iniciarPlanificacion();
void freeDeArray(char**);
void asignarRepartidorAPedido(pcb_pedido*);
void printearValorSemaforo(sem_t*, char*);
int modulo(int);
int distanciaDeRepartidorAObjetivo(repartidor*, pcb_pedido*);
pcb_pedido* obtenerSiguienteDeReady();
int codigoDesalojo(pcb_pedido*);

// SEMAFOROS
void waitSemaforoHabilitarCicloExec(uint32_t);
void signalSemaforoHabilitarCicloExec(uint32_t);
void waitSemaforoFinalizarCicloExec(uint32_t);
void signalSemaforoFinalizarCicloExec(uint32_t);

#endif /* SRC_PLANIFICACION_H_ */
