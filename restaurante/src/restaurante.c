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
// t_list cocineros = [t_cocinero_1, t_cocinero_2, t_cocinero3]
// t_list hornos = [t_horno]
// t_queue cola_horno = [t_plato]
// t_list milanesa
// t_list sandwich



//{
// t_list* cola
//char* afinidad


//}t_cola_afinidad

int main(){

	inicializarRestaurante();
	inicializar_colas();

	socket_sindicato = establecer_conexion(ip_sindicato,puerto_sindicato);

	iniciar_server(puerto_local);

	return 0;
}

//*******************MENSAJES DE RESTO*******************

void consultar_Platos(int32_t socket_cliente){
	respuesta_consultar_platos* platos = malloc(sizeof(respuesta_consultar_platos));
	consultar_platos* consulta;

	consulta = malloc(sizeof(consultar_platos));
	consulta->sizeNombre = strlen(nombreRestaurante);
	consulta->nombre = malloc(strlen(nombreRestaurante) + 1);
	strcpy(consulta->nombre, nombreRestaurante);

	mandar_mensaje(consulta,CONSULTAR_PLATOS,socket_sindicato);

    codigo_operacion codigoRecibido;
    bytesRecibidos(recv(socket_sindicato, &codigoRecibido, sizeof(codigo_operacion), MSG_WAITALL));

    printf("El codigo recibido del emisor es: %d", codigoRecibido);

    uint32_t sizePayload;
    bytesRecibidos(recv(socket_sindicato, &sizePayload, sizeof(uint32_t), MSG_WAITALL));

    printf("El size del buffer/payload es: %u", sizePayload);

    recibir_mensaje(platos,RESPUESTA_CONSULTAR_PLATOS,socket_sindicato);

    mandar_mensaje(platos,RESPUESTA_CONSULTAR_PLATOS,socket_cliente);

    free(consulta->nombre);
    free(consulta);
    free(platos->nombresPlatos);
    free(platos);
}

void crear_Pedido(int32_t socket_cliente){
	guardar_pedido* pedida_a_guardar;
	respuesta_crear_pedido* respuesta;
	Pedido* pedido= malloc(sizeof(Pedido));
	pedido->socket_cliente = socket_cliente;
	pedido->numPedido = crear_id_pedidos();

	list_add(listaPedidos,pedido);

	pedida_a_guardar = malloc(sizeof(guardar_pedido));
	pedida_a_guardar->idPedido = pedido->numPedido;
	pedida_a_guardar->largoNombreRestaurante = strlen(nombreRestaurante);
	pedida_a_guardar->nombreRestaurante = malloc(sizeof(strlen(nombreRestaurante) + 1));
	strcpy(pedida_a_guardar->nombreRestaurante,nombreRestaurante);

	mandar_mensaje(pedida_a_guardar,GUARDAR_PEDIDO,socket_sindicato);

	respuesta = malloc(sizeof(respuesta_crear_pedido));
	respuesta->idPedido = pedido->numPedido;

	mandar_mensaje(respuesta,RESPUESTA_CREAR_PEDIDO,socket_cliente);
	free(respuesta);
}

void aniadir_plato(a_plato* recibidoAPlato){

}

//*******************FUNCIONES DE RESTO*******************
void inicializar_semaforos(){
	semId = malloc(sizeof(sem_t));
	semLog = malloc(sizeof(sem_t));

	sem_init(semId, 0, 1);
	sem_init(semLog, 0, 1);
}

void inicializar_colas(){
	listaPedidos = list_create(); //inicializo la lista de pedidos
}

int32_t crear_id_pedidos(){
	sem_wait(semId);
	id_global += 1;
	sem_post(semId);
	return id_global;
}

int buscar_pedido_por_id(uint32_t id_pedido){
	Pedido* pedido;
	for(int i = 0; i < listaPedidos->elements_count; i++){
		pedido = list_get(listaPedidos,i);// conseguis el perfil del cliente

		if(pedido->numPedido == id_pedido){
			return i;
		}
	}
	return -2;
}

//*******************FUNCIONES DEL SERVER*******************

void process_request(codigo_operacion cod_op, int32_t socket_cliente, uint32_t sizeAAllocar)  {
	a_plato* recibidoAPlato;
	switch(cod_op){

	case CONSULTAR_PLATOS:
		consultar_Platos(socket_cliente);
		break;

	case CREAR_PEDIDO:
		crear_Pedido(socket_cliente);
		break;

	case A_PLATO:
		recibidoAPlato = malloc(sizeAAllocar);
		recibir_mensaje(recibidoAPlato,A_PLATO,socket_cliente);
		aniadir_plato(recibidoAPlato);
		free(recibidoAPlato);
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
