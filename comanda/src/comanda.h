#ifndef SRC_COMANDA_H_
#define SRC_COMANDA_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "estructurasCoMAnda.h"
#include "shared/estructuras.h"
#include "shared/cargador.h"
#include "shared/socket.h"

uint32_t PIDCoMAnda;
t_log* logger;
t_config* config;
int32_t socketEscucha;
int32_t socketApp;
int32_t socketCliente;

//cosas cargadas del archivo config
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

//cosas que necesita coMAnda para trabajar
void* MEMORIA_PRINCIPAL;
void* AREA_DE_SWAP;




//para manejo de mensajes que llegan
void recepcion_mensajes(void* argumento_de_adorno);
void esperar_conexiones(int32_t miSocket);
void escuchar_mensajes(int32_t socket_conexion_establecida);
void procesar_mensaje(codigo_operacion cod_op, int32_t sizeAAllocar, int32_t socket);





#endif /* SRC_COMANDA_H_ */
