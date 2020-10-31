#ifndef SRC_COMANDA_H_
#define SRC_COMANDA_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include<string.h>
#include<pthread.h>
#include <inttypes.h> // Para tener el uint32_t?
#include "semaphore.h" //wtf? ya esta incluido en manejoMemoria.h por que no lo toma?
#include "manejoMemoria.h"
#include "estructurasCoMAnda.h"
#include "shared/estructuras.h"
#include "shared/cargador.h"
#include "shared/socket.h"

uint32_t PIDCoMAnda;
t_log* logger;
t_config* config;
int32_t socketEscucha;

sem_t* semaforoLogger;

//cosas cargadas del archivo config***********************************************************
char* LOG_PATH;
char* cliente_IP;
char* app_IP;
char* mi_puerto;
char* app_puerto;
char* cliente_puerto;
uint32_t TAMANIO_MEMORIA_PRINCIPAL;
uint32_t TAMANIO_AREA_DE_SWAP;
uint32_t FRECUENCIA_COMPACTACION;
char* ALGOR_REEMPLAZO;

//para manejo de mensajes que llegan**********************************************************
pthread_t hilo_recibir_mensajes;

void recepcion_mensajes();
void esperar_conexiones(int32_t miSocket);
void escuchar_mensajes(int32_t* socket_conexion_establecida);
void procesar_mensaje(codigo_operacion cod_op, int32_t sizeAAllocar, int32_t socket);

//otros***************************************************************************************
void inicializarSemaforos(); //self explanatory

#endif /* SRC_COMANDA_H_ */
