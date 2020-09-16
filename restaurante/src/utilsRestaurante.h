/*
 * utilsRestaurante.h
 *
 *  Created on: 16 sep. 2020
 *      Author: utnso
 */

#ifndef SRC_UTILSRESTAURANTE_H_
#define SRC_UTILSRESTAURANTE_H_

#include"shared/estructuras.h"
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
char* LOG_PATH;
char* puerto_local;
char* ip_sindicato;
char* puerto_sindicato;
char* puerto_app;
char* ip_app;
uint32_t miPosicionX;
uint32_t miPosicionY;
char* algoritmoElegido;
uint32_t quantumElegido;
char* nombreRestaurante;

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



void inicializarRestaurante();
void obtenerRestaurante();


#endif /* SRC_UTILSRESTAURANTE_H_ */
