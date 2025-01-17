/*
 * conexiones.c
 *
 *  Created on: 11 sep. 2020
 *      Author: utnso
 */

#include "conexiones.h"

/*
 * ProcessRequest:
 *
 * Consultar platos - Done
 * Obtener restaurante - Done
 * Obtener receta - Done
 * Guardar pedido - Done
 * Obtener pedido - Done
 * Guardar plato - Done
 * Confirmar pedido - Done
 * Plato listo - Done
 * Terminar pedido - Done
 */


void process_request(codigo_operacion cod_op, int32_t* socket_cliente, uint32_t sizeAAllocar)  {

	switch(cod_op){
		case CONSULTAR_PLATOS:;
			consultar_platos* estructuraMensajePlatos = malloc(sizeof(consultar_platos));
			recibir_mensaje(estructuraMensajePlatos, CONSULTAR_PLATOS, *socket_cliente);

			log_info(logger, "[ReciboMSG] CONSULTAR_PLATOS | nombreResto=%s", estructuraMensajePlatos->nombreResto);
			consultarPlatos_sindicato(estructuraMensajePlatos->nombreResto, *socket_cliente);

			free(estructuraMensajePlatos->nombreResto);
			free(estructuraMensajePlatos->id);
			free(estructuraMensajePlatos);
	    break;

		case OBTENER_RESTAURANTE:;
			obtener_restaurante* estructuraMensajeRest = malloc(sizeof(obtener_restaurante));
            recibir_mensaje(estructuraMensajeRest, OBTENER_RESTAURANTE, *socket_cliente);

            log_info(logger, "[ReciboMSG] OBTENER_RESTAURANTE : nombreResto=%s", estructuraMensajeRest->nombreRestaurante);
            obtenerRestaurante_sindicato(estructuraMensajeRest->nombreRestaurante, *socket_cliente);

            free(estructuraMensajeRest->nombreRestaurante);
            free(estructuraMensajeRest);
		break;

		case OBTENER_RECETA:;
			obtener_receta* estructuraMensajeRecet = malloc(sizeof(obtener_receta));
			recibir_mensaje(estructuraMensajeRecet, OBTENER_RECETA, *socket_cliente);

			log_info(logger, "[ReciboMSG] OBTENER_RECETA : nombreReceta=%s", estructuraMensajeRecet->nombreReceta);
			obtenerReceta(estructuraMensajeRecet->nombreReceta, *socket_cliente);
			free(estructuraMensajeRecet->nombreReceta);
			free(estructuraMensajeRecet);
		break;

		case GUARDAR_PEDIDO:;
            guardar_pedido* estructuraMensajeGuardarPedido = malloc(sizeof(guardar_pedido));
            recibir_mensaje(estructuraMensajeGuardarPedido, GUARDAR_PEDIDO, *socket_cliente);

            log_info(logger, "[ReciboMSG] GUARDAR_PEDIDO : nombreResto=%s | idPedido=%i", estructuraMensajeGuardarPedido->nombreRestaurante, estructuraMensajeGuardarPedido->idPedido);
            guardarPedido_sindicato(estructuraMensajeGuardarPedido->nombreRestaurante, estructuraMensajeGuardarPedido->idPedido, *socket_cliente);
			free(estructuraMensajeGuardarPedido->nombreRestaurante);
			free(estructuraMensajeGuardarPedido);
		break;

		case OBTENER_PEDIDO:;
            guardar_pedido* estructuraMensajeObtenerP = malloc(sizeof(guardar_pedido));
            recibir_mensaje(estructuraMensajeObtenerP, OBTENER_PEDIDO, *socket_cliente);

            log_info(logger, "[ReciboMSG] OBTENER_PEDIDO : nombreResto=%s | idPedido=%i", estructuraMensajeObtenerP->nombreRestaurante, estructuraMensajeObtenerP->idPedido);
            obtenerPedido(estructuraMensajeObtenerP->nombreRestaurante, estructuraMensajeObtenerP->idPedido, *socket_cliente);
			free(estructuraMensajeObtenerP->nombreRestaurante);
			free(estructuraMensajeObtenerP);
		break;

		case GUARDAR_PLATO:;
			guardar_plato* estructuraMensajeGuardarPlato = malloc(sizeof(guardar_plato));
			recibir_mensaje(estructuraMensajeGuardarPlato, GUARDAR_PLATO, *socket_cliente);

			log_info(logger, "[ReciboMSG] GUARDAR_PLATO : nombreResto=%s | idPedido=%i | nombrePlato=%s | cantidadPlatos=%i",
					estructuraMensajeGuardarPlato->nombreRestaurante, estructuraMensajeGuardarPlato->idPedido, estructuraMensajeGuardarPlato->nombrePlato, estructuraMensajeGuardarPlato->cantidadPlatos);
			guardarPlato(estructuraMensajeGuardarPlato->nombreRestaurante,
					estructuraMensajeGuardarPlato->idPedido,
					estructuraMensajeGuardarPlato->nombrePlato,
					estructuraMensajeGuardarPlato->cantidadPlatos,
					*socket_cliente);
			free(estructuraMensajeGuardarPlato->nombreRestaurante);
			free(estructuraMensajeGuardarPlato);

        break;

		case CONFIRMAR_PEDIDO:;
			guardar_pedido* estructuraMensajeConfirmarP = malloc(sizeof(guardar_pedido));
			recibir_mensaje(estructuraMensajeConfirmarP, CONFIRMAR_PEDIDO, *socket_cliente);

			log_info(logger, "[ReciboMSG] CONFIRMAR_PEDIDO : nombreResto=%s | idPedido=%i", estructuraMensajeConfirmarP->nombreRestaurante, estructuraMensajeConfirmarP->idPedido);
			confirmarPedido_sindicato(estructuraMensajeConfirmarP->nombreRestaurante, estructuraMensajeConfirmarP->idPedido, *socket_cliente);
			free(estructuraMensajeConfirmarP->nombreRestaurante);
			free(estructuraMensajeConfirmarP);

		break;

		case PLATO_LISTO:;
			plato_listo* estructuraMensajePlatoL = malloc(sizeof(plato_listo));
			recibir_mensaje(estructuraMensajePlatoL, PLATO_LISTO, *socket_cliente);

			log_info(logger, "[ReciboMSG] PLATO_LISTO : nombreResto=%s | idPedido=%i | nombrePlato=%s", estructuraMensajePlatoL->nombreRestaurante, estructuraMensajePlatoL->idPedido, estructuraMensajePlatoL->nombrePlato);
			platoListo(estructuraMensajePlatoL->nombreRestaurante, estructuraMensajePlatoL->idPedido, estructuraMensajePlatoL->nombrePlato,  *socket_cliente);
			free(estructuraMensajePlatoL->nombreRestaurante);
			free(estructuraMensajePlatoL->nombrePlato);
			free(estructuraMensajePlatoL);

		break;

		case TERMINAR_PEDIDO:;
			guardar_pedido* estructuraMensajeTerminarP = malloc(sizeof(guardar_pedido));
			recibir_mensaje(estructuraMensajeTerminarP, TERMINAR_PEDIDO, *socket_cliente);

			log_info(logger, "[ReciboMSG] TERMINAR_PEDIDO : nombreResto=%s | idPedido=%i", estructuraMensajeTerminarP->nombreRestaurante, estructuraMensajeTerminarP->idPedido);
			terminarPedido(estructuraMensajeTerminarP->nombreRestaurante, estructuraMensajeTerminarP->idPedido,  *socket_cliente);
			free(estructuraMensajeTerminarP->nombreRestaurante);
			free(estructuraMensajeTerminarP);

		break;

		case HANDSHAKE:;
            handshake* elHandshake = malloc(sizeof(handshake));
            recibir_mensaje(elHandshake, HANDSHAKE, *socket_cliente);
            free(elHandshake->id);
            free(elHandshake);
	    break;

		case ERROR:
			pthread_exit(NULL);
		break;

		case DESCONEXION:
			pthread_exit(NULL);
		break;

		default:
			printf("ERROR | Sindicato recibe un mensaje que no sabe procesar.\n");
		break;
	}

}

void serve_client(int32_t* socketCliente)
{
		int32_t sizeAAllocar;
		codigo_operacion cod_op;
		int32_t recibidosSize = 0;

		int32_t recibidos = recv(*socketCliente, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);
		bytesRecibidos(recibidos);

		if(recibidos >= 1)
		{
			recibidosSize = recv(*socketCliente, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
			bytesRecibidos(recibidosSize);
			//printf("Tamaño de lo que sigue en el buffer: %u.\n", sizeAAllocar);

			process_request(cod_op, socketCliente, sizeAAllocar);
		}
		else
		{
			//free(socketCliente);
			pthread_exit(NULL);
		}

		recibidosSize = 0;
		recibidos = 0;
		free(socketCliente);
}

void esperar_cliente(int32_t socket_servidor)
{
	struct sockaddr_in dir_cliente;

	socklen_t tam_direccion = sizeof(struct sockaddr_in);
	pthread_t hiloConexionCliente;
	int32_t* socket_cliente = malloc(sizeof(int32_t));
	*socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);
	pthread_create(&hiloConexionCliente, NULL, (void*)serve_client, socket_cliente);
	pthread_detach(hiloConexionCliente);
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

