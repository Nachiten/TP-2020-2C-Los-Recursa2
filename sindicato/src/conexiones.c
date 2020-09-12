/*
 * conexiones.c
 *
 *  Created on: 11 sep. 2020
 *      Author: utnso
 */

#include "conexiones.h"

void process_request(codigo_operacion cod_op, int32_t socket_cliente, uint32_t sizeAAllocar)  {

	switch(cod_op){
		case CONSULTAR_PLATOS:
			break;
		case ERROR:
			pthread_exit(NULL);
			break;
		case DESCONEXION:
			pthread_exit(NULL);
			break;
		default:
			break;
	}

}

void serve_client(int32_t* socket)
{
	while(1){
		int32_t sizeAAllocar;
		codigo_operacion cod_op;
		int32_t recibidosSize = 0;

		int32_t recibidos = recv(*socket, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);
		bytesRecibidos(recibidos);

		if(recibidos >= 1)
		{
			//sem_wait(semRecibirMensajes);

			recibidosSize = recv(*socket, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
			bytesRecibidos(recibidosSize);
			printf("Tamaño de lo que sigue en el buffer: %u.\n", sizeAAllocar);

			process_request(cod_op, *socket, sizeAAllocar);

			//sem_post(semRecibirMensajes);
		}


		else
		{
			pthread_exit(NULL);
		}

		recibidosSize = 0;
		recibidos = 0;
	}
}

void esperar_cliente(int32_t socket_servidor)
{
	struct sockaddr_in dir_cliente;

	socklen_t tam_direccion = sizeof(struct sockaddr_in);
	pthread_t thread;
	int32_t socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	pthread_create(&thread, NULL, (void*)serve_client, &socket_cliente);
	pthread_detach(thread);
}

void iniciar_server(char* ip, char* puerto)
{
	int32_t socket_servidor;
	socket_servidor = crearSocketServidor(ip, puerto);

    while(1)
    {
    	esperar_cliente(socket_servidor);
    }

}
