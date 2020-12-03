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

//***VARIABLES***
t_list* listaRestos;
t_list* listaPedidos;
t_list* listaAsociaciones;

// OTROS
t_log* logger;
sem_t* semLog;


// VARIABLES DE CONFIG
int GRADO_MULTIPROCE;
int RETARDO_CICLO_CPU;
char* puerto_commanda;
char* ip_commanda;
int32_t posX_restoDefault;
int32_t posY_restoDefault;

// SEMAFOROS
sem_t* planificacionInicializada;
sem_t* contadorProcesosEnNew;
sem_t* mutexNew;
sem_t* mutexReady;
sem_t* mutexBlock;
sem_t* mutexPedidosListos;
sem_t* mutexListaRestos;
sem_t* mutexListaAsociaciones;
sem_t* mutexListaPedidos;
sem_t* contadorRepartidoresDisp;
// Semaforos para sincronizar hilos con HCM
t_list* listaSemHabilitarCicloExec;
t_list* listaSemFinalizarCicloExec;
sem_t* habilitarCicloBlockReady;
sem_t* finalizarCicloBlockReady;

// *** ESTRUCTURA DE PLANIFICACION ***
typedef struct{
	// ID Repartidor
	int numeroRepartidor;
	// Posicion Repartidor
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
	// Esta cansado o no
	int cansado;
	// Esta asignado a algun pedido
	int asignado;
}repartidor;

typedef struct{
	int32_t socket;
	char* nombre_resto;
	char* ip;
	char* puerto;
	int32_t posX;
	int32_t posY;
}info_resto;

typedef enum{
	// No esta en block
	NO_BLOCK,
	// No tiene nada que hacer (solo esta descansando)
	DESCANSANDO,
	// Esta esperando un msg (llego al restaurant)
	ESPERANDO_MSG,
	// Esperando para terminar el pedido (llego a cliente)
	ESPERANDO_EXIT,
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
double alpha;
uint32_t estimacion_inicial;

typedef enum{
	READY,
	EXIT,
}estado;

typedef struct{
	int32_t socket_cliente;
	char* idCliente;
	char* nombreRestaurante;
	uint32_t id_pedido_global;
	uint32_t id_pedido_resto;
	uint32_t perfilActivo;
	int32_t posClienteX;
	int32_t posClienteY;
}perfil_pedido;

typedef struct{
	char* idCliente;
	char* nombreRestaurante;
	int32_t posClienteX;
	int32_t posClienteY;
}asociacion_cliente;

typedef struct{
	int pedidoID;
	int pedidoIDGlobal;
	int instruccionesTotales;//rafaga a realizar
	int instruccionesRealizadas;//lo cumplido de la rafaga a realizar
	int instruccionesAnteriores;
	int estimacionActual;
	int estimacionAnterior;
	repartidor* repartidorAsignado;
	int posRestauranteX;
	int posRestauranteY;
	int posClienteX;
	int posClienteY;
	objetivoViaje objetivo;
	accionBlock accionBlocked;
	estado proximoEstado;
	int tiempoEspera;
	int32_t socketCliente;
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
uint32_t valor_para_switch_case_planificacion(char* algoritmo);
void freeDeArray(char**);
void asignarRepartidorAPedido(pcb_pedido*);
void printearValorSemaforo(sem_t*, char*);
int modulo(int);
int distanciaDeRepartidorAObjetivo(repartidor*, pcb_pedido*);
pcb_pedido* obtenerSiguienteDeReady();
pcb_pedido* obtenerSiguienteHRRN();
pcb_pedido* obtenerSiguienteSJFSD();
int codigoDesalojo(pcb_pedido*);
int sigoEjecutando(pcb_pedido*);

int buscarPedidoPorID(uint32_t);


// SEMAFOROS
void waitSemaforoHabilitarCicloExec(uint32_t);
void signalSemaforoHabilitarCicloExec(uint32_t);
void waitSemaforoFinalizarCicloExec(uint32_t);
void signalSemaforoFinalizarCicloExec(uint32_t);

#endif /* SRC_PLANIFICACION_H_ */
