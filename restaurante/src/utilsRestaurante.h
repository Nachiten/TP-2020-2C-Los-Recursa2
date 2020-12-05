/*
 * utilsRestaurante.h
 *
 *  Created on: 16 sep. 2020
 *      Author: utnso
 */

#ifndef SRC_UTILSRESTAURANTE_H_
#define SRC_UTILSRESTAURANTE_H_

#include"shared/estructuras.h"
#include"shared/socket.h"
#include"shared/cargador.h"
#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<semaphore.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/node.h>
#include<commons/collections/list.h>
#include<commons/string.h>
#include<stdbool.h>
#include<commons/collections/queue.h>
#include<pthread.h>
#include<math.h>

//me traje de restaurante.h
sem_t* semId;
int32_t id_global;

//cosas cargadas del archivo config
t_config* configuracion;
t_log* logger;
int32_t socket_sindicato;
char* LOG_PATH;
char* puerto_local;
char* ip_sindicato;
char* puerto_sindicato;
char* puerto_app;
char* ip_app;
char* algoritmoElegido;
uint32_t quantumElegido;
uint32_t RETARDO_CICLO_CPU;
char* nombreRestaurante;
char* appEnPruebas;
char* ip_local;
int32_t socket_sindicato;
uint32_t miPosicionX;
uint32_t miPosicionY;
uint32_t cantCocineros;
uint32_t cantRecetas;
uint32_t cantHornos;
char* platos;
char* afinidades;
char** listaPlatos;
char** listaAfinidades;

//Listas estrictamente usadas en la planificacion con sus mutuas exclusiones
t_queue* colaNew;
t_list* colaBlock;
t_list* listaDeColasReady;
t_queue* colaParaHornear;
t_list* platosHorneandose;
t_list* listaPedidos;
t_queue* colaReadySinAfinidad;

sem_t* semLog;
sem_t* semListaPedidos;
sem_t* mutexNew;
sem_t* mutexListaReady;
sem_t* mutexBlock;
//Creo que no seria necesario ya que 1 solo hilo manipula la cola para esperar y la de uso
sem_t* mutexColaHornos;
sem_t* contadorPlatosEnNew;

// Semaforos para sincronizar hilos con HCM
t_list* listaSemHabilitarCicloExec;
t_list* listaSemFinalizarCicloExec;
sem_t* habilitarCicloBlockReady;
sem_t* finalizarCicloBlockReady;
sem_t* habilitarCicloEntradaSalida;
sem_t* finalizarCicloEntradaSalida;

/*
typedef enum{
	NEW = 1,
	READY,
	EXEC,
	BLOCK,
	EXIT,
}t_estado;


typedef enum{
	FIFO = 1,
	RR,
}t_algoritmoplanif;
*/

typedef struct{
	int32_t numPedido;
	int32_t socket_cliente;
}perfil_pedido;

typedef enum{
	REPOSAR = 1,
	HORNEAR,
	OTRO,
}t_paso_receta;

typedef enum{
	NO_BLOCK = 1,
	REPOSO,
	HORNO,
}accionBlock;

typedef struct{
	uint32_t idPedido;
	char* nombrePlato;
	t_list* pasosReceta;
    int quantumRestante;
    //es necesario al final
    accionBlock motivoBlock;
    int duracionBlock;
}pcb_plato;

/*
typedef struct{
	int idHorno;
	int enUso;
}t_horno;
*/

typedef struct{
	t_paso_receta accion;
	uint32_t duracionAccion;
}paso_receta;

typedef struct{
	char* afinidad;
    t_queue* cola;
}cola_ready;

typedef struct{
	int idHilo;
	char* afinidad;
}credencialesCocinero;

void inicializarRestaurante(char*);
void obtenerMetadataRestaurante();
void crearColasPlanificacion();
void crearHilosPlanificacion();
//al final no la termine necesitando
//void crearHornos();
void hiloNewReady();
void hiloBlockReady();
void hiloEntradaSalida();
void hiloExecCocinero(credencialesCocinero*);
void agregarANew(pcb_plato*);
void agregarAReady(pcb_plato*);
void agregarABlock(pcb_plato*);
void agregarAExit(pcb_plato*);
pcb_plato* obtenerSiguienteDeReadySinAfinidad();
pcb_plato* obtenerSiguienteDeReadyConAfinidad(char*);

int buscar_pedido_por_id(uint32_t);

// PEDIDO LISTO
void chequearSiElPedidoEstaListo(int);

void inicializar_planificacion();
void iniciarSemaforos();
void iniciarSemaforosPlanificacion();
void iniciarSemaforosCiclos();
void hiloCiclosMaestro();
void waitSemaforoHabilitarCicloExec(uint32_t indice);
void signalSemaforoHabilitarCicloExec(uint32_t indice);
void waitSemaforoFinalizarCicloExec(uint32_t indice);
void signalSemaforoFinalizarCicloExec(uint32_t indice);

void freeDeArray(char**);
#endif /* SRC_UTILSRESTAURANTE_H_ */
