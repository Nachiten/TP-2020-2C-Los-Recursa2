/*
 * restaurante.c
 *
 *  Created on: 29 ago. 2020
 *      Author: utnso
 */


#include <stdlib.h>
#include <stdio.h>
#include "restaurante.h"

// t_list lista_cola_afinidades = [t_cola_afinidad, t_cola__afinidad2]
// t_list_cocineros = [t_cocinero_1, t_cocinero_2, t_cocinero3]
// t_list_hornos = [t_horno]
// t_queue cola_horno = [t_plato]
// t_list milanesa
// t_list sandwich



//{
// t_list* cola
//char* afinidad


//}t_cola_afinidad

int main(){

	inicializarRestaurante();

	socket_sindicato = establecer_conexion(ip_sindicato,puerto_sindicato);

	iniciar_server(puerto_local);

	return 0;
}

//*******************MENSAJES DE RESTO*******************

void consultar_platos(int32_t socket_cliente){

}

//*******************FUNCIONES DEL SERVER*******************

void process_request(codigo_operacion cod_op, int32_t socket_cliente, uint32_t sizeAAllocar)  {
	switch(cod_op){

	case CONSULTAR_PLATOS:
		consultar_platos(socket_cliente);
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
			recibidosSize = recv(*socket, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
			bytesRecibidos(recibidosSize);
			printf("Tamaño de lo que sigue en el buffer: %u.\n", sizeAAllocar);

			process_request(cod_op, *socket, sizeAAllocar);
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
	int32_t* socket_cliente = malloc(sizeof(int32_t));// todo verificar que no da problemas
	*socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	//pthread_create(&thread,NULL,(void*)serve_client,&socket_cliente);
	pthread_create(&thread,NULL,(void*)serve_client,socket_cliente);
	pthread_detach(thread);
}

void iniciar_server(char* puerto)
{
	int32_t socket_servidor;
	socket_servidor = reservarSocket(puerto);

    while(1)
    {
    	esperar_cliente(socket_servidor);
    }

}
