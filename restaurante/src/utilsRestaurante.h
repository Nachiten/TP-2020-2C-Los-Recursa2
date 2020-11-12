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

//cosas cargadas del archivo config
t_config* configuracion;
t_log* logger;
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
t_list* listaDeHornos;
t_queue* colaNew;
t_list* listaDeColasReady;
t_list* colaBlock;
t_queue* colaParaHornear;

sem_t* mutexNew;
sem_t* mutexListaReady;
sem_t* mutexBlock;
//los hornos no necesitarian mutex, usariamos un hilo que los va sacando y colocando en los mismos

sem_t* contadorPlatosEnNew;




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

typedef enum{
	REPOSAR = 1,
	HORNEAR,
	OTRO
}t_paso_receta;

/*
typedef enum{
	REPOSADO = 1,
	HORNEADO,
}accionBlock;
*/

typedef struct{
	uint32_t idPedido;
	char* nombrePlato;
	t_list* pasosReceta;
    int instruccionesRealizadasDeUnPaso;
    int quantumRestante;
    //puede que no sea necesario
    //accionBlock accionBlock
}pcb_plato;

typedef struct{
	int idHorno;
	int enUso;
}t_horno;

typedef struct{
	t_paso_receta accion;
	uint32_t duracionAccion;
}paso_receta;


typedef struct{
	char* afinidad;
    t_queue* cola;
}cola_ready;

void inicializarRestaurante();
void obtenerMetadataRestaurante();
void crearColasPlanificacion();
void hiloExecCocinero(char* afinidad);
void agregarANew(pcb_plato*);
void agregarAReady(pcb_plato*);
void agregarABlock(pcb_plato*);
void obtenerSiguiente(cola_ready*);
pcb_plato* obtenerSiguiente();


#endif /* SRC_UTILSRESTAURANTE_H_ */
