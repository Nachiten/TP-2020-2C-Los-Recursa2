#ifndef SRC_CLIENTE_H_
#define SRC_CLIENTE_H_

#include <stdlib.h>
#include <stdio.h>
#include "shared/estructuras.h"
#include "shared/cargador.h"

uint32_t PIDCliente;
t_log* logger;
t_config* config;
int32_t socketEscucha;
int32_t socketApp;
int32_t socketComanda;
int32_t socketSindicato;
int32_t socketRestaurante;

//cosas cargadas del archivo config
char* LOG_PATH;
char* comanda_IP;
char* restaurante_IP;
char* sindicato_IP;
char* app_IP;
char* app_puerto;
char* comanda_puerto;
char* sindicato_puerto;
char* restaurante_puerto;
char* mi_puerto;
uint32_t miPosicionX;
uint32_t miPosicionY;






#endif /* SRC_CLIENTE_H_ */
