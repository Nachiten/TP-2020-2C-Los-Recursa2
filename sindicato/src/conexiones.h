/*
 * conexiones.h
 *
 *  Created on: 11 sep. 2020
 *      Author: utnso
 */

#ifndef SRC_CONEXIONES_H_
#define SRC_CONEXIONES_H_

#include "sindicato.h"
#include<sys/socket.h>
//#include<unistd.h>
#include<netdb.h>

void serve_client(int32_t* socket);
void esperar_cliente(int32_t socket_servidor);
void iniciar_server(char* ip, char* puerto);

#endif /* SRC_CONEXIONES_H_ */
