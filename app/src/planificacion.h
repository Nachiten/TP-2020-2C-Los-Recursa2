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
t_list* pedidosListos;

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
sem_t* mutexPedidosListos;
// Contador de repartidores disponibles
sem_t* contadorRepartidoresDisp;
// Lista de semaforos binarios para sincronizar la cantidad de hilos exec que se dispongan
t_list* listaSemHabilitarCicloExec;
t_list* listaSemFinalizarCicloExec;
sem_t* habilitarCicloBlockReady;
sem_t* finalizarCicloBlockReady;

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
