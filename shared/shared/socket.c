/*
 * socket.c
 *
 *  Created on: 17 abr. 2020
 *      Author: utnso
 */

#include "socket.h"

int32_t reservarSocket(char* miPuerto)
{
	uint32_t miPuertoInt = cambia_a_int(miPuerto); //pasa a int la key del config

	//preparo una direccion con estas configuraciones
	struct sockaddr_in direccionMiSocket;
	direccionMiSocket.sin_family = AF_INET;
	direccionMiSocket.sin_addr.s_addr = INADDR_ANY;
	direccionMiSocket.sin_port = htons(miPuertoInt); //puerto en el que queremos que escuche

	//me armo el socket en el que quiero escuchar
	int32_t miSocket = socket(AF_INET, SOCK_STREAM, 0); //con estos parametros le decimos que use el protocolo TCP/IP

	//este cacho de codigo es para que si se cierra mal el programa, se pueda reutilizar el socket sin problema
	uint32_t verdadero = 1;
	setsockopt(miSocket, SOL_SOCKET, SO_REUSEADDR, &verdadero, sizeof(verdadero));

	//asocio el socket que creamos con la direccion(el puerto) que arme antes
	if(bind(miSocket, (void*) &direccionMiSocket, sizeof(direccionMiSocket)) != 0)
	{
		puts("El socket no se pudo asociar correctamente (Es posible que esté en uso).");
		return 1;
	}

	puts("Socket reservado correctamente.");
	listen(miSocket, SOMAXCONN); //SOMAXCONN podria ser un numero (Nº max de conexiones en la cola), pero el parametro significa que tiene el tamaño maximo de cola

	return miSocket;
}
