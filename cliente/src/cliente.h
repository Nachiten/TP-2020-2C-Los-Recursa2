#ifndef SRC_CLIENTE_H_
#define SRC_CLIENTE_H_

#include <stdlib.h>
#include <stdio.h>
#include "shared/estructuras.h"
#include "shared/cargador.h"
#include "shared/socket.h"
#include "shared/terminarPrograma.h"
#include "shared/utilidades.h"
#include "funcionesCliente.h"
#include <pthread.h>
#include <semaphore.h>


t_log* logger;
t_config* config;
int32_t socketEscucha;
pthread_t hiloConsola;
pthread_t hiloNotificaciones;
sem_t* comandoParaEjecutar;
sem_t* semLog;

//cosas cargadas del archivo config
char* LOG_PATH;
char* ip_destino;
char* puerto_destino;
char* puerto_local;
//char* puerto_APP;
//char* ip_APP;
char* idCliente;
uint32_t miPosicionX;
uint32_t miPosicionY;

int32_t socketDelHandshake;

//obtiene el comando que quiere ejecutar el usuario
void obtenerInputConsolaCliente();
//void recibirNotificaciones(int32_t* elSocketInicial);




#endif /* SRC_CLIENTE_H_ */
